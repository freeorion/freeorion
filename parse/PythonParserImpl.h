#ifndef _PythonParserImpl_h_
#define _PythonParserImpl_h_

#include "PythonParser.h"
#include "../util/ScopedTimer.h"

namespace py_parse { namespace detail {

    template <typename Grammar, typename Arg1>
    bool parse_file(const PythonParser& parser, const boost::filesystem::path& path, Arg1& arg1) {
        ScopedTimer timer("parse_file \"" + path.filename().string()  + "\"", std::chrono::milliseconds(1));

        std::string filename;
        std::string file_contents;

        Grammar grammar(parser);

        return parser.ParseFileCommon(path, [grammar, &arg1]() { return grammar(arg1); }, filename, file_contents);
    }

    template <typename Grammar, typename Arg1, typename Arg2>
    bool parse_file(const PythonParser& parser, const boost::filesystem::path& path, Arg1& arg1, Arg2& arg2) {
        ScopedTimer timer("parse_file \"" + path.filename().string()  + "\"", std::chrono::milliseconds(1));

        std::string filename;
        std::string file_contents;

        Grammar grammar(parser);

        return parser.ParseFileCommon(path, [grammar, &arg1, &arg2]() { return grammar(arg1, arg2); }, filename, file_contents);
    }

} }

#endif
