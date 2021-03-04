#ifndef _Python_Parse_h_
#define _Python_Parse_h_

#include "Parse.h"

class PythonCommon;

class FO_PARSE_API PythonParser {
public:
    PythonParser(PythonCommon& _python);
    ~PythonParser();

    PythonParser(const PythonParser&) = delete;

    PythonParser(PythonParser&&) = delete;

    const PythonParser& operator=(const PythonParser&) = delete;

    PythonParser& operator=(PythonParser&&) = delete;
private:
    PythonCommon& python;
};

#endif

