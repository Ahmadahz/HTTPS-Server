#include "proxy_handler.h"
#include <boost/log/trivial.hpp>

ProxyHandler::ProxyHandler(const std::string& location_path, const NginxConfig& config) : RequestHandler() {
  
}

http::response<http::string_body> ProxyHandler::handle_request(const http::request<http::string_body>& request) {
     
  http::response<http::string_body> response;
 
  return response;
}