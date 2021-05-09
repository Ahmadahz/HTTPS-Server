#include "404_handler.h"

_404Handler::_404Handler() {
  
}

std::vector<char> _404Handler::generate_response(const Request& request) {
  // Returns a 404 Response.
  std::string _404_response = "HTTP/1.1 404 Not Found\nContent-Length: 22\nContent-Type: text/html\n\n<h1>404 Not Found</h1>";
  std::vector<char> re(_404_response.begin(), _404_response.end());
  return re;
}
