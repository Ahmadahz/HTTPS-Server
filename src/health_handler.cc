#include "health_handler.h"
#include <boost/log/trivial.hpp>

HealthHandler::HealthHandler(const std::string& location_path, const NginxConfig& config) : RequestHandler() {
  
}

http::response<http::string_body> HealthHandler::handle_request(const http::request<http::string_body>& request) {
  BOOST_LOG_TRIVIAL(trace) << "Entered HealthHandler::handle_request.";
  
  http::response<http::string_body> response;
  response.result(200);
  response.set(http::field::content_type, "text/plain");
  response.body() = "OK";
  response.prepare_payload();

  return response;
}
