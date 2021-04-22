#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include "dispatcher.h"

using boost::asio::ip::tcp;

class session {
public:
  session(boost::asio::io_service& io_service);
  
  tcp::socket& socket();
  const char* get_data() { return data_; }
  void start();

  void handle_read(const boost::system::error_code& error,
      size_t bytes_transferred);
  
  void handle_write(const boost::system::error_code& error);

  void fill_data_with(const std::string& msg);
  
  tcp::socket socket_;

private:
  bool end_of_request();
  void append_data();
  void build_response();

  Dispatcher* dispatcher_;

  enum { max_length = 1024 };
  char data_[max_length];
};
