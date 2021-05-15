#ifndef _DISPATCHER_H_
#define _DISPATCHER_H_

#include <iostream>
#include <map>

#include "config_parser.h"
#include "handler.h"
/*
    Processes Config File to Determine which Handler should be used and deploys that handler
*/
class Dispatcher {
public:
    Dispatcher(const NginxConfig& config);
    int get_regnum() {return regnum_;}
    RequestHandler* get_request_handler(const std::string& uri) const;
    int init_handlers(const NginxConfig& config);
    static std::map<std::string, std::string> handler_info;
private:
    std::map<std::string, RequestHandler*> handlers_; 
    int regnum_;
    bool add_handler(const NginxConfig& config, std::string path, std::string handler_type);
};

#endif