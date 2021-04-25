#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>

#include "session.h"
#include "echo_handler.h"
#include "file_handler.h"

using boost::asio::ip::tcp;


session:: session(boost::asio::io_service& io_service, Dispatcher* dispatcher)
  : socket_(io_service) {
    dispatcher_ = dispatcher;
}

tcp::socket& session::socket() {
  return socket_;
}

void session::start() {
  BOOST_LOG_TRIVIAL(trace) << "In session::start()";
  memset(data_, '\0', sizeof(data_));
  socket_.async_read_some(boost::asio::buffer(data_, max_length),
      boost::bind(&session::handle_read, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

bool session::end_of_request() {
  std::string request = data_;
  return request.find("\r\n\r\n") != std::string::npos ||
         request.find("\n\n") != std::string::npos;
}

void session::append_data() {
  BOOST_LOG_TRIVIAL(trace) << "No double newline found, waiting for more data...";
  
  // Append data until double CRLF/LF is found.
  // WARNING: Assumes the request is at most `max_length' bytes.
  socket_.async_read_some(boost::asio::buffer(&data_[strlen(data_)], max_length),
      boost::bind(&session::handle_read, this,
	boost::asio::placeholders::error,
	boost::asio::placeholders::bytes_transferred));
}

void session::build_response() {
  BOOST_LOG_TRIVIAL(trace) << "In build_response.\n";
  std::string response;
  Request request = RequestHandler::parse_request(data_);
  BOOST_LOG_TRIVIAL(trace) << "File Path: " << request.path << "\n";
  
  switch (request.type) {
  case RequestType::File: {
    auto fh = dispatcher_ -> get_request_handler(request);
    response = fh->generate_response(request);
    break;
  }
  case RequestType::Echo: {
    EchoHandler eh;
    response = eh.generate_response(request);
    break;
  }
  default:
    // TODO(!): Request type was invalid. Generate an error response.
    response = "HTTP/1.1 404 Not Found\nContent-Length: 22\nContent-Type: text/html\n\n<h1>404 Not Found</h1>";
    break;
  }

  fill_data_with(response);

  BOOST_LOG_TRIVIAL(trace) << "Returning following response:\n" << response << std::endl;
}

void session::fill_data_with(const std::string& msg) {
  strncpy(data_, msg.c_str(), msg.size());
}

void session::handle_read(const boost::system::error_code& error,
    size_t bytes_transferred) {
  if (!error) {
    BOOST_LOG_TRIVIAL(trace) << "In handle_read received more messages. In Buffer:\n" << data_;
    
    // Implements the basic http echo requests from assignment 2.
    // Uses double newline characters to detect end of requests.
    
    if (!end_of_request()) {
      append_data();
    }
    else {
      build_response();
    
      boost::asio::async_write(socket_,
        boost::asio::buffer(data_, strlen(data_)),
          boost::bind(&session::handle_write, this,
          boost::asio::placeholders::error));
    }
  }
  else {
    BOOST_LOG_TRIVIAL(trace) << "Deleting in handle_read.";
    delete this;
  }
}

void session::handle_write(const boost::system::error_code& error)
{
  if (!error) {
    // TODO(?): Decide for certain if socket should be closed after receiving
    //          the signal for the end of the first request.
    //          If so, rename this function to something appropriate.
    
    memset(data_, '\0', sizeof(data_));
    BOOST_LOG_TRIVIAL(trace) << "Closing socket.";
    socket_.close();
  }
  else {
    BOOST_LOG_TRIVIAL(trace) << "Deleting in handle_write.";
    delete this;
  }
}

