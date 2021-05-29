#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <boost/asio/ssl.hpp>

using boost::asio::ip::tcp;
typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;

#include "session.h"
#include "dispatcher.h"
#include "config_parser.h"

class Logger;

class server {
public:
  server(boost::asio::io_service& io_service, short port, const NginxConfig &config);

  /* This constructor was initially intended for unit testing.
     WARNING: @new_session should be initialized with @io_service prior
              to this constructor being called. */
  server(session& new_session, boost::asio::io_service& io_service, short port, const NginxConfig &config);

  std::string get_password() const;
  void handle_accept(session* new_session,
      const boost::system::error_code& error);
  
  Dispatcher* get_dispatcher() const;
private:
  void start_accept();
  
  /* Overload allows for easier unit testing. */
  void start_accept(session* new_session);
 
  Dispatcher* dispatcher_;
  boost::asio::io_service& io_service_;
  tcp::acceptor acceptor_;
  boost::asio::ssl::context context_;
  Logger* logger_;
};
