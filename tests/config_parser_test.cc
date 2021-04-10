#include "gtest/gtest.h"
#include "config_parser.h"

class NginxParserTest : public ::testing::Test {
protected:
  NginxConfigParser parser_;
  NginxConfig out_config_;
  std::istringstream ss_;

  std::istream* str_to_istream(const std::string &str) {
    ss_.clear();
    ss_.str(str);
    return dynamic_cast<std::istream*>(&ss_);
  }

  bool parser_check(const std::string &str) {
    return parser_.Parse(str_to_istream(str), &out_config_);
  }
};

TEST_F(NginxParserTest, EmptyConfig) {
  EXPECT_TRUE(parser_check(""));
}

TEST_F(NginxParserTest, WhitespaceConfig) {
  EXPECT_TRUE(parser_check(" "));
  EXPECT_TRUE(parser_check("\t"));
  EXPECT_TRUE(parser_check("\n"));
  EXPECT_TRUE(parser_check("\r"));
  EXPECT_TRUE(parser_check(" \t   \r\n\n\r  \n\r\t"));
  EXPECT_FALSE(parser_check("  ;  \t;"));
}

TEST_F(NginxParserTest, NestedBracketConfig) {
  EXPECT_TRUE(parser_check("server{location /foo {root /var;} }"));
  EXPECT_FALSE(parser_check("server{bar;} } "));
  EXPECT_FALSE(parser_check("server{ foo {bar;}"));
}

TEST_F(NginxParserTest, QuoteEscapingConfig) {
  EXPECT_TRUE(parser_check("\"\\\"\";"));
  EXPECT_TRUE(parser_check("'\\\'';"));
  EXPECT_TRUE(parser_check("\"bar\";"));
  EXPECT_TRUE(parser_check("\"root\"{ location \"/foo\";}"));
  EXPECT_FALSE(parser_check("foo bar #;"));
  EXPECT_FALSE(parser_check("fizz \"buzz\"\\#;"));
  EXPECT_TRUE(parser_check("hot \"fuzz\\#\";"));
  EXPECT_TRUE(parser_check("location { /path\\ to\\ dir/;}"));
}

TEST_F(NginxParserTest, QuoteWhitespaceConfig) {
  EXPECT_TRUE(parser_check("'foo' bar;"));
  EXPECT_FALSE(parser_check("'foo'bar;"));
}
