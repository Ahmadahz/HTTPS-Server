#ifndef _404_HANDLER_H_
#define _404_HANDLER_H_

#include "handler.h"

class NginxConfig;

class _404Handler : public RequestHandler {
public:
  _404Handler(const std::string& location_path, const NginxConfig& config);
  
private:
  http::response<http::string_body> handle_request(const http::request<http::string_body>& request) override;
  
};

#endif
