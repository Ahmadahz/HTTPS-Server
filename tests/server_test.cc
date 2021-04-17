#include "gtest/gtest.h"
#include "server.h"
#include "config_parser.h"
#include "gmock/gmock.h"  // Brings in gMock.

// Not actually used here, need templates
class MockSession : public session {
 public:
  MockSession(boost::asio::io_service& io_service) 
  : session(io_service) {
  }
  MOCK_METHOD(void, handle_read, 
    (const boost::system::error_code& error, size_t bytes_transferred));
  MOCK_METHOD(void, start, ());
};

class ServerTest : public ::testing::Test {
protected:
  boost::asio::io_service io_service0;
  const boost::system::error_code err = make_error_code(boost::system::errc::success);
};

using ::testing::AtLeast; 
using ::testing::_; 

// Just tests that session is created and that with that created session handle_accept() can be called
TEST_F(ServerTest, SessionCreationAndStartTest) {
  server test_server(io_service0, 8080);
  session* test_session = test_server.new_session;
  test_server.handle_accept(test_session, err);
  EXPECT_TRUE(true);
}