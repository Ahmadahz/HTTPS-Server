#include "404_handler.h"
#include <boost/log/trivial.hpp>

_404Handler::_404Handler(const std::string& location_path, const NginxConfig& config) {
  
}

http::response<http::string_body> _404Handler::handle_request(const http::request<http::string_body>& request) {
  BOOST_LOG_TRIVIAL(trace) << "Entered 404Handler::handle_request.";
  
  http::response<http::string_body> response;
  response.result(404);
  response.body() = "<h1>404 Not Found</h1>";
  response.prepare_payload();

  return response;
}
