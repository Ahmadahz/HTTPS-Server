#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "session.h"

using boost::asio::ip::tcp;


session::session(boost::asio::io_service& io_service)
  : socket_(io_service) {
}

tcp::socket& session::socket() {
  return socket_;
}

void session::start() {
  socket_.async_read_some(boost::asio::buffer(data_, max_length),
      boost::bind(&session::handle_read, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

void session::handle_read(const boost::system::error_code& error,
    size_t bytes_transferred) {
  if (!error) {
    // Implements the basic http echo requests from assignment 2.
    // Uses double newline characters to detect end of requests.
    
    std::string request = data_;
    if (request.find("\r\n\r\n") == std::string::npos &&
        request.find("\n\n") == std::string::npos) {
      // Append data until double CRLF/LF is found.
      // WARNING: Assumes the request is at most `max_length' bytes.
      socket_.async_read_some(boost::asio::buffer(&data_[strlen(data_)], max_length),
          boost::bind(&session::handle_read, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    } 
    else {
      std::string response;
      response = "HTTP/1.1 200 OK\r\n";
      response += "Content-Type: text/plain\r\n";
      response += "\r\n";
      response += data_;
      strncpy(data_, response.c_str(), response.size());
    
      boost::asio::async_write(socket_,
          boost::asio::buffer(data_, response.size()),
          boost::bind(&session::handle_write, this,
          boost::asio::placeholders::error));

      memset(data_, '\0', sizeof(data_));
    }
  }
  else {
    delete this;
  }
}

void session::handle_write(const boost::system::error_code& error)
{
  if (!error) {
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&session::handle_read, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }
  else {
    delete this;
  }
}

