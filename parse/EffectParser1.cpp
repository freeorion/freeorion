#include "EffectParser1.h"

#include "../universe/Effect.h"
#include "../universe/ValueRef.h"
#include "../universe/Enums.h"

#include <boost/spirit/include/phoenix.hpp>
#include <boost/phoenix/function/adapt_function.hpp>

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

namespace parse { namespace detail {
    using PassedMessageParams =  std::vector<std::pair<std::string,
                                                       MovableEnvelope<ValueRef::ValueRefBase<std::string>>>>;

    effect_payload construct_GenerateSitRepMessage1(
        const std::string& message_string, const std::string& icon,
        const PassedMessageParams& message_parameters,
        const MovableEnvelope<ValueRef::ValueRefBase<int>>& recipient_empire_id,
        EmpireAffiliationType affiliation,
        const std::string label,
        bool stringtable_lookup,
        bool& pass)
    {
        return effect_payload(
            new Effect::GenerateSitRepMessage(
                message_string,
                icon,
                OpenEnvelopes(message_parameters, pass),
                recipient_empire_id.OpenEnvelope(pass),
                affiliation,
                label,
                stringtable_lookup
            )
        );
    }

    effect_payload construct_GenerateSitRepMessage2(
        const std::string& message_string, const std::string& icon,
        const PassedMessageParams& message_parameters,
        EmpireAffiliationType affiliation,
        const parse::detail::condition_payload& condition,
        const std::string label,
        bool stringtable_lookup,
        bool& pass)
    {
        return effect_payload(
            new Effect::GenerateSitRepMessage(
                message_string,
                icon,
                OpenEnvelopes(message_parameters, pass),
                affiliation,
                condition.OpenEnvelope(pass),
                label,
                stringtable_lookup
            )
        );
    }

    effect_payload construct_GenerateSitRepMessage3(
        const std::string& message_string, const std::string& icon,
        const PassedMessageParams& message_parameters,
        EmpireAffiliationType affiliation,
        const std::string& label,
        bool stringtable_lookup,
        bool& pass)
    {
        return effect_payload(
            new Effect::GenerateSitRepMessage(
                message_string,
                icon,
                OpenEnvelopes(message_parameters, pass),
                affiliation,
                label,
                stringtable_lookup
            )
        );
    }

    BOOST_PHOENIX_ADAPT_FUNCTION(effect_payload, construct_GenerateSitRepMessage1_, construct_GenerateSitRepMessage1, 8)
    BOOST_PHOENIX_ADAPT_FUNCTION(effect_payload, construct_GenerateSitRepMessage2_, construct_GenerateSitRepMessage2, 8)
    BOOST_PHOENIX_ADAPT_FUNCTION(effect_payload, construct_GenerateSitRepMessage3_, construct_GenerateSitRepMessage3, 7)


    effect_parser_rules_1::effect_parser_rules_1(
        const parse::lexer& tok,
        Labeller& labeller,
        const condition_parser_grammar& condition_parser,
        const value_ref_grammar<std::string>& string_grammar
    ) :
        effect_parser_rules_1::base_type(start, "effect_parser_rules_1"),
        int_rules(tok, labeller, condition_parser, string_grammar),
        double_rules(tok, labeller, condition_parser, string_grammar),
        empire_affiliation_type_enum(tok)
    {
        qi::_1_type _1;
        qi::_a_type _a;
        qi::_b_type _b;
        qi::_c_type _c;
        qi::_d_type _d;
        qi::_e_type _e;
        qi::_f_type _f;
        qi::_val_type _val;
        qi::eps_type eps;
        qi::_pass_type _pass;
        using phoenix::new_;
        using phoenix::construct;
        using phoenix::push_back;
        const boost::phoenix::function<construct_movable> construct_movable_;
        const boost::phoenix::function<deconstruct_movable> deconstruct_movable_;

        set_empire_meter_1
            =    (tok.SetEmpireMeter_ >>   labeller.rule(Empire_token))
            >    int_rules.expr [ _b = _1 ]
            >    labeller.rule(Meter_token)  >  tok.string [ _a = _1 ]
            >    labeller.rule(Value_token)  >  double_rules.expr [ _val = construct_movable_(new_<Effect::SetEmpireMeter>(
                deconstruct_movable_(_b, _pass),
                _a,
                deconstruct_movable_(_1, _pass))) ]
            ;

        set_empire_meter_2
            =    (tok.SetEmpireMeter_ >>   labeller.rule(Meter_token))
            >    tok.string [ _a = _1 ]
            >    labeller.rule(Value_token) >  double_rules.expr [ _val = construct_movable_(new_<Effect::SetEmpireMeter>(
                _a,
                deconstruct_movable_(_1, _pass))) ]
            ;

        give_empire_tech
            =   (   tok.GiveEmpireTech_
                    >   labeller.rule(Name_token) >      string_grammar [ _d = _1 ]
                    > -(labeller.rule(Empire_token) >    int_rules.expr    [ _b = _1 ])
                ) [ _val = construct_movable_(new_<Effect::GiveEmpireTech>(
                    deconstruct_movable_(_d, _pass),
                    deconstruct_movable_(_b, _pass))) ]
            ;

        set_empire_tech_progress
            =    tok.SetEmpireTechProgress_
            >    labeller.rule(Name_token)     >  string_grammar [ _a = _1 ]
            >    labeller.rule(Progress_token) >  double_rules.expr [ _b = _1 ]
            >    (
                (labeller.rule(Empire_token) > int_rules.expr [ _val = construct_movable_(new_<Effect::SetEmpireTechProgress>(
                        deconstruct_movable_(_a, _pass),
                        deconstruct_movable_(_b, _pass),
                        deconstruct_movable_(_1, _pass))) ])
                |  eps [ _val = construct_movable_(new_<Effect::SetEmpireTechProgress>(
                        deconstruct_movable_(_a, _pass),
                        deconstruct_movable_(_b, _pass))) ]
            )
            ;

        // Note: the NoStringtableLookup flag controls the lookup both of template in Vartext and of the label in SitrepPanel.
        generate_sitrep_message
            =    tok.GenerateSitrepMessage_
            >    labeller.rule(Message_token)    >  tok.string [ _a = _1 ]
            >    labeller.rule(Label_token)      >  tok.string [ _e = _1 ]
            >   (
                tok.NoStringtableLookup_ [ _f = false ]
                | eps [ _f = true ]
            )
            >  -(labeller.rule(Icon_token)       >  tok.string [ _b = _1 ] )
            >  -(labeller.rule(Parameters_token) >  string_and_string_ref_vector [_c = _1] )
            >   (
                (   // empire id specified, optionally with an affiliation type:
                    // useful to specify a single recipient empire, or the allies
                    // or enemies of a single empire
                    ((   (labeller.rule(Affiliation_token) > empire_affiliation_type_enum [ _d = _1 ])
                         |    eps [ _d = AFFIL_SELF ]
                     )
                     >>  labeller.rule(Empire_token)
                    ) > int_rules.expr

                    [ _val = construct_GenerateSitRepMessage1_(_a, _b, _c, _1, _d, _e, _f, _pass) ]
                )
                |   (   // condition specified, with an affiliation type of CanSee:
                    // used to specify CanSee affiliation
                    (labeller.rule(Affiliation_token) >>  tok.CanSee_)
                    >   labeller.rule(Condition_token)   >   condition_parser
                    [ _val = construct_GenerateSitRepMessage2_(_a, _b, _c, AFFIL_CAN_SEE, _1, _e, _f, _pass) ]
                )
                |   (   // condition specified, with an affiliation type of CanSee:
                    // used to specify CanSee affiliation
                    (labeller.rule(Affiliation_token) >>  tok.Human_)
                    >   labeller.rule(Condition_token)   >   condition_parser
                    [ _val = construct_GenerateSitRepMessage2_(_a, _b, _c, AFFIL_HUMAN, _1, _e, _f, _pass) ]
                )
                |   (   // no empire id or condition specified, with or without an
                    // affiliation type: useful to specify no or all empires
                    (   (labeller.rule(Affiliation_token) > empire_affiliation_type_enum [ _d = _1 ])
                        |    eps [ _d = AFFIL_ANY ]
                    )
                    [ _val = construct_GenerateSitRepMessage3_(
                            _a, _b, _c,
                            _d, _e, _f, _pass) ]
                )

            )
            ;

        set_overlay_texture
            =    tok.SetOverlayTexture_
            >    labeller.rule(Name_token)    > tok.string [ _a = _1 ]
            >    labeller.rule(Size_token)    > double_rules.expr [ _val = construct_movable_(new_<Effect::SetOverlayTexture>(
                _a,
                deconstruct_movable_(_1, _pass))) ]
            ;

        string_and_string_ref
            =
            (
                labeller.rule(Tag_token)  >  tok.string [ _a = _1 ] >
                labeller.rule(Data_token)
                >  ( int_rules.expr      [ _b = construct_movable_(new_<ValueRef::StringCast<int>>(deconstruct_movable_(_1, _pass))) ]
                     | double_rules.expr [ _b = construct_movable_(new_<ValueRef::StringCast<double>>(deconstruct_movable_(_1, _pass))) ]
                     | tok.string        [ _b = construct_movable_(new_<ValueRef::Constant<std::string>>(_1)) ]
                     | string_grammar    [ _b = _1 ]
                   )
            )
            [_val = construct<string_and_string_ref_pair>(_a, _b)]
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

} }
