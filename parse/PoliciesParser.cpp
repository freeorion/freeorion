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
    inline ostream& operator<<(ostream& os, const PolicyManager::PoliciesTypeMap&) { return os; }
    inline ostream& operator<<(ostream& os, const PolicyManager::PoliciesTypeMap::value_type&) { return os; }
}
#endif

namespace {
    const boost::phoenix::function<parse::detail::is_unique> is_unique_;

    struct policy_pod {
        policy_pod(std::string name_,
                   std::string description_,
                   std::string short_description_,
                   std::string category_,
                   const boost::optional<parse::detail::value_ref_payload<double>>& adoption_cost_,
                   const boost::optional<parse::effects_group_payload>& effects_,
                   const std::string& graphic_) :
            name(name_),
            description(description_),
            short_description(short_description_),
            category(category_),
            adoption_cost(adoption_cost_),
            effects(effects_),
            graphic(graphic_)
        {}

        std::string         name;
        std::string         description;
        std::string         short_description;
        std::string         category;
        const boost::optional<parse::detail::value_ref_payload<double>> adoption_cost;
        const boost::optional<parse::effects_group_payload>&            effects;
        const std::string&  graphic;
    };

    void insert_policy(PolicyManager::PoliciesTypeMap& policies, policy_pod policy_, bool& pass) {
        auto policy_ptr = boost::make_unique<Policy>(
            policy_.name,
            policy_.description,
            policy_.short_description,
            policy_.category,
            (policy_.adoption_cost ? policy_.adoption_cost->OpenEnvelope(pass) : nullptr),
            (policy_.effects ? OpenEnvelopes(*policy_.effects, pass) : std::vector<std::unique_ptr<Effect::EffectsGroup>>()),
            policy_.graphic);

        policies.insert(std::make_pair(policy_ptr->Name(), std::move(policy_ptr)));
    }

    BOOST_PHOENIX_ADAPT_FUNCTION(void, insert_policy_, insert_policy, 3)

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
            qi::_pass_type _pass;
            qi::_r1_type _r1;
            qi::eps_type eps;
            const boost::phoenix::function<parse::detail::deconstruct_movable> deconstruct_movable_;

            policy
                = (  tok.Policy_
                >    label(tok.Name_)               > tok.string
                >    label(tok.Description_)        > tok.string
                >    label(tok.Short_Description_)  > tok.string
                >    label(tok.Category_)           > tok.string
                >    label(tok.AdoptionCost_)       > double_rules.expr
                >  -(label(tok.EffectsGroups_)      > effects_group_grammar)
                >    label(tok.Graphic_)            > tok.string)
                [  _pass = is_unique_(_r1, _1, _2),
                   insert_policy_(_r1, phoenix::construct<policy_pod>(_2, _3, _4, _5, _6, _7, _8), _pass) ]
                ;

            start
                =   +policy(_r1)
                ;

            policy.name("Policy");

#if DEBUG_PARSERS
            debug(special);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        using policy_rule = parse::detail::rule<void (start_rule_payload&)>;
        using start_rule = parse::detail::rule<start_rule_signature>;

        parse::detail::Labeller             label;
        parse::conditions_parser_grammar    condition_parser;
        const parse::string_parser_grammar  string_grammar;
        parse::double_parser_rules          double_rules;
        parse::effects_group_grammar        effects_group_grammar;
        policy_rule                         policy;
        start_rule                          start;
    };
}


namespace parse {
    start_rule_payload policies(const boost::filesystem::path& path) {
        const lexer lexer;
        start_rule_payload policies_;

        for (const boost::filesystem::path& file : ListScripts(path)) {
            /*auto success =*/ detail::parse_file<grammar, start_rule_payload>(lexer, file, policies_);
        }

        return policies_;
    }
}
