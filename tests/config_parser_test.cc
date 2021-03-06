#include "gtest/gtest.h"
#include "server.h"
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

  // Wrapper function for parsing files or manually inputted strings.
  bool parser_check(const std::string& str, bool is_filename = false) {
    out_config_.Reset();
    if (is_filename)
      return parser_.Parse(str.c_str(), &out_config_);
    else
      return parser_.Parse(str_to_istream(str), &out_config_);
  }

  bool key_check() {
    return parser_.GetKeyPath(out_config_);
  }
};

TEST_F(NginxParserTest, EmptyConfig) {
  EXPECT_TRUE(parser_check(""));
}

TEST_F(NginxParserTest, EmptyBlockConfig) {
  EXPECT_TRUE(parser_check("block { }"));
}

TEST_F(NginxParserTest, NonExistentFile) {
  EXPECT_FALSE(parser_check("bogus.config", true));
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
  EXPECT_TRUE(parser_check("\\\'\\\'';"));
  EXPECT_TRUE(parser_check("\"bar\";"));
  EXPECT_TRUE(parser_check("\"root\"{ location \"/foo\";}"));
  EXPECT_FALSE(parser_check("foo bar #;"));
  EXPECT_FALSE(parser_check("fizz \"buzz\"\\#;"));
  EXPECT_TRUE(parser_check("hot \"fuzz\\#\";"));
  EXPECT_TRUE(parser_check("location { /path\\ to\\ dir/;}"));
}

TEST_F(NginxParserTest, VariousEscapingConfig) {
  EXPECT_TRUE(parser_check("Fake_comment follows \\#;"));
  EXPECT_TRUE(parser_check("\\# This is not a comment;"));
  EXPECT_TRUE(parser_check("root 'file \\{';"));
  EXPECT_TRUE(parser_check("root \\path\\to\\file;"));
}

TEST_F(NginxParserTest, QuoteWhitespaceConfig) {
  EXPECT_TRUE(parser_check("'foo' bar;"));
  EXPECT_FALSE(parser_check("'foo'bar;"));
}

TEST_F(NginxParserTest, MalformedConfig) {
  EXPECT_FALSE(parser_check("http {{ listen 8080; }}"));
}

TEST_F(NginxParserTest, ParseKeys) {
  EXPECT_TRUE(parser_check("private_key_root privroot; public_key_root pubroot;"));
  EXPECT_TRUE(key_check());
  EXPECT_EQ(parser_.GetSSLPrivateKeyPath(), "privroot");
  EXPECT_EQ(parser_.GetSSLPublicKeyPath(), "pubroot");
}


class HttpEchoTest : public ::testing::Test {
protected:
  NginxConfigParser parser_;
  NginxConfig out_config_;
  std::istringstream ss_;
  short port_number_;

  // Allows for using strings as config_files by converting to istream.
  std::istream* str_to_istream(const std::string &str) {
    ss_.clear();
    ss_.str(str);
    return dynamic_cast<std::istream*>(&ss_);
  }

  // Wrapper function for parsing files or manually inputted strings.
  bool parse(const std::string& str, bool is_filename = false) {
    out_config_.Reset();
    if (is_filename)
      return parser_.Parse(str.c_str(), &out_config_);
    else
      return parser_.Parse(str_to_istream(str), &out_config_);
  }

  // Wrapper function for getting port numbers.
  // parse() or equivalent should have already been called prior to this.
  bool port_number() {
    return parser_.GetPortNumber(out_config_, "HTTP", port_number_);
  }
};

TEST_F(HttpEchoTest, BasicConfig) {
  EXPECT_TRUE(parse("../config/server_config", true));
  EXPECT_TRUE(port_number());
  EXPECT_EQ(port_number_, 80);
}

TEST_F(HttpEchoTest, NoPortConfig) {
  EXPECT_TRUE(parse("listen;"));
  EXPECT_FALSE(port_number());
}

TEST_F(HttpEchoTest, NoListenKeyword) {
  EXPECT_TRUE(parse("root;"));
  EXPECT_FALSE(port_number());
}

TEST_F(HttpEchoTest, NonDecimalPort) {
  EXPECT_TRUE(parse("listen number;"));
  EXPECT_FALSE(port_number());
  
  EXPECT_TRUE(parse("listen 2020g;"));
  EXPECT_FALSE(port_number());
}

TEST_F(HttpEchoTest, InvalidPortNumber) {
  EXPECT_TRUE(parse("listen 88888;"));
  EXPECT_FALSE(port_number());

  EXPECT_TRUE(parse("listen -1;"));
  EXPECT_FALSE(port_number());
}

TEST_F(HttpEchoTest, IgnoreComments) {
  EXPECT_TRUE(parse("http \r\n # Comment.\r\n { listen 1000; }"));
  EXPECT_TRUE(port_number());
}

TEST_F(HttpEchoTest, TrailingQuotes) {
  EXPECT_FALSE(parse("listen 80; '"));
  EXPECT_FALSE(parse("listen 80; \" "));
}

// Need to check what this does 
TEST_F(HttpEchoTest, EscapedBackslash) {
  EXPECT_TRUE(parse("http \\{ listen 80; \\}"));
}

TEST_F(HttpEchoTest, EchoBlock) {
  EXPECT_TRUE(parse("location /echo/ EchoHandler {empty /stmt;}"));
}

TEST_F(HttpEchoTest, StaticBlock) {
  EXPECT_TRUE(parse("location /static/ FileHandler {root /bar;}"));
}

TEST_F(HttpEchoTest, QuotedPath) {
  EXPECT_TRUE(parse("location \"/static/ FileHandler\" {root /bar;}"));
}
