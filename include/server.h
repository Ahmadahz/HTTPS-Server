#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>

using boost::asio::ip::tcp;

#include "session.h"

class server {
public:
  server(boost::asio::io_service& io_service, short port);

  /* This constructor was initially intended for unit testing.
     WARNING: @new_session should be initialized with @io_service prior
              to this constructor being called. */
  server(session& new_session, boost::asio::io_service& io_service, short port);

  void handle_accept(session* new_session,
      const boost::system::error_code& error);
  
private:
  void start_accept();

  /* Overload allows for easier unit testing. */
  void start_accept(session* new_session);
 
  boost::asio::io_service& io_service_;
  tcp::acceptor acceptor_;
};
