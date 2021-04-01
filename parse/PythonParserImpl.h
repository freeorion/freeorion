#ifndef _PythonParserImpl_h_
#define _PythonParserImpl_h_

#include "PythonParser.h"
#include "../util/ScopedTimer.h"

#include <boost/python/stl_iterator.hpp>
#include <boost/python/extract.hpp>

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

    template <typename T, typename F, typename V>
    void flatten_list(const boost::python::object& l,
                      const F& f,
                      V& retval) {
        auto args = boost::python::extract<boost::python::list>(l);
        if (args.check()) {
            boost::python::stl_input_iterator<boost::python::object> args_begin(args), args_end;
            for (auto it = args_begin; it != args_end; ++ it) {
                flatten_list<T, F, V>(*it, f, retval);
            }
        } else {
            f(boost::python::extract<T>(l)(), retval);
        }
    }

} }

#endif
