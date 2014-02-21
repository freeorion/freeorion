#include "Label.h"


namespace parse {
    label_rule& label(const char* name) {
        static std::map<const char*, label_rule> rules;
        std::map<const char*, label_rule>::iterator it = rules.find(name);
        if (it == rules.end()) {
            const lexer& l = lexer::instance();
            label_rule& retval = rules[name];
            retval
                =   -(
                          l.name_token(name)
                      >>  '='
                     )
                ;
            retval.name(std::string(name) + " =");
            return retval;
        } else {
            return it->second;
        }
    }
}
