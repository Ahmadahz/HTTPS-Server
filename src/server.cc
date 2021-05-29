#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio/ssl.hpp>

using boost::asio::ip::tcp;
typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;

#include "server.h"
#include "logger.h"

void start_session(boost::asio::io_service& io_service, session* s) {
  s->start();
  io_service.run();
}

server::server(boost::asio::io_service& io_service, short port, const NginxConfig &config)
  : io_service_(io_service),
    acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
    context_(boost::asio::ssl::context::sslv23) {
  dispatcher_ = new Dispatcher(config);
  logger_ = new Logger();
  version_ = "http";
  BOOST_LOG_TRIVIAL(trace) << "In server constructor 1, version:" << version_;
  start_accept();
}

server::server(session& new_session, boost::asio::io_service& io_service, short port, const NginxConfig &config)
  : io_service_(io_service),
    acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
    context_(boost::asio::ssl::context::sslv23) {
  dispatcher_ = new Dispatcher(config);
  logger_ = new Logger();
  version_ = "http";
  BOOST_LOG_TRIVIAL(trace) << "In server constructor 2, version:" << version_;
  start_accept(&new_session);
}

server::server(boost::asio::io_service& io_service, short port, const NginxConfig &config, std::string version)
  : io_service_(io_service),
    acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
    context_(boost::asio::ssl::context::sslv23),
    version_(version) {
  BOOST_LOG_TRIVIAL(trace) << "In server constructor 3, before version check";
  BOOST_LOG_TRIVIAL(trace) << "In server constructor 3, version:" << version;
  if (version_ == "https") {
    BOOST_LOG_TRIVIAL(trace) << "In server constructor 3, version:" << version_;
    context_.set_options(
          boost::asio::ssl::context::default_workarounds
          | boost::asio::ssl::context::no_sslv2
          | boost::asio::ssl::context::single_dh_use);
    context_.set_password_callback(boost::bind(&server::get_password, this));
    BOOST_LOG_TRIVIAL(trace) << "Before certificate";
    context_.use_certificate_chain_file("localhost.pem");
    BOOST_LOG_TRIVIAL(trace) << "Before key";
    context_.use_private_key_file("localhost-key.pem", boost::asio::ssl::context::pem);
    BOOST_LOG_TRIVIAL(trace) << "Before dh file";
    context_.use_tmp_dh_file("dhparam4096.pem");
    BOOST_LOG_TRIVIAL(trace) << "In server constructor 3, dealt with keys and certs";
  }
  else if (version_ == "http") {
    BOOST_LOG_TRIVIAL(trace) << "In server constructor 3, version:" << version_;
  }
  else {
    BOOST_LOG_TRIVIAL(error) << "Version given to server is not supported. Only http or https supported.";
  }
  dispatcher_ = new Dispatcher(config);
  logger_ = new Logger();
  start_accept();
}

server::server(session& new_session, boost::asio::io_service& io_service, short port, const NginxConfig &config, std::string version)
  : io_service_(io_service),
    acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
    context_(boost::asio::ssl::context::sslv23),
    version_(version) {
  BOOST_LOG_TRIVIAL(trace) << "In server constructor 4, before version check";
  BOOST_LOG_TRIVIAL(trace) << "In server constructor 4, version:" << version;
  if (version_ == "https") {
    BOOST_LOG_TRIVIAL(trace) << "In server constructor 4, version:" << version_;
    context_.set_options(
          boost::asio::ssl::context::default_workarounds
          | boost::asio::ssl::context::no_sslv2
          | boost::asio::ssl::context::single_dh_use);
    context_.set_password_callback(boost::bind(&server::get_password, this));
    BOOST_LOG_TRIVIAL(trace) << "Before certificate";
    context_.use_certificate_chain_file("localhost.pem");
    BOOST_LOG_TRIVIAL(trace) << "Before key";
    context_.use_private_key_file("localhost-key.pem", boost::asio::ssl::context::pem);
    BOOST_LOG_TRIVIAL(trace) << "Before dh file";
    context_.use_tmp_dh_file("dhparam4096.pem");
    BOOST_LOG_TRIVIAL(trace) << "In server constructor 4, dealt with keys and certs";
  }
  else if (version_ == "http") {
    BOOST_LOG_TRIVIAL(trace) << "In server constructor 4, version:" << version_;
  }
  else {
    BOOST_LOG_TRIVIAL(error) << "Version given to server is not supported. Only http or https supported.";
  }
  dispatcher_ = new Dispatcher(config);
  logger_ = new Logger();
  start_accept(&new_session);
}

std::string server::get_password() const {
  BOOST_LOG_TRIVIAL(trace) << "server::get_password()";
  return "test";
}

void server::start_accept() {
  if (version_ == "https") {
    session* new_session = new session(io_service_, context_, dispatcher_);
    start_accept(new_session);
  }
  else if (version_ == "http") {
    session* new_session = new session(io_service_, dispatcher_);
    start_accept(new_session);
  }
}

void server::start_accept(session* new_session) {
  BOOST_LOG_TRIVIAL(trace) << "In server::start_accept()";

  if (version_ == "https") {
    acceptor_.async_accept(new_session->sslsocket(),
        boost::bind(&server::handle_accept, this, new_session,
                    boost::asio::placeholders::error));
  }
  else if (version_ == "http") {
    acceptor_.async_accept(new_session->socket(),
        boost::bind(&server::handle_accept, this, new_session,
                    boost::asio::placeholders::error));
  }
  
  BOOST_LOG_TRIVIAL(trace) << "In server::start_accept(), after async_accept()";
}

void server::handle_accept(session* new_session, const boost::system::error_code& error) {
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