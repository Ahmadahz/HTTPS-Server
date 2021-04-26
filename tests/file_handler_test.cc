#include "gtest/gtest.h"
#include "echo_handler.h"
#include "file_handler.h"
#include "file_handler.h"
#include "handler.h"

class FileHandlerTest : public ::testing::Test {
protected:
  std::string response_404 = "HTTP/1.1 404 Not Found\nContent-Length: 22\nContent-Type: text/html\n\n<h1>404 Not Found</h1>";
  std::string path_to_echo = "/echo/";
};

TEST_F(FileHandlerTest, mime_type_check){
  std::string path_to_ext1 = "/static/test.txt";
  std::string path_to_ext2 = "/static/test.html";
  std::string path_to_ext3 = "/static/test.zip";
  std::string path_to_ext4 = "/static/test.jpg";
  
  Request request_ext1 = RequestHandler::parse_request(path_to_ext1.c_str());
  Request request_ext2 = RequestHandler::parse_request(path_to_ext2.c_str());
  Request request_ext3 = RequestHandler::parse_request(path_to_ext3.c_str());
  Request request_ext4 = RequestHandler::parse_request(path_to_ext4.c_str());
  
  EXPECT_EQ(request_ext1.ext, ExtType::TXT);
  EXPECT_EQ(request_ext2.ext, ExtType::HTML);
  EXPECT_EQ(request_ext3.ext, ExtType::ZIP);
  EXPECT_EQ(request_ext4.ext, ExtType::JPG);
}

TEST_F(FileHandlerTest, echo_check){
  EchoHandler eh_test;
  std::vector<char> test_echo_response;
  std::string tmeps = "    \nHTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n/echo/";
  std::vector<char> ehco_response(tmeps.begin(), tmeps.end());
  
  Request request_echo = RequestHandler::parse_request(path_to_echo.c_str());
  test_echo_response = eh_test.generate_response(request_echo);
  EXPECT_EQ(ehco_response, test_echo_response);
}

TEST_F(FileHandlerTest, wrong_mime_type){
  std::string error_path = "/static/test.gif";
  Request request_error = RequestHandler::parse_request(error_path.c_str());
  
  EXPECT_NE(request_error.ext, ExtType::TXT);
  EXPECT_NE(request_error.ext, ExtType::HTML);
  EXPECT_NE(request_error.ext, ExtType::ZIP);
  EXPECT_NE(request_error.ext, ExtType::JPG);
}

/*
TEST_F(FileHandlerTest, error_404_check){
  std::string error_response;
  std::string error_path = "/static/none.html";
  Request request_error = RequestHandler::parse_request(error_path.c_str());
  Dispatcher* dispatcher_;
  error_response = (auto temp_fh )->generate_response(request_error);
  EXPECT_EQ(error_response, response_404);
}
*/