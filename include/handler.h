#ifndef _HANDLER_H_
#define _HANDLER_H_

#include <string>

enum class RequestType { File, Echo, Unknown };
enum class ExtType { HTML, JPG, ZIP, TXT };

struct Request {
  RequestType type;     // The type of request from the client.
  
  std::string content;  // The client's request in string form.
                        // Only used when type is Echo.
  
  ExtType ext;          // File extension type. Only used when type is File.
  std::string path;     // Path to the file specified by the client.
  std::string file;
};

class RequestHandler {
public:
  /*
   * This function takes in the data sent by the client to the server
   * and parses it to obtain the relevant information within it.
   *
   * @param request : The raw request received by the server.
   *
   * Returns a Request object to be used by a file handler.
  */
  static Request parse_request(const char* request);
  std::string get_path(const char* request);
  std::string get_location(const char* request);
  
  virtual std::string generate_response(const Request& request) = 0;
protected:
  RequestHandler() {};

  
private:
  
};

#endif
