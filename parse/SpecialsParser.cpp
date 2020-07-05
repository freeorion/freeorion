#include "Parse.h"

#include "EffectParser.h"

#include <boost/spirit/include/phoenix.hpp>
#include "../focs/Condition.h"
#include "../focs/Effect.h"
#include "../focs/ValueRef.h"
#include "../universe/Special.h"
#include "../util/Directories.h"


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

    struct special_data {
        special_data(boost::optional<double>& spawn_rate_,
                     boost::optional<int>& spawn_limit_,
                     std::string& graphic_) :
            spawn_rate(std::move(spawn_rate_)),
            spawn_limit(std::move(spawn_limit_)),
            graphic(std::move(graphic_))
        {}

        boost::optional<double> spawn_rate;
        boost::optional<int> spawn_limit;
        std::string graphic;
    };

    void insert_special(std::map<std::string, std::unique_ptr<Special>>& specials,
                        const special_data& special_pod,
                        std::string& name, std::string& description,
                        boost::optional<parse::detail::value_ref_payload<double>>& stealth,
                        boost::optional<parse::effects_group_payload>& effects,
                        boost::optional<parse::detail::value_ref_payload<double>>& initial_capacity,
                        boost::optional<parse::detail::condition_payload>& location,
                        bool& pass)
    {
        auto special_ptr = std::make_unique<Special>(
            std::move(name), std::move(description),
            (stealth ? std::move(stealth->OpenEnvelope(pass)) : nullptr),
            (effects ? std::move(OpenEnvelopes(*effects, pass)) : std::vector<std::unique_ptr<Effect::EffectsGroup>>{}),
            (special_pod.spawn_rate ? *special_pod.spawn_rate : 1.0),
            (special_pod.spawn_limit ? *special_pod.spawn_limit : 9999),
            (initial_capacity ? std::move(initial_capacity->OpenEnvelope(pass)) : nullptr),
            (location ? std::move(location->OpenEnvelope(pass)) : nullptr),
            special_pod.graphic);

        specials.emplace(special_ptr->Name(), std::move(special_ptr));
    }

    BOOST_PHOENIX_ADAPT_FUNCTION(void, insert_special_, insert_special, 9)

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
                >    label(tok.Name_)           > tok.string                // _2
                >    label(tok.Description_)    > tok.string                // _3
                >  -(label(tok.Stealth_)        > double_rules.expr)        // _4
                >  -(label(tok.SpawnRate_)      > double_rule)              // _5
                >  -(label(tok.SpawnLimit_)     > int_rule)                 // _6
                >  -(label(tok.Capacity_)       > double_rules.expr)        // _7
                >  -(label(tok.Location_)       > condition_parser)         // _8
                >  -(label(tok.EffectsGroups_)  > effects_group_grammar)    // _9
                >    label(tok.Graphic_)        > tok.string)               // _10
                [  _pass = is_unique_(_r1, _1, _2),
                   insert_special_(_r1, phoenix::construct<special_data>(_5, _6, _10),
                                   _2, _3, _4, _9, _7, _8, _pass) ]
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

        for (const auto& file : ListDir(path, IsFOCScript))
            /*auto success =*/ detail::parse_file<grammar, start_rule_payload>(lexer, file, specials_);

        return specials_;
    }
}
