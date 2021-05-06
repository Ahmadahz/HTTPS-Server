#include <iostream>
#include <iterator>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include "dispatcher.h"
#include "handler.h"
#include "file_handler.h"
#include "echo_handler.h"


/*
Constructor - Constructs a RequestHandler Set. After the constructor has
been called, all members should be immutable
*/
Dispatcher::Dispatcher(const NginxConfig& config) {
   regnum = init_handlers(config);
   BOOST_LOG_TRIVIAL(trace) << "In Dispatcher::Dispatcher: Number of Handlers Registered: " << regnum;
}

/*  
 get_request_handler() - Given a request, matches the longest prefix path
 with the corresponding handlertype

 The prefix should not have any trailing slashes, however "/" is a valid path
 e.g. /bar/ would be set as /bar  
*/
RequestHandler* Dispatcher::get_request_handler(const std::string& uri) const {
    
    std::string path = uri;
    BOOST_LOG_TRIVIAL(trace) << "In Dispatcher::get_request_hander: path before popping: " << path;
    //remove slashes at end of path
    while(path.length() > 1 && path.back() == '/') {
        path.pop_back();
    }

    RequestHandler* handler = nullptr;
    std::string prefix;
    BOOST_LOG_TRIVIAL(trace) << "In Dispatcher::get_request_hander: dispatcher looking for: " << path; 
    //match longest prefix and corresponding handler
    for(auto it = handlers_.begin(); it != handlers_.end(); it++)
    {
        if (path.substr(0, it->first.length()) == it->first) {
            if (it->first.length() > prefix.length()) {
                prefix = it->first;
                handler = it->second;
            }
        }
    }
    BOOST_LOG_TRIVIAL(trace) << "In Dispatcher::get_request_hander: handler with prefix found: " << prefix;
    return handler;
}


/*  
 init_handlers() - Given a config file, parses the file for a server block
 then looks for "location" blocks within the server block

 Returns the number of RequestHandlers that were initialized

*/
int Dispatcher::init_handlers(const NginxConfig& config) {
    size_t reg_num = 0;
        for (auto stmt : config.statements_) {
            if (stmt -> tokens_.size() < 3 && stmt -> tokens_[0] != "listen") {
                BOOST_LOG_TRIVIAL(trace) << "In Dispatcher::get_request_hander: child statement format error. Token[0] is: " << stmt -> tokens_[0];
                continue; //Formatting Error
            }
            if (stmt -> tokens_[0] != "location") {
                BOOST_LOG_TRIVIAL(trace) << "In Dispatcher::get_request_hander: location format error";
                continue; //Formatting Error
            }
            if (find_path(*(stmt-> child_block_), stmt -> tokens_[1], stmt -> tokens_[2])) {
                reg_num++;
            }
        }
    return reg_num;
}

/*
 find_path() called by init_handlers to add one handler
 Returns true if a handler was successfully added
*/
bool Dispatcher::find_path(const NginxConfig& config, std::string path, std::string handler_type) {
    while(path.length() > 1 && path.back() == '/') {
        path.pop_back();
    }

    if (handlers_.find(path) != handlers_.end()) {
        BOOST_LOG_TRIVIAL(trace) << "In Dispatcher::find_path: Path found but already handled: " << path;
        return false; // Already added
    }

    if (handler_type == RequestFile) {
        handlers_[path] = new FileHandler(config, path);
    }
    else if (handler_type == RequestEcho) {
        handlers_[path] = new EchoHandler();
    }
    else {
        BOOST_LOG_TRIVIAL(trace) << "In Dispatcher::find_path: Handlertype couldn't be found";
        return false;
    }
    BOOST_LOG_TRIVIAL(trace) << "In Dispatcher::find_path: Path found: " << path << " handled as: " << handler_type;
    return true;
}