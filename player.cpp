// CSE3310 Fall 2019


#include <iostream>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>
#include <assert.h>
#include "asio.hpp"
#include "chat_message.hpp"


//GtkWidget *fromView  = NULL;  // text from the chat server
//GtkWidget *dealerCards = NULL;
//GtkWidget *playerCards = NULL;

using asio::ip::tcp;

typedef std::deque<chat_message> chat_message_queue;

class chat_client
{
public:
  chat_client(asio::io_context& io_context,
      const tcp::resolver::results_type& endpoints)
    : io_context_(io_context),
      socket_(io_context)
  {
    do_connect(endpoints);
  }

  void write(const chat_message& msg)
  {
    asio::post(io_context_,
        [this, msg]()
        {
          bool write_in_progress = !write_msgs_.empty();
          write_msgs_.push_back(msg);
          if (!write_in_progress)
          {
            do_write();
          }
        });
  }

  void close()
  {
    asio::post(io_context_, [this]() { socket_.close(); });
  }

private:
  void do_connect(const tcp::resolver::results_type& endpoints)
  {
    asio::async_connect(socket_, endpoints,
        [this](std::error_code ec, tcp::endpoint)
        {
          if (!ec)
          {
            do_read_header();
          }
        });
  }

  void do_read_header()
  {
    asio::async_read(socket_,
        asio::buffer(read_msg_.data(), chat_message::header_length),
        [this](std::error_code ec, std::size_t /*length*/)
        {
          if (!ec && read_msg_.decode_header())
          {
            do_read_body();
          }
          else
          {
            socket_.close();
          }
        });
  }

  void do_read_body()
  {
    asio::async_read(socket_,
        asio::buffer(read_msg_.body(), read_msg_.body_length()),
        [this](std::error_code ec, std::size_t /*length*/)
        {
          if (!ec)
          {

               char outline[read_msg_.body_length() + 2];
                                       // '\n' + '\0' is 2 more chars
               outline[0] = '\n';
               outline[read_msg_.body_length() + 1] = '\0';
               std::memcpy ( &outline[1], read_msg_.body(), read_msg_.body_length() );
               std::cout<<outline<<std::endl;

               std::string p = "player "+ std::to_string (read_msg_.gs.player_cards[0][0]) + " " +
                                         std::to_string (read_msg_.gs.player_cards[0][1]) + " " +
                                         std::to_string (read_msg_.gs.player_cards[0][2]) + '\n' +
                                         std::to_string (read_msg_.gs.player_cards[1][0]) + " " +
                                         std::to_string (read_msg_.gs.player_cards[1][1]) + " " +
                                         std::to_string (read_msg_.gs.player_cards[1][2]);

               std::string d = "dealer " + std::to_string (read_msg_.gs.dealer_cards[0]) + " " +
                                          std::to_string (read_msg_.gs.dealer_cards[1]) + " " +
                                          std::to_string (read_msg_.gs.dealer_cards[2]);

               if (read_msg_.gs.player_cards_valid)
                   std::cout<<p.c_str()<<std::endl;
               else
                   std::cout<<"waiting"<<std::endl;

               if (read_msg_.gs.dealer_cards_valid)
                  std::cout<<d.c_str()<<std::endl;
               else
                  std::cout<<"waiting"<<std::endl;
            

            std::cout.write(read_msg_.body(), read_msg_.body_length());
            std::cout << "\n";
            do_read_header();
          }
          else
          {
            socket_.close();
          }
        });
  }

  void do_write()
  {
    asio::async_write(socket_,
        asio::buffer(write_msgs_.front().data(),
          write_msgs_.front().length()),
        [this](std::error_code ec, std::size_t /*length*/)
        {
          if (!ec)
          {
            write_msgs_.pop_front();
            if (!write_msgs_.empty())
            {
              do_write();
            }
          }
          else
          {
            socket_.close();
          }
        });
  }

private:
  asio::io_context& io_context_;
  tcp::socket socket_;
  chat_message read_msg_;
  chat_message_queue write_msgs_;
};

// global symbols
chat_client *c;
int wallet;


//do similar for double, stand, split
static void hitCallback()
{
   chat_message msg;
   msg.body_length (0);
   msg.ca.hit = true;
   msg.ca.stand = false;
   msg.ca.name_valid = false;
   msg.encode_header();
   assert ( c );  // this is a global class
   c->write(msg);
}

static void joinplayer()
{
   std::cerr << "Player has started: " << std::endl;

  char line[25];
  int start;
  std::cout<<"Please enter your name: ";
  std::cin.getline(line, 26);
  std::cout<<"Enter starting credits: ";
  std::cin>>start;


   if (line)
   {
      chat_message msg;
      msg.body_length (0);
      msg.ca.hit = false;
      msg.ca.stand = false;
      msg.ca.join = true;
      msg.ca.name_valid = true;
      msg.ca.double_hit = false;
      msg.ca.split = false;
      

      //change this line to take a bet
      msg.ca.bet = start;
      wallet = start;

      if (strlen(line) < sizeof(msg.ca.name))
      {
        strcpy(msg.ca.name,line);
      }

      msg.encode_header();
      assert ( c );  // this is a global class
      c->write(msg);
   }

}

int main(int argc, char *argv[])
{



//   hitButton = gtk_button_new_with_label("Hit");





   if (argc != 3)
   {
      std::cerr << "Usage: chat_client <host> <port>\n";
      return 1;
   }

   asio::io_context io_context;
   tcp::resolver resolver(io_context);
   auto endpoints = resolver.resolve(argv[1], argv[2]);
   c = new chat_client(io_context, endpoints);
   assert(c);

   std::thread t([&io_context](){ io_context.run(); });
   joinplayer();
   while(wallet > 0)
   {
   //enter actions for cards/hands
   //getline and switch for action?
   		std::string input; 
   		getline(std::cin, input);
   
   		if(input.compare("hit")==0)
   		{
   		//	hitCallback();
   			std::cout<<"test\n"<<std::endl;				 
   		}
   }
   c->close();
   t.join();
   
   return 0;
}
