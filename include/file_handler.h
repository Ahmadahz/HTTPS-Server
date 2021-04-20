#ifndef _FILE_HANDLER_H_
#define _FILE_HANDLER_H_

#include "handler.h"

class FileHandler : public RequestHandler {
public:
  FileHandler();

  std::string generate_response(const Request& request) override;
  
private:
  
};

#endif
