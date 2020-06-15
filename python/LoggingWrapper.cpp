#include <string>

#include "../util/Logger.h"

#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/python.hpp>

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
            ss.str("");
            ss.clear();
            ss << line;
        }

        // Report any errors
        else if (ss.bad() || ss.fail()) {
            ErrorLogger() << "Logger stream from python experienced an error " << ss.rdstate();
            ss.str("");
            ss.clear();
        }
    }

    DeclareThreadSafeLogger(python);

    template<LogLevel log_level>
    void PythonLoggerWrapper(const std::string& msg, const std::string& filename,
                             const int lineno)
    {
        static std::stringstream log_stream("");
        auto logger_func = [&](const std::string& arg) {
            FO_LOGGER(log_level, python) << boost::log::add_value("SrcFilename", filename)
                                         << boost::log::add_value("SrcLinenum", lineno)
                                         << arg;
        };
        send_to_log(log_stream, msg, logger_func);
    }
}

namespace FreeOrionPython {
    using boost::python::def;
    void WrapLogger() {
        def("debug", PythonLoggerWrapper<LogLevel::debug>);
        def("info", PythonLoggerWrapper<LogLevel::info>);
        def("warn", PythonLoggerWrapper<LogLevel::warn>);
        def("error", PythonLoggerWrapper<LogLevel::error>);
        def("fatal", PythonLoggerWrapper<LogLevel::error>);
    }
}
