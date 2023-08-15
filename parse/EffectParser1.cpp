#include "EffectParser1.h"

#include "../universe/Effects.h"
#include "../universe/ValueRef.h"
#include "../universe/Enums.h"

#include <boost/phoenix.hpp>
#include <boost/phoenix/function/adapt_function.hpp>

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

namespace parse { namespace detail {
    using PassedMessageParams =
        std::vector<std::pair<std::string, MovableEnvelope<ValueRef::ValueRef<std::string>>>>;

    effect_payload construct_GenerateSitRepMessage1(
        std::string& message_string, std::string& icon,
        const PassedMessageParams& message_parameters,
        const MovableEnvelope<ValueRef::ValueRef<int>>& recipient_empire_id,
        EmpireAffiliationType affiliation,
        std::string label,
        bool stringtable_lookup,
        bool& pass)
    {
        return effect_payload(
            std::make_unique<Effect::GenerateSitRepMessage>(
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
        std::string& message_string, std::string& icon,
        const PassedMessageParams& message_parameters,
        EmpireAffiliationType affiliation,
        const parse::detail::condition_payload& condition,
        std::string label,
        bool stringtable_lookup,
        bool& pass)
    {
        return effect_payload(
            std::make_unique<Effect::GenerateSitRepMessage>(
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
        std::string& message_string, std::string& icon,
        const PassedMessageParams& message_parameters,
        EmpireAffiliationType affiliation,
        std::string label,
        bool stringtable_lookup,
        bool& pass)
    {
        return effect_payload(
            std::make_unique<Effect::GenerateSitRepMessage>(
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
        Labeller& label,
        const condition_parser_grammar& condition_parser,
        const value_ref_grammar<std::string>& string_grammar
    ) :
        effect_parser_rules_1::base_type(start, "effect_parser_rules_1"),
        int_rules(tok, label, condition_parser, string_grammar),
        double_rules(tok, label, condition_parser, string_grammar),
        empire_affiliation_type_enum(tok),
        unlock_item_type_enum(tok),
        one_or_more_string_and_string_ref_pair(string_and_string_ref)
    {
        qi::_1_type _1;
        qi::_2_type _2;
        qi::_3_type _3;
        qi::_a_type _a;
        qi::_b_type _b;
        qi::_c_type _c;
        qi::_d_type _d;
        qi::_e_type _e;
        qi::_f_type _f;
        qi::_val_type _val;
        qi::eps_type eps;
        qi::_pass_type _pass;
        qi::omit_type omit_;
        using phoenix::new_;
        using phoenix::construct;
        using phoenix::push_back;
        const boost::phoenix::function<construct_movable> construct_movable_;
        const boost::phoenix::function<deconstruct_movable> deconstruct_movable_;

        set_empire_meter_1 =
            ((  omit_[tok.SetEmpireMeter_]  >>  label(tok.empire_))
              > int_rules.expr
              > label(tok.meter_)           >   tok.string
              > label(tok.value_)           >   double_rules.expr
            ) [ _val = construct_movable_(
                new_<Effect::SetEmpireMeter>(
                    deconstruct_movable_(_1, _pass),
                    _2,
                    deconstruct_movable_(_3, _pass))) ]
            ;

        set_empire_meter_2
            = (( omit_[tok.SetEmpireMeter_]
               >> label(tok.meter_))        >   tok.string
               >  label(tok.value_)         >   double_rules.expr
              ) [ _val = construct_movable_(new_<Effect::SetEmpireMeter>(
                   _1,
                   deconstruct_movable_(_2, _pass))) ]
            ;

        give_empire_content
            =   (   unlock_item_type_enum
                >   label(tok.name_)    >   string_grammar
                > -(label(tok.empire_)  >   int_rules.expr)
                ) [ _val = construct_movable_(new_<Effect::GiveEmpireContent>(
                    deconstruct_movable_(_2, _pass),
                    _1,
                    deconstruct_movable_(_3, _pass))) ]
            ;

        set_empire_tech_progress
            = (     omit_[tok.SetEmpireTechProgress_]
                >   label(tok.name_)        >   string_grammar
                >   label(tok.progress_)    >   double_rules.expr
                > -(label(tok.empire_)      >   int_rules.expr)
              ) [ _val = construct_movable_(new_<Effect::SetEmpireTechProgress>(
                        deconstruct_movable_(_1, _pass),
                        deconstruct_movable_(_2, _pass),
                        deconstruct_movable_(_3, _pass))) ]
            ;

        // Note: the NoStringtableLookup flag controls the lookup both of template in Vartext and of the label in SitrepPanel.
        generate_sitrep_message
            =    tok.GenerateSitRepMessage_
            >    label(tok.message_)        >   tok.string [ _a = _1 ]
            >    label(tok.label_)          >   tok.string [ _e = _1 ]
            >   (
                tok.NoStringtableLookup_ [ _f = false ]
                | eps [ _f = true ]
            )
            >  -(label(tok.icon_)           >   tok.string [ _b = _1 ] )
            >  -(label(tok.parameters_)     >   one_or_more_string_and_string_ref_pair [_c = _1] )
            >   (
                (   // empire id specified, optionally with an affiliation type:
                    // useful to specify a single recipient empire, or the allies
                    // or enemies of a single empire
                    ((   (label(tok.affiliation_) > empire_affiliation_type_enum [ _d = _1 ])
                         | eps [ _d = EmpireAffiliationType::AFFIL_SELF ]
                     )
                     >>  label(tok.empire_)
                    ) > int_rules.expr

                    [ _val = construct_GenerateSitRepMessage1_(_a, _b, _c, _1, _d, _e, _f, _pass) ]
                )
                |   (   // condition specified, with an affiliation type of CanSee:
                    // used to specify CanSee affiliation
                    (   label(tok.affiliation_)     >>  tok.CanSee_)
                    >   label(tok.condition_)       >   condition_parser
                    [ _val = construct_GenerateSitRepMessage2_(_a, _b, _c, EmpireAffiliationType::AFFIL_CAN_SEE, _1, _e, _f, _pass) ]
                )
                |   (   // condition specified, with an affiliation type of Human:
                    // used to specify Human affiliation
                    (   label(tok.affiliation_)     >>  tok.Human_)
                    >   label(tok.condition_)       >   condition_parser
                    [ _val = construct_GenerateSitRepMessage2_(_a, _b, _c, EmpireAffiliationType::AFFIL_HUMAN, _1, _e, _f, _pass) ]
                )
                |   (   // no empire id or condition specified, with or without an
                    // affiliation type: useful to specify no or all empires
                    (  (label(tok.affiliation_)     >   empire_affiliation_type_enum [ _d = _1 ])
                       | eps [ _d = EmpireAffiliationType::AFFIL_ANY ]
                    )
                    [ _val = construct_GenerateSitRepMessage3_(
                            _a, _b, _c,
                            _d, _e, _f, _pass) ]
                )

            )
            ;

        set_overlay_texture
            = ( omit_[tok.SetOverlayTexture_]
                > label(tok.name_)    > tok.string
                > label(tok.size_)    > double_rules.expr
              ) [ _val = construct_movable_(new_<Effect::SetOverlayTexture>(
                  _1,
                  deconstruct_movable_(_2, _pass))) ]
            ;

        string_and_string_ref
            =
            (
                label(tok.tag_)  >  tok.string [ _a = _1 ] >
                label(tok.data_)
                >  (   int_rules.expr    [ _b = construct_movable_(new_<ValueRef::StringCast<int>>(deconstruct_movable_(_1, _pass))) ]
                     | double_rules.expr [ _b = construct_movable_(new_<ValueRef::StringCast<double>>(deconstruct_movable_(_1, _pass))) ]
                     | tok.string        [ _b = construct_movable_(new_<ValueRef::Constant<std::string>>(_1)) ]
                     | string_grammar    [ _b = _1 ]
                   )
            )
            [_val = construct<string_and_string_ref_pair>(_a, _b)]
            ;

        start
            %=   set_empire_meter_1
            |    set_empire_meter_2
            |    give_empire_content
            |    set_empire_tech_progress
            |    generate_sitrep_message
            |    set_overlay_texture
            ;

        set_empire_meter_1.name("SetEmpireMeter (w/empire ID)");
        set_empire_meter_2.name("SetEmpireMeter");
        give_empire_content.name("GiveEmpireContent");
        set_empire_tech_progress.name("SetEmpireTechProgress");
        generate_sitrep_message.name("GenerateSitrepMessage");
        set_overlay_texture.name("SetOverlayTexture");
        string_and_string_ref.name("Tag and Data (string reference)");


#if DEBUG_EFFECT_PARSERS
        debug(set_empire_meter_1);
        debug(set_empire_meter_2);
        debug(give_empire_content);
        debug(set_empire_tech_progress);
        debug(generate_sitrep_message);
        debug(set_overlay_texture);
#endif
    }

} }
