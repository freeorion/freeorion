#include "Parse.h"

#include "EffectParser.h"

#include "../universe/ValueRef.h"
#include "../universe/Condition.h"
#include "../universe/Effect.h"
#include "../Empire/Government.h"
#include "../util/Directories.h"

#include <boost/spirit/include/phoenix.hpp>

#define DEBUG_PARSERS 0


#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const parse::effects_group_payload&) { return os; }
    inline ostream& operator<<(ostream& os, const PolicyManager::PoliciesTypeMap&) { return os; }
    inline ostream& operator<<(ostream& os, const PolicyManager::PoliciesTypeMap::value_type&) { return os; }
}
#endif

namespace {
    const boost::phoenix::function<parse::detail::is_unique> is_unique_;

    struct policy_pod {
        policy_pod(std::string& name_,
                   std::string& description_,
                   std::string& short_description_,
                   std::string& category_,
                   const boost::optional<parse::detail::value_ref_payload<double>>& adoption_cost_,
                   std::set<std::string>& prerequisites_,
                   std::set<std::string>& exclusions_,
                   const boost::optional<parse::effects_group_payload>& effects_,
                   const std::string& graphic_) :
            name(std::move(name_)),
            description(std::move(description_)),
            short_description(std::move(short_description_)),
            category(std::move(category_)),
            exclusions(std::move(exclusions_)),
            prerequisites(std::move(prerequisites_)),
            adoption_cost(adoption_cost_),
            effects(effects_),
            graphic(graphic_)
        {}

        std::string             name;
        std::string             description;
        std::string             short_description;
        std::string             category;
        std::set<std::string>   exclusions;
        std::set<std::string>   prerequisites;
        const boost::optional<parse::detail::value_ref_payload<double>> adoption_cost;
        const boost::optional<parse::effects_group_payload>&            effects;
        const std::string&      graphic;
    };

    void insert_policy(PolicyManager::PoliciesTypeMap& policies, policy_pod policy_, bool& pass) {
        auto policy_ptr = std::make_unique<Policy>(
            std::move(policy_.name),
            std::move(policy_.description),
            std::move(policy_.short_description),
            std::move(policy_.category),
            (policy_.adoption_cost ? policy_.adoption_cost->OpenEnvelope(pass) : nullptr),
            std::move(policy_.prerequisites),
            std::move(policy_.exclusions),
            (policy_.effects ? OpenEnvelopes(*policy_.effects, pass) : std::vector<std::unique_ptr<Effect::EffectsGroup>>()),
            policy_.graphic);

        auto& policy_name = policy_ptr->Name();
        policies.emplace(policy_name, std::move(policy_ptr));
    }

    BOOST_PHOENIX_ADAPT_FUNCTION(void, insert_policy_, insert_policy, 3)

    using stringset_rule_type = parse::detail::rule<std::set<std::string> ()>;
    using stringset_grammar_type = parse::detail::grammar<std::set<std::string> ()>;

    struct prereqs_grammar : public stringset_grammar_type {
        prereqs_grammar(const parse::lexer& tok, parse::detail::Labeller& label) :
            prereqs_grammar::base_type(start, "prereqs_grammar"),
            one_or_more_string_tokens(tok)
        {
            start %= -(label(tok.prerequisites_) >> one_or_more_string_tokens);
            start.name("Prerequisites");
#if DEBUG_PARSERS
            debug(start);
#endif
        }

        stringset_rule_type start;
        parse::detail::single_or_repeated_string<std::set<std::string>> one_or_more_string_tokens;
    };

    struct exclusions_grammar : public stringset_grammar_type {
        exclusions_grammar(const parse::lexer& tok, parse::detail::Labeller& label) :
            exclusions_grammar::base_type(start, "exclusions_grammar"),
            one_or_more_string_tokens(tok)
        {
            start %= -(label(tok.exclusions_) >> one_or_more_string_tokens);
            start.name("Exclusions");
#if DEBUG_PARSERS
            debug(start);
#endif
        }

        stringset_rule_type start;
        parse::detail::single_or_repeated_string<std::set<std::string>> one_or_more_string_tokens;
    };

    using start_rule_payload = PolicyManager::PoliciesTypeMap;
    using start_rule_signature = void(start_rule_payload&);

    struct grammar : public parse::detail::grammar<start_rule_signature> {
        grammar(const parse::lexer& tok,
                const std::string& filename,
                const parse::text_iterator& first, const parse::text_iterator& last) :
            grammar::base_type(start),
            condition_parser(tok, label),
            string_grammar(tok, label, condition_parser),
            double_rules(tok, label, condition_parser, string_grammar),
            prerequisites(tok, label),
            exclusions(tok, label),
            effects_group_grammar(tok, label, condition_parser, string_grammar)
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
            //const boost::phoenix::function<parse::detail::deconstruct_movable> deconstruct_movable_;

            policy
                = (  tok.Policy_                                            // _1
                >    label(tok.name_)               > tok.string            // _2
                >    label(tok.description_)        > tok.string            // _3
                >    label(tok.short_description_)  > tok.string            // _4
                >    label(tok.category_)           > tok.string            // _5
                >    label(tok.adoptioncost_)       > double_rules.expr     // _6
                >    prerequisites                                          // _7
                >    exclusions                                             // _8
                >  -(label(tok.effectsgroups_)      > effects_group_grammar)// _9
                >    label(tok.graphic_)            > tok.string)           // _10
                [  _pass = is_unique_(_r1, _1, _2),
                   insert_policy_(_r1, phoenix::construct<policy_pod>(
                       _2, _3, _4, _5, _6, _7, _8, _9, _10), _pass) ]
                ;

            start
                =   +policy(_r1)
                ;

            policy.name("Policy");
            prerequisites.name("Prerequisites");
            exclusions.name("Exclusions");

#if DEBUG_PARSERS
            debug(policy);
            debug(prerequisites);
            debug(exclusions)
#endif

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        using policy_rule = parse::detail::rule<void (start_rule_payload&)>;
        using start_rule = parse::detail::rule<start_rule_signature>;

        parse::detail::Labeller             label;
        parse::conditions_parser_grammar    condition_parser;
        const parse::string_parser_grammar  string_grammar;
        parse::double_parser_rules          double_rules;
        prereqs_grammar                     prerequisites;
        exclusions_grammar                  exclusions;
        parse::effects_group_grammar        effects_group_grammar;
        policy_rule                         policy;
        start_rule                          start;
    };
}


namespace parse {
    const lexer policy_lexer;

    start_rule_payload policies(const boost::filesystem::path& path) {
        start_rule_payload policies_;

        ScopedTimer timer("Policies Parsing", true);

        for (const auto& file : ListDir(path, IsFOCScript))
            detail::parse_file<grammar, start_rule_payload>(policy_lexer, file, policies_);

        return policies_;
    }
}
