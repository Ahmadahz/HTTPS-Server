#ifndef _DISPATCHER_H_
#define _DISPATCHER_H_

#include <iostream>
#include <map>

#include "config_parser.h"
#include "handler.h"

static const std::string RequestFile = "FileHandler";
static const std::string RequestEcho = "EchoHandler";

/*
    Processes Config File to Determine which Handler should be used and deploys that handler
*/
class Dispatcher {
public:
    Dispatcher(const NginxConfig& config);

    RequestHandler* get_request_handler(const Request& request) const;
private:
    std::map<std::string, RequestHandler*> handlers_; 
    size_t init_handlers(const NginxConfig& config);
    bool find_path(const NginxConfig& config, std::string path, std::string handler_type);
    size_t reg_num;
};

#endif