#include "EffectParser.h"

#include "ConditionParserImpl.h"
#include "EnumParser.h"
#include "Label.h"
#include "ValueRefParser.h"
#include "../universe/Effect.h"
#include "../universe/ValueRef.h"

#include <GG/ReportParseError.h>

#include <boost/spirit/home/phoenix.hpp>


namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;


#define DEBUG_PARSERS 0

namespace {
    struct effect_parser_rules {
        effect_parser_rules() {
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
            const parse::value_ref_parser_rule<StarType>::type& star_type_value_ref =       parse::value_ref_parser<StarType>();

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

            set_empire_stockpile
                =   (
                            tok.SetEmpireFoodStockpile_ [ _a = RE_FOOD ]
                        |   tok.SetEmpireMineralStockpile_ [ _a = RE_MINERALS ]
                        |   tok.SetEmpireTradeStockpile_ [ _a = RE_TRADE ]
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

            create_planet
                =    tok.CreatePlanet_
                >    parse::label(Type_name)        > planet_type_value_ref [ _a = _1 ]
                >    parse::label(PlanetSize_name)  > planet_size_value_ref [ new_<Effect::CreatePlanet>(_a, _1) ]
                ;

            create_building
                =    tok.CreateBuilding_
                >    parse::label(Name_name) > string_value_ref [ _val = new_<Effect::CreateBuilding>(_1) ]
                ;

            create_ship_1
                =    tok.CreateShip_
                >>   parse::label(DesignName_name) >> int_value_ref [ _b = _1 ] // TODO: DesignName -> DesignID.
                >    parse::label(Empire_name)     >  int_value_ref [ _c = _1 ]
                >    parse::label(Species_name)    >  string_value_ref [ _val = new_<Effect::CreateShip>(_b, _c, _1) ]
                ;

            create_ship_2
                =    tok.CreateShip_
                >>   parse::label(DesignName_name) >> tok.string [ _a = _1 ]
                >>   parse::label(Empire_name)     >> int_value_ref [ _b = _1 ]
                >>   parse::label(Species_name)    >> string_value_ref [ _val = new_<Effect::CreateShip>(_a, _b, _1) ]
                ;

            create_ship_3
                =    tok.CreateShip_
                >>   parse::label(DesignName_name) >> tok.string [ _a = _1 ]
                >>   parse::label(Empire_name)     >> int_value_ref [ _val = new_<Effect::CreateShip>(_a, _1) ]
                ;

            create_ship_4
                =    tok.CreateShip_
                >    parse::label(DesignName_name) > tok.string [ _val = new_<Effect::CreateShip>(_1) ]
                ;

            move_to
                =    tok.MoveTo_
                >    parse::label(Destination_name) > parse::detail::condition_parser [ _val = new_<Effect::MoveTo>(_1) ]
                ;

            move_in_orbit
                =    tok.MoveInOrbit_
                >>  (
                        (
                            parse::label(Speed_name) >> double_value_ref[ _a = _1 ]
                        >>  parse::label(Focus_name) >> parse::detail::condition_parser [ _val = new_<Effect::MoveInOrbit>(_a, _1) ]
                        )
                    |   (
                            parse::label(Speed_name) >> double_value_ref [ _a = _1 ]
                        >>  parse::label(X_name) >>     double_value_ref [ _b = _1 ]
                        >>  parse::label(Y_name) >>     double_value_ref [ _val = new_<Effect::MoveInOrbit>(_a, _b, _1) ]
                        )
                    )
                ;

            set_destination
                =    tok.SetDestination_
                >    parse::label(Destination_name) > parse::detail::condition_parser [ _val = new_<Effect::SetDestination>(_1) ]
                ;

            destroy
                =    tok.Destroy_ [ _val = new_<Effect::Destroy>() ]
                ;

            victory
                =    tok.Victory_
                >    parse::label(Reason_name) > tok.string [ _val = new_<Effect::Victory>(_1) ]
                ;

            add_special
                =    tok.AddSpecial_
                >    parse::label(Name_name) > tok.string [ _val = new_<Effect::AddSpecial>(_1) ]
                ;

            remove_special
                =    tok.RemoveSpecial_
                >    parse::label(Name_name) > tok.string [ _val = new_<Effect::RemoveSpecial>(_1) ]
                ;

            add_starlanes
                =    tok.AddStarlanes_
                >    parse::label(Endpoint_name) > parse::detail::condition_parser [ _val = new_<Effect::AddStarlanes>(_1) ]
                ;

            remove_starlanes
                =    tok.RemoveStarlanes_
                >    parse::label(Endpoint_name) > parse::detail::condition_parser [ _val = new_<Effect::RemoveStarlanes>(_1) ]
                ;

            set_star_type
                =    tok.SetStarType_
                >    parse::label(Type_name) > star_type_value_ref [ _val = new_<Effect::SetStarType>(_1) ]
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
                >>   parse::label(Name_name) >>     tok.string [ _a = _1 ]
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
                >    parse::label(Message_name) > tok.string [ _a = _1 ]
                >> -(
                        parse::label(Parameters_name) >> string_and_string_ref_vector [ _b = _1 ]
                    )
                >>  (
                        (
                            (
                                parse::label(Affiliation_name) >> parse::enum_parser<EmpireAffiliationType>() [ _c = _1 ]
                            |   eps [ _c = AFFIL_SELF ]
                            )
                        >>  parse::label(Empire_name) >> int_value_ref [ _val = new_<Effect::GenerateSitRepMessage>(_a, _b, _1, _c) ]
                        )
                    |   (
                            parse::label(Affiliation_name) >> parse::enum_parser<EmpireAffiliationType>() [ _c = _1 ]
                        |   eps [ _c = AFFIL_ANY ]
                        )
                        [ _val = new_<Effect::GenerateSitRepMessage>(_a, _b, _c) ]
                    )
                ;

            set_overlay_texture
                =    tok.SetOverlayTexture_
                >    parse::label(Name_name)    > tok.string [ _a = _1 ]
                >>  (
                        parse::label(Size_name) >> double_value_ref [ _val = new_<Effect::SetOverlayTexture>(_a, _1) ]
                    |   eps [ _val = new_<Effect::SetOverlayTexture>(_a) ]
                    )
                ;

            string_and_string_ref // TODO: Try to make this simpler.
                =    parse::label(Tag_name)  >> tok.string [ _a = _1 ]
                >>   parse::label(Data_name) >  string_value_ref [ _val = construct<string_and_string_ref_pair>(_a, _1) ]
                ;

            string_and_string_ref_vector
                =    '[' > +string_and_string_ref [ push_back(_val, _1) ] > ']'
                |    string_and_string_ref [ push_back(_val, _1) ]
                ;

            start
                %=   set_meter
                |    set_ship_part_meter
                |    set_empire_meter_1
                |    set_empire_meter_2
                |    set_empire_stockpile
                |    set_empire_capital
                |    set_planet_type
                |    set_planet_size
                |    set_species
                |    set_owner
                |    create_planet
                |    create_building
                |    create_ship_1
                |    create_ship_2
                |    create_ship_3
                |    create_ship_4
                |    move_to
                |    move_in_orbit
                |    set_destination
                |    destroy
                |    victory
                |    add_special
                |    remove_special
                |    add_starlanes
                |    remove_starlanes
                |    set_star_type
                |    give_empire_tech
                |    set_empire_tech_progress
                |    generate_sitrep_message
                |    set_overlay_texture
                ;

            set_meter.name("SetMeter");
            set_ship_part_meter.name("SetShipPartMeter");
            set_empire_meter_1.name("SetEmpireMeter (w/empire ID)");
            set_empire_meter_2.name("SetEmpireMeter");
            set_empire_stockpile.name("SetEmpireStockpile");
            set_empire_capital.name("SetEmpireCapital");
            set_planet_type.name("SetPlanetType");
            set_planet_size.name("SetPlanetSize");
            set_species.name("SetSpecies");
            set_owner.name("SetOwner");
            create_planet.name("CreatePlanet");
            create_building.name("CreateBuilding");
            create_ship_1.name("CreateShip (int DesignID)");
            create_ship_2.name("CreateShip (empire and species)");
            create_ship_3.name("CreateShip (string DesignName and empire)");
            create_ship_4.name("CreateShip (string DesignName only)");
            move_to.name("MoveTo");
            move_in_orbit.name("MoveInOrbit");
            set_destination.name("SetDestination");
            destroy.name("Destroy");
            victory.name("Victory");
            add_special.name("AddSpecial");
            remove_special.name("RemoveSpecial");
            add_starlanes.name("AddStarlanes");
            remove_starlanes.name("RemoveStarlanes");
            set_star_type.name("SetStarType");
            give_empire_tech.name("GiveEmpireTech");
            set_empire_tech_progress.name("SetEmpireTechProgress");
            generate_sitrep_message.name("GenerateSitrepMessage");
            set_overlay_texture.name("SetOverlayTexture");

#if DEBUG_PARSER
            debug(set_meter);
            debug(set_ship_part_meter);
            debug(set_empire_meter_1);
            debug(set_empire_meter_2);
            debug(set_empire_stockpile);
            debug(set_empire_capital);
            debug(set_planet_type);
            debug(set_planet_size);
            debug(set_species);
            debug(set_owner);
            debug(create_planet);
            debug(create_building);
            debug(create_ship_1);
            debug(create_ship_2);
            debug(create_ship_3);
            debug(create_ship_4);
            debug(move_to);
            debug(set_destination);
            debug(destroy);
            debug(victory);
            debug(add_special);
            debug(remove_special);
            debug(add_starlanes);
            debug(remove_starlanes);
            debug(set_star_type);
            debug(give_empire_tech);
            debug(set_empire_tech_progress);
            debug(generate_sitrep_message);
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
                std::string,
                ValueRef::ValueRefBase<double>*
            >,
            parse::skipper_type
        > string_and_doubleref_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Effect::EffectBase* (),
            qi::locals<
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<double>*
            >,
            parse::skipper_type
        > doubles_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Effect::EffectBase* (),
            qi::locals<
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

        set_meter_rule                      set_meter;
        set_meter_rule                      set_ship_part_meter;
        string_and_intref_and_intref_rule   set_empire_meter_1;
        string_and_intref_and_intref_rule   set_empire_meter_2;
        set_ship_part_meter_suffix_rule     set_ship_part_meter_suffix_1;
        set_ship_part_meter_suffix_rule     set_ship_part_meter_suffix_2;
        set_ship_part_meter_suffix_rule     set_ship_part_meter_suffix_3;
        set_empire_stockpile_rule           set_empire_stockpile;
        parse::effect_parser_rule           set_empire_capital;
        parse::effect_parser_rule           set_planet_type;
        parse::effect_parser_rule           set_planet_size;
        parse::effect_parser_rule           set_species;
        parse::effect_parser_rule           set_owner;
        create_planet_rule                  create_planet;
        parse::effect_parser_rule           create_building;
        string_and_intref_and_intref_rule   create_ship_1;
        string_and_intref_and_intref_rule   create_ship_2;
        string_and_intref_and_intref_rule   create_ship_3;
        string_and_intref_and_intref_rule   create_ship_4;
        parse::effect_parser_rule           move_to;
        doubles_rule                        move_in_orbit;
        parse::effect_parser_rule           set_destination;
        parse::effect_parser_rule           destroy;
        parse::effect_parser_rule           victory;
        parse::effect_parser_rule           add_special;
        parse::effect_parser_rule           remove_special;
        parse::effect_parser_rule           add_starlanes;
        parse::effect_parser_rule           remove_starlanes;
        parse::effect_parser_rule           set_star_type;
        string_and_intref_and_intref_rule   give_empire_tech;
        string_and_doubleref_rule           set_empire_tech_progress;
        generate_sitrep_message_rule        generate_sitrep_message;
        string_and_doubleref_rule           set_overlay_texture;
        string_and_string_ref_rule          string_and_string_ref;
        string_and_string_ref_vector_rule   string_and_string_ref_vector;
        parse::effect_parser_rule           start;
    };
}

namespace parse {
    effect_parser_rule& effect_parser() {
        static effect_parser_rules retval;
        return retval.start;
    }
}
