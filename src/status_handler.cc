#include "status_handler.h"
#include <boost/log/trivial.hpp>

StatusHandler::StatusHandler(const std::string& location_path, const NginxConfig& config) {
  
}

http::response<http::string_body> StatusHandler::handle_request(const http::request<http::string_body>& request) {
  BOOST_LOG_TRIVIAL(trace) << "Entered StatusHandler::handle_request.";
  
  http::response<http::string_body> response;
  response.result(404);
  response.body() = "<h1>404 Not Found</h1>";
  response.prepare_payload();

  return response;
}