#ifndef _HEALTH_HANDLER_H_
#define _HEALTH_HANDLER_H_

#include "handler.h"

class NginxConfig;

class HealthHandler : public RequestHandler {
public:
  HealthHandler(const std::string& location_path, const NginxConfig& config);
  
private:
  http::response<http::string_body> handle_request(const http::request<http::string_body>& request) override;
};

#endif
