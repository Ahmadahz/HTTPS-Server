#include "file_handler.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <streambuf>
#include <vector>
#include <boost/log/trivial.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

namespace http = boost::beast::http;

FileHandler::FileHandler(const std::string& location_path, const NginxConfig& config) {
  prefix = location_path;
  root = "/";
  for (auto statement : config.statements_) {
    if (statement->tokens_[0] == "root" && statement->tokens_.size() == 2) {
      root = statement->tokens_[1];
    }
  }

  BOOST_LOG_TRIVIAL(trace) << "The FileHandler root path is: " << root << std::endl;
}

http::response<http::string_body> FileHandler::handle_request(const http::request<http::string_body>& request) {
  BOOST_LOG_TRIVIAL(trace) << "Entered FileHandler handle_request.";
  
  http::response<http::string_body> response;

  switch (request.method()) {
  case http::verb::get: {
    std::ifstream ifile;
    
    size_t file_start_pos = request.target().find("/", 1);
    if ((file_start_pos == request.target().size() - 1) || (file_start_pos == std::string::npos)) {
      BOOST_LOG_TRIVIAL(info) << "Need file path, file_start_pos = : " << file_start_pos;
      BOOST_LOG_TRIVIAL(info) << "request.target().size() = : " << request.target().size();
      
      response.result(404);
      response.set(http::field::content_type, "text/html");
      response.body() = "<h1>404 Not Found</h1>";
      response.prepare_payload();
      return response;
    }

    std::string file(request.target().substr(file_start_pos, std::string::npos).data(), request.target().size() - file_start_pos);
    
    std::string path(root + file, 0);
    
    BOOST_LOG_TRIVIAL(trace) << "Opening file: " << path;
    
    ifile.open(path);
    if (ifile) {
      ifile.close();
    }
    else {
      BOOST_LOG_TRIVIAL(info) << "Failed to open the file: " << path;
      
      response.result(404);
      response.set(http::field::content_type, "text/html");
      response.body() = "<h1>404 Not Found</h1>";
      response.prepare_payload();
      return response;
    }
    
    std::ifstream in(path, std::ios::binary);
    std::string payload((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    BOOST_LOG_TRIVIAL(trace) << "Response payload length: "
                             << std::to_string(payload.size());
    
    response.result(200);
    response.content_length(payload.size());
    response.set(http::field::content_type, mime_type(request));
    response.body() = payload;

    BOOST_LOG_TRIVIAL(trace) << "Mime type found: " << mime_type(request);
    
    break;
  }
  default:
    BOOST_LOG_TRIVIAL(trace) << "Unexpected request type: "
                             << request.method_string();
    break;
  }

  return response;
}

const char* FileHandler::mime_type(const http::request<http::string_body>& request) {
  std::string target(request.target().data(), request.target().size());

  size_t pos = target.find(".");
  if (pos == std::string::npos) {
    BOOST_LOG_TRIVIAL(warning) << "No extension found in the request target.";
    return "";
  }

  if (target.find(".html") != std::string::npos) return "text/html";
  if (target.find(".txt") != std::string::npos) return "text/plain";
  if (target.find(".jpg") != std::string::npos ||
      target.find(".jpeg") != std::string::npos) return "image/jpeg";
  if (target.find(".zip") != std::string::npos) return "application/zip";

  BOOST_LOG_TRIVIAL(trace) << "Unsupported file type requested: " << target;
  
  return "unsupported";
}
