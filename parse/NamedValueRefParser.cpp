#include "Parse.h"

#include "ParseImpl.h"
#include "ConditionParser.h"
#include "ValueRefParser.h"
#include "EnumValueRefRules.h"

#include "MovableEnvelope.h"
#include "../universe/NamedValueRefManager.h"
#include "../universe/ValueRefs.h"
#include "../universe/ValueRef.h"
#include "../util/Directories.h"

#include <boost/spirit/include/qi.hpp>
#include <boost/phoenix/core.hpp>

#include <typeinfo>

#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::map<int, int>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::map<std::string, std::map<int, int>>&) { return os; }
}
#endif

namespace parse {

    template <typename T>
    void insert_named_ref(std::map<std::string, std::unique_ptr<ValueRef::ValueRefBase>, std::less<>>& named_refs,
                          const std::string& name,
                          const parse::detail::MovableEnvelope<ValueRef::ValueRef<T>>& ref_envelope,
                          bool& pass)
    {
        DebugLogger() << "Registering from named_values.focs.txt : " << name << " ! ValueRef<" << typeid(T).name() << ">";
        // Note: Other parsers might also register value refs, so the normal pending
        // mechanism does not suffice. So, we do not collect the value refs in the
        // given named_refs reference, but instead register directly here...
        auto vref = ref_envelope.OpenEnvelope(pass);
        // Signal to log an error if CurrentContent is used
        if constexpr (std::is_same_v<std::decay_t<T>, std::string>)
            vref->SetTopLevelContent(std::string{ValueRef::Constant<std::string>::no_current_content});
        ::RegisterValueRef<T>(name, std::move(vref));
    }

    BOOST_PHOENIX_ADAPT_FUNCTION(void, insert_named_ref_, insert_named_ref, 4)

    using start_rule_payload = NamedValueRefManager::NamedValueRefParseMap;
    using start_rule_signature = void(start_rule_payload&);

    struct grammar : public parse::detail::grammar<start_rule_signature> {
        grammar(const parse::lexer& tok, const std::string& filename,
                const parse::text_iterator first, const parse::text_iterator last) :
            grammar::base_type(start, "named_value_ref_grammar"),
            condition_parser(tok, label),
            planet_type_rules(tok, label, condition_parser),
            planet_environment_rules(tok, label, condition_parser),
            string_grammar(tok, label, condition_parser),
            int_rules(tok, label, condition_parser, string_grammar),
            double_rules(tok, label, condition_parser, string_grammar)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_pass_type _pass;
            qi::omit_type omit_;
            qi::_r1_type _r1;

            named_ref
               =
                    ((omit_[tok.Named_] >> tok.Integer_)
                   > label(tok.name_) > tok.string
                   > label(tok.value_) > qi::as<parse::detail::MovableEnvelope<ValueRef::ValueRef<int>>>()[int_rules.expr]
                    ) [ insert_named_ref_(_r1, _2, _3, _pass) ]
                    |
                    ((omit_[tok.Named_] >> tok.Real_)
                   > label(tok.name_) > tok.string
                   > label(tok.value_) > qi::as<parse::detail::MovableEnvelope<ValueRef::ValueRef<double>>>()[double_rules.expr]
                    ) [ insert_named_ref_(_r1, _2, _3, _pass) ]
                    |
                    ((omit_[tok.Named_] >> tok.String_)
                   > label(tok.name_) > tok.string
                   > label(tok.value_) > qi::as<parse::detail::MovableEnvelope<ValueRef::ValueRef<std::string>>>()[string_grammar.expr]
                    ) [ insert_named_ref_(_r1, _2, _3, _pass) ]
                    |
                    ((omit_[tok.Named_] >> tok.PlanetType_) > label(tok.name_) > tok.string > label(tok.value_) > planet_type_rules.expr
                    ) [ insert_named_ref_(_r1, _2, _3, _pass) ]
                    |
                    ((omit_[tok.Named_] >> tok.Environment_) > label(tok.name_) > tok.string > label(tok.value_) > planet_environment_rules.expr
                    ) [ insert_named_ref_(_r1, _2, _3, _pass) ] 
                ;


            start
              = +named_ref(_r1)
                ;

            named_ref.name("NamedValueRef");

#if DEBUG_PARSERS
            debug(named_ref);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        using named_value_ref_rule = parse::detail::rule<
            void (std::map<std::string, std::unique_ptr<ValueRef::ValueRefBase>, std::less<>>&)>;

        using start_rule = parse::detail::rule<start_rule_signature>;

        parse::detail::Labeller                         label;
        const parse::conditions_parser_grammar          condition_parser;
        parse::detail::planet_type_parser_rules         planet_type_rules;
        parse::detail::planet_environment_parser_rules  planet_environment_rules;
        const parse::string_parser_grammar              string_grammar;
        const parse::int_arithmetic_rules               int_rules;
        const parse::double_parser_rules                double_rules;
        named_value_ref_rule                            named_ref;
        start_rule                                      start;
    };
}

namespace parse {
    start_rule_payload named_value_refs(const boost::filesystem::path& path) {
        start_rule_payload named_value_refs;

        ScopedTimer timer("Named ValueRef Parsing");

        for (const auto& file : ListDir(path, IsFOCScript))
            detail::parse_file<grammar, start_rule_payload>(GetLexer(), file, named_value_refs);

        for (auto& k_v : named_value_refs)
            ErrorLogger() << "Should have not returned anything: named_value_refs : " << k_v.first;

        return named_value_refs;
    }
}
