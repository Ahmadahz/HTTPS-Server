#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ssl.hpp>
#include "dispatcher.h"

using boost::asio::ip::tcp;
namespace http = boost::beast::http;
typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;

class session {
public:
  session(boost::asio::io_service& io_service, Dispatcher* dispatcher);
  session(boost::asio::io_service& io_service, boost::asio::ssl::context& context, Dispatcher* dispatcher);
  
  tcp::socket& socket();
  ssl_socket::lowest_layer_type& sslsocket();
  const char* get_data() { return data_; }
  void start();

  void handle_handshake(const boost::system::error_code& error);

  void handle_read(const boost::system::error_code& error,
      size_t bytes_transferred);
  
  void handle_write(const boost::system::error_code& error);

  void fill_data_with(const std::string& msg);

  static int request_count;
  static std::map<std::string, std::vector<int>> handled_requests;

private:
  bool end_of_request() const;
  void append_data();
  void send_response();
  std::string get_handler_name(const std::string& uri) const;

  Dispatcher* dispatcher_;
  tcp::socket socket_;
  std::shared_ptr<ssl_socket> sslsocket_ptr_;
  // ssl_socket sslsocket_;
  std::string version_;

  http::response<http::string_body> buffer_;
  enum { max_length = 1024000 };
  char data_[max_length];
  size_t data_len_;
};