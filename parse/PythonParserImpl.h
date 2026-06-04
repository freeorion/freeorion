#ifndef _PythonParserImpl_h_
#define _PythonParserImpl_h_

#include "PythonParser.h"
#include "../util/ScopedTimer.h"

#include <boost/python/stl_iterator.hpp>
#include <boost/python/extract.hpp>

namespace py_parse { namespace detail {

    template <typename Grammar>
    [[nodiscard]] bool parse_file(const PythonParser& parser, const std::filesystem::path& path, const Grammar& grammar) {
        ScopedTimer timer("parse_file \"" + path.filename().string()  + "\"", std::chrono::milliseconds(1));

        std::string filename;
        std::string file_contents;

        return parser.ParseFileCommon(path, filename, file_contents);
    }
} }

#endif
