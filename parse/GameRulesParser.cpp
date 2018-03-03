#include "Parse.h"

#include "ParseImpl.h"

#include "../util/Directories.h"
#include "../util/GameRules.h"

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

        void operator()(GameRules& game_rules, const std::string& name,
                        const std::string& desc, const std::string& category,
                        bool default_value) const
        {
            DebugLogger() << "Adding Boolean game rule with name: " << name
                          << ", desc: " << desc << ", default: " << default_value;
            game_rules.Add<bool>(name, desc, category, default_value, false);
        }

        void operator()(GameRules& game_rules, const std::string& name,
                        const std::string& desc, const std::string& category,
                        int default_value, int min, int max) const
        {
            DebugLogger() << "Adding Integer game rule with name: " << name
                          << ", desc: " << desc << ", default: " << default_value
                          << ", min: " << min << ", max: " << max;
            game_rules.Add<int>(name, desc, category, default_value, false,
                                RangedValidator<int>(min, max));
        }

        void operator()(GameRules& game_rules, const std::string& name,
                        const std::string& desc, const std::string& category,
                        double default_value, double min, double max) const
        {
            DebugLogger() << "Adding Double game rule with name: " << name
                          << ", desc: " << desc << ", default: " << default_value
                          << ", min: " << min << ", max: " << max;
            game_rules.Add<double>(name, desc, category, default_value,
                                   false, RangedValidator<double>(min, max));
        }

        void operator()(GameRules& game_rules, const std::string& name,
                        const std::string& desc, const std::string& category,
                        const std::string& default_value,
                        const std::set<std::string>& allowed = {}) const
        {
            std::string allowed_values_string;
            for (const auto& e : allowed)
                allowed_values_string += "\"" + e + "\", ";
            DebugLogger() << "Adding String game rule with name: " << name
                          << ", desc: " << desc << ", default: \"" << default_value
                          << "\", allowed: " << allowed_values_string;

            if (allowed.empty()) {
                game_rules.Add<std::string>(name, desc, category, default_value, false);
            } else {
                game_rules.Add<std::string>(name, desc, category, default_value, false,
                                            DiscreteValidator<std::string>(allowed));
            }
        }
    };
    const boost::phoenix::function<insert_rule_> add_rule;

    using start_rule_payload = GameRules;
    using start_rule_signature = void(start_rule_payload&);

    struct grammar : public parse::detail::grammar<start_rule_signature> {
        grammar(const parse::lexer& tok,
                const std::string& filename,
                const parse::text_iterator& first, const parse::text_iterator& last) :
            grammar::base_type(start),
            one_or_more_string_tokens(tok),
            double_rule(tok),
            int_rule(tok)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            using phoenix::insert;
            using phoenix::clear;

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
            qi::_j_type _j;
            qi::_r1_type _r1;
            qi::eps_type eps;
            boost::spirit::qi::repeat_type repeat_;

            game_rule_bool
                =   (tok.GameRule_
                    >> (label(tok.Name_) >          tok.string [ _a = _1 ])
                    >> (label(tok.Description_) >   tok.string [ _b = _1 ])
                    >> (label(tok.Category_) >      tok.string [ _j = _1 ])
                    >>  label(tok.Type_) >>         tok.Toggle_
                    )
                > ((label(tok.Default_)
                    >   (
                            tok.On_ [ _i = true ]
                        |   tok.Off_ [ _i = false ]
                        )
                   ) | eps [ _i = false ]
                  )
                   [ add_rule(_r1, _a, _b, _j, _i) ]
                ;

            game_rule_int
                =   (tok.GameRule_
                    >> (label(tok.Name_) >          tok.string [ _a = _1 ])
                    >> (label(tok.Description_) >   tok.string [ _b = _1 ])
                    >> (label(tok.Category_) >      tok.string [ _j = _1 ])
                    >>  label(tok.Type_) >>         tok.Integer_
                    )
                >   label(tok.Default_) >       int_rule [ _f = _1 ]
                >   label(tok.Min_) >           int_rule [ _g = _1 ]
                >   label(tok.Max_) >           int_rule
                    [ add_rule(_r1, _a, _b, _j, _f, _g, _1 ) ]
                ;

            game_rule_double
                =   (tok.GameRule_
                    >> (label(tok.Name_) >          tok.string [ _a = _1 ])
                    >> (label(tok.Description_) >   tok.string [ _b = _1 ])
                    >> (label(tok.Category_) >      tok.string [ _j = _1 ])
                    >>  label(tok.Type_) >>         tok.Real_
                    )
                >   label(tok.Default_) >       double_rule [ _c = _1 ]
                >   label(tok.Min_) >           double_rule [ _d = _1 ]
                >   label(tok.Max_) >           double_rule
                    [ add_rule(_r1, _a, _b, _j, _c, _d, _1 ) ]
                ;

            game_rule_string
                =   (tok.GameRule_
                    >> (label(tok.Name_) >          tok.string [ _a = _1 ])
                    >> (label(tok.Description_) >   tok.string [ _b = _1 ])
                    >> (label(tok.Category_) >      tok.string [ _j = _1 ])
                    >>  label(tok.Type_) >>         tok.String_
                    )
                >   label(tok.Default_) >       tok.string [ _e = _1 ]
                >  -( label(tok.Allowed_) > one_or_more_string_tokens [_h = _1])
                  [ add_rule(_r1, _a, _b, _j, _e, _h) ]
                ;

            all_game_rules
                %=  game_rule_bool(_r1)
                |   game_rule_int(_r1)
                |   game_rule_double(_r1)
                |   game_rule_string(_r1)
                ;

            start
                =  *(all_game_rules(_r1))
                ;

            game_rule_bool.name("Boolean GameRule");
            game_rule_int.name("Integer GameRule");
            game_rule_double.name("Double GameRule");
            game_rule_string.name("String GameRule");
            game_rule_string_list.name("String List GameRule");
            all_game_rules.name("GameRule");
            start.name("GameRules");

#if DEBUG_PARSERS
            debug(game_rule_bool);
            debug(game_rule_int);
            debug(game_rule_double);
            debug(game_rule_string);
            debug(game_rule_string_list);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        typedef parse::detail::rule<
            void (GameRules&),
            boost::spirit::qi::locals<
                std::string,            // _a : name
                std::string,            // _b : description
                double,                 // _c : default double
                double,                 // _d : min int (max passed as immediate)
                std::string,            // _e : default string
                int,                    // _f : default int
                int,                    // _g : min int (max passed as immediate)
                std::set<std::string>,  // _h : default string list
                bool,                   // _i : allowed string (list) entries
                std::string             // _j : category
            >
        > game_rule_rule;

        using start_rule = parse::detail::rule<start_rule_signature>;

        parse::detail::Labeller label;
        parse::detail::single_or_repeated_string<std::set<std::string>> one_or_more_string_tokens;
        parse::detail::double_grammar double_rule;
        parse::detail::int_grammar int_rule;
        game_rule_rule  game_rule_bool;
        game_rule_rule  game_rule_int;
        game_rule_rule  game_rule_double;
        game_rule_rule  game_rule_string;
        game_rule_rule  game_rule_string_list;
        game_rule_rule  all_game_rules;
        start_rule      start;
    };
}

namespace parse {
    GameRules game_rules(const boost::filesystem::path& path) {
        GameRules game_rules;
        const lexer lexer;
        /*auto success =*/ detail::parse_file<grammar, GameRules>(lexer, path, game_rules);
        return game_rules;
    }
}
