#ifndef _HANDLER_H_
#define _HANDLER_H_

#include <string>
#include <vector>

#include <boost/beast/http.hpp>
#include <boost/beast/http/message.hpp>

namespace http = boost::beast::http;

class NginxConfig;

class RequestHandler {
public:
  std::string get_path(const char* request);
  std::string get_location(const char* request);
  
  virtual http::response<http::string_body> handle_request(const http::request<http::string_body>& request) = 0;
  
protected:
  RequestHandler() {};
  
private:
  
};

#endif
