#ifndef _STATUS_HANDLER_H_
#define _STATUS_HANDLER_H_

#include "handler.h"
class NginxConfig;

class StatusHandler : public RequestHandler {
public:
  StatusHandler(const std::string& location_path, const NginxConfig& config);
  std::string to_string() const override {return "StatusHandler";}
private:
  http::response<http::string_body> handle_request(const http::request<http::string_body>& request) override;
  
};

#endif
