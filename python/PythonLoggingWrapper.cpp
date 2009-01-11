#include <string>

#include "../util/AppInterface.h"

#include <boost/python.hpp>

namespace {
    // Expose interface for redirecting standard output and error to FreeOrion logging.  Can be imported
    // before loading the main FreeOrion AI interface library.
    static const int MAX_SINGLE_CHUNK_TEXT_SIZE = 1000; 

    // stdout logger
    static std::string log_buffer("");
    void LogText(const char* text) {
        // Python sends text as several null-terminated array of char which need to be
        // concatenated before they are output to the logger.  There's probably a better
        // way to do this, but I don't know what it is, and this seems reasonably safe...
        if (!text) return;
        for (int i = 0; i < MAX_SINGLE_CHUNK_TEXT_SIZE; ++i) {
            if (text[i] == '\0') break;
            if (text[i] == '\n' || i == MAX_SINGLE_CHUNK_TEXT_SIZE - 1) {
                Logger().debugStream() << log_buffer;
                log_buffer = "";
            } else {
                log_buffer += text[i];
            }
        }
    }

    // stderr logger
    static std::string error_buffer("");
    void ErrorText(const char* text) {
        // Python sends text as several null-terminated array of char which need to be
        // concatenated before they are output to the logger.  There's probably a better
        // way to do this, but I don't know what it is, and this seems reasonably safe...
       if (!text) return;
        for (int i = 0; i < MAX_SINGLE_CHUNK_TEXT_SIZE; ++i) {
            if (text[i] == '\0') break;
            if (text[i] == '\n' || i == MAX_SINGLE_CHUNK_TEXT_SIZE - 1) {
                Logger().errorStream() << error_buffer;
                error_buffer = "";
            } else {
                error_buffer += text[i];
            }
        }
    }
}

namespace FreeOrionPython {
    using boost::python::def;
    void WrapLogger() {
        def("log",                    LogText);
        def("error",                  ErrorText);
    }
}
