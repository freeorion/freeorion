#include <string>

#include "../util/Logger.h"
#include <boost/log/trivial.hpp>

#include <boost/python.hpp>

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


    // debug/stdout logger
    void PythonLoggerCoreDebug(const std::string &s) {
        DebugLogger() << s;
    }
    void PythonLoggerDebug(const std::string & text) {
        static std::stringstream log_stream("");
        send_to_log(log_stream, text, &PythonLoggerCoreDebug);
    }

    // info logger
    void PythonLoggerCoreInfo(const std::string &s) {
        // The extra space aligns info messages with debug messages.
            InfoLogger() << " " << s;
    }
    void PythonLoggerInfo(const std::string & text) {
        static std::stringstream log_stream("");
        send_to_log(log_stream, text, &PythonLoggerCoreInfo);
    }

    // warn logger
    void PythonLoggerCoreWarn(const std::string &s) {
        WarnLogger() << s;
    }
    void PythonLoggerWarn(const std::string & text) {
        static std::stringstream log_stream("");
        send_to_log(log_stream, text, &PythonLoggerCoreWarn);
    }

    // error logger
    void PythonLoggerCoreError(const std::string &s) {
        ErrorLogger() << s;
    }
    void PythonLoggerError(const std::string & text) {
        static std::stringstream log_stream("");
        send_to_log(log_stream, text, &PythonLoggerCoreError);
    }

    // critical logger
    void PythonLoggerCoreFatal(const std::string &s) {
        ErrorLogger() << s;
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
