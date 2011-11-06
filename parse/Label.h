// -*- C++ -*-
#ifndef _Label_h_
#define _Label_h_

#include "Lexer.h"

#include <boost/spirit/home/qi.hpp>


namespace parse {

    typedef boost::spirit::qi::rule<
        token_iterator,
        void (),
        skipper_type
    > label_rule;

    label_rule& label(adobe::name_t name);

}

#endif
