#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ssl.hpp>


#include "session.h"

using boost::asio::ip::tcp;
namespace http = boost::beast::http;
typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;

session::session(boost::asio::io_service& io_service, Dispatcher* dispatcher)
  : socket_(io_service) {
  dispatcher_ = dispatcher;
  version_ = "http";
}

session::session(boost::asio::io_service& io_service, boost::asio::ssl::context& context, Dispatcher* dispatcher)
  : socket_(io_service),
    sslsocket_ptr_(new ssl_socket(io_service, context)) {
  dispatcher_ = dispatcher;
  version_ = "https";
}

tcp::socket& session::socket() {
  return socket_;
}

ssl_socket::lowest_layer_type& session::sslsocket() {
  return sslsocket_ptr_->lowest_layer();
}

void session::start() {
  if (version_ == "https") {
    BOOST_LOG_TRIVIAL(trace) << "In https_session::start(), ssl socket";
    sslsocket_ptr_->async_handshake(boost::asio::ssl::stream_base::server,
        boost::bind(&session::handle_handshake, this,
          boost::asio::placeholders::error));
  }
  else if (version_ == "http") {
    BOOST_LOG_TRIVIAL(trace) << "In http_session::start(); tcp socket";
    BOOST_LOG_TRIVIAL(trace) << "In http_session::start()";
    memset(data_, '\0', sizeof(data_));
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&session::handle_read, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }
}

void session::handle_handshake(const boost::system::error_code& error) {
  if (!error)
  {
    BOOST_LOG_TRIVIAL(trace) << "In session::handle_handshake()";
    memset(data_, '\0', sizeof(data_));
    sslsocket_ptr_->async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&session::handle_read, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }
  else
  {
    BOOST_LOG_TRIVIAL(trace) << "In session::handle_handshake(): Handshake error: " << error.message();
    if (error.value() == ERR_PACK(ERR_LIB_SSL, ERR_GET_FUNC(error.value()), SSL_R_HTTP_REQUEST)) {
      BOOST_LOG_TRIVIAL(error) << "Http request on ssl port";
      delete this;
    }
    else {
      delete this;
    }
  }
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
  if (version_ == "https") {
    sslsocket_ptr_->async_read_some(boost::asio::buffer(&data_[strlen(data_)], max_length),
        boost::bind(&session::handle_read, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }
  else if (version_ == "http") {
    socket_.async_read_some(boost::asio::buffer(&data_[strlen(data_)], max_length),
        boost::bind(&session::handle_read, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }
}

/*
  Iterate through stored handlers in dispatcher to get the 
  name of the handler servicing a given uri
*/
std::string session::get_handler_name(const std::string& uri) const {
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

void session::send_response() {
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
    if (version_ == "https") {
      http::async_write(*sslsocket_ptr_, buffer_,
        boost::bind(&session::handle_write, this,
                    boost::asio::placeholders::error));
        BOOST_LOG_TRIVIAL(error) << "Handled malformed https request" << std::endl;
    }
    else if (version_ == "http") {
      http::async_write(socket_, buffer_,
        boost::bind(&session::handle_write, this,
                    boost::asio::placeholders::error));
        BOOST_LOG_TRIVIAL(error) << "Handled malformed http request" << std::endl;
    }
    return;
  }
  
  http::request<http::string_body> req = parser.get();

  std::string uri(req.target().data(), req.target().size());
  
  RequestHandler* handler = dispatcher_->get_request_handler(uri);

  BOOST_LOG_TRIVIAL(trace) << "In session::send_response: Request handler found for: " << uri;

  buffer_ = handler->handle_request(req);
  request_count++;
  handled_requests[uri].push_back(buffer_.result_int());
  BOOST_LOG_TRIVIAL(trace) << "Body of response: " << buffer_.body();
  std::string handler_name = get_handler_name(uri);
  if (version_ == "https") {
    std::string request_ip = sslsocket_ptr_->lowest_layer().remote_endpoint().address().to_string();
    BOOST_LOG_TRIVIAL(trace) << "[ResponseMetrics] https_ request_ip:" << request_ip
			     << " request_path:" << uri
			     << " matched_handler:" << handler_name
			     << " response_code:" << buffer_.result_int() << std::endl;

    http::async_write(*sslsocket_ptr_, buffer_,
      boost::bind(&session::handle_write, this,
		  boost::asio::placeholders::error));
  }
  else if (version_ == "http") {
    std::string request_ip = socket_.remote_endpoint().address().to_string();
    BOOST_LOG_TRIVIAL(trace) << "[ResponseMetrics] http_ request_ip:" << request_ip
			     << " request_path:" << uri
			     << " matched_handler:" << handler_name
			     << " response_code:" << buffer_.result_int() << std::endl;

    http::async_write(socket_, buffer_,
      boost::bind(&session::handle_write, this,
		  boost::asio::placeholders::error));
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
    memset(data_, '\0', sizeof(data_));
    
    BOOST_LOG_TRIVIAL(trace)
      << "Response sent, closing socket.";
      // << socket_.remote_endpoint().address().to_string();
    
    if (version_ == "https") {
      sslsocket_ptr_->lowest_layer().close();
    }
    else if (version_ == "http") {
      socket_.close();
    }
  }
  else {
    BOOST_LOG_TRIVIAL(trace) << "Deleting in session::handle_write.";
    delete this;
  }
}

int session::request_count = 0;
std::map<std::string, std::vector<int>> session::handled_requests;
