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

    // Python sends text as several null-terminated array of char which need to be
    // concatenated before they are output to the logger.  There's probably a better
    // way to do this, but I don't know what it is, and this seems reasonably safe...
    void send_to_log(std::stringstream & ss, const std::string & input, void  (*logger) (const std::string &)) {
        if (input.empty()) return;
        ss <<  ((input.size() < MAX_SINGLE_CHUNK_TEXT_SIZE) ? input : input.substr(0, MAX_SINGLE_CHUNK_TEXT_SIZE));
        std::string line;
        std::getline(ss, line);
        while (ss.good()) {
            logger(line);
            std::getline(ss, line);
        }

        if (ss.eof()) {
            ss.clear();
            ss << line;
        } else if (ss.bad() || ss.fail()) {
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
            << channel_name
            << " : "
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


    // debug/stdout logger
    void PythonLoggerCoreDebug(const std::string &s) {
        DebugLogger(python) << s;
    }
    void PythonLoggerDebug(const std::string & text) {
        static std::stringstream log_stream("");
        send_to_log(log_stream, text, &PythonLoggerCoreDebug);
    }

    // info logger
    void PythonLoggerCoreInfo(const std::string &s) {
        // The extra space aligns info messages with debug messages.
        InfoLogger(python) << " " << s;
    }
    void PythonLoggerInfo(const std::string & text) {
        static std::stringstream log_stream("");
        send_to_log(log_stream, text, &PythonLoggerCoreInfo);
    }

    // warn logger
    void PythonLoggerCoreWarn(const std::string &s) {
        WarnLogger(python) << s;
    }
    void PythonLoggerWarn(const std::string & text) {
        static std::stringstream log_stream("");
        send_to_log(log_stream, text, &PythonLoggerCoreWarn);
    }

    // error logger
    void PythonLoggerCoreError(const std::string &s) {
        ErrorLogger(python) << s;
    }
    void PythonLoggerError(const std::string & text) {
        static std::stringstream log_stream("");
        send_to_log(log_stream, text, &PythonLoggerCoreError);
    }

    // critical logger
    void PythonLoggerCoreFatal(const std::string &s) {
        ErrorLogger(python) << s;
    }
    void PythonLoggerFatal(const std::string & text) {
        static std::stringstream log_stream("");
        send_to_log(log_stream, text, &PythonLoggerCoreFatal);
    }
}

namespace FreeOrionPython {
    using boost::python::def;
    void WrapLogger() {
        def("debug", PythonLoggerDebug);
        def("info", PythonLoggerInfo);
        def("warn", PythonLoggerWarn);
        def("error", PythonLoggerError);
        def("fatal", PythonLoggerFatal);
    }
}
