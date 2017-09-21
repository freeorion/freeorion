#include "EffectParserImpl.h"

#include "ParseImpl.h"
#include "ConditionParserImpl.h"
#include "EnumParser.h"
#include "ValueRefParser.h"
#include "../universe/Effect.h"
#include "../universe/ValueRef.h"
#include "../universe/Enums.h"

#include <boost/spirit/include/phoenix.hpp>

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

namespace {
    struct effect_parser_rules_1 {
        effect_parser_rules_1() {
            qi::_1_type _1;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_c_type _c;
            qi::_d_type _d;
            qi::_e_type _e;
            qi::_f_type _f;
            qi::_val_type _val;
            qi::eps_type eps;
            using phoenix::new_;
            using phoenix::construct;
            using phoenix::push_back;

            const parse::lexer& tok = parse::lexer::instance();

            set_empire_meter_1
                =    (tok.SetEmpireMeter_ >>   parse::detail::label(Empire_token))
                >    parse::int_value_ref() [ _b = _1 ]
                >    parse::detail::label(Meter_token)  >  tok.string [ _a = _1 ]
                >    parse::detail::label(Value_token)  >  parse::double_value_ref() [ _val = new_<Effect::SetEmpireMeter>(_b, _a, _1) ]
                ;

            set_empire_meter_2
                =    (tok.SetEmpireMeter_ >>   parse::detail::label(Meter_token))
                >    tok.string [ _a = _1 ]
                >    parse::detail::label(Value_token) >  parse::double_value_ref() [ _val = new_<Effect::SetEmpireMeter>(_a, _1) ]
                ;

            give_empire_tech
                =   (   tok.GiveEmpireTech_
                    >   parse::detail::label(Name_token) >      parse::string_value_ref() [ _d = _1 ]
                    > -(parse::detail::label(Empire_token) >    parse::int_value_ref()    [ _b = _1 ])
                    ) [ _val = new_<Effect::GiveEmpireTech>(_d, _b) ]
                ;

            set_empire_tech_progress
                =    tok.SetEmpireTechProgress_
                >    parse::detail::label(Name_token)     >  parse::string_value_ref() [ _a = _1 ]
                >    parse::detail::label(Progress_token) >  parse::double_value_ref() [ _b = _1 ]
                >    (
                        (parse::detail::label(Empire_token) > parse::int_value_ref() [ _val = new_<Effect::SetEmpireTechProgress>(_a, _b, _1) ])
                     |  eps [ _val = new_<Effect::SetEmpireTechProgress>(_a, _b) ]
                     )
                ;

            // Note: the NoStringtableLookup flag controls the lookup both of template in Vartext and of the label in SitrepPanel.
            generate_sitrep_message
                =    tok.GenerateSitrepMessage_
                >    parse::detail::label(Message_token)    >  tok.string [ _a = _1 ]
                >    parse::detail::label(Label_token)      >  tok.string [ _e = _1 ]
                >   (
                       tok.NoStringtableLookup_ [ _f = false ]
                     | eps [ _f = true ]
                    )
                >  -(parse::detail::label(Icon_token)       >  tok.string [ _b = _1 ] )
                >  -(parse::detail::label(Parameters_token) >  string_and_string_ref_vector [ _c = _1 ] )
                >   (
                        (   // empire id specified, optionally with an affiliation type:
                            // useful to specify a single recipient empire, or the allies
                            // or enemies of a single empire
                            ((   (parse::detail::label(Affiliation_token) > parse::empire_affiliation_type_enum() [ _d = _1 ])
                             |    eps [ _d = AFFIL_SELF ]
                             )
                            >>  parse::detail::label(Empire_token)
                            ) > parse::int_value_ref()
                            [ _val = new_<Effect::GenerateSitRepMessage>(_a, _b, _c, _1, _d, _e, _f) ]
                        )
                    |   (   // condition specified, with an affiliation type of CanSee:
                            // used to specify CanSee affiliation
                        (parse::detail::label(Affiliation_token) >>  tok.CanSee_)
                        >   parse::detail::label(Condition_token)   >   parse::detail::condition_parser
                            [ _val = new_<Effect::GenerateSitRepMessage>(_a, _b, _c, AFFIL_CAN_SEE, _1, _e, _f) ]
                        )
                    |   (   // condition specified, with an affiliation type of CanSee:
                            // used to specify CanSee affiliation
                        (parse::detail::label(Affiliation_token) >>  tok.Human_)
                        >   parse::detail::label(Condition_token)   >   parse::detail::condition_parser
                            [ _val = new_<Effect::GenerateSitRepMessage>(_a, _b, _c, AFFIL_HUMAN, _1, _e, _f) ]
                        )
                    |   (   // no empire id or condition specified, with or without an
                            // affiliation type: useful to specify no or all empires
                            (   (parse::detail::label(Affiliation_token) > parse::empire_affiliation_type_enum() [ _d = _1 ])
                            |    eps [ _d = AFFIL_ANY ]
                            )
                            [ _val = new_<Effect::GenerateSitRepMessage>(_a, _b, _c, _d, _e, _f) ]
                        )
                    )
                ;

            set_overlay_texture
                =    tok.SetOverlayTexture_
                >    parse::detail::label(Name_token)    > tok.string [ _a = _1 ]
                >    parse::detail::label(Size_token)    > parse::double_value_ref() [ _val = new_<Effect::SetOverlayTexture>(_a, _1) ]
                ;

            string_and_string_ref
                =    parse::detail::label(Tag_token)  >  tok.string [ _a = _1 ]
                >    parse::detail::label(Data_token)
                >  ( parse::int_value_ref()      [ _val = construct<string_and_string_ref_pair>(_a, new_<ValueRef::StringCast<int>>(_1)) ]
                   | parse::double_value_ref()   [ _val = construct<string_and_string_ref_pair>(_a, new_<ValueRef::StringCast<double>>(_1)) ]
                   | tok.string                  [ _val = construct<string_and_string_ref_pair>(_a, new_<ValueRef::Constant<std::string>>(_1)) ]
                   | parse::string_value_ref()   [ _val = construct<string_and_string_ref_pair>(_a, _1) ]
                   )
                ;

            string_and_string_ref_vector
                =    ('[' > *string_and_string_ref [ push_back(_val, _1) ] > ']')
                |     string_and_string_ref [ push_back(_val, _1) ]
                ;

            start
                =    set_empire_meter_1
                |    set_empire_meter_2
                |    give_empire_tech
                |    set_empire_tech_progress
                |    generate_sitrep_message
                |    set_overlay_texture
                ;

            set_empire_meter_1.name("SetEmpireMeter (w/empire ID)");
            set_empire_meter_2.name("SetEmpireMeter");
            give_empire_tech.name("GiveEmpireTech");
            set_empire_tech_progress.name("SetEmpireTechProgress");
            generate_sitrep_message.name("GenerateSitrepMessage");
            set_overlay_texture.name("SetOverlayTexture");
            string_and_string_ref.name("Tag and Data (string reference)");
            string_and_string_ref_vector.name("List of Tags and Data");


#if DEBUG_EFFECT_PARSERS
            debug(set_empire_meter_1);
            debug(set_empire_meter_2);
            debug(give_empire_tech);
            debug(set_empire_tech_progress);
            debug(generate_sitrep_message);
            debug(set_overlay_texture);
#endif
        }

        typedef parse::detail::rule<
            Effect::EffectBase* (),
            qi::locals<
                std::string,
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<std::string>*
            >
        > string_and_intref_and_intref_rule;

        typedef parse::detail::rule<
            Effect::EffectBase* (),
            qi::locals<
                ValueRef::ValueRefBase<std::string>*,
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<double>*
            >
        > stringref_and_doubleref_rule;

        typedef parse::detail::rule<
            Effect::EffectBase* (),
            qi::locals<
                std::string,
                std::string,
                std::vector<std::pair<std::string, ValueRef::ValueRefBase<std::string>*>>,
                EmpireAffiliationType,
                std::string,
                bool
            >
        > generate_sitrep_message_rule;

        typedef std::pair<std::string, ValueRef::ValueRefBase<std::string>*> string_and_string_ref_pair;

        typedef parse::detail::rule<
            string_and_string_ref_pair (),
            qi::locals<std::string>
        > string_and_string_ref_rule;

        typedef parse::detail::rule<
            std::vector<string_and_string_ref_pair> ()
        > string_and_string_ref_vector_rule;

        string_and_intref_and_intref_rule   set_empire_meter_1;
        string_and_intref_and_intref_rule   set_empire_meter_2;
        string_and_intref_and_intref_rule   give_empire_tech;
        stringref_and_doubleref_rule        set_empire_tech_progress;
        generate_sitrep_message_rule        generate_sitrep_message;
        string_and_intref_and_intref_rule   set_overlay_texture;
        string_and_string_ref_rule          string_and_string_ref;
        string_and_string_ref_vector_rule   string_and_string_ref_vector;
        parse::effect_parser_rule           start;
    };
}

namespace parse { namespace detail {
    const effect_parser_rule& effect_parser_1() {
        static effect_parser_rules_1 retval;
        return retval.start;
    }
} }
