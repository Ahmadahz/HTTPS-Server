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
  session* new_session;

  void handle_accept(session* new_session,
      const boost::system::error_code& error);

private:
  void start_accept();
 
  boost::asio::io_service& io_service_;
  tcp::acceptor acceptor_;
};