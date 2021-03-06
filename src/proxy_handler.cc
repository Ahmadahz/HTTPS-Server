#include "proxy_handler.h"
#include "config_parser.h"

//boost lib
#include <boost/log/trivial.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>

//stb lib
#include <functional>
#include <tuple>
#include <string>
#include <iostream>       // std::cout, std::endl


using tcp = boost::asio::ip::tcp;
typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;

/*
  Extract protocol, host, and path from a given url.
  For example, if url is "http://www.example.com/index.html", return value will be 
  {"http", "www.example.com", "index.html"}
*/
std::tuple<std::string, std::string, std::string> parse_url(std::string const& url) {
  std::function<std::string(std::string const&)> remove_end_slash = [&remove_end_slash](std::string const& s) {
    if (s[s.length() - 1] == '/') return remove_end_slash(s.substr(0, s.length() - 1));
    else return s;
  };

  std::string modified_url = remove_end_slash(url);
  size_t host_start = modified_url.find("://");
  if (host_start == std::string::npos) {
    return {"", "", ""};
  }
  else {
    size_t pos_after_protocol = host_start + 3;
    size_t pos_after_host = modified_url.find("/", pos_after_protocol);
    std::string protocol = modified_url.substr(0, host_start);
    std::string host = modified_url.substr(pos_after_protocol, pos_after_host - pos_after_protocol);
    std::string path = pos_after_host != std::string::npos ? modified_url.substr(pos_after_host) : "/";
    return { protocol, host, path };
  }
}

ProxyHandler::ProxyHandler(const std::string& location_path, const NginxConfig& config) : RequestHandler(), prefix(location_path) {
  for (auto const& statement : config.statements_) {
    if (statement->tokens_[0] == "host" && statement->tokens_.size() == 2) {
      auto url_components = parse_url(statement->tokens_[1]);
      if(std::get<0>(url_components) != "http" && std::get<0>(url_components) != "https") {
        BOOST_LOG_TRIVIAL(error) << "Unspecified or unsupported network protocol\n";
        throw std::runtime_error("bad config file\n");
      }
      else {
        host = std::get<1>(url_components);
        path = std::get<2>(url_components);
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
}

http::response<http::string_body> issue_outside_request(std::string const& host, std::string const& port, std::string const& target) {
  // This one is modified from boost official website
  http::response<http::string_body> response;
  try {
    if (port == "80") {
      boost::asio::io_context ioc;
      tcp::resolver resolver(ioc);
      boost::beast::tcp_stream stream(ioc);
      auto const results = resolver.resolve(host, port);
      stream.connect(results);

      // Form request and send
      http::request<http::string_body> req{ http::verb::get, target, 11 };
      req.set(http::field::host, host);
      http::write(stream, req);

      // Read response from the server
      boost::beast::flat_buffer buffer;
      http::read(stream, buffer, response);
      
      boost::beast::error_code ec;
      stream.socket().shutdown(tcp::socket::shutdown_both, ec);
      if (ec && ec != boost::beast::errc::not_connected)
        throw boost::beast::system_error{ ec };
    }
    else if (port == "443") {
      BOOST_LOG_TRIVIAL(trace) << "Handling https redirect" << std::endl;
      
      boost::asio::io_context ioc;
      tcp::resolver resolver(ioc);
      tcp::resolver::query query(host, port);
      tcp::resolver::iterator iterator = resolver.resolve(query);
      boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);
      ctx.set_default_verify_paths();
      //https://www.boost.org/doc/libs/1_61_0/doc/html/boost_asio/overview/ssl.html
      ssl_socket sock(ioc, ctx);
      boost::asio::connect(sock.lowest_layer(), iterator);
      sock.lowest_layer().set_option(tcp::no_delay(true));
      sock.handshake(ssl_socket::client);
      
      BOOST_LOG_TRIVIAL(trace) << "After handshake" << std::endl;

      // Form request and send
      http::request<http::string_body> req{ http::verb::get, target, 11 };
      req.set(http::field::host, host);
      http::write(sock, req);

      // Read the response from the server
      boost::beast::flat_buffer buffer;
      http::read(sock, buffer, response);
      
      boost::beast::error_code ec;
      sock.lowest_layer().close();
      BOOST_LOG_TRIVIAL(trace) << "SSL socket shutdown" << std::endl;
    }
  }
  catch (std::exception const& e) {
    BOOST_LOG_TRIVIAL(error) << "Error: " << e.what() << std::endl;
  }

  if (http::to_status_class(response.result()) == http::status_class::redirection) {
    BOOST_LOG_TRIVIAL(trace) << "Redirection found\n";
    std::string redirected_url = std::string(response[http::field::location].data(), response[http::field::location].size());
    auto url_components = parse_url(redirected_url);
    if(std::get<0>(url_components) != "http" && std::get<0>(url_components) != "https") {
      BOOST_LOG_TRIVIAL(error) << "Error: Redirection into non http or https"<< std::endl;
      return response;
    }
    else if (std::get<0>(url_components) == "https") {
      BOOST_LOG_TRIVIAL(trace) << "https redirection:" << std::get<1>(url_components) << ", " << std::get<2>(url_components) << std::endl;
      return issue_outside_request(std::get<1>(url_components), "443", std::get<2>(url_components));
    }
    else {
      return issue_outside_request(std::get<1>(url_components), "80", std::get<2>(url_components));
    }
  }
  else return response;
}

http::response<http::string_body> ProxyHandler::handle_request(const http::request<http::string_body>& request) {
  BOOST_LOG_TRIVIAL(trace) << "In ProxyHandler::handle_request method" << std::endl;
  std::string target = std::string(request.target().data(), request.target().size()).substr(prefix.length());
  if (target == "") target = "/";
  auto response = issue_outside_request(host, port, target);
  std::string content_type = std::string(response[http::field::content_type].data(), response[http::field::content_type].size());
  size_t content_type_loc = content_type.find(";");
  if (content_type_loc != std::string::npos) {
    response.set(http::field::content_type, content_type.substr(0, content_type_loc));
  }

  return response;
}


