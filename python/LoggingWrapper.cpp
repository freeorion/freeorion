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

    DeclareThreadSafeLogger(python);

    // Assemble a python message that is the same as the C++ message format.
    void PythonLogger(const std::string& msg,
                      const LogLevel log_level,
                      const std::string& python_logger,
                      const std::string& filename,
                      // const std::string& function_name,
                      const std::string& linenostr)
    {
        int lineno{0};

        try {
            lineno = std::stoi(linenostr);
        } catch(...)
        {}

        // Assembling the log in the stream input to the logger means that the
        // string assembly is gated by the log level.  logs are not assembled
        // if that log level is disabled.
        switch (log_level) {
        case LogLevel::trace:
            FO_LOGGER(python, LogLevel::trace) << python_logger << boost::log::add_value("SrcFilename", filename) << boost::log::add_value("SrcLinenum", lineno) << " : " << msg;
            break;
        case LogLevel::debug:
            FO_LOGGER(python, LogLevel::debug) << python_logger << boost::log::add_value("SrcFilename", filename) << boost::log::add_value("SrcLinenum", lineno) << " : " << msg;
            break;
        case LogLevel::info:
            FO_LOGGER(python, LogLevel::info)  << python_logger << boost::log::add_value("SrcFilename", filename) << boost::log::add_value("SrcLinenum", lineno) << " : " << msg;
            break;
        case LogLevel::warn:
            FO_LOGGER(python, LogLevel::warn)  << python_logger << boost::log::add_value("SrcFilename", filename) << boost::log::add_value("SrcLinenum", lineno) << " : " << msg;
            break;
        case LogLevel::error:
            FO_LOGGER(python, LogLevel::error) << python_logger << boost::log::add_value("SrcFilename", filename) << boost::log::add_value("SrcLinenum", lineno) << " : " << msg;
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
