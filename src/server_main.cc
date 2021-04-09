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

using boost::asio::ip::tcp;

#include "server.h"
#include "config_parser.h"

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: server <config_file>\n";
      return 1;
    }

    boost::asio::io_service io_service;

    NginxConfigParser parser;
    NginxConfig out_config;

    if (parser.Parse(argv[1], &out_config)) {
      std::vector<std::shared_ptr<NginxConfigStatement>> statements;
      statements = out_config.statements_;
      
      // Searches each statement found by the parser for the port number.
      // A config statement is either a single line of tokens or an entire code block.
      for (int i = 0; i < statements.size(); ++i) {
        
        // Assumes that the statement containing the port number is of the form:
        // http {
        //    ...
        //    listen  XX;
        // }
        // where `http' is the token (hence size being 1).
        // The token size check is to ensure that the config file is properly formed
        // in a way that the config parser doesn't currently check for.
        if (statements[i]->tokens_.size() == 1 &&
	    statements[i]->tokens_[0] == "http") {
	  std::string port_statement = statements[i]->ToString(0);
	  size_t port_pos = port_statement.find("listen");
	  
	  if (port_pos != std::string::npos) {
	    port_pos += 6; // Go to position after the end of "listen"
            
            // Ignore whitespace after "listen" is found in search of start and end
            // positions of the port number.
	    size_t port_start_pos = port_statement.find_first_not_of(" \t\n\r", port_pos);
	    size_t port_end_pos = port_statement.find_first_not_of("0123456789", port_start_pos + 1);
	    
	    if (port_start_pos == std::string::npos ||
		port_end_pos == std::string::npos) {
	      std::cerr << "No valid port number provided." << std::endl;
	      return 1;
	    }
	    short port_number = stoi(port_statement.substr(port_start_pos, port_end_pos - port_start_pos));
	    std::cerr << std::to_string(port_number) << std::endl;

	    server s(io_service, port_number);

	    io_service.run();
	    
	  } else {
	    std::cerr << "\"listen\" token not found." << std::endl;
	    return 1;
	  }
        }
      }
    } else {
      // Error message already provided in NginxConfigParser.
      return 1;
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
