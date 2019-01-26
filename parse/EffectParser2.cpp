#include "EffectParser2.h"

#include "ValueRefParser.h"
#include "../universe/Effect.h"
#include "../universe/Enums.h"
#include "../universe/ValueRef.h"

#include <boost/spirit/include/phoenix.hpp>

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

namespace parse { namespace detail {
    effect_parser_rules_2::effect_parser_rules_2(
        const parse::lexer& tok, Labeller& label,
        const condition_parser_grammar& condition_parser,
        const value_ref_grammar<std::string>& string_grammar
    ) :
        effect_parser_rules_2::base_type(start, "effect_parser_rules_2"),
        int_rules(tok, label, condition_parser, string_grammar),
        double_rules(tok, label, condition_parser, string_grammar),
        visibility_rules(tok, label, condition_parser),
        planet_type_rules(tok, label, condition_parser),
        planet_size_rules(tok, label, condition_parser),
        empire_affiliation_type_enum(tok),
        set_non_ship_part_meter_type_enum(tok),
        set_ship_part_meter_type_enum(tok)
    {
        qi::_1_type _1;
        qi::_2_type _2;
        qi::_3_type _3;
        qi::_a_type _a;
        qi::_b_type _b;
        qi::_c_type _c;
        qi::_d_type _d;
        qi::_e_type _e;
        qi::_val_type _val;
        qi::eps_type eps;
        qi::_pass_type _pass;
        using phoenix::new_;
        using phoenix::construct;
        const boost::phoenix::function<construct_movable> construct_movable_;
        const boost::phoenix::function<deconstruct_movable> deconstruct_movable_;

        set_meter =
            ((
                /* has some overlap with set_ship_part_meter_type_enum so can't use '>' */
                set_non_ship_part_meter_type_enum [ _a = _1 ]
                >>  label(tok.Value_)
            )
            >   double_rules.expr [ _b = _1 ]
            > -(label(tok.AccountingLabel_) > tok.string) [ _c = _1]
            ) [ _val = construct_movable_(new_<Effect::SetMeter>(
                _a,
                deconstruct_movable_(_b, _pass),
                _c)) ]
            ;

        set_ship_part_meter
            = ((set_ship_part_meter_type_enum  >>   label(tok.PartName_))   > string_grammar
               >    label(tok.Value_)      > double_rules.expr
              ) [ _val = construct_movable_(new_<Effect::SetShipPartMeter>(
                  _1,
                  deconstruct_movable_(_2, _pass),
                  deconstruct_movable_(_3, _pass))) ]
            ;

        set_empire_stockpile
            =   tok.SetEmpireStockpile_ [ _a = RE_INDUSTRY ]
            >   (
                (   label(tok.Empire_) > int_rules.expr [ _b = _1 ]
                    >   label(tok.Value_)  > double_rules.expr [ _val = construct_movable_(new_<Effect::SetEmpireStockpile>(
                        deconstruct_movable_(_b, _pass),
                        _a,
                        deconstruct_movable_(_1, _pass))) ]
                )
                |  (label(tok.Value_)  > double_rules.expr [ _val = construct_movable_(new_<Effect::SetEmpireStockpile>(
                            _a,
                            deconstruct_movable_(_1, _pass))) ]
                       )
            )
            ;

        set_empire_capital
            =    tok.SetEmpireCapital_
            >   (
                (label(tok.Empire_) > int_rules.expr [ _val = construct_movable_(
                        new_<Effect::SetEmpireCapital>(deconstruct_movable_(_1, _pass))) ])
                |    eps [ _val = construct_movable_(new_<Effect::SetEmpireCapital>()) ]
            )
            ;

        set_planet_type
            =    tok.SetPlanetType_
            >    label(tok.Type_) > planet_type_rules.expr [ _val = construct_movable_(
                new_<Effect::SetPlanetType>(deconstruct_movable_(_1, _pass))) ]
            ;

        set_planet_size
            =    tok.SetPlanetSize_
            >    label(tok.PlanetSize_) > planet_size_rules.expr [
                _val = construct_movable_(new_<Effect::SetPlanetSize>(deconstruct_movable_(_1, _pass))) ]
            ;

        set_species
            =    tok.SetSpecies_
            >    label(tok.Name_) > string_grammar [
                _val = construct_movable_(new_<Effect::SetSpecies>(deconstruct_movable_(_1, _pass))) ]
            ;

        set_owner
            =    tok.SetOwner_
            >    label(tok.Empire_) > int_rules.expr [
                _val = construct_movable_(new_<Effect::SetOwner>(deconstruct_movable_(_1, _pass))) ]
            ;

        set_species_opinion
            =    tok.SetSpeciesOpinion_
            >    label(tok.Species_) >   string_grammar [ _a = _1 ]
            > (
                (   label(tok.Empire_) >  int_rules.expr [ _c = _1 ]
                    >  label(tok.Opinion_) > double_rules.expr
                    [ _val = construct_movable_(new_<Effect::SetSpeciesEmpireOpinion>(
                            deconstruct_movable_(_a, _pass),
                            deconstruct_movable_(_c, _pass),
                            deconstruct_movable_(_1, _pass))) ])
                |
                (   label(tok.Species_) > string_grammar [ _b = _1 ]
                    >   label(tok.Opinion_) > double_rules.expr
                    [ _val = construct_movable_(new_<Effect::SetSpeciesSpeciesOpinion>(
                            deconstruct_movable_(_a, _pass),
                            deconstruct_movable_(_b, _pass),
                            deconstruct_movable_(_1, _pass))) ])
            )
            ;

        set_visibility
            =    tok.SetVisibility_
            >   (
                  (
                    (   // empire id specified, optionally with an affiliation type:
                        // useful to specify a single recipient empire, or the allies
                        // or enemies of a single empire
                        (
                            (   (label(tok.Affiliation_) > empire_affiliation_type_enum [ _d = _1 ])
                                |    eps [ _d = AFFIL_SELF ]
                            )
                            >>  label(tok.Empire_)
                        ) > int_rules.expr [ _b = _1 ]
                    )
                    |  (   // no empire id or condition specified, with or without an
                        // affiliation type: useful to specify no or all empires
                        (   (label(tok.Affiliation_) > empire_affiliation_type_enum [ _d = _1 ])
                            |    eps [ _d = AFFIL_ANY ]
                        )
                    )
                )
                >  label(tok.Visibility_) > visibility_rules.expr [ _c = _1 ]
                >-(label(tok.Condition_) > condition_parser [ _e = _1 ])
            ) [ _val = construct_movable_(
                new_<Effect::SetVisibility>(deconstruct_movable_(_c, _pass),
                                            _d,
                                            deconstruct_movable_(_b, _pass),
                                            deconstruct_movable_(_e, _pass))) ]
            ;

        start
            %=  set_ship_part_meter
            |   set_meter
            |   set_empire_stockpile
            |   set_empire_capital
            |   set_planet_type
            |   set_planet_size
            |   set_species
            |   set_species_opinion
            |   set_owner
            |   set_visibility
            ;

        set_meter.name("SetMeter");
        set_ship_part_meter.name("SetShipPartMeter");
        set_empire_stockpile.name("SetEmpireStockpile");
        set_empire_capital.name("SetEmpireCapital");
        set_planet_type.name("SetPlanetType");
        set_planet_size.name("SetPlanetSize");
        set_species.name("SetSpecies");
        set_species_opinion.name("SetSpeciesOpinion");
        set_owner.name("SetOwner");
        set_visibility.name("SetVisibility");


#if DEBUG_EFFECT_PARSERS
        debug(set_meter);
        debug(set_ship_part_meter);
        debug(set_empire_stockpile);
        debug(set_empire_capital);
        debug(set_planet_type);
        debug(set_planet_size);
        debug(set_species);
        debug(set_species_opinion);
        debug(set_owner);
        debug(set_visibility);
#endif
    }

} }
