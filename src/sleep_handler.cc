#include "sleep_handler.h"
#include <boost/log/trivial.hpp>
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>

SleepHandler::SleepHandler(const std::string& location_path, const NginxConfig& config) : RequestHandler() {
  
}

http::response<http::string_body> SleepHandler::handle_request(const http::request<http::string_body>& request) {
  BOOST_LOG_TRIVIAL(trace) << "Entered SleepHandler::handle_request.";
  
  http::response<http::string_body> response;
  std::string payload = "I slept for 5 seconds!";
  response.result(200);
  response.set(http::field::content_type, "text/plain");
  response.body() = payload;
  response.prepare_payload();

  boost::this_thread::sleep_for(boost::chrono::milliseconds(5000));
  
  return response;
}
