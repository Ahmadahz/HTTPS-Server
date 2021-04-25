#include "gtest/gtest.h"
#include "server.h"
#include "config_parser.h"
#include "gmock/gmock.h"  // Brings in gMock.

// Not actually used here, need templates
class MockSession : public session {
 public:
  MockSession(boost::asio::io_service& io_service) 
  : session(io_service, nullptr) {
  }
  
  MOCK_METHOD(void, handle_read, 
    (const boost::system::error_code& error, size_t bytes_transferred));
  MOCK_METHOD(void, start, ());
};

class ServerTest : public ::testing::Test {
protected:
  boost::asio::io_service io_service0;
  const boost::system::error_code err = make_error_code(boost::system::errc::success);

  NginxConfig out_config_;
  Dispatcher* dispatcher;
  session* create_session() {
    return new session(io_service0, dispatcher);
  }
};

using ::testing::AtLeast; 
using ::testing::_; 

// Tests that a session is created and that handle_accept() can be called
TEST_F(ServerTest, BasicSessionCreation) {
  server test_server(io_service0, 8080, out_config_);
}

TEST_F(ServerTest, HandleAcceptSuccess) {
  session* test_session = create_session();
  server test_server(*test_session, io_service0, 8080, out_config_);
  test_server.handle_accept(test_session, err);
  delete test_session;
}

TEST_F(ServerTest, HandleAcceptFailure) {
  session* test_session = create_session();
  server test_server(*test_session, io_service0, 8080, out_config_);

  boost::system::error_code fail_error = make_error_code(boost::system::errc::connection_reset);
  test_server.handle_accept(test_session, fail_error);
}
