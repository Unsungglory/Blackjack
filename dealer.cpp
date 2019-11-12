//
// blackjack_game.cpp
// ~~~~~~~~~~~~~~~
//


#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include "asio.hpp"
#include "chat_message.hpp"

using asio::ip::tcp;

//----------------------------------------------------------------------

typedef std::deque<chat_message> chat_message_queue;

//----------------------------------------------------------------------
class hand
{
public:
   hand () 
   { 
     cards[0] = 0;
     cards[1] = 0;
     cards[2] = 0;
     cards[3] = 0;
     cards[4] = 0;
     cards[5] = 0;
   }
   int cards[6];  // this is just an example
};

//----------------------------------------------------------------------
class player
{
public:
  std::string name;
  hand Hand;
   
  player() 
  {
     std::cout << "Creating a new player " << std::endl;
  }
  virtual ~player() {}
  virtual void deliver(const chat_message& msg) = 0;
};

typedef std::shared_ptr<player> player_ptr;

//----------------------------------------------------------------------

class dealer
{
public:
  dealer() 
  {
     std::cout << "Creating a dealer" << std::endl;
     current_player = NULL;
     card_idx=0;
  }
  void deal()
  {
  	//add card class, and add random, and shuffle
      Hand.cards[card_idx] = 8;
      card_idx++;
      if (card_idx>2)
        card_idx = 0;
  }
  void next_player ( player_ptr p )
  {
     current_player = p;
  }
  player_ptr current_player;
  hand Hand;
  int card_idx;
};

//----------------------------------------------------------------------

class blackjack_table
{
public:
  void join(player_ptr player_)
  {
    std::cout << "joining the table" << std::endl;
    players.insert(player_);
    //for (auto msg: recent_msgs_)
    //  player_->deliver(msg);
  }

  void leave(player_ptr player_)
  {
    players.erase(player_);
  }

  void deliver(const chat_message& msg)
  {
    recent_msgs_.push_back(msg);
    while (recent_msgs_.size() > max_recent_msgs)
      recent_msgs_.pop_front();

    for (auto player_: players)
      player_->deliver(msg);
  }
	//change to all players have bet
  bool all_players_have_a_name()
  {
     bool retval = true;
     for (auto player_: players)
     {
       retval = retval &&  (player_->name > "");
     }
     return retval;
  }

  dealer Dealer;
private:
  std::set<player_ptr> players;
  enum { max_recent_msgs = 100 };
  chat_message_queue recent_msgs_;
};

//----------------------------------------------------------------------
//TODO create wincheck, hit (actual), split, double
//
//
//

int wallet = 500;
class blackjack_player
  : public player,
    public std::enable_shared_from_this<blackjack_player>
{
public:
  blackjack_player(tcp::socket socket, blackjack_table& table)
    : socket_(std::move(socket)),
      table_(table)
  {
     std::cout << "Creating a blackjack_player " << std::endl;
  }

  void start()
  {
    table_.join(shared_from_this());
    do_read_header();
  }

  void deliver(const chat_message& msg)
  {
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress)
    {
      do_write();
    }
  }

private:
  void do_read_header()
  {
    auto self(shared_from_this());
    asio::async_read(socket_,
        asio::buffer(read_msg_.data(), chat_message::header_length),
        [this, self](std::error_code ec, std::size_t /*length*/)
        {
          if (!ec && read_msg_.decode_header())
          {
            do_read_body();
          }
          else
          {
            table_.leave(shared_from_this());
          }
        });
  }

  void do_read_body()
  {
  //
  //hit,double, all_players_stand(this is where the dealer draws), then win check,split
  //
  
    auto self(shared_from_this());
    asio::async_read(socket_,
        asio::buffer(read_msg_.body(), read_msg_.body_length()),
        [this, self](std::error_code ec, std::size_t /*length*/)
        {
          if (!ec)
          {
            // Here is where messages arrive from the client
            // this is where the design makes it easy or hard

            // ignore anything in the message body
            read_msg_.body_length(0);
            read_msg_.gs.dealer_cards_valid = false;
            read_msg_.gs.player_cards_valid = false;
            // is it a join
            if (read_msg_.ca.join /*&& read_msg_.ca.name_valid*/)
            {
               std::cout << "the name is " << read_msg_.ca.name << std::endl;
               std::cout << "the starting bet is " << read_msg_.ca.bet << std::endl;
               std::string m = std::string(read_msg_.ca.name) + " has joined.";
               strcpy(read_msg_.body(),m.c_str());
               read_msg_.body_length(strlen(read_msg_.body()));
               self->name = std::string (read_msg_.ca.name);
            }
            //if bet?
            if (self->name>"") // quick way to see if they have entered a name
            {
                if (read_msg_.ca.hit) // also need to check in order, since everyone has a turn
                {
                   // call the dealer class with some kind of method and
                   // argument
                   std::string m = self->name + " has asked for a hit.";
                   strcpy(read_msg_.body(),m.c_str());
                   read_msg_.body_length(strlen(read_msg_.body()));
                   // also set read_msg.gs.XXX to whatever needs to go to the clients
                }
            }

            // display the cards if everyone has joined
            //if all players have bet
            if (table_.all_players_have_a_name ())
            {
               table_.Dealer.deal();
               read_msg_.gs.dealer_cards_valid = true;
               read_msg_.gs.dealer_cards[0] = table_.Dealer.Hand.cards[0];
               read_msg_.gs.dealer_cards[1] = table_.Dealer.Hand.cards[1];
               read_msg_.gs.dealer_cards[2] = table_.Dealer.Hand.cards[2];
               read_msg_.gs.dealer_cards[3] = table_.Dealer.Hand.cards[3];
               read_msg_.gs.dealer_cards[4] = table_.Dealer.Hand.cards[4];
               read_msg_.gs.dealer_cards[5] = table_.Dealer.Hand.cards[5];
            }
            //construct player in gs
            read_msg_.encode_header(); // so the body text above gets sent
            table_.deliver(read_msg_);
            do_read_header();
          }
          else
          {
            table_.leave(shared_from_this());
          }
        });
  }

  void do_write()
  {
    auto self(shared_from_this());
    asio::async_write(socket_,
        asio::buffer(write_msgs_.front().data(),
          write_msgs_.front().length()),
        [this, self](std::error_code ec, std::size_t /*length*/)
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
            table_.leave(shared_from_this());
          }
        });
  }
  tcp::socket socket_;
  blackjack_table& table_;
  chat_message read_msg_;
  chat_message_queue write_msgs_;
};

//----------------------------------------------------------------------
class blackjack_game
{
public:
  blackjack_game(asio::io_context& io_context,
      const tcp::endpoint& endpoint)
    : acceptor_(io_context, endpoint)
  {
    std::cout << "Creating a blackjack_game" << std::endl;
    do_accept();
  }

private:
  void do_accept()
  {
    acceptor_.async_accept(
        [this](std::error_code ec, tcp::socket socket)
        {
          if (!ec)
          {
            std::make_shared<blackjack_player>(std::move(socket), table_)->start();
          }

          do_accept();
        });
  }

  tcp::acceptor acceptor_;
  blackjack_table table_;
};

//----------------------------------------------------------------------

int main(int argc, char* argv[])
{
  try
  {
    if (argc < 2)
    {
      std::cerr << "Usage: blackjack_game <port> [<port> ...]\n";
      return 1;
    }

    asio::io_context io_context;

    std::list<blackjack_game> servers;
    for (int i = 1; i < argc; ++i)
    {
      tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[i]));
      servers.emplace_back(io_context, endpoint);
    }

    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
