#include "EffectParserImpl.h"

#include "ParseImpl.h"
#include "EnumParser.h"
#include "ValueRefParser.h"
#include "ValueRefParserImpl.h"
#include "ConditionParserImpl.h"
#include "../universe/Effect.h"
#include "../universe/Enums.h"

#include <boost/spirit/include/phoenix.hpp>

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

namespace parse { namespace detail {
    effect_parser_rules_2::effect_parser_rules_2(
        const parse::lexer& tok, Labeller& labeller,
        const parse::condition_parser_rule& condition_parser
    ) :
        effect_parser_rules_2::base_type(start, "effect_parser_rules_2"),
        int_rules(tok, condition_parser),
        double_rules(tok, condition_parser)
    {
        qi::_1_type _1;
        qi::_a_type _a;
        qi::_b_type _b;
        qi::_c_type _c;
        qi::_d_type _d;
        qi::_e_type _e;
        qi::_val_type _val;
        qi::eps_type eps;
        using phoenix::new_;

        set_meter =
            (
                /* has some overlap with parse::set_ship_part_meter_type_enum() so can't use '>' */
                parse::set_non_ship_part_meter_type_enum() [ _a = _1 ]
                >>  labeller.rule(Value_token)
            )
            >   double_rules.expr [ _c = _1 ]
            >   (
                (labeller.rule(AccountingLabel_token) > tok.string [ _val = new_<Effect::SetMeter>(_a, _c, _1) ] )
                |    eps [ _val = new_<Effect::SetMeter>(_a, _c) ]
            )
            ;

        set_ship_part_meter
            =    (parse::set_ship_part_meter_type_enum() [ _a = _1 ] >>   labeller.rule(PartName_token))   > parse::string_value_ref() [ _b = _1 ]
            >    labeller.rule(Value_token)      > double_rules.expr [ _val = new_<Effect::SetShipPartMeter>(_a, _b, _1) ]
            ;

        set_empire_stockpile
            =   tok.SetEmpireTradeStockpile_ [ _a = RE_TRADE ]
            >   (
                (   labeller.rule(Empire_token) > int_rules.expr [ _b = _1 ]
                    >   labeller.rule(Value_token)  > double_rules.expr [ _val = new_<Effect::SetEmpireStockpile>(_b, _a, _1) ]
                )
                |  (labeller.rule(Value_token)  > double_rules.expr [ _val = new_<Effect::SetEmpireStockpile>(_a, _1) ])
            )
            ;

        set_empire_capital
            =    tok.SetEmpireCapital_
            >   (
                (labeller.rule(Empire_token) > int_rules.expr [ _val = new_<Effect::SetEmpireCapital>(_1) ])
                |    eps [ _val = new_<Effect::SetEmpireCapital>() ]
            )
            ;

        set_planet_type
            =    tok.SetPlanetType_
            >    labeller.rule(Type_token) > parse::detail::planet_type_rules().expr [ _val = new_<Effect::SetPlanetType>(_1) ]
            ;

        set_planet_size
            =    tok.SetPlanetSize_
            >    labeller.rule(PlanetSize_token) > parse::detail::planet_size_rules().expr [ _val = new_<Effect::SetPlanetSize>(_1) ]
            ;

        set_species
            =    tok.SetSpecies_
            >    labeller.rule(Name_token) > parse::string_value_ref() [ _val = new_<Effect::SetSpecies>(_1) ]
            ;

        set_owner
            =    tok.SetOwner_
            >    labeller.rule(Empire_token) > int_rules.expr [ _val = new_<Effect::SetOwner>(_1) ]
            ;

        set_species_opinion
            =    tok.SetSpeciesOpinion_
            >    labeller.rule(Species_token) >   parse::string_value_ref() [ _a = _1 ]
            > (
                (   labeller.rule(Empire_token) >  int_rules.expr [ _c = _1 ]
                    >  labeller.rule(Opinion_token) > double_rules.expr
                    [ _val = new_<Effect::SetSpeciesEmpireOpinion>(_a, _c, _1) ])
                |
                (   labeller.rule(Species_token) > parse::string_value_ref() [ _b = _1 ]
                    >   labeller.rule(Opinion_token) > double_rules.expr
                    [ _val = new_<Effect::SetSpeciesSpeciesOpinion>(_a, _b, _1) ])
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
                            (   (labeller.rule(Affiliation_token) > parse::empire_affiliation_type_enum() [ _d = _1 ])
                                |    eps [ _d = AFFIL_SELF ]
                            )
                            >>  labeller.rule(Empire_token)
                        ) > int_rules.expr [ _b = _1 ]
                    )
                    |  (   // no empire id or condition specified, with or without an
                        // affiliation type: useful to specify no or all empires
                        (   (labeller.rule(Affiliation_token) > parse::empire_affiliation_type_enum() [ _d = _1 ])
                            |    eps [ _d = AFFIL_ANY ]
                        )
                    )
                )
                >  labeller.rule(Visibility_token) > parse::detail::visibility_rules().expr [ _c = _1 ]
                >-(labeller.rule(Condition_token) > condition_parser [ _e = _1 ])
            ) [ _val = new_<Effect::SetVisibility>(_c, _d, _b, _e) ]
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
