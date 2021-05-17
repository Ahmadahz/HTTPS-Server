#ifndef _PROXY_HANDLER_H_
#define _PROXY_HANDLER_H_

#include "handler.h"

class NginxConfig;

class ProxyHandler : public RequestHandler {
public:
  ProxyHandler(const std::string& location_path, const NginxConfig& config);
  http::response<http::string_body> handle_request(const http::request<http::string_body>& request) override;

private:
  std::string prefix;
  std::string host;
  std::string path;
  std::string port;
};

#endif
