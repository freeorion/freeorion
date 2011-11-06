#include "Label.h"


namespace parse {

    label_rule& label(adobe::name_t name)
    {
        static std::map<adobe::name_t, label_rule> rules;
        std::map<adobe::name_t, label_rule>::iterator it = rules.find(name);
        if (it == rules.end()) {
            const lexer& l = lexer::instance();
            label_rule& retval = rules[name];
            retval
                =   -(
                          l.name_token(name)
                      >>  '='
                     )
                ;
            retval.name(std::string(name.c_str()) + " =");
            return retval;
        } else {
            return it->second;
        }
    }

}
