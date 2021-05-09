#include "gtest/gtest.h"
#include "session.h"

class SessionTest : public ::testing::Test {
protected:
  const boost::system::error_code success_code = make_error_code(boost::system::errc::success);
  const boost::system::error_code fail_code = make_error_code(boost::system::errc::connection_reset);
  
  std::string request;
  boost::asio::io_service io_service0;
};

// TEST_F(SessionTest, ReadAndDetectEndTest) {
//   session test_session = session(io_service0, nullptr);
//   test_session.start();

//   request = "GET Request\n";
//   test_session.fill_data_with(request);
//   test_session.handle_read(success_code, request.size());

//   char expected[13] = "GET Request\n";
//   EXPECT_STREQ(test_session.get_data(), expected);

//   request = "GET Request\nBody\n\n";
//   test_session.fill_data_with(request);
//   test_session.handle_read(success_code, request.size());

//   std::string received = test_session.get_data();
//   EXPECT_TRUE(received.find("HTTP") != std::string::npos);
// }


TEST_F(SessionTest, DeleteCloseOnWriteTest) {
  session test_session = session(io_service0, nullptr);
  test_session.start();

  request = "GET Request\r\n";
  test_session.fill_data_with(request);
  test_session.handle_write(success_code);
  
  char expected2[1] = "";
  EXPECT_STREQ(test_session.get_data(), expected2);
}

TEST_F(SessionTest, HandleReadError) {
  session *test_session = new session(io_service0, nullptr);
  test_session->start();

  std::string request = "Data.";
  test_session->fill_data_with(request);
  test_session->handle_read(fail_code, request.size());
}

TEST_F(SessionTest, HandleWriteError) {
  session *test_session = new session(io_service0, nullptr);
  test_session->start();

  test_session->handle_write(fail_code);
}

TEST_F(SessionTest, AppendData) {
  session test_session = session(io_service0, nullptr);
  test_session.start();

  request = "This request";
  test_session.fill_data_with(request);
  test_session.handle_read(success_code, request.size());

  EXPECT_STREQ(request.c_str(), test_session.get_data());
}

TEST_F(SessionTest, CallSocket) {
  session test_session = session(io_service0, nullptr);
  test_session.start();

  EXPECT_FALSE(test_session.socket().is_open());
}
