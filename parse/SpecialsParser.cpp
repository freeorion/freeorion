#include "Parse.h"

#include "EffectParser.h"

#include "../universe/ValueRef.h"
#include "../universe/Condition.h"
#include "../universe/Effect.h"
#include "../universe/Special.h"

#include <boost/spirit/include/phoenix.hpp>
//TODO: replace with std::make_unique when transitioning to C++14
#include <boost/smart_ptr/make_unique.hpp>

#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const parse::effects_group_payload&) { return os; }
    inline ostream& operator<<(ostream& os, const std::map<std::string, std::unique_ptr<Special>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, std::unique_ptr<Special>&) { return os; }
}
#endif

namespace {
    const boost::phoenix::function<parse::detail::is_unique> is_unique_;

    struct special_pod {
        special_pod(std::string name_,
                    std::string description_,
                    const boost::optional<parse::detail::value_ref_payload<double>>& stealth_,
                    const boost::optional<parse::effects_group_payload>& effects_,
                    boost::optional<double> spawn_rate_,
                    boost::optional<int> spawn_limit_,
                    const boost::optional<parse::detail::value_ref_payload<double>>& initial_capacity_,
                    const boost::optional<parse::detail::condition_payload>& location_,
                    const std::string& graphic_) :
            name(name_),
            description(description_),
            stealth(stealth_),
            effects(effects_),
            spawn_rate(spawn_rate_),
            spawn_limit(spawn_limit_),
            initial_capacity(initial_capacity_),
            location(location_),
            graphic(graphic_)
        {}

        std::string name;
        std::string description;
        const boost::optional<parse::detail::value_ref_payload<double>> stealth;
        const boost::optional<parse::effects_group_payload>& effects;
        boost::optional<double> spawn_rate;
        boost::optional<int> spawn_limit;
        boost::optional<const parse::detail::value_ref_payload<double>> initial_capacity;
        boost::optional<parse::detail::condition_payload> location;
        const std::string& graphic;
    };

    void insert_special(std::map<std::string, std::unique_ptr<Special>>& specials, special_pod special_, bool& pass) {
        auto special_ptr = boost::make_unique<Special>(
            special_.name, special_.description,
            (special_.stealth ? special_.stealth->OpenEnvelope(pass) : nullptr),
            (special_.effects ? OpenEnvelopes(*special_.effects, pass) : std::vector<std::unique_ptr<Effect::EffectsGroup>>()),
            (special_.spawn_rate ? *special_.spawn_rate : 1.0),
            (special_.spawn_limit ? *special_.spawn_limit : 9999),
            (special_.initial_capacity ? special_.initial_capacity->OpenEnvelope(pass) : nullptr),
            (special_.location ? special_.location->OpenEnvelope(pass) : nullptr),
            special_.graphic);

        specials.insert(std::make_pair(special_ptr->Name(), std::move(special_ptr)));
    }

    BOOST_PHOENIX_ADAPT_FUNCTION(void, insert_special_, insert_special, 3)

    using start_rule_payload = SpecialsManager::SpecialsTypeMap;
    using start_rule_signature = void(start_rule_payload&);

    struct grammar : public parse::detail::grammar<start_rule_signature> {
        grammar(const parse::lexer& tok,
                const std::string& filename,
                const parse::text_iterator& first, const parse::text_iterator& last) :
            grammar::base_type(start),
            condition_parser(tok, label),
            string_grammar(tok, label, condition_parser),
            double_rules(tok, label, condition_parser, string_grammar),
            effects_group_grammar(tok, label, condition_parser, string_grammar),
            double_rule(tok),
            int_rule(tok)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_5_type _5;
            qi::_6_type _6;
            qi::_7_type _7;
            qi::_8_type _8;
            qi::_9_type _9;
            phoenix::actor<boost::spirit::argument<9>> _10; // qi::_10_type is not predefined
            qi::_pass_type _pass;
            qi::_r1_type _r1;
            qi::eps_type eps;
            const boost::phoenix::function<parse::detail::deconstruct_movable> deconstruct_movable_;

            special
                = (  tok.Special_
                >    label(tok.Name_)           > tok.string
                >    label(tok.Description_)    > tok.string
                >  -(label(tok.Stealth_)        > double_rules.expr)
                >  -(label(tok.SpawnRate_)      > double_rule)
                >  -(label(tok.SpawnLimit_)     > int_rule)
                >  -(label(tok.Capacity_)       > double_rules.expr)
                >  -(label(tok.Location_)       > condition_parser)
                >  -(label(tok.EffectsGroups_)  > effects_group_grammar)
                >    label(tok.Graphic_)        > tok.string)
                [  _pass = is_unique_(_r1, _1, _2),
                   insert_special_(_r1, phoenix::construct<special_pod>(_2, _3, _4, _9, _5, _6, _7, _8, _10), _pass) ]
                ;

            start
                =   +special(_r1)
                ;

            special.name("Special");

#if DEBUG_PARSERS
            debug(special);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        using special_rule = parse::detail::rule<void (start_rule_payload&)>;
        using start_rule = parse::detail::rule<start_rule_signature>;

        parse::detail::Labeller             label;
        parse::conditions_parser_grammar    condition_parser;
        const parse::string_parser_grammar  string_grammar;
        parse::double_parser_rules          double_rules;
        parse::effects_group_grammar        effects_group_grammar;
        parse::detail::double_grammar       double_rule;
        parse::detail::int_grammar          int_rule;
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
