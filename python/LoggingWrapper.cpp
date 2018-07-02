#include <string>

#include "../util/Logger.h"

#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/python.hpp>

namespace expr = boost::log::expressions;

namespace {
    // Expose interface for redirecting standard output and error to FreeOrion
    // logging.  Can be imported before loading the main FreeOrion AI interface
    // library.
    static const std::size_t MAX_SINGLE_CHUNK_TEXT_SIZE = 4096;

    /** Python streams text as strings which need to be concatenated before
        they are output to the logger.  Each \p input may end with an
        incomplete line that is continued in the next \p input from the python
        executable.  \p ss should be a persistent stringstream per output sink
        to persist the incomplete line of text. \p is a logger associated with
        the sink that \p ss represents.

*/
    void send_to_log(std::stringstream& ss, const std::string& input, const std::function<void(const std::string&)>& logger) {
        if (input.empty()) return;
        ss <<  ((input.size() < MAX_SINGLE_CHUNK_TEXT_SIZE) ? input : input.substr(0, MAX_SINGLE_CHUNK_TEXT_SIZE));
        std::string line;

        // Grab all complete lines of text.
        std::getline(ss, line);
        while (ss.good()) {
            logger(line);
            std::getline(ss, line);
        }

        // If ss is good, store any partial line of text for the next call to send_to_log.
        if (ss.eof()) {
            ss.clear();
            ss << line;
        }

        // Report any errors
        else if (ss.bad() || ss.fail()) {
            ErrorLogger() << "Logger stream from python experienced an error " << ss.rdstate();
            ss.clear();
        }
    }

    void ConfigurePythonFileSinkFrontEnd(LoggerTextFileSinkFrontend& sink_frontend, const std::string& channel_name) {
        // Create the format
        sink_frontend.set_formatter(
            expr::stream
            << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%H:%M:%S.%f")
            << " [" << log_severity << "] "
            << expr::message
        );

        // Set a filter to only format this channel
        sink_frontend.set_filter(log_channel == channel_name);
    }

    // Setup file sink, formatting, and \p name channel filter for \p logger.
    void ConfigurePythonLogger(NamedThreadedLogger& logger, const std::string& name) {
        if (name.empty())
            return;

        SetLoggerThreshold(name, default_log_level_threshold);

        ApplyConfigurationToFileSinkFrontEnd(
            name,
            std::bind(ConfigurePythonFileSinkFrontEnd, std::placeholders::_1, name));

        LoggerCreatedSignal(name);
    }

    // Place in source file to create the previously defined global logger \p name
#define DeclareThreadSafePythonLogger(name)                       \
    BOOST_LOG_INLINE_GLOBAL_LOGGER_INIT(                          \
        FO_GLOBAL_LOGGER_NAME(name), NamedThreadedLogger)         \
    {                                                             \
        auto lg = NamedThreadedLogger(                            \
            (boost::log::keywords::severity = LogLevel::debug),   \
            (boost::log::keywords::channel = #name));             \
        ConfigurePythonLogger(lg, #name);                         \
        return lg;                                                \
    }

    DeclareThreadSafePythonLogger(python);

    // Assemble a python message that is the same as the C++ message format.
    void PythonLogger(const std::string& msg,
                      const LogLevel log_level,
                      const std::string& python_logger,
                      const std::string& filename,
                      // const std::string& function_name,
                      const std::string& lineno)
    {
        // Assembling the log in the stream input to the logger means that the
        // string assembly is gated by the log level.  logs are not assembled
        // if that log level is disabled.
        switch (log_level) {
        case LogLevel::trace:
            TraceLogger(python) << python_logger << " : " << filename << ":" /*<< function_name << ":"*/ << lineno << " : " << msg;
            break;
        case LogLevel::debug:
            DebugLogger(python) << python_logger << " : " << filename << ":" /*<< function_name << ":"*/ << lineno << " : " << msg;
            break;
        case LogLevel::info:
            InfoLogger(python)  << python_logger << " : " << filename << ":" /*<< function_name << ":"*/ << lineno << " : " << msg;
            break;
        case LogLevel::warn:
            WarnLogger(python)  << python_logger << " : " << filename << ":" /*<< function_name << ":"*/ << lineno << " : " << msg;
            break;
        case LogLevel::error:
            ErrorLogger(python) << python_logger << " : " << filename << ":" /*<< function_name << ":"*/ << lineno << " : " << msg;
            break;
        }
    }

    void PythonLoggerWrapper(const LogLevel log_level, const std::string& msg,
                             const std::string& logger_name, const std::string& filename,
                             /*const std::string& function_name,*/ const std::string& lineno)
    {
        static std::stringstream log_stream("");
        send_to_log(log_stream, msg,
                    std::bind(&PythonLogger, std::placeholders::_1,
                              log_level, logger_name, filename, /*function_name,*/ lineno));
    }


    // debug/stdout logger
    void PythonLoggerDebug(const std::string& msg, const std::string& logger_name,
                           const std::string& filename, const std::string& function_name, const std::string& lineno)
    {
        PythonLoggerWrapper(LogLevel::debug, msg, logger_name, filename, /*function_name,*/ lineno);
    }


    // info logger
    void PythonLoggerInfo(const std::string& msg, const std::string& logger_name,
                          const std::string& filename, const std::string& function_name, const std::string& lineno)
    {
        PythonLoggerWrapper(LogLevel::info, msg, logger_name, filename, /*function_name,*/ lineno);
    }

    // warn logger
    void PythonLoggerWarn(const std::string& msg, const std::string& logger_name,
                          const std::string& filename, const std::string& function_name, const std::string& lineno)
    {
        PythonLoggerWrapper(LogLevel::warn, msg, logger_name, filename, /*function_name,*/ lineno);
    }

    // error logger
    void PythonLoggerError(const std::string& msg, const std::string& logger_name,
                           const std::string& filename, const std::string& function_name, const std::string& lineno)
    {
        PythonLoggerWrapper(LogLevel::error, msg, logger_name, filename, /*function_name,*/ lineno);
    }
}

namespace FreeOrionPython {
    using boost::python::def;
    void WrapLogger() {
        def("debug", PythonLoggerDebug);
        def("info", PythonLoggerInfo);
        def("warn", PythonLoggerWarn);
        def("error", PythonLoggerError);
        def("fatal", PythonLoggerError);
    }
}
