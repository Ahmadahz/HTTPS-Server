#ifndef _ECHO_HANDLER_H_
#define _ECHO_HANDLER_H_

#include "handler.h"

class EchoHandler : public RequestHandler {
public:
  EchoHandler();
  
  std::string generate_response(const Request& request) override;
  
private:
  
};

#endif
