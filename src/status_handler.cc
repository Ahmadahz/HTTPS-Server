#include "status_handler.h"

StatusHandler::StatusHandler(const std::string& location_path, const NginxConfig& config) : RequestHandler() {
  
}

/*
  Produces a count for each unique request path and corresponding response code
  from the session::handled_requests map
*/
std::map<std::string, int> StatusHandler::condense() {
  std::map<std::string, int> condense_codes;
  std::string response;
  for (auto it = session::handled_requests.begin(); it != session::handled_requests.end(); it++) {
    for (auto code : (it->second)) {
      response = it->first + " " + std::to_string(code);
      if (condense_codes.count(response) != 0 ) {
        condense_codes[response]++;
      }
      else {
        condense_codes[response] = 1;
      }
    }
  }
  return condense_codes;
}

/*
  Splits a string s with delimiter c and returns a vector containing each string split by the delimiter
*/
const std::vector<std::string> StatusHandler::split(const std::string& s, const char& c) {
  std::string buff;
  std::vector<std::string> v;
  for (auto n : s) {
    if (n != c) {
      buff += n; 
    } 
    else if (n == c && buff != "") { 
      v.push_back(buff); 
      buff = ""; 
    }
  }
  
  if (buff != "") {
    v.push_back(buff);
  }
  
  return v;
}

http::response<http::string_body> StatusHandler::handle_request(const http::request<http::string_body>& request) {
  BOOST_LOG_TRIVIAL(trace) << "Entered StatusHandler::handle_request.";
  std::string request_info;
  std::string handler_info;
  std::string display_html_content;
  std::map<std::string, int> condense_codes = condense();
  for (auto it = condense_codes.begin(); it != condense_codes.end(); it++) {
      std::string row = it->first + " " + std::to_string(it->second);
      std::vector<std::string> tokens = split(row, ' ');
      request_info += "<tr><td>" + tokens[0] + "</td><td>" + tokens[1] + "</td><td>" + tokens[2] + "</td></tr>";
  }

  for (auto it = Dispatcher::handler_info.begin(); it != Dispatcher::handler_info.end(); it++) {
    handler_info += "<tr><td>" + it->first + "</td><td>" + it->second + "</td></tr>" ;
  }

  std::string display_content = "<h1>Total number of requests: "+ std::to_string(session::request_count) +"</h1>"
    "<h2>Detail Request Status</h2>"
    "<table>"
    "<tr><th>URL Requested</th><th>Return Code</th><th>Count</th></tr>" + request_info + "</table>"
    "<h2>Request Handlers</h2>"
    "<table>"
    "<tr><th>URL Prefix</th><th>Handler</th></tr>" + handler_info +  "</table>";
  http::response<http::string_body> response;
  response.result(200);
  response.body() = display_content;
  response.prepare_payload();

  return response;
}
