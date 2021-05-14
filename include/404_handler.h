#ifndef _404_HANDLER_H_
#define _404_HANDLER_H_

#include "handler.h"

class NginxConfig;

class _404Handler : public RequestHandler {
public:
  _404Handler(const std::string& location_path, const NginxConfig& config);
  std::string to_string() const override {return "404Handler";}
  
private:
  http::response<http::string_body> handle_request(const http::request<http::string_body>& request) override;
};

#endif
