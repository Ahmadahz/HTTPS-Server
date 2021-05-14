#ifndef _HANDLER_H_
#define _HANDLER_H_

#include <string>
#include <boost/beast/http.hpp>

namespace http = boost::beast::http;

class RequestHandler {
public:  
  virtual http::response<http::string_body> handle_request(const http::request<http::string_body>& request) = 0;
  virtual std::string to_string() const = 0;
  
protected:
  RequestHandler() {};
  
private:
  
};

#endif
