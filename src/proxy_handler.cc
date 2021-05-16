#include "proxy_handler.h"
#include "config_parser.h"
#include <boost/log/trivial.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio.hpp>
#include <functional>

namespace asio = boost::asio;
using tcp = boost::asio::ip::tcp;

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

http::response<http::string_body> ProxyHandler::issue_outside_request(const http::request<http::string_body>& request) {
  BOOST_LOG_TRIVIAL(trace) << "Request: " << std::endl << request << std::endl;
  boost::asio::streambuf request_buf;
  std::ostream oss(&request_buf);
  oss << request;
  asio::io_service io_service;
  tcp::resolver resolver(io_service);
  tcp::resolver::query query(host, port);
  boost::beast::error_code ec;
  auto endpoint_iterator = resolver.resolve(query, ec);
  if (ec)
    BOOST_LOG_TRIVIAL(error) << "Failed to resolve host " << host << std::endl;
  tcp::socket socket(io_service);
  BOOST_LOG_TRIVIAL(trace) << "Start connecting to redirected website..." << std::endl;
  asio::connect(socket, endpoint_iterator);
  BOOST_LOG_TRIVIAL(trace) << "Sending request..." << std::endl;
  asio::write(socket, request_buf);
  boost::asio::streambuf response_buf;
  BOOST_LOG_TRIVIAL(trace) << "Reading response..." << std::endl;
  asio::read_until(socket, response_buf, "\r\n");
  std::string response_str((std::istreambuf_iterator<char>(&response_buf)),std::istreambuf_iterator<char>());
  BOOST_LOG_TRIVIAL(trace) << "Response received: " << std::endl << response_str << std::endl;
  http::response_parser<http::string_body> res_parser;
  res_parser.put(asio::buffer(response_str), ec);
  http::response<http::string_body> response = res_parser.get();
  return response;
}

http::response<http::string_body> ProxyHandler::handle_request(const http::request<http::string_body>& request) {
  BOOST_LOG_TRIVIAL(trace) << "ProxyHandler start handling request." << std::endl;
  http::request<http::string_body> m_request = request;
  BOOST_LOG_TRIVIAL(trace) << "Request body: " << request.body() << std::endl;
  BOOST_LOG_TRIVIAL(trace) << "Request version: " << request.version() << std::endl;
  BOOST_LOG_TRIVIAL(trace) << "Request target: " << request.target() << std::endl;
  BOOST_LOG_TRIVIAL(trace) << "Request method: " << request.method() << std::endl;
  for (auto const& field : request)
    BOOST_LOG_TRIVIAL(trace) << "Request field name: " << field.name() << ", value: " << field.value() << std::endl;
  m_request.set("Host", host);
  std::string redirect_path = (path=="/"?"":path) + std::string(request.target()).substr(prefix.length());
  redirect_path = redirect_path == ""?"/":redirect_path;
  m_request.target(redirect_path);
  BOOST_LOG_TRIVIAL(trace) << "Modified request body: " << m_request.body() << std::endl;
  BOOST_LOG_TRIVIAL(trace) << "Modified request version: " << m_request.version() << std::endl;
  BOOST_LOG_TRIVIAL(trace) << "Modified request target: " << m_request.target() << std::endl;
  BOOST_LOG_TRIVIAL(trace) << "Modified request method: " << m_request.method() << std::endl;
  for (auto const& field : m_request)
    BOOST_LOG_TRIVIAL(trace) << "Modified request field name: " << field.name() << ", value: " << field.value() << std::endl;
  
  
  http::response<http::string_body> response = issue_outside_request(m_request);
  BOOST_LOG_TRIVIAL(trace) << "Response received: " << response << std::endl;
 
  return response;
}
