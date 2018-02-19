#include "Parse.h"

#include "EffectParser.h"

#include "../universe/ValueRef.h"
#include "../universe/Condition.h"
#include "../universe/Effect.h"
#include "../Empire/Government.h"

#include <boost/spirit/include/phoenix.hpp>
//TODO: replace with std::make_unique when transitioning to C++14
#include <boost/smart_ptr/make_unique.hpp>

#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const parse::effects_group_payload&) { return os; }
    inline ostream& operator<<(ostream& os, const std::map<std::string, std::unique_ptr<Policy>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, std::unique_ptr<Policy>&) { return os; }
}
#endif

namespace {
    const boost::phoenix::function<parse::detail::is_unique> is_unique_;

    struct policy_pod {
        policy_pod(std::string name_,
                   std::string description_,
                   const parse::effects_group_payload& effects_,
                   const std::string& graphic_) :
            name(name_),
            description(description_),
            effects(effects_),
            graphic(graphic_)
        {}

        std::string name;
        std::string description;
        const parse::effects_group_payload& effects;
        const std::string& graphic;
    };

    void insert_policy(std::map<std::string, std::unique_ptr<Policy>>& specials, policy_pod policy_, bool& pass) {
        auto policy_ptr = boost::make_unique<Policy>(
            special_.name, special_.description, special_.stealth.OpenEnvelope(pass),
            OpenEnvelopes(special_.effects, pass),
            special_.spawn_rate, special_.spawn_limit, special_.initial_capacity.OpenEnvelope(pass),
            special_.location.OpenEnvelope(pass), special_.graphic);

        specials.insert(std::make_pair(special_ptr->Name(), std::move(special_ptr)));
    }

    BOOST_PHOENIX_ADAPT_FUNCTION(void, insert_special_, insert_special, 3)

    using start_rule_payload = std::map<std::string, std::unique_ptr<Special>>;
    using start_rule_signature = void(start_rule_payload&);

    struct grammar : public parse::detail::grammar<start_rule_signature> {
        grammar(const parse::lexer& tok,
                const std::string& filename,
                const parse::text_iterator& first, const parse::text_iterator& last) :
            grammar::base_type(start),
            labeller(tok),
            condition_parser(tok, labeller),
            string_grammar(tok, labeller, condition_parser),
            double_rules(tok, labeller, condition_parser, string_grammar),
            effects_group_grammar(tok, labeller, condition_parser, string_grammar),
            double_rule(tok),
            int_rule(tok)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

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
            qi::_pass_type _pass;
            qi::_r1_type _r1;
            qi::_r2_type _r2;
            qi::_r3_type _r3;
            qi::eps_type eps;
            const boost::phoenix::function<parse::detail::deconstruct_movable> deconstruct_movable_;

            special_prefix
                =    tok.Special_
                >    labeller.rule(Name_token)
                >    tok.string        [ _pass = is_unique_(_r1, Special_token, _1), _r2 = _1 ]
                >    labeller.rule(Description_token)        > tok.string [ _r3 = _1 ]
                ;

            spawn
                =    (      (labeller.rule(SpawnRate_token)   > double_rule [ _r1 = _1 ])
                        |    eps [ _r1 = 1.0 ]
                     )
                >    (      (labeller.rule(SpawnLimit_token)  > int_rule [ _r2 = _1 ])
                        |    eps [ _r2 = 9999 ]
                     )
                ;

            special
                =    special_prefix(_r1, _a, _b)
                >  -(labeller.rule(Stealth_token)            > double_rules.expr [ _g = _1 ])
                >    spawn(_c, _d)
                >  -(labeller.rule(Capacity_token)           > double_rules.expr [ _h = _1 ])
                >  -(labeller.rule(Location_token)           > condition_parser [ _e = _1 ])
                >  -(labeller.rule(EffectsGroups_token)      > effects_group_grammar [ _f = _1 ])
                >    labeller.rule(Graphic_token)            > tok.string
                [ insert_special_(_r1, phoenix::construct<special_pod>(_a, _b, _g, _f, _c, _d, _h, _e, _1), _pass) ]
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

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        typedef parse::detail::rule<
            void (const start_rule_payload&, std::string&, std::string&)
        > special_prefix_rule;

        typedef parse::detail::rule<
            void (double&, int&)
        > spawn_rule;

        typedef parse::detail::rule<
            void (start_rule_payload&),
            boost::spirit::qi::locals<
                std::string,
                std::string,
                double,
                int,
                parse::detail::condition_payload,
                parse::effects_group_payload,
                parse::detail::value_ref_payload<double>,
                parse::detail::value_ref_payload<double>
            >
        > special_rule;

        using start_rule = parse::detail::rule<start_rule_signature>;

        parse::detail::Labeller             labeller;
        parse::conditions_parser_grammar    condition_parser;
        const parse::string_parser_grammar  string_grammar;
        parse::double_parser_rules          double_rules;
        parse::effects_group_grammar        effects_group_grammar;
        parse::detail::double_grammar       double_rule;
        parse::detail::int_grammar          int_rule;
        special_prefix_rule                 special_prefix;
        spawn_rule                          spawn;
        special_rule                        special;
        start_rule                          start;
    };
}

namespace parse {
    start_rule_payload specials(const boost::filesystem::path& path) {
        const lexer lexer;
        start_rule_payload specials_;

        for (const boost::filesystem::path& file : ListScripts(path)) {
            /*auto success =*/ detail::parse_file<grammar, start_rule_payload>(lexer, file, specials_);
        }

        return specials_;
    }
}
