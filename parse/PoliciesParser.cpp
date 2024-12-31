#include "Parse.h"

#include "EffectParser.h"
#include "../universe/Condition.h"
#include "../universe/Effect.h"
#include "../universe/ValueRef.h"
#include "../universe/UnlockableItem.h"
#include "../Empire/Government.h"
#include "../util/Directories.h"

#include <boost/phoenix.hpp>

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

    struct PolicyStrings {
        PolicyStrings() = default;
        PolicyStrings(std::string& name_, std::string& desc_,
                      std::string& short_desc_, std::string& cat_) :
            name(std::move(name_)),
            desc(std::move(desc_)),
            short_desc(std::move(short_desc_)),
            cat(std::move(cat_))
        {}

        std::string name;
        std::string desc;
        std::string short_desc;
        std::string cat;
    };

    struct policy_pod {
        policy_pod(PolicyStrings& strings_,
                   const boost::optional<parse::detail::value_ref_payload<double>>& adoption_cost_,
                   std::set<std::string>& prerequisites_,
                   std::set<std::string>& exclusions_,
                   const boost::optional<parse::effects_group_payload>& effects_,
                   boost::optional<std::vector<UnlockableItem>>& unlocked_items_,
                   std::string& graphic_) :
            name(std::move(strings_.name)),
            description(std::move(strings_.desc)),
            short_description(std::move(strings_.short_desc)),
            category(std::move(strings_.cat)),
            exclusions(std::move(exclusions_)),
            prerequisites(std::move(prerequisites_)),
            adoption_cost(adoption_cost_),
            effects(effects_),
            unlocked_items(std::move(unlocked_items_)),
            graphic(std::move(graphic_))
        {}

        std::string             name;
        std::string             description;
        std::string             short_description;
        std::string             category;
        std::set<std::string>   exclusions;
        std::set<std::string>   prerequisites;
        const boost::optional<parse::detail::value_ref_payload<double>> adoption_cost;
        const boost::optional<parse::effects_group_payload>&            effects;
        boost::optional<std::vector<UnlockableItem>>                    unlocked_items;
        std::string             graphic;
    };

    void insert_policy(std::vector<Policy>& policies, policy_pod policy_, bool& pass) {
        policies.emplace_back(std::move(policy_.name), std::move(policy_.description),
                              std::move(policy_.short_description), std::move(policy_.category),
                              (policy_.adoption_cost ? policy_.adoption_cost->OpenEnvelope(pass) : nullptr),
                              std::move(policy_.prerequisites), std::move(policy_.exclusions),
                              (policy_.effects ? OpenEnvelopes(*policy_.effects, pass) : std::vector<std::unique_ptr<Effect::EffectsGroup>>{}),
                              (policy_.unlocked_items ? std::move(*policy_.unlocked_items) : std::vector<UnlockableItem>{}),
                              std::move(policy_.graphic));
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

    using start_rule_payload = std::vector<Policy>;
    using start_rule_signature = void(start_rule_payload&);

    struct grammar : public parse::detail::grammar<start_rule_signature> {
        grammar(const parse::lexer& tok,
                const std::string& filename,
                const parse::text_iterator first, const parse::text_iterator last) :
            grammar::base_type(start),
            condition_parser(tok, label),
            string_grammar(tok, label, condition_parser),
            double_rules(tok, label, condition_parser, string_grammar),
            prerequisites(tok, label),
            exclusions(tok, label),
            effects_group_grammar(tok, label, condition_parser, string_grammar),
            unlockable_item_parser(tok, label),
            one_or_more_unlockable_items(unlockable_item_parser)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            using phoenix::construct;

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_5_type _5;
            qi::_6_type _6;
            qi::_7_type _7;
            qi::_pass_type _pass;
            qi::_r1_type _r1;
            qi::_val_type _val;
            qi::eps_type eps;

            policy_strings
                = (  tok.Policy_                                            // _1
                >    label(tok.name_)               > tok.string            // _2
                >    label(tok.description_)        > tok.string            // _3
                >    label(tok.short_description_)  > tok.string            // _4
                >    label(tok.category_)           > tok.string            // _5
                  ) [   _pass = is_unique_(_r1, _1, _2),
                        _val = construct<PolicyStrings>(_2, _3, _4, _5) ]
                ;

            policy
                = (  policy_strings(_r1)                                            // _1
                >    label(tok.adoptioncost_)       > double_rules.expr             // _2
                >    prerequisites                                                  // _3
                >    exclusions                                                     // _4
                >  -(label(tok.unlock_)             > one_or_more_unlockable_items) // _5
                >  -(label(tok.effectsgroups_)      > effects_group_grammar)        // _6
                >    label(tok.graphic_)            > tok.string)                   // _7
                [   insert_policy_(
                        _r1,
                        phoenix::construct<policy_pod>(
                            _1, _2, _3, _4, _6, _5, _7),
                        _pass
                    ) ]
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
        using unlocks_rule = parse::detail::rule<std::vector<UnlockableItem> ()>;
        using policy_strings_rule = parse::detail::rule<PolicyStrings (const start_rule_payload&)>;
        using unlockable_items = parse::detail::single_or_bracketed_repeat<parse::detail::unlockable_item_grammar>;

        parse::detail::Labeller                 label;
        parse::conditions_parser_grammar        condition_parser;
        const parse::string_parser_grammar      string_grammar;
        parse::double_parser_rules              double_rules;
        prereqs_grammar                         prerequisites;
        exclusions_grammar                      exclusions;
        parse::effects_group_grammar            effects_group_grammar;
        parse::detail::unlockable_item_grammar  unlockable_item_parser;
        unlockable_items                        one_or_more_unlockable_items;
        policy_strings_rule                     policy_strings;
        policy_rule                             policy;
        start_rule                              start;
    };
}


namespace parse {
    template <typename P>
    P policies(const boost::filesystem::path& path) {
        static_assert(std::is_same_v<P, std::vector<Policy>>);
        std::vector<Policy> policies_;

        ScopedTimer timer("Policies Parsing");

        for (const auto& file : ListDir(path, IsFOCScript))
            detail::parse_file<grammar, start_rule_payload>(GetLexer(), file, policies_);

        return policies_;
    }
}

// explicitly instantiate policies parser.
// This allows Tech.h to only be included in this .cpp file and not Parse.h
// which recompiles all parsers if Tech.h changes.
template FO_PARSE_API std::vector<Policy> parse::policies<std::vector<Policy>>(
    const boost::filesystem::path& path);

