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
#include <boost/log/trivial.hpp>

using boost::asio::ip::tcp;

#include "server.h"
#include "config_parser.h"

int main(int argc, char* argv[]) {
  try {
    if (argc != 2) {
      std::cerr << "Usage: server <config_file>\n";
      return 1;
    }

    boost::asio::io_service io_service;

    NginxConfigParser parser;
    NginxConfig out_config;

    if (parser.Parse(argv[1], &out_config)) {
      short port_number;
      if (parser.GetPortNumber(out_config, port_number)) {
				server s(io_service, port_number, out_config);
        BOOST_LOG_TRIVIAL(trace) << "Created server listening on port: " << port_number;
				io_service.run();
        BOOST_LOG_TRIVIAL(trace) << "io_service ran and all work has finished";
      }
      else {
        BOOST_LOG_TRIVIAL(error) << "Port number error";
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
