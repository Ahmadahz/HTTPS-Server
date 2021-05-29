//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/log/trivial.hpp>
#include <boost/asio/ssl.hpp>

using boost::asio::ip::tcp;
typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;

#include "server.h"
#include "https_server.h"
#include "config_parser.h"

int main(int argc, char* argv[]) {
  try {
    if (argc != 2) {
      std::cerr << "Usage: server <config_file>\n";
      return 1;
    }

    boost::asio::io_service io_service;
    boost::asio::io_service https_io_service;

    NginxConfigParser parser;
    NginxConfig out_config;

    if (parser.Parse(argv[1], &out_config)) {
      short port_number;
      short ssl_port_number;
      if (parser.GetPortNumber(out_config, "HTTP", port_number)) {    
        if (parser.GetPortNumber(out_config, "HTTPS", ssl_port_number) && parser.GetKeyPath(out_config))  {
            BOOST_LOG_TRIVIAL(trace) << "Created server listening  on SSL port: " << ssl_port_number;
            BOOST_LOG_TRIVIAL(trace) << "Private path SSL: " << parser.GetSSLPrivateKeyPath();
            BOOST_LOG_TRIVIAL(trace) << "Public path SSL: " << parser.GetSSLPublicKeyPath();
        }
        BOOST_LOG_TRIVIAL(trace) << "Created server listening on port: " << port_number;
        

        // https://stackoverflow.com/questions/15496950/using-multiple-io-service-objects
        server s(io_service, port_number, out_config);
        https_server https_s(https_io_service, ssl_port_number, out_config);

        // io_service.run();
        boost::thread_group threads;
        threads.create_thread(boost::bind(&boost::asio::io_service::run, &https_io_service));
        io_service.run();
        threads.join_all();
        
        BOOST_LOG_TRIVIAL(trace) << "io_services ran and all work has finished";
      }
      else {
        BOOST_LOG_TRIVIAL(error) << "In main: Port number error";
        return 1;
      }
    }
    else {
      // Error message already provided in NginxConfigParser.
      return 1;
    }
  }
  catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
