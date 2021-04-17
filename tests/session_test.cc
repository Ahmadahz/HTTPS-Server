#include "gtest/gtest.h"
#include "session.h"

class SessionTest : public ::testing::Test {
protected:
  const boost::system::error_code err = make_error_code(boost::system::errc::success);
  std::string request = "";
};

TEST_F(SessionTest, ReadAndDetectEndTest) {
  boost::asio::io_service io_service0;
  session test_session = session(io_service0);
  test_session.start();

  request += "GET Request\r\n";
  strncpy(test_session.data_, request.c_str(), request.size());
  test_session.handle_read(err, request.size());

  char expected[14] = "GET Request\r\n";
  EXPECT_EQ(strcmp(test_session.data_, expected), 0);


  request += "Body\r\n\r\n";
  strncpy(test_session.data_, request.c_str(), request.size());
  test_session.handle_read(err, request.size());

  char expected2[72] = "    \nHTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nGET Request\r\nBody\r\n\r\n";
  EXPECT_EQ(strcmp(test_session.data_, expected2), 0);
}
