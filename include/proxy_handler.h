#ifndef _PROXY_HANDLER_H_
#define _PROXY_HANDLER_H_

#include "handler.h"

class NginxConfig;

class ProxyHandler : public RequestHandler {
public:
  ProxyHandler(const std::string& location_path, const NginxConfig& config);
  
private:
  http::response<http::string_body> handle_request(const http::request<http::string_body>& request) override;
  http::response<http::string_body> issue_outside_request(const http::request<http::string_body>& request);

  std::string prefix;
  std::string host;
  std::string path;
  std::string port;
};

#endif
