#ifndef _Python_Parse_h_
#define _Python_Parse_h_

#include "Parse.h"

class PythonCommon;

class FO_PARSE_API PythonParser {
public:
    PythonParser(const PythonCommon& python);
    ~PythonParser();
};

#endif

