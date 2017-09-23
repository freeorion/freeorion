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

namespace {
    struct effect_parser_rules_2 {
        effect_parser_rules_2() {
            qi::_1_type _1;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_c_type _c;
            qi::_d_type _d;
            qi::_e_type _e;
            qi::_val_type _val;
            qi::eps_type eps;
            using phoenix::new_;

            const parse::lexer& tok = parse::lexer::instance();

            set_meter =
                (
                    /* has some overlap with parse::set_ship_part_meter_type_enum() so can't use '>' */
                    parse::set_non_ship_part_meter_type_enum() [ _a = _1 ]
                    >>  parse::detail::label(Value_token)
                )
                >   parse::double_value_ref() [ _c = _1 ]
                >   (
                        (parse::detail::label(AccountingLabel_token) > tok.string [ _val = new_<Effect::SetMeter>(_a, _c, _1) ] )
                    |    eps [ _val = new_<Effect::SetMeter>(_a, _c) ]
                    )
                ;

            set_ship_part_meter
                =    (parse::set_ship_part_meter_type_enum() [ _a = _1 ] >> parse::detail::label(PartName_token))   > parse::string_value_ref() [ _b = _1 ]
                >    parse::detail::label(Value_token) >                    parse::double_value_ref() [ _val = new_<Effect::SetShipPartMeter>(_a, _b, _1) ]
                ;

            set_empire_stockpile
                =   tok.SetEmpireTradeStockpile_ [ _a = RE_TRADE ]
                >   (
                        (   parse::detail::label(Empire_token) > parse::int_value_ref() [ _b = _1 ]
                        >   parse::detail::label(Value_token)  > parse::double_value_ref() [ _val = new_<Effect::SetEmpireStockpile>(_b, _a, _1) ]
                        )
                        |  (parse::detail::label(Value_token)  > parse::double_value_ref() [ _val = new_<Effect::SetEmpireStockpile>(_a, _1) ])
                    )
                ;

            set_empire_capital
                =    tok.SetEmpireCapital_
                >   (
                        (parse::detail::label(Empire_token) > parse::int_value_ref() [ _val = new_<Effect::SetEmpireCapital>(_1) ])
                    |    eps [ _val = new_<Effect::SetEmpireCapital>() ]
                    )
                ;

            set_planet_type
                =    tok.SetPlanetType_
                >    parse::detail::label(Type_token) > parse::detail::planet_type_rules().expr [ _val = new_<Effect::SetPlanetType>(_1) ]
                ;

            set_planet_size
                =    tok.SetPlanetSize_
                >    parse::detail::label(PlanetSize_token) > parse::detail::planet_size_rules().expr [ _val = new_<Effect::SetPlanetSize>(_1) ]
                ;

            set_species
                =    tok.SetSpecies_
                >    parse::detail::label(Name_token) > parse::string_value_ref() [ _val = new_<Effect::SetSpecies>(_1) ]
                ;

            set_owner
                =    tok.SetOwner_
                >    parse::detail::label(Empire_token) > parse::int_value_ref() [ _val = new_<Effect::SetOwner>(_1) ]
                ;

            set_species_opinion
                =    tok.SetSpeciesOpinion_
                >    parse::detail::label(Species_token) >    parse::string_value_ref() [ _a = _1 ]
                > (
                    (   parse::detail::label(Empire_token) >  parse::int_value_ref() [ _c = _1 ]
                     >  parse::detail::label(Opinion_token) > parse::double_value_ref()
                        [ _val = new_<Effect::SetSpeciesEmpireOpinion>(_a, _c, _1) ])
                   |
                    (   parse::detail::label(Species_token) > parse::string_value_ref() [ _b = _1 ]
                    >   parse::detail::label(Opinion_token) > parse::double_value_ref()
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
                                (   (parse::detail::label(Affiliation_token) > parse::empire_affiliation_type_enum() [ _d = _1 ])
                                |    eps [ _d = AFFIL_SELF ]
                                )
                            >>  parse::detail::label(Empire_token)
                            ) > parse::int_value_ref() [ _b = _1 ]
                        )
                     |  (   // no empire id or condition specified, with or without an
                            // affiliation type: useful to specify no or all empires
                            (   (parse::detail::label(Affiliation_token) > parse::empire_affiliation_type_enum() [ _d = _1 ])
                            |    eps [ _d = AFFIL_ANY ]
                            )
                        )
                     )
                    >  parse::detail::label(Visibility_token) > parse::detail::visibility_rules().expr [ _c = _1 ]
                    >-(parse::detail::label(Condition_token) > parse::detail::condition_parser [ _e = _1 ])
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

        typedef parse::detail::rule<
            Effect::EffectBase* (),
            qi::locals<
                MeterType,
                ValueRef::ValueRefBase<std::string>*,
                ValueRef::ValueRefBase<double>*,
                std::string
            >
        > set_meter_rule;

        typedef parse::detail::rule<
            Effect::EffectBase* (),
            qi::locals<
                ResourceType,
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<Visibility>*,
                EmpireAffiliationType,
                Condition::ConditionBase*
            >
        > set_stockpile_or_vis_rule;

        typedef parse::detail::rule<
            Effect::EffectBase* (),
            qi::locals<
                ValueRef::ValueRefBase<std::string>*,
                ValueRef::ValueRefBase<std::string>*,
                ValueRef::ValueRefBase<int>*
            >
        > string_string_int_rule;

        set_meter_rule                      set_meter;
        set_meter_rule                      set_ship_part_meter;
        set_stockpile_or_vis_rule           set_empire_stockpile;
        parse::effect_parser_rule           set_empire_capital;
        parse::effect_parser_rule           set_planet_type;
        parse::effect_parser_rule           set_planet_size;
        parse::effect_parser_rule           set_species;
        string_string_int_rule              set_species_opinion;
        parse::effect_parser_rule           set_owner;
        set_stockpile_or_vis_rule           set_visibility;
        parse::effect_parser_rule           start;
    };
}

namespace parse { namespace detail {
    const effect_parser_rule& effect_parser_2() {
        static effect_parser_rules_2 retval;
        return retval.start;
    }
} }
