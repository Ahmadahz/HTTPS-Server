#include "handler.h"

Request RequestHandler::parse_request(const char* request) {
  Request parsed_request;
  std::string req(request);
  parsed_request.content = req;

  // Parse the request for type, file path, and, if applicable, extension type.
  

  return parsed_request;
}
