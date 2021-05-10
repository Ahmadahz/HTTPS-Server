#include "gtest/gtest.h"
#include "dispatcher.h"

class DispatcherTest : public ::testing::Test {
protected:
  NginxConfig config_;
  NginxConfigParser parser_;
};

TEST_F(DispatcherTest, init_check) {
  parser_.Parse("one_valid", &config_);
  Dispatcher* dis = new Dispatcher(config_);
  EXPECT_EQ(dis -> get_regnum(), 2);
}

TEST_F(DispatcherTest, get_check) {
  parser_.Parse("one_valid", &config_);
  Dispatcher* dis = new Dispatcher(config_);
  EXPECT_NE(dis -> get_request_handler("/static/one"), nullptr);
}

// Tests that we have at least one handler since the 404 handler should have been initialized
TEST_F(DispatcherTest, null_check) {
  parser_.Parse("one_valid", &config_);
  Dispatcher* dis = new Dispatcher(config_);
  EXPECT_NE(dis -> get_request_handler("/statics/one"), nullptr);
}
