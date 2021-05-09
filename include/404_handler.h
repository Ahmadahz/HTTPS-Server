#ifndef _404_HANDLER_H_
#define _404_HANDLER_H_

#include "handler.h"

class _404Handler : public RequestHandler {
public:
  _404Handler();
  
  std::vector<char> generate_response(const Request& request) override;
  
private:
  
};

#endif