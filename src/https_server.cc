#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio/ssl.hpp>

using boost::asio::ip::tcp;
typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;

#include "https_server.h"
#include "logger.h"

void start_session(boost::asio::io_service& io_service, https_session* s) {
  s->start();
  io_service.run();
}

https_server::https_server(boost::asio::io_service& io_service, short port, const NginxConfig &config)
  : io_service_(io_service),
    acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
    context_(boost::asio::ssl::context::sslv23) {
  BOOST_LOG_TRIVIAL(trace) << "In https_server constructor";
  context_.set_options(
        boost::asio::ssl::context::default_workarounds
        | boost::asio::ssl::context::no_sslv2
        | boost::asio::ssl::context::single_dh_use);
  context_.set_password_callback(boost::bind(&https_server::get_password, this));
  BOOST_LOG_TRIVIAL(trace) << "Before certificate";
  context_.use_certificate_chain_file("localhost.pem");
  BOOST_LOG_TRIVIAL(trace) << "Before key";
  context_.use_private_key_file("localhost-key.pem", boost::asio::ssl::context::pem);
  BOOST_LOG_TRIVIAL(trace) << "Before dh file";
  context_.use_tmp_dh_file("dhparam4096.pem");
  BOOST_LOG_TRIVIAL(trace) << "In https_server constructor, dealt with keys and certs";
  dispatcher_ = new Dispatcher(config);
  logger_ = new Logger();
  start_accept();
}

https_server::https_server(https_session& new_session, boost::asio::io_service& io_service, short port, const NginxConfig &config)
  : io_service_(io_service),
    acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
    context_(boost::asio::ssl::context::sslv23) {
  BOOST_LOG_TRIVIAL(trace) << "In https_server constructor, threads";
  context_.set_options(
        boost::asio::ssl::context::default_workarounds
        | boost::asio::ssl::context::no_sslv2
        | boost::asio::ssl::context::single_dh_use);
  context_.set_password_callback(boost::bind(&https_server::get_password, this));
  BOOST_LOG_TRIVIAL(trace) << "Before certificate";
  context_.use_certificate_chain_file("localhost.pem");
  BOOST_LOG_TRIVIAL(trace) << "Before key";
  context_.use_private_key_file("localhost-key.pem", boost::asio::ssl::context::pem);
  BOOST_LOG_TRIVIAL(trace) << "Before dh file";
  context_.use_tmp_dh_file("dhparam4096.pem");
  BOOST_LOG_TRIVIAL(trace) << "In https_server constructor, threads, dealt with keys and certs";
  dispatcher_ = new Dispatcher(config);
  logger_ = new Logger();
  start_accept(&new_session);
}

std::string https_server::get_password() const {
  BOOST_LOG_TRIVIAL(trace) << "https_server::get_password()";
  return "test";
}

void https_server::start_accept() {
  https_session* new_session = new https_session(io_service_, context_, dispatcher_);
  start_accept(new_session);
}

void https_server::start_accept(https_session* new_session) {
  BOOST_LOG_TRIVIAL(trace) << "In server::start_accept()";

  acceptor_.async_accept(new_session->sslsocket(),
      boost::bind(&https_server::handle_accept, this, new_session,
                  boost::asio::placeholders::error));
  
  BOOST_LOG_TRIVIAL(trace) << "In server::start_accept(), after async_accept()";
}

void https_server::handle_accept(https_session* new_session, const boost::system::error_code& error) {
  if (!error) {
    BOOST_LOG_TRIVIAL(trace) << "In server::handle_accept(). Starting session.";

    boost::thread thread(start_session, boost::ref(io_service_), new_session);
  }
  else {
    BOOST_LOG_TRIVIAL(trace) << "In server::handle_accept(). Error connecting, deleting session.";
    delete new_session;
  }

  start_accept();
}