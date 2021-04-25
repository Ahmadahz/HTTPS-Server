#include "file_handler.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <streambuf>

FileHandler::FileHandler() {
  
}

std::string FileHandler::generate_response(const Request& request) {
  // TODO(!): Get contents of the file requested, and set the HTTP content type
  //          based on the request type. Send a 404 response code if the file
  //          does not exist.
  
  std::string response;

  //Check the existence of the file 
  std::ifstream ifile;
  
  ifile.open(request.path);
  if(ifile) {
	ifile.close();
	//Set the OK response
	std::string ok200("HTTP/1.1 200 OK\n");
	response = response + ok200;
  
	//Set the content type
	if (request.ext == ExtType::HTML){
		std::string htmlR("Content-Type: text/html\n");
		response = response + htmlR;  
	}
	else if (request.ext == ExtType::JPG){
		std::string jpgR("Content-Type: image/jpeg\n");
		response = response + jpgR;    
	}
	else if (request.ext == ExtType::ZIP){
		std::string zipR("Content-Type: application/zip\n");
		response = response + zipR;
	}
	else if (request.ext == ExtType::TXT){
		std::string txtR("Content-Type: text/plain\n");
		response = response + txtR;  
	}

	//Set the length
	std::string lengthR("Content-Length: ");
	std::ifstream in_file(request.path, std::ios::binary);
	in_file.seekg(0, std::ios::end);
	int file_size = in_file.tellg();
	in_file.seekg(0, std::ios::beg);
	std::string length_num = std::to_string(file_size);
	lengthR = lengthR + length_num;
	lengthR = lengthR + "\n\n";
	response = response + lengthR;
	in_file.close();
	
	//Get the content of the file
	std::ifstream content_file(request.path);
	std::string file_contents((std::istreambuf_iterator<char>(content_file)), std::istreambuf_iterator<char>());
	response = response + file_contents;
	

	} else {
	//404 error
		response = "HTTP/1.1 404 Not Found\nContent-Length: 22\nContent-Type: text/html\n\n<h1>404 Not Found</h1>";
	}
  
  return response;
}
