#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/support/date_time.hpp>

#include "logger.h"

namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;

Logger::Logger() {
  init();
}

void Logger::init() {
  logging::register_simple_formatter_factory<logging::trivial::severity_level, char>("Severity");
  
  logging::add_file_log
  (
    keywords::file_name = "server_%N.log",
    keywords::rotation_size = 10 * 1024 * 1024,
    keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
    keywords::auto_flush = true,
    keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%]: %Message%"
  );

  boost::log::add_console_log
  (
    std::cerr,
    keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%]: %Message%"
  );

  boost::log::add_common_attributes();
}
