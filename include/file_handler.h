#ifndef _FILE_HANDLER_H_
#define _FILE_HANDLER_H_

#include "handler.h"
#include "config_parser.h"

class FileHandler : public RequestHandler {
public:
  FileHandler(const NginxConfig &config, const std::string &prefix);

  std::string generate_response(const Request& request) override;
  
private:
  std::string root; //Root path where we'll serve files
  std::string prefix; //Prefix that will be substituted with root
};

#endif
