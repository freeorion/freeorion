#include "EnumParser.h"

#include "../universe/Enums.h"


namespace qi = boost::spirit::qi;

#define DEBUG_PARSERS 0

namespace {
    qi::_val_type _val;
}

namespace parse {
    empire_affiliation_enum_grammar::empire_affiliation_enum_grammar(const parse::lexer& tok) :
        empire_affiliation_enum_grammar::base_type(rule, "empire_affiliation_enum_grammar")
    {
        rule
            =   tok.TheEmpire_  [ _val = AFFIL_SELF ]
            |   tok.EnemyOf_    [ _val = AFFIL_ENEMY ]
            |   tok.AllyOf_     [ _val = AFFIL_ALLY ]
            |   tok.AnyEmpire_  [ _val = AFFIL_ANY ]
            |   tok.None_       [ _val = AFFIL_NONE ]
            |   tok.CanSee_     [ _val = AFFIL_CAN_SEE ]
            |   tok.Human_      [ _val = AFFIL_HUMAN ]
            ;
    }

    unlockable_item_enum_grammar::unlockable_item_enum_grammar(const parse::lexer& tok) :
        unlockable_item_enum_grammar::base_type(rule, "unlockable_item_enum_grammar")
    {
        rule
            =   tok.Building_       [ _val = UIT_BUILDING ]
            |   tok.ShipPart_       [ _val = UIT_SHIP_PART ]
            |   tok.ShipHull_       [ _val = UIT_SHIP_HULL ]
            |   tok.ShipDesign_     [ _val = UIT_SHIP_DESIGN ]
            |   tok.Tech_           [ _val = UIT_TECH ]
            ;
    }

    ship_slot_enum_grammar::ship_slot_enum_grammar(const parse::lexer& tok) :
        ship_slot_enum_grammar::base_type(rule, "ship_slot_enum_grammar")
    {
        rule
            =   tok.External_   [ _val = SL_EXTERNAL ]
            |   tok.Internal_   [ _val = SL_INTERNAL ]
            |   tok.Core_       [ _val = SL_CORE ]
            ;
    }

    ship_part_class_enum_grammar::ship_part_class_enum_grammar(const parse::lexer& tok) :
        ship_part_class_enum_grammar::base_type(rule, "ship_part_class_enum_grammar")
    {
        rule
            =   tok.ShortRange_         [ _val = PC_DIRECT_WEAPON ]
            |   tok.FighterBay_         [ _val = PC_FIGHTER_BAY ]
            |   tok.FighterHangar_      [ _val = PC_FIGHTER_HANGAR ]
            |   tok.Shield_             [ _val = PC_SHIELD ]
            |   tok.Armour_             [ _val = PC_ARMOUR ]
            |   tok.Troops_             [ _val = PC_TROOPS ]
            |   tok.Detection_          [ _val = PC_DETECTION ]
            |   tok.Stealth_            [ _val = PC_STEALTH ]
            |   tok.Fuel_               [ _val = PC_FUEL ]
            |   tok.Colony_             [ _val = PC_COLONY ]
            |   tok.Speed_              [ _val = PC_SPEED ]
            |   tok.General_            [ _val = PC_GENERAL ]
            |   tok.Bombard_            [ _val = PC_BOMBARD ]
            |   tok.Research_           [ _val = PC_RESEARCH ]
            |   tok.Industry_           [ _val = PC_INDUSTRY ]
            |   tok.Trade_              [ _val = PC_TRADE ]
            |   tok.ProductionLocation_ [ _val = PC_PRODUCTION_LOCATION ]
            ;
    }

    capture_result_enum_grammar::capture_result_enum_grammar(const parse::lexer& tok) :
        capture_result_enum_grammar::base_type(rule, "capture_result_enum_grammar")
    {
        rule
            =   tok.Capture_            [ _val = CR_CAPTURE ]
            |   tok.Retain_             [ _val = CR_RETAIN ]
            |   tok.Destroy_            [ _val = CR_DESTROY ]
            ;
    }

    statistic_enum_grammar::statistic_enum_grammar(const parse::lexer& tok) :
        statistic_enum_grammar::base_type(rule, "statistic_enum_grammar")
    {
        rule
            =   tok.CountUnique_    [ _val = ValueRef::UNIQUE_COUNT ]
            |   tok.Count_          [ _val = ValueRef::COUNT ]
            |   tok.If_             [ _val = ValueRef::IF ]
            |   tok.Sum_            [ _val = ValueRef::SUM ]
            |   tok.Mean_           [ _val = ValueRef::MEAN ]
            |   tok.RMS_            [ _val = ValueRef::RMS ]
            |   tok.Mode_           [ _val = ValueRef::MODE ]
            |   tok.Max_            [ _val = ValueRef::MAX ]
            |   tok.Min_            [ _val = ValueRef::MIN ]
            |   tok.Spread_         [ _val = ValueRef::SPREAD ]
            |   tok.StDev_          [ _val = ValueRef::STDEV ]
            |   tok.Product_        [ _val = ValueRef::PRODUCT ]
            ;
    }

    non_ship_part_meter_enum_grammar::non_ship_part_meter_enum_grammar(const parse::lexer& tok) :
        non_ship_part_meter_enum_grammar::base_type(rule, "non_ship_part_meter_enum_grammar")
    {
        rule
            =   tok.TargetConstruction_     [ _val = METER_TARGET_CONSTRUCTION ]
            |   tok.TargetIndustry_         [ _val = METER_TARGET_INDUSTRY ]
            |   tok.TargetPopulation_       [ _val = METER_TARGET_POPULATION ]
            |   tok.TargetResearch_         [ _val = METER_TARGET_RESEARCH ]
            |   tok.TargetTrade_            [ _val = METER_TARGET_TRADE ]
            |   tok.TargetHappiness_        [ _val = METER_TARGET_HAPPINESS ]
            |   tok.MaxDefense_             [ _val = METER_MAX_DEFENSE ]
            |   tok.MaxFuel_                [ _val = METER_MAX_FUEL ]
            |   tok.MaxShield_              [ _val = METER_MAX_SHIELD ]
            |   tok.MaxStructure_           [ _val = METER_MAX_STRUCTURE ]
            |   tok.MaxTroops_              [ _val = METER_MAX_TROOPS ]
            |   tok.MaxSupply_              [ _val = METER_MAX_SUPPLY ]
            |   tok.MaxStockpile_           [ _val = METER_MAX_STOCKPILE ]

            |   tok.Construction_           [ _val = METER_CONSTRUCTION ]
            |   tok.Industry_               [ _val = METER_INDUSTRY ]
            |   tok.Population_             [ _val = METER_POPULATION ]
            |   tok.Research_               [ _val = METER_RESEARCH ]
            |   tok.Trade_                  [ _val = METER_TRADE ]
            |   tok.Happiness_              [ _val = METER_HAPPINESS ]

            |   tok.Defense_                [ _val = METER_DEFENSE ]
            |   tok.Fuel_                   [ _val = METER_FUEL ]
            |   tok.Shield_                 [ _val = METER_SHIELD ]
            |   tok.Structure_              [ _val = METER_STRUCTURE ]
            |   tok.Troops_                 [ _val = METER_TROOPS ]
            |   tok.Supply_                 [ _val = METER_SUPPLY ]
            |   tok.Stockpile_              [ _val = METER_STOCKPILE ]

            |   tok.RebelTroops_            [ _val = METER_REBEL_TROOPS ]
            |   tok.Stealth_                [ _val = METER_STEALTH ]
            |   tok.Detection_              [ _val = METER_DETECTION ]
            |   tok.Speed_                  [ _val = METER_SPEED ]

            |   tok.Size_                   [ _val = METER_SIZE ]
            ;
    }

    ship_part_meter_enum_grammar::ship_part_meter_enum_grammar(const parse::lexer& tok) :
        ship_part_meter_enum_grammar::base_type(rule, "ship_part_meter_enum_grammar")
    {
        rule
            =   tok.MaxCapacity_            [ _val = METER_MAX_CAPACITY ]
            |   tok.MaxDamage_              [ _val = METER_MAX_CAPACITY ]
            |   tok.Capacity_               [ _val = METER_CAPACITY ]
            |   tok.Damage_                 [ _val = METER_CAPACITY ]
            |   tok.SecondaryStat_          [ _val = METER_SECONDARY_STAT ]
            |   tok.MaxSecondaryStat_       [ _val = METER_MAX_SECONDARY_STAT ]
            ;
    }

    set_non_ship_part_meter_enum_grammar::set_non_ship_part_meter_enum_grammar(const parse::lexer& tok) :
        set_non_ship_part_meter_enum_grammar::base_type(rule, "set_non_ship_part_meter_enum_grammar")
    {
        rule
            =   tok.SetTargetConstruction_  [ _val = METER_TARGET_CONSTRUCTION ]
            |   tok.SetTargetIndustry_      [ _val = METER_TARGET_INDUSTRY ]
            |   tok.SetTargetPopulation_    [ _val = METER_TARGET_POPULATION ]
            |   tok.SetTargetResearch_      [ _val = METER_TARGET_RESEARCH ]
            |   tok.SetTargetTrade_         [ _val = METER_TARGET_TRADE ]
            |   tok.SetTargetHappiness_     [ _val = METER_TARGET_HAPPINESS ]

            |   tok.SetMaxDefense_          [ _val = METER_MAX_DEFENSE ]
            |   tok.SetMaxFuel_             [ _val = METER_MAX_FUEL ]
            |   tok.SetMaxShield_           [ _val = METER_MAX_SHIELD ]
            |   tok.SetMaxStructure_        [ _val = METER_MAX_STRUCTURE ]
            |   tok.SetMaxTroops_           [ _val = METER_MAX_TROOPS ]
            |   tok.SetMaxSupply_           [ _val = METER_MAX_SUPPLY ]
            |   tok.SetMaxStockpile_        [ _val = METER_MAX_STOCKPILE ]

            |   tok.SetConstruction_        [ _val = METER_CONSTRUCTION ]
            |   tok.SetIndustry_            [ _val = METER_INDUSTRY ]
            |   tok.SetPopulation_          [ _val = METER_POPULATION ]
            |   tok.SetResearch_            [ _val = METER_RESEARCH ]
            |   tok.SetTrade_               [ _val = METER_TRADE ]
            |   tok.SetHappiness_           [ _val = METER_HAPPINESS ]

            |   tok.SetDefense_             [ _val = METER_DEFENSE ]
            |   tok.SetFuel_                [ _val = METER_FUEL ]
            |   tok.SetShield_              [ _val = METER_SHIELD ]
            |   tok.SetStructure_           [ _val = METER_STRUCTURE ]
            |   tok.SetTroops_              [ _val = METER_TROOPS ]
            |   tok.SetSupply_              [ _val = METER_SUPPLY ]
            |   tok.SetStockpile_           [ _val = METER_STOCKPILE ]

            |   tok.SetRebelTroops_         [ _val = METER_REBEL_TROOPS ]
            |   tok.SetStealth_             [ _val = METER_STEALTH ]
            |   tok.SetDetection_           [ _val = METER_DETECTION ]
            |   tok.SetSpeed_               [ _val = METER_SPEED ]

            |   tok.SetSize_                [ _val = METER_SIZE ]
            ;
    }

    set_ship_part_meter_enum_grammar::set_ship_part_meter_enum_grammar(const parse::lexer& tok) :
        set_ship_part_meter_enum_grammar::base_type(rule, "set_ship_part_meter_enum_grammar")
    {
        rule
            =   tok.SetMaxCapacity_         [ _val = METER_MAX_CAPACITY ]
            |   tok.SetMaxDamage_           [ _val = METER_MAX_CAPACITY ]
            |   tok.SetMaxSecondaryStat_    [ _val = METER_MAX_SECONDARY_STAT ]
            |   tok.SetCapacity_            [ _val = METER_CAPACITY ]
            |   tok.SetDamage_              [ _val = METER_CAPACITY ]
            |   tok.SetSecondaryStat_       [ _val = METER_SECONDARY_STAT ]
            ;
    }
}
