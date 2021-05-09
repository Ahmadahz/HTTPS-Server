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

FileHandler::FileHandler(const NginxConfig &config, const std::string &prefix) {
  this->prefix = prefix;
  root = "/";
  for (auto statement : config.statements_) {
    if (statement->tokens_[0] == "root" && statement->tokens_.size() == 2) {
      root = statement ->tokens_[1];
    }
  }

  BOOST_LOG_TRIVIAL(trace) << "root is: " << root << std::endl;
}

std::vector<char> FileHandler::generate_response(const Request& request) {
  std::string response;
  std::vector<char> res;

  // Check the existence of the file 
  std::ifstream ifile;
  BOOST_LOG_TRIVIAL(trace) << "filehandler servicing: " << request.path;
  std::string path = root + "/" + request.file;
  BOOST_LOG_TRIVIAL(trace) << "pulling file from: " << path;
  ifile.open(path);
  if (ifile) {
    ifile.close();
    // Set the OK response
    std::string ok200("HTTP/1.1 200 OK\n");
    response = response + ok200;

    // Set the content type
    if (request.ext == ExtType::HTML) {
      std::string htmlR("Content-Type: text/html\n");
      response = response + htmlR;  
    }
    else if (request.ext == ExtType::JPG) {
      std::string jpgR("Content-Type: image/jpeg\n");
      response = response + jpgR;
    }
    else if (request.ext == ExtType::ZIP) {
      std::string zipR("Content-Type: application/zip\n");
      response = response + zipR;
    }
    else if (request.ext == ExtType::TXT) {
      std::string txtR("Content-Type: text/plain\n");
      response = response + txtR;  
    }

    // Set the length
    std::string lengthR("Content-Length: ");
    std::ifstream in_file(path, std::ios::binary);
    in_file.seekg(0, std::ios::end);
    int file_size = in_file.tellg();
    in_file.seekg(0, std::ios::beg);
    std::string length_num = std::to_string(file_size);
    lengthR += length_num + "\n\n";
    response += lengthR;
    in_file.close();
    std::copy(response.begin(), response.end(), std::back_inserter(res));

    // Get the content of the file
    std::ifstream in(path, std::ios::binary);
    std::vector<char> contents = std::vector<char>(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    res.insert(res.end(), contents.begin(), contents.end());
    in.close();

    /* The below is used in case we switch back to using strings instead
       of vector<char> */
    //std::string file_contents((std::istreambuf_iterator<char>(content_file)), std::istreambuf_iterator<char>());
    //response = response + contents;

    BOOST_LOG_TRIVIAL(trace) << "Actual response length: " << std::to_string(res.size());

    } else {
      // 404 error
      BOOST_LOG_TRIVIAL(trace) << "404 from filehandler";
      response = "HTTP/1.1 404 Not Found\nContent-Length: 22\nContent-Type: text/html\n\n<h1>404 Not Found</h1>";
      std::copy(response.begin(), response.end(), std::back_inserter(res));
    }

  return res;
}

http::response<http::string_body> FileHandler::handle_request(const http::request<http::string_body>& request) {
  BOOST_LOG_TRIVIAL(trace) << "Entered FileHandler handle_request.";
  
  http::response<http::string_body> response;

  switch (request.method()) {
  case http::verb::get: {
    std::ifstream ifile;
    
    size_t file_start_pos = request.target().find("/", 1);
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

  return "";
}
