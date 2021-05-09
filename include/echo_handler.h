#ifndef _ECHO_HANDLER_H_
#define _ECHO_HANDLER_H_

#include "handler.h"

class NginxConfig;

class EchoHandler : public RequestHandler {
public:
  EchoHandler(const std::string& location_path, const NginxConfig& config);
  
private:
  http::response<http::string_body> handle_request(const http::request<http::string_body>& request) override;
  
};

#endif
