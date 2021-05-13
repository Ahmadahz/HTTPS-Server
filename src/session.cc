#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <boost/beast/http.hpp>

#include "session.h"

using boost::asio::ip::tcp;
namespace http = boost::beast::http;

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

bool session::end_of_request() const {
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

void session::send_response() {
  BOOST_LOG_TRIVIAL(trace) << "In session::send_response: In send_response.\n";
  
  boost::system::error_code ec;
  http::request_parser<http::string_body> parser;
  parser.eager(true);
  parser.put(boost::asio::buffer(data_, strlen(data_)), ec);

  if (ec) {
    BOOST_LOG_TRIVIAL(error) << "Error parsing the client HTTP request.";
    return;
  }
  
  http::request<http::string_body> req = parser.get();

  std::string file_path(req.target().data(), req.target().size());

  BOOST_LOG_TRIVIAL(trace) << "Request target: " << file_path;
  
  RequestHandler* handler = dispatcher_->get_request_handler(file_path);
  if (handler) {
    BOOST_LOG_TRIVIAL(trace) << "In session::send_response: Request handler found for: " << file_path;
    
    buffer_ = handler->handle_request(req);

    BOOST_LOG_TRIVIAL(trace) << "Body of response: " << buffer_.body();

    http::async_write(socket_, buffer_,
      boost::bind(&session::handle_write, this,
                  boost::asio::placeholders::error));
  }
  else {
    // This should never be entered since the 404 handler should always
    // be created from get_request_handler in exceptional cases.
    
    BOOST_LOG_TRIVIAL(info) << "In session::send_response: No request handler found for: " << file_path;
  }
}

void session::fill_data_with(const std::string& msg) {
  strncpy(data_, msg.data(), msg.size());
}

void session::handle_read(const boost::system::error_code& error,
    size_t bytes_transferred) {
  if (!error) {
    BOOST_LOG_TRIVIAL(trace) << "In session::handle_read, received more data. In Buffer:\n" << data_;
    
    // Implements the basic http echo requests from assignment 2.
    // Uses double newline characters to detect end of requests.
    
    if (!end_of_request()) {
      append_data();
    }
    else {
      send_response();
    }
  }
  else {
    BOOST_LOG_TRIVIAL(trace) << "Deleting in session::handle_read.";
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
    
	/*
    BOOST_LOG_TRIVIAL(trace)
      << "Closing socket of IP: "
      << socket_.remote_endpoint().address().to_string();
    */
    socket_.close();
  }
  else {
    BOOST_LOG_TRIVIAL(trace) << "Deleting in session::handle_write.";
    delete this;
  }
}

int session::request_count = 0;
std::map<std::string, std::vector<int>> session::handled_requests;