#include "handler.h"
#include <iostream>
#include <regex>
#include <boost/log/trivial.hpp>

Request RequestHandler::parse_request(const char* request) {
  Request parsed_request;
  std::string req(request);
  parsed_request.content = req;

  //Find the Request Type
  // This is actually not needed anymore. And just finding the url for a string might get confused if the 
  //   filename is one of the keywords
  //   Suggestion: remove RequestType, dispatcher does all of it. We just need to check for static
  //               so we can see whether we need to check for extension
  // POSSIBLY NEED CHANGE
  std::string s1("static");
  std::string s2("echo");
  if (req.find(s1) != std::string::npos) {
    parsed_request.type = RequestType::File;
  }
  else if (req.find(s2) != std::string::npos) {
    parsed_request.type = RequestType::Echo;
  }
  else{
	parsed_request.type = RequestType::_404;
  }
  // POSSIBLY NEED CHANGE
  
  //Find the path
  // if (parsed_request.type == RequestType::File){
  if (true) { // POSSIBLY JUST REMOVE
  	std::string file_path;
  	for (std::string::size_type j = 4; j < req.size(); ++j) {
  		if (req[j] == ' ') { 
  			break;
  		}
  		else {
  			file_path = file_path + req[j];
  		}
  	}
  	std::string file;
  	for( int i = file_path.length() - 1; i >= 0; i--) {	
  		if (file_path[i] == '/')
  			break;
  		file += file_path[i];
  	}
  	std::reverse(file.begin(), file.end());

  	// while(file_path.length() > 1 && file_path.back() != '/') {
  	// 	file = file_path.back();
      //     file_path.pop_back();
      // }
  	//Is this path correct?!?!?! 
  	// file_path = std::regex_replace(file_path, std::regex("static"), "bar");
  	std::cerr << "file is: "  << std::endl;
  	std::cerr << "file path: " << file_path << std::endl;
  	parsed_request.path = file_path;
  	parsed_request.file = file;
    BOOST_LOG_TRIVIAL(trace) << "In RequestHandler::parsed_request: " << "file is: "  << file << "file path: " << file_path << std::endl;
  }
  
  //Find the extension type
  if (parsed_request.type == RequestType::File){ // POSSIBLY ADD JUST CHECKING AND NOT REQUEST TYPE
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
	// for (int i = 0; i < parsed_request.file.size(); ++i) {
	// 	if (parsed_request.file[i] == ' ') { 
	// 		break;
	// 	}
	// 	else if (parsed_request.file[i] == '.') { 
	// 		extensionset = true; 
	// 	}
	// 	else if (extensionset) {
	// 		extension = extension + parsed_request.file[i];
	// 	}
	// }
	
	std::cerr << "extension: " << extension << std::endl;
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
