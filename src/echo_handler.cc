#include "echo_handler.h"

EchoHandler::EchoHandler() {
  
}

std::string EchoHandler::generate_response(const Request& request) {
  // Echo with the usual response.
  
  std::string response;
  response += "    \n"; // This line is needed because otherwise
                        // the HTTP header disappears in the browser.

  response += "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: text/plain\r\n";
  response += "\r\n";
  response += request.content;

  return response;
}
