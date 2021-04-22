#include "file_handler.h"

FileHandler::FileHandler(const NginxConfig &config, const std::string &prefix) {

    this->prefix = prefix;
    root = "/";
    for (auto statement : config.statements_) {
      if (statement->tokens_[0] == "root" && statement->tokens_.size() == 2) {
        root = statement ->tokens_[1];
      }
    }
}

std::string FileHandler::generate_response(const Request& request) {
  // TODO(!): Get contents of the file requested, and set the HTTP content type
  //          based on the request type. Send a 404 response code if the file
  //          does not exist.
  
  //placeholder for now
  std::string path = request.path;
  return path;
}
