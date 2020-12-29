#ifndef _Python_Parse_h_
#define _Python_Parse_h_

#include "Parse.h"

#include <boost/python/dict.hpp>

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
                         const boost::python::dict& globals,
                         std::string& filename, std::string& file_contents) const;
private:
    PythonCommon& python;
};

#endif

