// #include "gtest/gtest.h"
// #include "server.h"
// #include "config_parser.h"
// #include "gmock/gmock.h"  // Brings in gMock.

// // // Not actually used here, need templates
// // class MockSession : public session {
// //  public:
// //   MockSession(boost::asio::io_service& io_service, boost::asio::ssl::context& context) 
// //   : session(io_service, context, nullptr) {
// //   }
  
// //   MOCK_METHOD(void, handle_read, 
// //     (const boost::system::error_code& error, size_t bytes_transferred));
// //   MOCK_METHOD(void, start, ());
// // };

// class ServerTest : public ::testing::Test {
// protected:
//   boost::asio::io_service io_service0;
//   boost::asio::ssl::context context0;
//   const boost::system::error_code err = make_error_code(boost::system::errc::success);

//   NginxConfig out_config_;
//   Dispatcher* dispatcher;

//   ServerTest() 
//     : context0(boost::asio::ssl::context::sslv23) {
//   }
//   session* create_session() {
//     context0.set_options(
//         boost::asio::ssl::context::default_workarounds
//         | boost::asio::ssl::context::no_sslv2
//         | boost::asio::ssl::context::single_dh_use);
//     context0.set_password_callback(boost::bind(&server::get_password, this));
//     context0.use_certificate_chain_file("./keys/localhost.pem");
//     context0.use_private_key_file("./keys/localhost-key.pem", boost::asio::ssl::context::pem);
//     context0.use_tmp_dh_file("./keys/dhparam4096.pem");
//     return new session(io_service0, context0, dispatcher);
//   }
// };

// using ::testing::AtLeast; 
// using ::testing::_; 

// // Tests that a session is created and that handle_accept() can be called
// TEST_F(ServerTest, BasicSessionCreation) {
//   server test_server(io_service0, 8080, out_config_);
// }

// // TEST_F(ServerTest, HandleAcceptSuccess) {
// //   session* test_session = create_session();
// //   server test_server(*test_session, io_service0, 8080, out_config_);
// //   test_server.handle_accept(test_session, err);
// // }

// TEST_F(ServerTest, HandleAcceptFailure) {
//   session* test_session = create_session();
//   server test_server(*test_session, io_service0, 8080, out_config_);

//   boost::system::error_code fail_error = make_error_code(boost::system::errc::connection_reset);
//   test_server.handle_accept(test_session, fail_error);
// }
