#include "echo_handler.h"
#include <boost/log/trivial.hpp>

EchoHandler::EchoHandler(const std::string& location_path, const NginxConfig& config) : RequestHandler() {
  
}

std::vector<char> EchoHandler::generate_response(const Request& request) {
  // Echo with the usual response.
  
  std::string response;
  response += "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: text/plain\r\n";
  response += "\r\n";
  response += request.content;
  std::vector<char> re(response.begin(), response.end());
  return re;
}

http::response<http::string_body> EchoHandler::handle_request(const http::request<http::string_body>& request) {
  BOOST_LOG_TRIVIAL(trace) << "Entered EchoHandler handle_request.";
  
  http::response<http::string_body> response;
  response.result(200);
  response.set(http::field::content_type, "text/plain");

  std::string payload;
  auto req_header = request.base();

  payload += req_header.method_string().data();
  payload += " ";
  payload += req_header.target().data();
  payload += " HTTP/";
  payload += req_header.version() == 11 ? "1.1\r\n" : "1.0\r\n";
  
  for (auto& field : request) {
    payload += to_string(field.name()).data();
    payload += ": ";
    payload.append(field.value().data(), field.value().size());
    payload += "\r\n";
  }

  BOOST_LOG_TRIVIAL(trace) << "Echo response payload: " << payload;
  
  response.body() = payload;
  response.prepare_payload();
  return response;
}
