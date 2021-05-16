#include "gtest/gtest.h"
#include "handler.h"
#include "echo_handler.h"
#include "file_handler.h"
#include "404_handler.h"
#include "proxy_handler.h"
#include "config_parser.h"

namespace http = boost::beast::http;

class FileHandlerTest : public ::testing::Test {
protected:
  http::request<http::string_body> make_request(const std::string& request) {
    boost::system::error_code ec;
    http::request_parser<http::string_body> parser;
    parser.eager(true);
    parser.put(boost::asio::buffer(request, request.size()), ec);
    http::request<http::string_body> req = parser.get();

    set_config("root test_files;");
    
    return req;
  };

  NginxConfigParser config_parser_;
  NginxConfig config_;
  NginxConfig proxy_config_;
  std::istringstream ss_;
  
  std::string response_404 = "HTTP/1.1 404 Not Found\nContent-Length: 22\nContent-Type: text/html\n\n<h1>404 Not Found</h1>";
  std::string path_to_echo = "/echo/";

  std::string html_req = "GET /static/file.html HTTP/1.1\r\n";
  std::string txt_req = "GET /static/file.txt HTTP/1.1\r\n";
  std::string jpg_req = "GET /static/file.jpg HTTP/1.1\r\n";
  std::string zip_req = "GET /static/file.zip HTTP/1.1\r\n";
  std::string echo_req = "GET /echo HTTP/1.1\r\n";
  std::string proxy_req = "GET /proxy/ HTTP/1.1\r\n";
  // std::string status_req = "GET /status HTTP/1.1\r\n";

  std::istream* str_to_istream(const std::string &str) {
    ss_.clear();
    ss_.str(str);
    return dynamic_cast<std::istream*>(&ss_);
  }

private:
  // Wrapper function for parsing files or manually inputted strings.
  void parse_config(const std::string& str, bool is_filename = false) {
    config_.Reset();
    if (is_filename)
      config_parser_.Parse(str.c_str(), &config_);
    else
      config_parser_.Parse(str_to_istream(str), &config_);
  }

  void set_config(const std::string& config) {
    parse_config(config);
  };

};

TEST_F(FileHandlerTest, Mime_Type_Check) {
  http::request<http::string_body> html_request = make_request(html_req);
  http::request<http::string_body> txt_request = make_request(txt_req);
  http::request<http::string_body> jpg_request = make_request(jpg_req);
  http::request<http::string_body> zip_request = make_request(zip_req);

  RequestHandler* handler = new FileHandler("", config_);

  http::response<http::string_body> response;
  response = handler->handle_request(html_request);
  EXPECT_EQ(response[http::field::content_type], "text/html");
  
  response = handler->handle_request(txt_request);
  EXPECT_EQ(response[http::field::content_type], "text/plain");
  
  response = handler->handle_request(jpg_request);
  EXPECT_EQ(response[http::field::content_type], "image/jpeg");
  
  response = handler->handle_request(zip_request);
  EXPECT_EQ(response[http::field::content_type], "application/zip");
}

TEST_F(FileHandlerTest, Echo_Check) {
  http::request<http::string_body> echo_request = make_request(echo_req);
  RequestHandler* handler = new EchoHandler("", config_);

  http::response<http::string_body> response;
  response = handler->handle_request(echo_request);
  std::string response_body(response.body().data(), response.body().size());
  EXPECT_EQ(response_body, echo_req);
}

TEST_F(FileHandlerTest, Unsupported_Mime) {
  http::request<http::string_body> gif_request = make_request("GET /static/file.gif HTTP/1.1\r\n");

  RequestHandler* handler = new FileHandler("", config_);

  http::response<http::string_body> response;
  response = handler->handle_request(gif_request);
  EXPECT_EQ(response[http::field::content_type], "unsupported");
}

TEST_F(FileHandlerTest, 404Handler_Check) {
  http::request<http::string_body> bad_request = make_request("GET /bad/file.txt HTTP/1.1\r\n");
  RequestHandler* handler = new _404Handler("", config_);

  http::response<http::string_body> response;
  response = handler->handle_request(bad_request);
  std::string response_body(response.body().data(), response.body().size());
  EXPECT_EQ(response_body, "<h1>404 Not Found</h1>");
}

TEST_F(FileHandlerTest, Proxy_Check) {
  config_parser_.Parse(str_to_istream("host \"http://www.example.com/\";\nport 80;"), &proxy_config_);
  http::request<http::string_body> proxy_request = make_request(proxy_req);

  RequestHandler* handler = new ProxyHandler("/proxy/", proxy_config_);

  http::response<http::string_body> response;
  response = handler->handle_request(proxy_request);
  EXPECT_EQ(response[http::field::content_type], "text/html");

}

// TEST_F(FileHandlerTest, StatusHandler_Check) {
//   http::request<http::string_body> status_request = make_request(status_req);
//   RequestHandler* handler = new StatusHandler("", config_);

//   http::response<http::string_body> response;
//   response = handler->handle_request(status_request);
//   std::string response_body(response.body().data(), response.body().size());
//   EXPECT_EQ(response_body, response_body);
// }
// <h1>Total number of requests</h1>
// <div>0</div>
// <h2>Detail Request Status</h2>
// <table>
// <tbody>
// <tr><th>URL Requested</th><th>Return Code</th></tr>
// </tr></tbody>
// </table>
// <h2>Request Handlers</h2>
// <table><tbody><tr><th>URL Prefix</th><th>Handler</th></tr>
// </tr></tbody></table>