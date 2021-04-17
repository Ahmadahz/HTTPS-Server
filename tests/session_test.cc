#include "gtest/gtest.h"
#include "session.h"

class SessionTest : public ::testing::Test {
protected:
  const boost::system::error_code err = make_error_code(boost::system::errc::success);
  
  std::string request;
  boost::asio::io_service io_service0;
};

TEST_F(SessionTest, ReadAndDetectEndTest) {
  session test_session = session(io_service0);
  test_session.start();

  request += "GET Request\r\n";
  test_session.fill_data_with(request);
  test_session.handle_read(err, request.size());

  char expected[14] = "GET Request\r\n";
  EXPECT_EQ(strcmp(test_session.get_data(), expected), 0);


  request += "Body\r\n\r\n";
  test_session.fill_data_with(request);
  test_session.handle_read(err, request.size());

  char expected2[72] = "    \nHTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nGET Request\r\nBody\r\n\r\n";
  EXPECT_EQ(strcmp(test_session.get_data(), expected2), 0);
}

TEST_F(SessionTest, DeleteCloseOnWriteTest) {
  session test_session = session(io_service0);
  test_session.start();

  request += "GET Request\r\n";
  test_session.fill_data_with(request);
  test_session.handle_read(err, request.size());

  char expected[14] = "GET Request\r\n";
  EXPECT_EQ(strcmp(test_session.get_data(), expected), 0);

  test_session.handle_write(err);
  char expected2[1] = "";
  EXPECT_EQ(strcmp(test_session.get_data(), expected2), 0);
}
