#include "Parse.h"

#include "ParseImpl.h"

#include "../util/Directories.h"
#include "../util/MultiplayerCommon.h"

#include <boost/spirit/include/phoenix.hpp>


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::map<int, int>&) { return os; 
    inline ostream& operator<<(ostream& os, const std::map<std::string, std::map<int, int>>&) { return os; }
}
#endif

namespace {
    struct insert_rule_ {
        typedef void result_type;

        void operator()(GameRules& game_rules, const std::string& name, const std::string& desc,
                        bool default_value) const
        {
            game_rules.Add<bool>(name, desc, default_value, false);
        }

        void operator()(GameRules& game_rules, const std::string& name, const std::string& desc,
                        int default_value, int min, int max) const
        {
            game_rules.Add<int>(name, desc, default_value, false, RangedValidator<int>(min, max));
        }

        void operator()(GameRules& game_rules, const std::string& name, const std::string& desc,
                        double default_value, double min, double max) const
        {
            game_rules.Add<double>(name, desc, default_value, false, RangedValidator<double>(min, max));
        }

        void operator()(GameRules& game_rules, const std::string& name, const std::string& desc,
                        const std::string& default_value) const // TBD: allowed values
        {
            game_rules.Add<std::string>(name, desc, default_value, false);
        }

        //void operator()(GameRules& game_rules, const std::string& name, const std::string& desc,
        //                const std::vector<std::string>& default_value) const // TBD: allowed values
        //{
        //    game_rules.Add<std::vector<std::string>>(name, desc, default_value, false);
        //}
    };
    const boost::phoenix::function<insert_rule_> add_rule;

    struct rules {
        rules() {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            using phoenix::construct;

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
            qi::_i_type _i;
            qi::_r1_type _r1;

            const parse::lexer& tok = parse::lexer::instance();

            game_rule_bool
                =   tok.GameRule_
                >>  parse::detail::label(Name_token) >>         tok.string [ _a = _1 ]
                >>  parse::detail::label(Description_token) >>  tok.string [ _b = _1 ]
                >>  parse::detail::label(Type_token) >>         tok.TrueFalse_
                >   parse::detail::label(Default_token) >       tok.int_
                    [ add_rule(_r1, _a, _b, _1 )]
                ;

            game_rule_int
                =   tok.GameRule_
                >>  parse::detail::label(Name_token) >>         tok.string [ _a = _1 ]
                >>  parse::detail::label(Description_token) >>  tok.string [ _b = _1 ]
                >>  parse::detail::label(Type_token) >>         tok.Integer_
                >   parse::detail::label(Default_token) >       tok.int_ [ _f = _1 ]
                >   parse::detail::label(Min_token) >           tok.int_ [ _g = _1 ]
                >   parse::detail::label(Max_token) >           tok.int_
                    [ add_rule(_r1, _a, _b, _f, _g, _1 )]
                ;

            game_rule_double
                =   tok.GameRule_
                >>  parse::detail::label(Name_token) >>         tok.string [ _a = _1 ]
                >>  parse::detail::label(Description_token) >>  tok.string [ _b = _1 ]
                >>  parse::detail::label(Type_token) >>         tok.Real_
                >   parse::detail::label(Default_token) >       tok.double_ [ _c = _1 ]
                >   parse::detail::label(Min_token) >           tok.double_ [ _d = _1 ]
                >   parse::detail::label(Max_token) >           tok.double_
                    [ add_rule(_r1, _a, _b, _c, _d, _1 )]
                ;

            game_rule_string
                =   tok.GameRule_
                >>  parse::detail::label(Name_token) >>         tok.string [ _a = _1 ]
                >>  parse::detail::label(Description_token) >>  tok.string [ _b = _1 ]
                >>  parse::detail::label(Type_token) >>         tok.String_
                >   parse::detail::label(Default_token) >       tok.string [ _e = _1 ]
                    [ add_rule(_r1, _a, _b, _e)]
                ;

            start
                =  *game_rule_int(_r1)
                ;

            game_rule_bool.name("Boolean GameRule");
            game_rule_int.name("Integer GameRule");
            game_rule_double.name("Double GameRule");
            game_rule_string.name("String GameRule");
            game_rule_string_list.name("String List GameRule");
            start.name("GameRules");

#if DEBUG_PARSERS
            debug(game_rule_bool);
            debug(game_rule_int);
            debug(game_rule_double);
            debug(game_rule_string);
            debug(game_rule_string_list);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(_1, _2, _3, _4));
        }

        typedef parse::detail::rule<
            void (GameRules&),
            boost::spirit::qi::locals<
                std::string,                // _a : name
                std::string,                // _b : description
                double,                     // _c : default double
                double,                     // _d : min int (max passed as immediate)
                std::string,                // _e : default string
                int,                        // _f : default int
                int,                        // _g : min int (max passed as immediate)
                std::vector<std::string>,   // _h : default string list
                std::vector<std::string>    // _i : allowed string (list) entries
            >
        > game_rule_rule;

        typedef parse::detail::rule<
            void (GameRules&)
        > start_rule;

        game_rule_rule  game_rule_bool;
        game_rule_rule  game_rule_int;
        game_rule_rule  game_rule_double;
        game_rule_rule  game_rule_string;
        game_rule_rule  game_rule_string_list;
        start_rule      start;
    };
}

namespace parse {
    bool game_rules(GameRules& game_rules) {
        boost::filesystem::path path = GetResourceDir() / "scripting/game_rules.focs.txt";
        return detail::parse_file<rules, GameRules>(path, game_rules);
    }
}
