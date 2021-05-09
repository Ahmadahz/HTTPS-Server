#ifndef _FILE_HANDLER_H_
#define _FILE_HANDLER_H_

#include "handler.h"
#include "config_parser.h"

class FileHandler : public RequestHandler {
public:
  FileHandler(const std::string& location_path, const NginxConfig& config);
  
  http::response<http::string_body> handle_request(const http::request<http::string_body>& request) override;
  
private:
  const char* mime_type(const http::request<http::string_body>& header);
  
  std::string prefix;
  std::string root;
};

#endif
