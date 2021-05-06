// An nginx config file parser.
//
// See:
//   http://wiki.nginx.org/Configuration
//   http://blog.martinfjordvald.com/2010/07/nginx-primer/
//
// How Nginx does it:
//   http://lxr.nginx.org/source/src/core/ngx_conf_file.c

#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include <vector>

#include "config_parser.h"

std::string NginxConfig::ToString(int depth) {
  std::string serialized_config;
  for (const auto& statement : statements_) {
    serialized_config.append(statement->ToString(depth));
  }
  return serialized_config;
}

void NginxConfig::Reset() {
  statements_.clear();
}

std::string NginxConfigStatement::ToString(int depth) {
  std::string serialized_statement;
  for (int i = 0; i < depth; ++i) {
    serialized_statement.append("  ");
  }
  for (unsigned int i = 0; i < tokens_.size(); ++i) {
    if (i != 0) {
      serialized_statement.append(" ");
    }
    serialized_statement.append(tokens_[i]);
  }
  if (child_block_.get() != nullptr) {
    serialized_statement.append(" {\n");
    serialized_statement.append(child_block_->ToString(depth + 1));
    for (int i = 0; i < depth; ++i) {
      serialized_statement.append("  ");
    }
    serialized_statement.append("}");
  } 
  else {
    serialized_statement.append(";");
  }
  serialized_statement.append("\n");
  return serialized_statement;
}

const char* NginxConfigParser::TokenTypeAsString(TokenType type) {
  switch (type) {
    case TOKEN_TYPE_START:         return "TOKEN_TYPE_START";
    case TOKEN_TYPE_NORMAL:        return "TOKEN_TYPE_NORMAL";
    case TOKEN_TYPE_START_BLOCK:   return "TOKEN_TYPE_START_BLOCK";
    case TOKEN_TYPE_END_BLOCK:     return "TOKEN_TYPE_END_BLOCK";
    case TOKEN_TYPE_COMMENT:       return "TOKEN_TYPE_COMMENT";
    case TOKEN_TYPE_STATEMENT_END: return "TOKEN_TYPE_STATEMENT_END";
    case TOKEN_TYPE_EOF:           return "TOKEN_TYPE_EOF";
    case TOKEN_TYPE_ERROR:         return "TOKEN_TYPE_ERROR";
    default:                       return "Unknown token type";
  }
}

NginxConfigParser::TokenType NginxConfigParser::ParseToken(std::istream* input,
                                                           std::string* value) {
  TokenParserState state = TOKEN_STATE_INITIAL_WHITESPACE;
  while (input->good()) {
    const char c = input->get();
    if (!input->good() || input->fail()) {
      break;
    }
    switch (state) {
      case TOKEN_STATE_INITIAL_WHITESPACE:
        switch (c) {
          case '{':
            *value = c;
            return TOKEN_TYPE_START_BLOCK;
          case '}':
            *value = c;
            return TOKEN_TYPE_END_BLOCK;
          case '#':
            *value = c;
            state = TOKEN_STATE_TOKEN_TYPE_COMMENT;
            continue;
          case '"':
            *value = c;
            state = TOKEN_STATE_DOUBLE_QUOTE;
            continue;
          case '\'':
            *value = c;
            state = TOKEN_STATE_SINGLE_QUOTE;
            continue;
          case ';':
            *value = c;
            return TOKEN_TYPE_STATEMENT_END;
          case ' ':
          case '\t':
          case '\n':
          case '\r':
            continue;
          default:
        if (c == '\\') {
          const char ch = input->get();
          if (!input->good() || input->fail()) {
          break;
          }
          if (ch == ' ' || ch == '\t' || ch == ';' ||
              ch == '{' || ch == '}' || ch == '#') {
            *value += ch;
          } else {
            *value += c;
            input->unget();
          }
          continue; 
        }
            *value += c;
            state = TOKEN_STATE_TOKEN_TYPE_NORMAL;
            continue;
        }
      case TOKEN_STATE_SINGLE_QUOTE:
        // TODO: Maybe also define a QUOTED_STRING token type.
	if (c == '\\') {
	  const char ch = input->get();
	  if (!input->good() || input->fail()) {
	    break;
	  }
	  if (ch == '\'') {
	    *value += ch;
	  } else {
	    *value += c;
	    input->unget();
	  }
	  continue; 
	}
	*value += c;
	if (c == '\'') {
	  const char ch = input->get();
	  if (!input->good() || input->fail()) {
	    break;
	  }
	  if (!(ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' ||
		ch == ';' || ch == '{')) {
	    return TOKEN_TYPE_ERROR;
	  }
	  input->unget();
          return TOKEN_TYPE_NORMAL;
	}
        continue;
      case TOKEN_STATE_DOUBLE_QUOTE:
	if (c == '\\') {
	  const char ch = input->get();
	  if (!input->good() || input->fail()) {
	    break;
	  }
	  if (ch == '\"') {
	    *value += ch;
	  } else {
	    *value += c;
	    input->unget();
	  }
	  continue; 
	}
	*value += c;
	if (c == '"') {
	  const char ch = input->get();
	  if (!input->good() || input->fail()) {
	    break;
	  }
	  if (!(ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' ||
		ch == ';' || ch == '{')) {
	    return TOKEN_TYPE_ERROR;
	  }
	  input->unget();
          return TOKEN_TYPE_NORMAL;
        }
        continue;
      case TOKEN_STATE_TOKEN_TYPE_COMMENT:
        if (c == '\n' || c == '\r') {
          return TOKEN_TYPE_COMMENT;
        }
        *value += c;
        continue;
      case TOKEN_STATE_TOKEN_TYPE_NORMAL:
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r' ||
            c == ';' || c == '{' || c == '}') {
          input->unget();
          return TOKEN_TYPE_NORMAL;
        }
	if (c == '\\') {
	  const char ch = input->get();
	  if (!input->good() || input->fail()) {
	    break;
	  }
	  if (ch == ' ' || ch == '\t' || ch == ';' ||
	      ch == '{' || ch == '}' || ch == '#') {
	    *value += ch;
	  } else {
	    *value += c;
	    input->unget();
	  }
	  continue; 
	}
        *value += c;
        continue;
    }
  }

  // If we get here, we reached the end of the file.
  if (state == TOKEN_STATE_SINGLE_QUOTE ||
      state == TOKEN_STATE_DOUBLE_QUOTE) {
    return TOKEN_TYPE_ERROR;
  }

  return TOKEN_TYPE_EOF;
}

bool NginxConfigParser::Parse(std::istream* config_file, NginxConfig* config) {
  std::stack<NginxConfig*> config_stack;
  config_stack.push(config);
  TokenType last_token_type = TOKEN_TYPE_START;
  TokenType token_type;
  std::stack<TokenType> bracket_stack;
  while (true) {
    std::string token;
    token_type = ParseToken(config_file, &token);
    printf ("%s: %s\n", TokenTypeAsString(token_type), token.c_str());
    if (token_type == TOKEN_TYPE_ERROR) {
      break;
    }

    if (token_type == TOKEN_TYPE_COMMENT) {
      // Skip comments.
      continue;
    }

    if (token_type == TOKEN_TYPE_START) {
      // Error.
      break;
    } else if (token_type == TOKEN_TYPE_NORMAL) {
      if (last_token_type == TOKEN_TYPE_START ||
          last_token_type == TOKEN_TYPE_STATEMENT_END ||
          last_token_type == TOKEN_TYPE_START_BLOCK ||
          last_token_type == TOKEN_TYPE_END_BLOCK ||
          last_token_type == TOKEN_TYPE_NORMAL) {
        if (last_token_type != TOKEN_TYPE_NORMAL) {
          config_stack.top()->statements_.emplace_back(
              new NginxConfigStatement);
        }
        config_stack.top()->statements_.back().get()->tokens_.push_back(
            token);
      } else {
        // Error.
        break;
      }
    } else if (token_type == TOKEN_TYPE_STATEMENT_END) {
      if (last_token_type != TOKEN_TYPE_NORMAL) {
        // Error.
        break;
      }
    } else if (token_type == TOKEN_TYPE_START_BLOCK) {
      if (last_token_type != TOKEN_TYPE_NORMAL) {
        // Error.
        break;
      }
      NginxConfig* const new_config = new NginxConfig;
      config_stack.top()->statements_.back().get()->child_block_.reset(
          new_config);
      config_stack.push(new_config);

      bracket_stack.push(token_type);
    } else if (token_type == TOKEN_TYPE_END_BLOCK) {
      if (last_token_type != TOKEN_TYPE_STATEMENT_END &&
	  last_token_type != TOKEN_TYPE_END_BLOCK) {
        // Error.
        break;
      }
      config_stack.pop();
      
      if (bracket_stack.empty()) {
	// Error.
	break;
      }
      bracket_stack.pop();
    } else if (token_type == TOKEN_TYPE_QUOTED_STRING) {
      ;
    } else if (token_type == TOKEN_TYPE_EOF) {
      if (last_token_type != TOKEN_TYPE_STATEMENT_END &&
          last_token_type != TOKEN_TYPE_END_BLOCK &&
	  last_token_type != TOKEN_TYPE_START) {
        // Error.
        break;
      }
      if (!bracket_stack.empty()) {
	// Error.
	break;
      }
      return true;
    } else {
      // Error. Unknown token.
      break;
    }
    last_token_type = token_type;
  }

  printf ("Bad transition from %s to %s\n",
          TokenTypeAsString(last_token_type),
          TokenTypeAsString(token_type));
  return false;
}

bool NginxConfigParser::Parse(const char* file_name, NginxConfig* config) {
  std::ifstream config_file;
  config_file.open(file_name);
  if (!config_file.good()) {
    printf ("Failed to open config file: %s\n", file_name);
    return false;
  }

  const bool return_value =
      Parse(dynamic_cast<std::istream*>(&config_file), config);
  config_file.close();
  return return_value;
}

bool NginxConfigParser::GetPortNumber(const NginxConfig& out_config, short& port_number) {
  std::vector<std::shared_ptr<NginxConfigStatement>> statements;
  statements = out_config.statements_;

  // Searches each statement found by the parser for the port number.
  // A config statement is either a single line of tokens or an entire code block.
  for (int i = 0; i < statements.size(); ++i) {

    // Assumes that the statement containing the port number is of the form:
    // http {
    //    ...
    //    listen  XX;
    // }
    // where `http' is the token (hence size being 1).
    // The token size check is to ensure that the config file is properly formed
    // in a way that the config parser doesn't currently check for.
    std::string port_statement = statements[i]->ToString(0);
    size_t port_pos = port_statement.find("listen");

    if (port_pos != std::string::npos) {
      port_pos += 6; // Go to position after the end of "listen"

      // Ignore whitespace after "listen" is found in search of start and end
      // positions of the port number.
      size_t port_start_pos = port_statement.find_first_not_of(" \t\n\r", port_pos);
      size_t port_end_pos = port_statement.find_first_of(" \t;",port_start_pos);

      // No port number found between `listen' and `;'.
      if (port_end_pos == port_start_pos) {
        std::cerr << "No port number provided." << std::endl;
        return false;
      }

      std::string port = port_statement.substr(port_start_pos,
          port_end_pos - port_start_pos);
        
      if (port.find_first_not_of("0123456789") != std::string::npos) {
        std::cerr << "Port number must be a postive integer. Port given: " + \
            port << std::endl;
            return false;
          }

     int int_port = stoi(port);
      if (int_port < 0 || int_port > 65535) {
        std::cerr << "Port numbers must be between 0 and 65535." << std::endl;
        return false;
      }

      port_number = int_port;

      return true;
    }
    else {
      std::cerr << "\"listen\" token not found." << std::endl;
      return false;
    }
  }
  return false;
}
