#include "handler.h"
#include <regex>

Request RequestHandler::parse_request(const char* request) {
  Request parsed_request;
  std::string req(request);
  parsed_request.content = req;

  //Find the Request Type
  std::string s1("static");
  std::string s2("echo");
  if (req.find(s1) != std::string::npos) {
    parsed_request.type = RequestType::File;
  }
  else if (req.find(s2) != std::string::npos) {
    parsed_request.type = RequestType::Echo;
  }
  else{
	parsed_request.type = RequestType::Unknown;
  }
  
  //Find the path
  if (parsed_request.type == RequestType::File){
	std::string file_path;
	for (std::string::size_type j = 4; j < req.size(); ++j) {
		if (req[j] == ' ') { 
			break;
		}
		else {
			file_path = file_path + req[j];
		}
	}
	
	//Is this path correct?!?!?! 
	file_path = std::regex_replace(file_path, std::regex("static"), "bar");
	parsed_request.path = file_path;
  }
  
  //Find the extension type
  if (parsed_request.type == RequestType::File){
	bool extensionset = false;
	std::string extension;
	for (std::string::size_type i = 5; i < req.size(); ++i) {
		if (req[i] == ' ') { 
			break;
		}
		else if (req[i] == '.') { 
			extensionset = true; 
		}
		else if (extensionset) {
			extension = extension + req[i];
		}
	}
	
	//Set the extension type
	if (extension.compare("html") == 0){
		parsed_request.ext = ExtType::HTML;
    }
	else if (extension.compare("jpg") == 0){
       parsed_request.ext = ExtType::JPG; 
    }
	else if (extension.compare("zip") == 0){
       parsed_request.ext = ExtType::ZIP; 
    }
	else if (extension.compare("txt") == 0){
       parsed_request.ext = ExtType::TXT;  
    }
  }
  
  return parsed_request;
}
