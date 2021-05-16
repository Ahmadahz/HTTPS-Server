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
      auto modified_url = remove_end_slash(statement->tokens_[1]);
      auto host_start = modified_url.find("//");
      if (host_start == std::string::npos) {
        BOOST_LOG_TRIVIAL(error) << "The ProxyHandler config did not specify protocol or host." << std::endl;
      }
      else {
        auto pos_after_protocol = host_start + 2;
        auto pos_after_host = modified_url.find("/", pos_after_protocol);
        host = modified_url.substr(pos_after_protocol, pos_after_host-pos_after_protocol);
        path = pos_after_host!=std::string::npos ? modified_url.substr(pos_after_host) : "/";
      }
    }
    else if (statement->tokens_[0] == "port" && statement->tokens_.size() == 2) {
      port = statement->tokens_[1];
    }
  }

  BOOST_LOG_TRIVIAL(trace) << "The ProxyHandler prefix is: " << prefix << std::endl;
  BOOST_LOG_TRIVIAL(trace) << "The ProxyHandler host is: " << host << std::endl;
  BOOST_LOG_TRIVIAL(trace) << "The ProxyHandler path is: " << path << std::endl;
  BOOST_LOG_TRIVIAL(trace) << "The ProxyHandler port is: " << port << std::endl;
  assert(host != "" && port != "");
}

http::response<http::string_body> ProxyHandler::handle_request(const http::request<http::string_body>& request) {

  http::response<http::string_body> response;
 
  return response;
}
