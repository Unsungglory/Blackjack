//
// chat_server.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include "asio.hpp"
//#include "chat_message.hpp"

using asio::ip::tcp;

//----------------------------------------------------------------------

//typedef std::deque<chat_message> chat_message_queue;

//----------------------------------------------------------------------

class game_player
{
public:
  virtual ~game_player() {}
 // virtual void deliver(const chat_message& msg) = 0;
};

typedef std::shared_ptr<game_player> game_player_ptr;

//----------------------------------------------------------------------

class game_room
{
public:
  void join(game_player_ptr participant)
  {
    participants_.insert(participant);
    //when the player joins the game room
    //enter code to enter name and stuff
  }

  void leave(game_player_ptr participant)
  {
    //player leaves the table
    participants_.erase(participant);
  }

//  void deliver(const chat_message& msg)
//  {
//    recent_msgs_.push_back(msg);
//    std::cout<<"There are "<<recent_msgs_.size()<<" waiting on the server."<<std::endl;
//    while (recent_msgs_.size() > max_recent_msgs)
//      recent_msgs_.pop_front();

//    for (auto participant: participants_)
//      participant->deliver(msg);
//  }

private:
  std::set<game_player_ptr> participants_;
};

//----------------------------------------------------------------------

class game_session
  : public game_player,
    public std::enable_shared_from_this<game_session>
{
public:
  game_session(tcp::socket socket, game_room& room)
    : socket_(std::move(socket)),
      room_(room)
  {
  }

  void start()
  {
    room_.join(shared_from_this());
   // do_read_header();
  }

  //void deliver(const chat_message& msg)
  //{
  //  bool write_in_progress = !write_msgs_.empty();
  //  write_msgs_.push_back(msg);
  //  if (!write_in_progress)
  //  {
  //    do_write();
  //  }
  //}

private:
//  void do_read_header()
//	I dont know how to reuse this code just yet
//	whichever function calls the do_read_header function 
//  {
//    auto self(shared_from_this());
//    asio::async_read(socket_,
//        asio::buffer(read_msg_.data(), chat_message::header_length),
//        [this, self](std::error_code ec, std::size_t /*length*/)
//        {
//          if (!ec && read_msg_.decode_header())
//          {
//            do_read_body();
//          }
//          else
//          {
//            room_.leave(shared_from_this());
//          }
//        });
//  }

//  void do_read_body()
//  {
//	same with do_read_header--unsure how to reuse this yet
//    auto self(shared_from_this());
//    asio::async_read(socket_,
//        asio::buffer(read_msg_.body(), read_msg_.body_length()),
//        [this, self](std::error_code ec, std::size_t /*length*/)
//        {
//          if (!ec)
//          {
//            room_.deliver(read_msg_);
//            do_read_header();
//          }
//          else
//          {
//            room_.leave(shared_from_this());
//          }
//        });
//  }

//  void do_write()
//  {
//    auto self(shared_from_this());
//    asio::async_write(socket_,
//        asio::buffer(write_msgs_.front().data(),
//          write_msgs_.front().length()),
//        [this, self](std::error_code ec, std::size_t /*length*/)
//      {
//          if (!ec)
//          {
//            write_msgs_.pop_front();
//            if (!write_msgs_.empty())
//            {
//              do_write();
//            }
//          }
//          else
//          {
//            room_.leave(shared_from_this());
//          }
//       });
//  }

  tcp::socket socket_;
  game_room& room_;
 // chat_message read_msg_;
 // chat_message_queue write_msgs_;
};

//----------------------------------------------------------------------

class dealer
{
public:
  dealer(asio::io_context& io_context,
      const tcp::endpoint& endpoint)
    : acceptor_(io_context, endpoint)
  {
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
            std::make_shared<game_session>(std::move(socket), room_)->start();
          }

          do_accept();
        });
  }

  tcp::acceptor acceptor_;
  game_room room_;
};

//----------------------------------------------------------------------

int main(int argc, char* argv[])
{
  try
  {
    if (argc < 2)
    {
      std::cerr << "Usage: dealer <port> [<port> ...]\n";
      return 1;
    }

    asio::io_context io_context;

    std::list<dealer> dealer;
    for (int i = 1; i < argc; ++i)
    {
      tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[i]));
      dealer.emplace_back(io_context, endpoint);
    }

    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
