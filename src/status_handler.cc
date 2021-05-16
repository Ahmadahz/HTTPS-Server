#include "status_handler.h"

StatusHandler::StatusHandler(const std::string& location_path, const NginxConfig& config) : RequestHandler() {
  
}

http::response<http::string_body> StatusHandler::handle_request(const http::request<http::string_body>& request) {
  BOOST_LOG_TRIVIAL(trace) << "Entered StatusHandler::handle_request.";
  std::string request_info;
  std::string handler_info;
  std::string display_html_content;

  for (auto it = session::handled_requests.begin(); it != session::handled_requests.end(); it++) {
    for (auto code : (it->second)) {
      request_info += "<tr><td>" + it->first + "</td><td>" + std::to_string(code) + "</td></tr>";
    }
  }

  for (auto it = Dispatcher::handler_info.begin(); it != Dispatcher::handler_info.end(); it++) {
    handler_info += "<tr><td>" + it->first + "</td><td>" + it->second + "</td></tr>" ;
  }

  std::string display_content = "<h1>Total number of requests: "+ std::to_string(session::request_count) +"</h1>"
    "<h2>Detail Request Status</h2>"
    "<table>"
    "<tr><th>URL Requested</th><th>Return Code</th></tr>" + request_info + "</table>"
    "<h2>Request Handlers</h2>"
    "<table>"
    "<tr><th>URL Prefix</th><th>Handler</th></tr>" + handler_info +  "</table>";
  http::response<http::string_body> response;
  response.result(200);
  response.body() = display_content;
  response.prepare_payload();

  return response;
}