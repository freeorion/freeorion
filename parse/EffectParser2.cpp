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
    struct effect_parser_rules_2 {
        effect_parser_rules_2() {
            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_c_type _c;
            qi::_d_type _d;
            qi::_e_type _e;
            qi::_r1_type _r1;
            qi::_val_type _val;
            qi::eps_type eps;
            using phoenix::new_;
            using phoenix::construct;
            using phoenix::push_back;

            const parse::lexer& tok =                                                       parse::lexer::instance();

            const parse::value_ref_parser_rule<int>::type& int_value_ref =                  parse::value_ref_parser<int>();
            const parse::value_ref_parser_rule<double>::type& double_value_ref =            parse::value_ref_parser<double>();
            const parse::value_ref_parser_rule<std::string>::type& string_value_ref =       parse::value_ref_parser<std::string>();
            const parse::value_ref_parser_rule<PlanetType>::type& planet_type_value_ref =   parse::value_ref_parser<PlanetType>();
            const parse::value_ref_parser_rule<PlanetSize>::type& planet_size_value_ref =   parse::value_ref_parser<PlanetSize>();

            set_meter
                =    parse::set_non_ship_part_meter_type_enum() [ _a = _1 ]
                >>   parse::label(Value_name) >> double_value_ref [ _val = new_<Effect::SetMeter>(_a, _1) ]
                ;

            set_ship_part_meter
                =    parse::set_ship_part_meter_type_enum() [ _a = _1 ]
                >>  (
                            set_ship_part_meter_suffix_1(_a) [ _val = _1 ]
                        |   set_ship_part_meter_suffix_2(_a) [ _val = _1 ]
                        |   set_ship_part_meter_suffix_3(_a) [ _val = _1 ]
                    )
                ;

            set_ship_part_meter_suffix_1
                =    parse::label(PartClass_name) >> parse::enum_parser<ShipPartClass>() [ _a = _1 ] // TODO: PartClass should match "Class" from ShipPartsParser.cpp.
                >    parse::label(Value_name)     >  double_value_ref [ _d = _1 ]
                >    parse::label(SlotType_name)  >  parse::enum_parser<ShipSlotType>() [ _val = new_<Effect::SetShipPartMeter>(_r1, _a, _d, _1) ]
                ;

            set_ship_part_meter_suffix_2
                =    parse::label(FighterType_name) >> parse::enum_parser<CombatFighterType>() [ _b = _1 ]
                >    parse::label(Value_name)       >  double_value_ref [ _d = _1 ]
                >    parse::label(SlotType_name)    >  parse::enum_parser<ShipSlotType>() [ _val = new_<Effect::SetShipPartMeter>(_r1, _b, _d, _1) ]
                ;

            set_ship_part_meter_suffix_3
                =    parse::label(PartName_name) > tok.string [ _c = _1 ]
                >    parse::label(Value_name)    > double_value_ref [ _d = _1 ]
                >    parse::label(SlotType_name) > parse::enum_parser<ShipSlotType>() [ _val = new_<Effect::SetShipPartMeter>(_r1, _c, _d, _1) ]
                ;

            set_empire_stockpile
                =   (
                            tok.SetEmpireTradeStockpile_ [ _a = RE_TRADE ]
                    )
                >>  (
                        (
                            parse::label(Empire_name) >> int_value_ref [ _b = _1 ]
                        >>  parse::label(Value_name)  >> double_value_ref [ _val = new_<Effect::SetEmpireStockpile>(_b, _a, _1) ]
                        )
                    |   (
                            parse::label(Value_name)  > double_value_ref [ _val = new_<Effect::SetEmpireStockpile>(_a, _1) ]
                        )
                    )
                ;

            set_empire_capital
                =    tok.SetEmpireCapital_
                >>  (
                        (
                            parse::label(Empire_name) >> int_value_ref [ _val = new_<Effect::SetEmpireCapital>(_1) ]
                        )
                    |   eps [ _val = new_<Effect::SetEmpireCapital>() ]
                    )
                ;

            set_planet_type
                =    tok.SetPlanetType_
                >    parse::label(Type_name) > planet_type_value_ref [ _val = new_<Effect::SetPlanetType>(_1) ]
                ;

            set_planet_size
                =    tok.SetPlanetSize_
                >    parse::label(PlanetSize_name) > planet_size_value_ref [ _val = new_<Effect::SetPlanetSize>(_1) ]
                ;

            set_species
                =    tok.SetSpecies_
                >    parse::label(Name_name) > string_value_ref [ _val = new_<Effect::SetSpecies>(_1) ]
                ;

            set_owner
                =    tok.SetOwner_
                >    parse::label(Empire_name) > int_value_ref [ _val = new_<Effect::SetOwner>(_1) ]
                ;

            start
                %=   set_meter
                |    set_ship_part_meter
                |    set_empire_stockpile
                |    set_empire_capital
                |    set_planet_type
                |    set_planet_size
                |    set_species
                |    set_owner
                ;

            set_meter.name("SetMeter");
            set_ship_part_meter.name("SetShipPartMeter");
            set_empire_stockpile.name("SetEmpireStockpile");
            set_empire_capital.name("SetEmpireCapital");
            set_planet_type.name("SetPlanetType");
            set_planet_size.name("SetPlanetSize");
            set_species.name("SetSpecies");
            set_owner.name("SetOwner");


#if DEBUG_EFFECT_PARSERS
            debug(set_meter);
            debug(set_ship_part_meter);
            debug(set_empire_stockpile);
            debug(set_empire_capital);
            debug(set_planet_type);
            debug(set_planet_size);
            debug(set_species);
            debug(set_owner);
#endif
        }

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Effect::EffectBase* (),
            qi::locals<MeterType>,
            parse::skipper_type
        > set_meter_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Effect::EffectBase* (MeterType),
            qi::locals<
                ShipPartClass,
                CombatFighterType,
                std::string,
                ValueRef::ValueRefBase<double>*
            >,
            parse::skipper_type
        > set_ship_part_meter_suffix_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Effect::EffectBase* (),
            qi::locals<
                ResourceType,
                ValueRef::ValueRefBase<int>*
            >,
            parse::skipper_type
        > set_empire_stockpile_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Effect::EffectBase* (),
            qi::locals<
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<double>*
            >,
            parse::skipper_type
        > doubles_rule;

        set_meter_rule                      set_meter;
        set_meter_rule                      set_ship_part_meter;
        set_ship_part_meter_suffix_rule     set_ship_part_meter_suffix_1;
        set_ship_part_meter_suffix_rule     set_ship_part_meter_suffix_2;
        set_ship_part_meter_suffix_rule     set_ship_part_meter_suffix_3;
        set_empire_stockpile_rule           set_empire_stockpile;
        parse::effect_parser_rule           set_empire_capital;
        parse::effect_parser_rule           set_planet_type;
        parse::effect_parser_rule           set_planet_size;
        parse::effect_parser_rule           set_species;
        parse::effect_parser_rule           set_owner;
        parse::effect_parser_rule           start;
    };
}

namespace parse { namespace detail {
    const effect_parser_rule& effect_parser_2() {
        static effect_parser_rules_2 retval;
        return retval.start;
    }
} }
