#include "EffectParserImpl.h"

#include "ConditionParserImpl.h"
#include "EnumParser.h"
#include "Label.h"
#include "ValueRefParser.h"
#include "../universe/Effect.h"
//#include "../universe/ValueRef.h"

#include <boost/spirit/home/phoenix.hpp>

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
            qi::_val_type _val;
            qi::eps_type eps;
            using phoenix::new_;
            using phoenix::construct;
            using phoenix::push_back;

            const parse::lexer& tok =                                                       parse::lexer::instance();

            const parse::value_ref_parser_rule<int>::type& int_value_ref =                  parse::value_ref_parser<int>();
            const parse::value_ref_parser_rule<double>::type& double_value_ref =            parse::value_ref_parser<double>();
            const parse::value_ref_parser_rule<std::string>::type& string_value_ref =       parse::value_ref_parser<std::string>();

            set_empire_meter_1
                =    tok.SetEmpireMeter_
                >>   parse::label(Empire_name) >> int_value_ref [ _b = _1 ]
                >    parse::label(Meter_name)  >  tok.string [ _a = _1 ]
                >    parse::label(Value_name)  >  double_value_ref [ _val = new_<Effect::SetEmpireMeter>(_b, _a, _1) ]
                ;

            set_empire_meter_2
                =    tok.SetEmpireMeter_
                >>   parse::label(Meter_name) >> tok.string [ _a = _1 ]
                >    parse::label(Value_name) >  double_value_ref [ _val = new_<Effect::SetEmpireMeter>(_a, _1) ]
                ;

            give_empire_tech
                =    tok.GiveEmpireTech_
                >>   parse::label(Name_name) >>     tok.string [ _a = _1 ]
                >>   (
                        (
                            parse::label(Empire_name) >> int_value_ref [ _val = new_<Effect::GiveEmpireTech>(_a, _1) ]
                        )
                     |  eps [ _val = new_<Effect::GiveEmpireTech>(_a) ]
                     )
                ;

            set_empire_tech_progress
                =    tok.SetEmpireTechProgress_
                >>   parse::label(Name_name) >>     string_value_ref [ _a = _1 ]
                >>   parse::label(Progress_name) >> double_value_ref [ _b = _1 ]
                >>   (
                        (
                            parse::label(Empire_name) >> int_value_ref [ _val = new_<Effect::SetEmpireTechProgress>(_a, _b, _1) ]
                        )
                     |  eps [ _val = new_<Effect::SetEmpireTechProgress>(_a, _b) ]
                     )
                ;

            generate_sitrep_message
                =    tok.GenerateSitrepMessage_
                >    parse::label(Message_name) >> tok.string [ _a = _1 ]
                >> -(
                        parse::label(Icon_name) >> tok.string [ _b = _1 ]
                    )
                >> -(
                        parse::label(Parameters_name) >> string_and_string_ref_vector [ _c = _1 ]
                    )
                >>  (
                        (
                            (
                                parse::label(Affiliation_name) >> parse::enum_parser<EmpireAffiliationType>() [ _d = _1 ]
                            |   eps [ _d = AFFIL_SELF ]
                            )
                        >>  parse::label(Empire_name) >> int_value_ref [ _val = new_<Effect::GenerateSitRepMessage>(_a, _b, _c, _1, _d) ]
                        )
                    |   (
                            parse::label(Affiliation_name) >> parse::enum_parser<EmpireAffiliationType>() [ _d = _1 ]
                        |   eps [ _d = AFFIL_ANY ]
                        )
                        [ _val = new_<Effect::GenerateSitRepMessage>(_a, _b, _c, _d) ]
                    )
                ;

            set_overlay_texture
                =    tok.SetOverlayTexture_
                >    parse::label(Name_name)    > tok.string [ _a = _1 ]
                >    parse::label(Size_name)    > double_value_ref [ _val = new_<Effect::SetOverlayTexture>(_a, _1) ]
                ;

            string_and_string_ref // TODO: Try to make this simpler.
                =    parse::label(Tag_name)  >> tok.string [ _a = _1 ]
                >>   parse::label(Data_name) >> string_value_ref [ _val = construct<string_and_string_ref_pair>(_a, _1) ]
                ;

            string_and_string_ref_vector
                =    '[' >> *string_and_string_ref [ push_back(_val, _1) ] >> ']'
                |    string_and_string_ref [ push_back(_val, _1) ]
                ;

            start
                %=   set_empire_meter_1
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

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Effect::EffectBase* (),
            qi::locals<ValueRef::ValueRefBase< ::PlanetType>*>,
            parse::skipper_type
        > create_planet_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Effect::EffectBase* (),
            qi::locals<
                std::string,
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<int>*
            >,
            parse::skipper_type
        > string_and_intref_and_intref_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Effect::EffectBase* (),
            qi::locals<
                ValueRef::ValueRefBase<std::string>*,
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<double>*
            >,
            parse::skipper_type
        > stringref_and_doubleref_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Effect::EffectBase* (),
            qi::locals<
                std::string,
                std::string,
                std::vector<std::pair<std::string, const ValueRef::ValueRefBase<std::string>*> >,
                EmpireAffiliationType
            >,
            parse::skipper_type
        > generate_sitrep_message_rule;

        typedef std::pair<std::string, const ValueRef::ValueRefBase<std::string>*> string_and_string_ref_pair;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            string_and_string_ref_pair (),
            qi::locals<std::string>, // TODO: Consider making this an adobe::name_t, and removing the quotes in the script source files.
            parse::skipper_type
        > string_and_string_ref_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            std::vector<string_and_string_ref_pair> (),
            parse::skipper_type
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
