#include "proxy_handler.h"
#include "config_parser.h"
#include <boost/log/trivial.hpp>
#include <functional>


ProxyHandler::ProxyHandler(const std::string& location_path, const NginxConfig& config) : RequestHandler(), prefix(location_path) {
  for (auto const& statement : config.statements_) {
    if (statement->tokens_[0] == "host" && statement->tokens_.size() == 2) {
        std::function<std::string(std::string const&)> remove_end_slash = [&remove_end_slash](std::string const& s) {
            if(s[s.length() - 1] == '/') return remove_end_slash(s.substr(0, s.length()-1));
            else return s;
        };
        host = remove_end_slash(statement->tokens_[1]);
    }
    else if (statement->tokens_[0] == "port" && statement->tokens_.size() == 2) {
        port = statement->tokens_[1];
    }
  }

  BOOST_LOG_TRIVIAL(trace) << "The ProxyHandler prefix is: " << prefix << std::endl;
  BOOST_LOG_TRIVIAL(trace) << "The ProxyHandler host is: " << host << std::endl;
  BOOST_LOG_TRIVIAL(trace) << "The ProxyHandler port is: " << port << std::endl;
  assert(host != "" && port != "");
}

http::response<http::string_body> ProxyHandler::handle_request(const http::request<http::string_body>& request) {

  http::response<http::string_body> response;
 
  return response;
}