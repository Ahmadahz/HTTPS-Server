#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>

using boost::asio::ip::tcp;

#include "session.h"
#include "dispatcher.h"
#include "config_parser.h"

class server {
public:
  server(boost::asio::io_service& io_service, short port, const NginxConfig &config);

  /* This constructor was initially intended for unit testing.
     WARNING: @new_session should be initialized with @io_service prior
              to this constructor being called. */
  server(session& new_session, boost::asio::io_service& io_service, short port, const NginxConfig &config);

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
};
