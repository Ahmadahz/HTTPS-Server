#ifndef _STATUS_HANDLER_H_
#define _STATUS_HANDLER_H_

#include "handler.h"
#include <boost/log/trivial.hpp>
#include "session.h"
#include "dispatcher.h"

class NginxConfig;

class StatusHandler : public RequestHandler {
public:
  StatusHandler(const std::string& location_path, const NginxConfig& config);
private:
  http::response<http::string_body> handle_request(const http::request<http::string_body>& request) override;
  std::map<std::string, int> condense();
  const std::vector<std::string> split(const std::string& s, const char& c);
};

#endif
