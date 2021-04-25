#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>

using boost::asio::ip::tcp;

#include "server.h"

server::server(boost::asio::io_service& io_service, short port, const NginxConfig &config)
  : io_service_(io_service),
    acceptor_(io_service, tcp::endpoint(tcp::v4(), port)) {
  dispatcher_ = new Dispatcher(config);
  start_accept();
}

server::server(session& new_session, boost::asio::io_service& io_service, short port, const NginxConfig &config)
  : io_service_(io_service),
    acceptor_(io_service, tcp::endpoint(tcp::v4(), port)) {
  dispatcher_ = new Dispatcher(config);
  start_accept(&new_session);
}

void server::start_accept() {
  session* new_session = new session(io_service_, dispatcher_);
  start_accept(new_session);
}

void server::start_accept(session* new_session) {
  BOOST_LOG_TRIVIAL(trace) << "In server::start_accept()";
  
  acceptor_.async_accept(new_session->socket(),
      boost::bind(&server::handle_accept, this, new_session,
	          boost::asio::placeholders::error));

  BOOST_LOG_TRIVIAL(trace) << "In server::start_accept(), after async_accept()";
}

void server::handle_accept(session* new_session, const boost::system::error_code& error) {
  if (!error) {
    BOOST_LOG_TRIVIAL(trace) << "In server::handle_accept(), calling session->start()";
    new_session->start();
  }
  else {
    BOOST_LOG_TRIVIAL(trace) << "in server::handle_accept(). Deleting session.";
    delete new_session;
  }

  start_accept();
}
