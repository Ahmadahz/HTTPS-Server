#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ssl.hpp>

#include "https_session.h"

using boost::asio::ip::tcp;
namespace http = boost::beast::http;
// typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;

// session::session(boost::asio::io_service& io_service, boost::asio::ssl::context& context, Dispatcher* dispatcher)
//   : tcpsocket_(io_service) {
//     dispatcher_ = dispatcher;
// }

https_session::https_session(boost::asio::io_service& io_service, boost::asio::ssl::context& context, Dispatcher* dispatcher)
  : sslsocket_(io_service, context) {
    dispatcher_ = dispatcher;
    https = true;
}


// tcp::socket& session::tcpsocket() {
//   return tcpsocket_;
// }

boost::asio::ssl::stream<boost::asio::ip::tcp::socket>::lowest_layer_type& https_session::sslsocket() {
  return sslsocket_.lowest_layer();
}

void https_session::start() {
  if (https) {
    BOOST_LOG_TRIVIAL(trace) << "In session::start(), ssl socket";
    sslsocket_.async_handshake(boost::asio::ssl::stream_base::server,
        boost::bind(&https_session::handle_handshake, this,
          boost::asio::placeholders::error));
  }
  // else {
  //   BOOST_LOG_TRIVIAL(trace) << "In session::start(); tcp socket";
  //   memset(data_, '\0', sizeof(data_));
  //   socket().async_read_some(boost::asio::buffer(data_, max_length),
  //       boost::bind(&session::handle_read, this,
  //         boost::asio::placeholders::error,
  //         boost::asio::placeholders::bytes_transferred));
  // }
  
}

void https_session::handle_handshake(const boost::system::error_code& error) {
  if (!error)
  {
    BOOST_LOG_TRIVIAL(trace) << "In session::handle_handshake()";
    memset(data_, '\0', sizeof(data_));
    sslsocket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&https_session::handle_read, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }
  else
  {
    BOOST_LOG_TRIVIAL(trace) << "In session::handle_handshake(): Handshake error: " << error.message();
    if (error.value() == ERR_PACK(ERR_LIB_SSL, ERR_GET_FUNC(error.value()), SSL_R_HTTP_REQUEST)) {
        BOOST_LOG_TRIVIAL(trace) << "Handle http request";
        https = false;
        memset(data_, '\0', sizeof(data_));
        sslsocket_.next_layer().async_read_some(boost::asio::buffer(data_, max_length),
            boost::bind(&https_session::handle_read, this,
              boost::asio::placeholders::error,
              boost::asio::placeholders::bytes_transferred));
    }
    else {
        delete this;
    }
  }
}

bool https_session::end_of_request() const {
  std::string request = data_;
  return request.find("\r\n\r\n") != std::string::npos ||
         request.find("\n\n") != std::string::npos;
}

void https_session::append_data() {
  BOOST_LOG_TRIVIAL(trace) << "No double newline found, waiting for more data...";
  
  // Append data until double CRLF/LF is found.
  // WARNING: Assumes the request is at most `max_length' bytes.
  if (https) {
    sslsocket_.async_read_some(boost::asio::buffer(&data_[strlen(data_)], max_length),
        boost::bind(&https_session::handle_read, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }
  else {
    sslsocket_.next_layer().async_read_some(boost::asio::buffer(&data_[strlen(data_)], max_length),
        boost::bind(&https_session::handle_read, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }
}

/*
  Iterate through stored handlers in dispatcher to get the 
  name of the handler servicing a given uri
*/
std::string https_session::get_handler_name(const std::string& uri) const {
  std::string path = uri;
  std::string handler_name;
  // Remove slashes at end of path
  while (path.length() > 1 && path.back() == '/') {
    path.pop_back();
  }

  std::string prefix;
  // Match longest prefix and corresponding handler
  for (auto it = Dispatcher::handler_info.begin(); it != Dispatcher::handler_info.end(); it++) {
    if (path.substr(0, it->first.length()) == it->first) {
      if (it->first.length() > prefix.length()) {
        prefix = it->first;
        handler_name = it->second;
      }
    }
  }
  return handler_name;
}

void https_session::send_response() {
  BOOST_LOG_TRIVIAL(trace) << "In session::send_response: In send_response.\n";
  
  boost::system::error_code ec;
  http::request_parser<http::string_body> parser;
  parser.eager(true);
  parser.put(boost::asio::buffer(data_, strlen(data_)), ec);

  if (ec) {
    BOOST_LOG_TRIVIAL(error) << "Error 400: The format for HTTP request is wrong.";
  http::response<http::string_body> response;
  response.result(400);
  response.body() = "<h1>400 Bad Request</h1>";
  response.prepare_payload();
  buffer_ = response;
  if (https) {
    http::async_write(sslsocket_, buffer_,
      boost::bind(&https_session::handle_write, this,
                  boost::asio::placeholders::error));
  }
  else {
    http::async_write(sslsocket_.next_layer(), buffer_,
      boost::bind(&https_session::handle_write, this,
                  boost::asio::placeholders::error));
  }
  return;
  }
  
  http::request<http::string_body> req = parser.get();

  std::string uri(req.target().data(), req.target().size());
  
  RequestHandler* handler = dispatcher_->get_request_handler(uri);
  if (handler) {
    BOOST_LOG_TRIVIAL(trace) << "In session::send_response: Request handler found for: " << uri;
    
    buffer_ = handler->handle_request(req);
    request_count++;
    handled_requests[uri].push_back(buffer_.result_int());
    BOOST_LOG_TRIVIAL(trace) << "Body of response: " << buffer_.body();
    std::string handler_name = get_handler_name(uri);
    if (https) {
      std::string request_ip = sslsocket_.lowest_layer().remote_endpoint().address().to_string();
      BOOST_LOG_TRIVIAL(trace) << "[ResponseMetrics] request_ip:" << request_ip << " request_path:" << uri 
        << " matched_handler:" << handler_name <<  " response_code:" << buffer_.result_int() << std::endl;

      http::async_write(sslsocket_, buffer_,
        boost::bind(&https_session::handle_write, this,
                    boost::asio::placeholders::error));
    }
    else {
      std::string request_ip = sslsocket_.next_layer().remote_endpoint().address().to_string();
      BOOST_LOG_TRIVIAL(trace) << "[ResponseMetrics] request_ip:" << request_ip << " request_path:" << uri 
        << " matched_handler:" << handler_name <<  " response_code:" << buffer_.result_int() << std::endl;

      http::async_write(sslsocket_.next_layer(), buffer_,
        boost::bind(&https_session::handle_write, this,
                    boost::asio::placeholders::error));
    }
  }
  else {
    // This should never be entered since the 404 handler should always
    // be created from get_request_handler in exceptional cases.
    
    BOOST_LOG_TRIVIAL(info) << "In session::send_response: No request handler found for: " << uri;
  }
}

void https_session::fill_data_with(const std::string& msg) {
  strncpy(data_, msg.data(), msg.size());
}

void https_session::handle_read(const boost::system::error_code& error,
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

void https_session::handle_write(const boost::system::error_code& error)
{
  if (!error) {
    // TODO(?): Decide for certain if socket should be closed after receiving
    //          the signal for the end of the first request.
    //          If so, rename this function to something appropriate.
    
    memset(data_, '\0', sizeof(data_));
    
    BOOST_LOG_TRIVIAL(trace)
      << "Response sent, closing socket.";
      // << socket_.remote_endpoint().address().to_string();
    
    if (https) {
      sslsocket_.lowest_layer().close();
    }
    else {
      sslsocket_.next_layer().close();
    }
    
  }
  else {
    BOOST_LOG_TRIVIAL(trace) << "Deleting in session::handle_write.";
    delete this;
  }
}

int https_session::request_count = 0;
std::map<std::string, std::vector<int>> https_session::handled_requests;