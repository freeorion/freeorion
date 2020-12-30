#ifndef _Python_Parse_h_
#define _Python_Parse_h_

#include "Parse.h"

#include <boost/python/dict.hpp>
#include <boost/python/object_fwd.hpp>

class PythonCommon;

class FO_PARSE_API PythonParser {
public:
    PythonParser(PythonCommon& _python);
    ~PythonParser();

    PythonParser(const PythonParser&) = delete;

    PythonParser(PythonParser&&) = delete;

    const PythonParser& operator=(const PythonParser&) = delete;

    PythonParser& operator=(PythonParser&&) = delete;

    bool ParseFileCommon(const boost::filesystem::path& path,
                         std::function<boost::python::dict()> globals,
                         std::string& filename, std::string& file_contents) const;

    boost::python::object type_int;
    boost::python::object type_float;
    boost::python::object type_bool;
    boost::python::object type_str;
private:
    PythonCommon& python;
};

#endif

