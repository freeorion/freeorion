#ifndef _Label_h_
#define _Label_h_

#include "Lexer.h"
#include "ParseImpl.h"

#include <boost/spirit/include/qi.hpp>

namespace parse {
    typedef detail::rule<> label_rule;

    label_rule& label(const char* name);
}

#endif
