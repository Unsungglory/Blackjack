
#ifndef CHAT_MESSAGE_HPP
#define CHAT_MESSAGE_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>

// these two classes are examples of sending an
// entire structure as part of the header
class game_state
{
public:
   bool dealer_cards_valid;
   int dealer_cards[6];
   bool player_cards_valid;
   int player_cards[6][3]; // [players][cards]
  // note you can't use std::string 
  // or pointers
};

class client_action
{
public:
  bool hit;   
  bool stand;
  bool join;
  bool double_hit;
  bool split;
  bool name_valid;
  char name[25]; 
  int bet;
};

class chat_message
{
public:
  enum { header_length = 4 + sizeof(client_action) + sizeof(game_state) };
  enum { max_body_length = 512 };

  chat_message()
    : body_length_(0)
  {
  }

  const char* data() const
  {
    return data_;
  }

  char* data()
  {
    return data_;
  }

  std::size_t length() const
  {
    return header_length + body_length_;
  }

  const char* body() const
  {
    return data_ + header_length;
  }

  char* body()
  {
    return data_ + header_length;
  }

  std::size_t body_length() const
  {
    return body_length_;
  }

  void body_length(std::size_t new_length)
  {
    body_length_ = new_length;
    if (body_length_ > max_body_length)
      body_length_ = max_body_length;
  }

  bool decode_header()
  {
    char header[header_length + 1] = "";
    std::strncat(header, data_, 4);
    body_length_ = std::atoi(header);
    char *p = data_ + 4; // skip the integer
    std::memcpy(&ca,p,sizeof(client_action));
    std::memcpy(&gs,p+sizeof(client_action),sizeof(game_state));

    if (body_length_ > max_body_length)
    {
      body_length_ = 0;
      return false;
    }
    return true;
  }

  void encode_header()
  {
    char header[4 + 1] = "";
    std::sprintf(header, "%4d", static_cast<int>(body_length_));
    std::memcpy(data_, header, header_length);

    char *p = data_+4; // skip over the int we just extracted
    std::memcpy(p,&ca,sizeof(client_action));
    std::memcpy(p+sizeof(client_action),&gs,sizeof(game_state));
  }

private:
  char data_[header_length + max_body_length];
  std::size_t body_length_;
public:
  client_action ca;
  game_state gs;
};

#endif // CHAT_MESSAGE_HPP

