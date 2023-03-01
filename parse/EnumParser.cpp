#include "EnumParser.h"

#include "../universe/BuildingType.h"
#include "../universe/Condition.h"
#include "../universe/Enums.h"
#include "../universe/ShipHull.h"
#include "../universe/ShipPart.h"
#include "../universe/UnlockableItem.h"
#include "../Empire/ResourcePool.h"

#include <boost/phoenix/object/dynamic_cast.hpp>
#include <boost/phoenix/operator/self.hpp>

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
            =   tok.TheEmpire_  [ _val = EmpireAffiliationType::AFFIL_SELF ]
            |   tok.EnemyOf_    [ _val = EmpireAffiliationType::AFFIL_ENEMY ]
            |   tok.PeaceWith_  [ _val = EmpireAffiliationType::AFFIL_PEACE ]
            |   tok.AllyOf_     [ _val = EmpireAffiliationType::AFFIL_ALLY ]
            |   tok.AnyEmpire_  [ _val = EmpireAffiliationType::AFFIL_ANY ]
            |   tok.None_       [ _val = EmpireAffiliationType::AFFIL_NONE ]
            |   tok.CanSee_     [ _val = EmpireAffiliationType::AFFIL_CAN_SEE ]
            |   tok.Human_      [ _val = EmpireAffiliationType::AFFIL_HUMAN ]
        ;
    }

    unlockable_item_enum_grammar::unlockable_item_enum_grammar(const parse::lexer& tok) :
        unlockable_item_enum_grammar::base_type(rule, "unlockable_item_enum_grammar")
    {
        rule
            =   tok.Building_           [ _val = UnlockableItemType::UIT_BUILDING ]
            |   tok.ShipPart_           [ _val = UnlockableItemType::UIT_SHIP_PART ]
            |   tok.ShipHull_           [ _val = UnlockableItemType::UIT_SHIP_HULL ]
            |   tok.ShipDesign_         [ _val = UnlockableItemType::UIT_SHIP_DESIGN ]
            |   tok.Tech_               [ _val = UnlockableItemType::UIT_TECH ]
            |   tok.Policy_             [ _val = UnlockableItemType::UIT_POLICY ]
        ;
    }

    give_empire_unlockable_item_enum_grammar::give_empire_unlockable_item_enum_grammar(const parse::lexer& tok) :
        give_empire_unlockable_item_enum_grammar::base_type(rule, "give_empire_unlockable_item_enum_grammar")
    {
        rule
            =   tok.UnlockBuildingType_ [ _val = UnlockableItemType::UIT_BUILDING ]
            |   tok.UnlockShipPart_     [ _val = UnlockableItemType::UIT_SHIP_PART ]
            |   tok.UnlockShipHull_     [ _val = UnlockableItemType::UIT_SHIP_HULL ]
            |   tok.GiveEmpireTech_     [ _val = UnlockableItemType::UIT_TECH ]
            |   tok.UnlockPolicy_       [ _val = UnlockableItemType::UIT_POLICY ]
        ;
    }

    ship_slot_enum_grammar::ship_slot_enum_grammar(const parse::lexer& tok) :
        ship_slot_enum_grammar::base_type(rule, "ship_slot_enum_grammar")
    {
        rule
            =   tok.External_   [ _val = ShipSlotType::SL_EXTERNAL ]
            |   tok.Internal_   [ _val = ShipSlotType::SL_INTERNAL ]
            |   tok.Core_       [ _val = ShipSlotType::SL_CORE ]
        ;
    }

    ship_part_class_enum_grammar::ship_part_class_enum_grammar(const parse::lexer& tok) :
        ship_part_class_enum_grammar::base_type(rule, "ship_part_class_enum_grammar")
    {
        rule
            =   tok.ShortRange_         [ _val = ShipPartClass::PC_DIRECT_WEAPON ]
            |   tok.FighterBay_         [ _val = ShipPartClass::PC_FIGHTER_BAY ]
            |   tok.FighterHangar_      [ _val = ShipPartClass::PC_FIGHTER_HANGAR ]
            |   tok.Shield_             [ _val = ShipPartClass::PC_SHIELD ]
            |   tok.Armour_             [ _val = ShipPartClass::PC_ARMOUR ]
            |   tok.Troops_             [ _val = ShipPartClass::PC_TROOPS ]
            |   tok.Detection_          [ _val = ShipPartClass::PC_DETECTION ]
            |   tok.Stealth_            [ _val = ShipPartClass::PC_STEALTH ]
            |   tok.Fuel_               [ _val = ShipPartClass::PC_FUEL ]
            |   tok.Colony_             [ _val = ShipPartClass::PC_COLONY ]
            |   tok.Speed_              [ _val = ShipPartClass::PC_SPEED ]
            |   tok.General_            [ _val = ShipPartClass::PC_GENERAL ]
            |   tok.Bombard_            [ _val = ShipPartClass::PC_BOMBARD ]
            |   tok.Research_           [ _val = ShipPartClass::PC_RESEARCH ]
            |   tok.Industry_           [ _val = ShipPartClass::PC_INDUSTRY ]
            |   tok.Influence_          [ _val = ShipPartClass::PC_INFLUENCE ]
            |   tok.ProductionLocation_ [ _val = ShipPartClass::PC_PRODUCTION_LOCATION ]
        ;
    }

    capture_result_enum_grammar::capture_result_enum_grammar(const parse::lexer& tok) :
        capture_result_enum_grammar::base_type(rule, "capture_result_enum_grammar")
    {
        rule
            =   tok.Capture_            [ _val = CaptureResult::CR_CAPTURE ]
            |   tok.Retain_             [ _val = CaptureResult::CR_RETAIN ]
            |   tok.Destroy_            [ _val = CaptureResult::CR_DESTROY ]
        ;
    }

    statistic_enum_grammar::statistic_enum_grammar(const parse::lexer& tok) :
        statistic_enum_grammar::base_type(rule, "statistic_enum_grammar")
    {
        rule
            =   tok.CountUnique_    [ _val = ValueRef::StatisticType::UNIQUE_COUNT ]
            |   tok.If_             [ _val = ValueRef::StatisticType::IF ]
            |   tok.Count_          [ _val = ValueRef::StatisticType::COUNT ]
            |   tok.HistogramMax_   [ _val = ValueRef::StatisticType::HISTO_MAX ]
            |   tok.HistogramMin_   [ _val = ValueRef::StatisticType::HISTO_MIN ]
            |   tok.HistogramSpread_[ _val = ValueRef::StatisticType::HISTO_SPREAD ]
            |   tok.Sum_            [ _val = ValueRef::StatisticType::SUM ]
            |   tok.Mean_           [ _val = ValueRef::StatisticType::MEAN ]
            |   tok.RMS_            [ _val = ValueRef::StatisticType::RMS ]
            |   tok.Mode_           [ _val = ValueRef::StatisticType::MODE ]
            |   tok.Max_            [ _val = ValueRef::StatisticType::MAX ]
            |   tok.Min_            [ _val = ValueRef::StatisticType::MIN ]
            |   tok.Spread_         [ _val = ValueRef::StatisticType::SPREAD ]
            |   tok.StDev_          [ _val = ValueRef::StatisticType::STDEV ]
            |   tok.Product_        [ _val = ValueRef::StatisticType::PRODUCT ]
        ;
    }

    resource_type_grammar::resource_type_grammar(const parse::lexer& tok) :
        resource_type_grammar::base_type(rule, "resource_type_enum_grammar")
    {
        rule
            =   tok.Research_       [ _val = ResourceType::RE_RESEARCH ]
            |   tok.Industry_       [ _val = ResourceType::RE_INDUSTRY ]
            |   tok.Influence_      [ _val = ResourceType::RE_INFLUENCE ]
        ;
    }

    non_ship_part_meter_enum_grammar::non_ship_part_meter_enum_grammar(const parse::lexer& tok) :
        non_ship_part_meter_enum_grammar::base_type(rule, "non_ship_part_meter_enum_grammar")
    {
        rule
            =   tok.TargetConstruction_     [ _val = MeterType::METER_TARGET_CONSTRUCTION ]
            |   tok.TargetIndustry_         [ _val = MeterType::METER_TARGET_INDUSTRY ]
            |   tok.TargetPopulation_       [ _val = MeterType::METER_TARGET_POPULATION ]
            |   tok.TargetResearch_         [ _val = MeterType::METER_TARGET_RESEARCH ]
            |   tok.TargetInfluence_        [ _val = MeterType::METER_TARGET_INFLUENCE ]
            |   tok.TargetHappiness_        [ _val = MeterType::METER_TARGET_HAPPINESS ]
            |   tok.MaxDefense_             [ _val = MeterType::METER_MAX_DEFENSE ]
            |   tok.MaxFuel_                [ _val = MeterType::METER_MAX_FUEL ]
            |   tok.MaxShield_              [ _val = MeterType::METER_MAX_SHIELD ]
            |   tok.MaxStructure_           [ _val = MeterType::METER_MAX_STRUCTURE ]
            |   tok.MaxTroops_              [ _val = MeterType::METER_MAX_TROOPS ]
            |   tok.MaxSupply_              [ _val = MeterType::METER_MAX_SUPPLY ]
            |   tok.MaxStockpile_           [ _val = MeterType::METER_MAX_STOCKPILE ]

            |   tok.Construction_           [ _val = MeterType::METER_CONSTRUCTION ]
            |   tok.Industry_               [ _val = MeterType::METER_INDUSTRY ]
            |   tok.Population_             [ _val = MeterType::METER_POPULATION ]
            |   tok.Research_               [ _val = MeterType::METER_RESEARCH ]
            |   tok.Influence_              [ _val = MeterType::METER_INFLUENCE ]
            |   tok.Happiness_              [ _val = MeterType::METER_HAPPINESS ]

            |   tok.Defense_                [ _val = MeterType::METER_DEFENSE ]
            |   tok.Fuel_                   [ _val = MeterType::METER_FUEL ]
            |   tok.Shield_                 [ _val = MeterType::METER_SHIELD ]
            |   tok.Structure_              [ _val = MeterType::METER_STRUCTURE ]
            |   tok.Troops_                 [ _val = MeterType::METER_TROOPS ]
            |   tok.Supply_                 [ _val = MeterType::METER_SUPPLY ]
            |   tok.Stockpile_              [ _val = MeterType::METER_STOCKPILE ]

            |   tok.RebelTroops_            [ _val = MeterType::METER_REBEL_TROOPS ]
            |   tok.Stealth_                [ _val = MeterType::METER_STEALTH ]
            |   tok.Detection_              [ _val = MeterType::METER_DETECTION ]
            |   tok.Speed_                  [ _val = MeterType::METER_SPEED ]

            |   tok.Size_                   [ _val = MeterType::METER_SIZE ]
        ;
    }

    ship_part_meter_enum_grammar::ship_part_meter_enum_grammar(const parse::lexer& tok) :
        ship_part_meter_enum_grammar::base_type(rule, "ship_part_meter_enum_grammar")
    {
        rule
            =   tok.MaxCapacity_            [ _val = MeterType::METER_MAX_CAPACITY ]
            |   tok.MaxDamage_              [ _val = MeterType::METER_MAX_CAPACITY ]
            |   tok.Capacity_               [ _val = MeterType::METER_CAPACITY ]
            |   tok.Damage_                 [ _val = MeterType::METER_CAPACITY ]
            |   tok.SecondaryStat_          [ _val = MeterType::METER_SECONDARY_STAT ]
            |   tok.MaxSecondaryStat_       [ _val = MeterType::METER_MAX_SECONDARY_STAT ]
        ;
    }

    set_non_ship_part_meter_enum_grammar::set_non_ship_part_meter_enum_grammar(const parse::lexer& tok) :
        set_non_ship_part_meter_enum_grammar::base_type(rule, "set_non_ship_part_meter_enum_grammar")
    {
        rule
            =   tok.SetTargetConstruction_  [ _val = MeterType::METER_TARGET_CONSTRUCTION ]
            |   tok.SetTargetIndustry_      [ _val = MeterType::METER_TARGET_INDUSTRY ]
            |   tok.SetTargetPopulation_    [ _val = MeterType::METER_TARGET_POPULATION ]
            |   tok.SetTargetResearch_      [ _val = MeterType::METER_TARGET_RESEARCH ]
            |   tok.SetTargetInfluence_     [ _val = MeterType::METER_TARGET_INFLUENCE ]
            |   tok.SetTargetHappiness_     [ _val = MeterType::METER_TARGET_HAPPINESS ]

            |   tok.SetMaxDefense_          [ _val = MeterType::METER_MAX_DEFENSE ]
            |   tok.SetMaxFuel_             [ _val = MeterType::METER_MAX_FUEL ]
            |   tok.SetMaxShield_           [ _val = MeterType::METER_MAX_SHIELD ]
            |   tok.SetMaxStructure_        [ _val = MeterType::METER_MAX_STRUCTURE ]
            |   tok.SetMaxTroops_           [ _val = MeterType::METER_MAX_TROOPS ]
            |   tok.SetMaxSupply_           [ _val = MeterType::METER_MAX_SUPPLY ]
            |   tok.SetMaxStockpile_        [ _val = MeterType::METER_MAX_STOCKPILE ]

            |   tok.SetConstruction_        [ _val = MeterType::METER_CONSTRUCTION ]
            |   tok.SetIndustry_            [ _val = MeterType::METER_INDUSTRY ]
            |   tok.SetPopulation_          [ _val = MeterType::METER_POPULATION ]
            |   tok.SetResearch_            [ _val = MeterType::METER_RESEARCH ]
            |   tok.SetInfluence_           [ _val = MeterType::METER_INFLUENCE ]
            |   tok.SetHappiness_           [ _val = MeterType::METER_HAPPINESS ]

            |   tok.SetDefense_             [ _val = MeterType::METER_DEFENSE ]
            |   tok.SetFuel_                [ _val = MeterType::METER_FUEL ]
            |   tok.SetShield_              [ _val = MeterType::METER_SHIELD ]
            |   tok.SetStructure_           [ _val = MeterType::METER_STRUCTURE ]
            |   tok.SetTroops_              [ _val = MeterType::METER_TROOPS ]
            |   tok.SetSupply_              [ _val = MeterType::METER_SUPPLY ]
            |   tok.SetStockpile_           [ _val = MeterType::METER_STOCKPILE ]

            |   tok.SetRebelTroops_         [ _val = MeterType::METER_REBEL_TROOPS ]
            |   tok.SetStealth_             [ _val = MeterType::METER_STEALTH ]
            |   tok.SetDetection_           [ _val = MeterType::METER_DETECTION ]
            |   tok.SetSpeed_               [ _val = MeterType::METER_SPEED ]

            |   tok.SetSize_                [ _val = MeterType::METER_SIZE ]
        ;
    }

    set_ship_part_meter_enum_grammar::set_ship_part_meter_enum_grammar(const parse::lexer& tok) :
        set_ship_part_meter_enum_grammar::base_type(rule, "set_ship_part_meter_enum_grammar")
    {
        rule
            =   tok.SetMaxCapacity_         [ _val = MeterType::METER_MAX_CAPACITY ]
            |   tok.SetMaxDamage_           [ _val = MeterType::METER_MAX_CAPACITY ]
            |   tok.SetMaxSecondaryStat_    [ _val = MeterType::METER_MAX_SECONDARY_STAT ]
            |   tok.SetCapacity_            [ _val = MeterType::METER_CAPACITY ]
            |   tok.SetDamage_              [ _val = MeterType::METER_CAPACITY ]
            |   tok.SetSecondaryStat_       [ _val = MeterType::METER_SECONDARY_STAT ]
        ;
    }
}
