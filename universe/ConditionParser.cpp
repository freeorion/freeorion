#include "Parser.h"

using namespace boost::spirit::classic;
using namespace phoenix;

extern rule<Scanner, ConditionClosure::context_t> condition1_p;
extern rule<Scanner, ConditionClosure::context_t> condition2_p;
extern rule<Scanner, ConditionClosure::context_t> condition3_p;

rule<Scanner, ConditionClosure::context_t> condition_p;

namespace {
    bool Init()
    {
        condition_p =
            condition1_p[condition_p.this_ = arg1] |
            condition2_p[condition_p.this_ = arg1] |
            condition3_p[condition_p.this_ = arg1];
        return true;
    }
    bool dummy = Init();
}
