#include "404_handler.h"
#include <boost/log/trivial.hpp>

_404Handler::_404Handler(const std::string& location_path, const NginxConfig& config) {
  
}

std::vector<char> _404Handler::generate_response(const Request& request) {
  // Returns a 404 Response.
  std::string _404_response = "HTTP/1.1 404 Not Found\nContent-Length: 22\nContent-Type: text/html\n\n<h1>404 Not Found</h1>";
  std::vector<char> re(_404_response.begin(), _404_response.end());
  return re;
}

http::response<http::string_body> _404Handler::handle_request(const http::request<http::string_body>& request) {
  BOOST_LOG_TRIVIAL(trace) << "Entered 404Handler handle_request.";
  
  http::response<http::string_body> response;
  response.result(404);
  response.prepare_payload();

  return response;
}
