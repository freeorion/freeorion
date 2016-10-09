#include "ConditionParserImpl.h"
#include "Double.h"
#include "Int.h"
#include "Label.h"
#include "ValueRefParser.h"
#include "Parse.h"
#include "ParseImpl.h"
#include "../universe/Special.h"

#include <boost/spirit/include/phoenix.hpp>

#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<boost::shared_ptr<Effect::EffectsGroup> >&) { return os; }
    inline ostream& operator<<(ostream& os, const std::map<std::string, Special*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, Special*>&) { return os; }
}
#endif

namespace {
    struct insert_ {
        typedef void result_type;

        void operator()(std::map<std::string, Special*>& specials, Special* special) const {
            if (!specials.insert(std::make_pair(special->Name(), special)).second) {
                std::string error_str = "ERROR: More than one special in specials.txt has the name " + special->Name();
                throw std::runtime_error(error_str.c_str());
            }
        }
    };
    const boost::phoenix::function<insert_> insert;

    struct rules {
        rules() {
            const parse::lexer& tok =                                               parse::lexer::instance();
            const parse::value_ref_parser_rule<double>::type& double_value_ref =    parse::value_ref_parser<double>();

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_c_type _c;
            qi::_d_type _d;
            qi::_e_type _e;
            qi::_f_type _f;
            qi::_g_type _g;
            qi::_h_type _h;
            qi::_r1_type _r1;
            qi::_r2_type _r2;
            qi::eps_type eps;
            using phoenix::new_;

            special_prefix
                =    tok.Special_
                >    parse::label(Name_token)               > tok.string [ _r1 = _1 ]
                >    parse::label(Description_token)        > tok.string [ _r2 = _1 ]
                ;

            spawn
                =    (      parse::label(SpawnRate_token)   > parse::double_ [ _r1 = _1 ]
                        |   eps [ _r1 = 1.0 ]
                     )
                >    (      parse::label(SpawnLimit_token)  > parse::int_ [ _r2 = _1 ]
                        |   eps [ _r2 = 9999 ]
                     )
                ;

            special
                =    special_prefix(_a, _b)
                >  -(parse::label(Stealth_token)            > double_value_ref [ _g = _1 ])
                >    spawn(_c, _d)
                >  -(parse::label(Capacity_token)           > double_value_ref [ _h = _1 ])
                >  -(parse::label(Location_token)           > parse::detail::condition_parser [ _e = _1 ])
                >  -(parse::label(EffectsGroups_token)      > parse::detail::effects_group_parser() [ _f = _1 ])
                >    parse::label(Graphic_token)            > tok.string
                [ insert(_r1, new_<Special>(_a, _b, _g, _f, _c, _d, _h, _e, _1)) ]
                ;

            start
                =   +special(_r1)
                ;

            special_prefix.name("Special");
            special.name("Special");
            spawn.name("SpawnRate and SpawnLimit");

#if DEBUG_PARSERS
            debug(special_prefix);
            debug(spawn);
            debug(special);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(_1, _2, _3, _4));
        }

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::string&, std::string&),
            parse::skipper_type
        > special_prefix_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (double&, int&),
            parse::skipper_type
        > spawn_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::map<std::string, Special*>&),
            qi::locals<
                std::string,
                std::string,
                double,
                int,
                Condition::ConditionBase*,
                std::vector<boost::shared_ptr<Effect::EffectsGroup> >,
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<double>*
            >,
            parse::skipper_type
        > special_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::map<std::string, Special*>&),
            parse::skipper_type
        > start_rule;


        special_prefix_rule special_prefix;
        spawn_rule          spawn;
        special_rule        special;
        start_rule          start;
    };
}

namespace parse {
    bool specials(std::map<std::string, Special*>& specials_) {
        bool result = true;

        std::vector<boost::filesystem::path> file_list = ListScripts("scripting/specials");

        for (std::vector<boost::filesystem::path>::iterator file_it = file_list.begin();
             file_it != file_list.end(); ++file_it)
        {
            result &= detail::parse_file<rules, std::map<std::string, Special*> >(*file_it, specials_);
        }

        return result;
    }
}
