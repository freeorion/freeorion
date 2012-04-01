#include "EnumParser.h"


namespace qi = boost::spirit::qi;

#define DEBUG_PARSERS 0

namespace {
    qi::_1_type _1;
    qi::_val_type _val;
}

namespace parse {
    template <>
    enum_parser_rule<PlanetSize>::type& enum_parser<PlanetSize>()
    {
        const parse::lexer& tok = parse::lexer::instance();
        static enum_parser_rule<PlanetSize>::type retval
            =    tok.Tiny_ [ _val = SZ_TINY ]
            |    tok.Small_ [ _val = SZ_SMALL ]
            |    tok.Medium_ [ _val = SZ_MEDIUM ]
            |    tok.Large_ [ _val = SZ_LARGE ]
            |    tok.Huge_ [ _val = SZ_HUGE ]
            |    tok.Asteroids_ [ _val = SZ_ASTEROIDS ]
            |    tok.GasGiant_ [ _val = SZ_GASGIANT ]
            ;
        static bool once = true;
        if (once) {
            retval.name("PlanetSize");
#if DEBUG_PARSERS
            debug(retval);
#endif
            once = false;
        }
        return retval;
    }

    template <>
    enum_parser_rule<PlanetType>::type& enum_parser<PlanetType>()
    {
        const parse::lexer& tok = parse::lexer::instance();
        static enum_parser_rule<PlanetType>::type retval
            =    tok.Swamp_ [ _val = PT_SWAMP ]
            |    tok.Toxic_ [ _val = PT_TOXIC ]
            |    tok.Inferno_ [ _val = PT_INFERNO ]
            |    tok.Radiated_ [ _val = PT_RADIATED ]
            |    tok.Barren_ [ _val = PT_BARREN ]
            |    tok.Tundra_ [ _val = PT_TUNDRA ]
            |    tok.Desert_ [ _val = PT_DESERT ]
            |    tok.Terran_ [ _val = PT_TERRAN ]
            |    tok.Ocean_ [ _val = PT_OCEAN ]
            |    tok.Asteroids_ [ _val = PT_ASTEROIDS ]
            |    tok.GasGiant_ [ _val = PT_GASGIANT ]
            ;
        static bool once = true;
        if (once) {
            retval.name("PlanetType");
#if DEBUG_PARSERS
            debug(retval);
#endif
            once = false;
        }
        return retval;
    }

    template <>
    enum_parser_rule<PlanetEnvironment>::type& enum_parser<PlanetEnvironment>()
    {
        const parse::lexer& tok = parse::lexer::instance();
        static enum_parser_rule<PlanetEnvironment>::type retval
            =    tok.Uninhabitable_ [ _val = PE_UNINHABITABLE ]
            |    tok.Hostile_ [ _val = PE_HOSTILE ]
            |    tok.Poor_ [ _val = PE_POOR ]
            |    tok.Adequate_ [ _val = PE_ADEQUATE ]
            |    tok.Good_ [ _val = PE_GOOD ]
            ;
        static bool once = true;
        if (once) {
            retval.name("PlanetEnvironment");
#if DEBUG_PARSERS
            debug(retval);
#endif
            once = false;
        }
        return retval;
    }

    template <>
    enum_parser_rule<UniverseObjectType>::type& enum_parser<UniverseObjectType>()
    {
        const parse::lexer& tok = parse::lexer::instance();
        static enum_parser_rule<UniverseObjectType>::type retval
            =    tok.Building_ [ _val = OBJ_BUILDING ]
            |    tok.Ship_ [ _val = OBJ_SHIP ]
            |    tok.Fleet_ [ _val = OBJ_FLEET ]
            |    tok.Planet_ [ _val = OBJ_PLANET ]
            |    tok.PopulationCenter_ [ _val = OBJ_POP_CENTER ]
            |    tok.ProductionCenter_ [ _val = OBJ_PROD_CENTER ]
            |    tok.System_ [ _val = OBJ_SYSTEM ]
            ;
        static bool once = true;
        if (once) {
            retval.name("UniverseObjectType");
#if DEBUG_PARSERS
            debug(retval);
#endif
            once = false;
        }
        return retval;
    }

    template <>
    enum_parser_rule<StarType>::type& enum_parser<StarType>()
    {
        const parse::lexer& tok = parse::lexer::instance();
        static enum_parser_rule<StarType>::type retval
            =    tok.Blue_ [ _val = STAR_BLUE ]
            |    tok.White_ [ _val = STAR_WHITE ]
            |    tok.Yellow_ [ _val = STAR_YELLOW ]
            |    tok.Orange_ [ _val = STAR_ORANGE ]
            |    tok.Red_ [ _val = STAR_RED ]
            |    tok.Neutron_ [ _val = STAR_NEUTRON ]
            |    tok.BlackHole_ [ _val = STAR_BLACK ]
            |    tok.NoStar_ [ _val = STAR_NONE ]
            ;
        static bool once = true;
        if (once) {
            retval.name("StarType");
#if DEBUG_PARSERS
            debug(retval);
#endif
            once = false;
        }
        return retval;
    }

    template <>
    enum_parser_rule<EmpireAffiliationType>::type& enum_parser<EmpireAffiliationType>()
    {
        const parse::lexer& tok = parse::lexer::instance();
        static enum_parser_rule<EmpireAffiliationType>::type retval
            =    tok.TheEmpire_ [ _val = AFFIL_SELF ]
            |    tok.EnemyOf_ [ _val = AFFIL_ENEMY ]
            |    tok.AllyOf_ [ _val = AFFIL_ALLY ]
            |    tok.AnyEmpire_ [ _val = AFFIL_ANY ]
            ;
        static bool once = true;
        if (once) {
            retval.name("EmpireAffiliationType");
#if DEBUG_PARSERS
            debug(retval);
#endif
            once = false;
        }
        return retval;
    }

    template <>
    enum_parser_rule<UnlockableItemType>::type& enum_parser<UnlockableItemType>()
    {
        const parse::lexer& tok = parse::lexer::instance();
        static enum_parser_rule<UnlockableItemType>::type retval
            =    tok.Building_ [ _val = UIT_BUILDING ]
            |    tok.ShipPart_ [ _val = UIT_SHIP_PART ]
            |    tok.ShipHull_ [ _val = UIT_SHIP_HULL ]
            |    tok.ShipDesign_ [ _val = UIT_SHIP_DESIGN ]
            |    tok.Tech_ [ _val = UIT_TECH ]
            ;
        static bool once = true;
        if (once) {
            retval.name("UnlockableItemType");
#if DEBUG_PARSERS
            debug(retval);
#endif
            once = false;
        }
        return retval;
    }

    template <>
    enum_parser_rule<TechType>::type& enum_parser<TechType>()
    {
        const parse::lexer& tok = parse::lexer::instance();
        static enum_parser_rule<TechType>::type retval
            =    tok.Theory_ [ _val = TT_THEORY ]
            |    tok.Application_ [ _val = TT_APPLICATION ]
            |    tok.Refinement_ [ _val = TT_REFINEMENT ]
            ;
        static bool once = true;
        if (once) {
            retval.name("TechType");
#if DEBUG_PARSERS
            debug(retval);
#endif
            once = false;
        }
        return retval;
    }

    template <>
    enum_parser_rule<ShipSlotType>::type& enum_parser<ShipSlotType>()
    {
        const parse::lexer& tok = parse::lexer::instance();
        static enum_parser_rule<ShipSlotType>::type retval
            =    tok.External_ [ _val = SL_EXTERNAL ]
            |    tok.Internal_ [ _val = SL_INTERNAL ]
            ;
        static bool once = true;
        if (once) {
            retval.name("ShipSlotType");
#if DEBUG_PARSERS
            debug(retval);
#endif
            once = false;
        }
        return retval;
    }

    template <>
    enum_parser_rule<ShipPartClass>::type& enum_parser<ShipPartClass>()
    {
        const parse::lexer& tok = parse::lexer::instance();
        static enum_parser_rule<ShipPartClass>::type retval
            =    tok.ShortRange_ [ _val = PC_SHORT_RANGE ]
            |    tok.Missiles_ [ _val = PC_MISSILES ]
            |    tok.Fighters_ [ _val = PC_FIGHTERS ]
            |    tok.PointDefense_ [ _val = PC_POINT_DEFENSE ]
            |    tok.Shield_ [ _val = PC_SHIELD ]
            |    tok.Armour_ [ _val = PC_ARMOUR ]
            |    tok.Troops_ [ _val = PC_TROOPS ]
            |    tok.Detection_ [ _val = PC_DETECTION ]
            |    tok.Stealth_ [ _val = PC_STEALTH ]
            |    tok.Fuel_ [ _val = PC_FUEL ]
            |    tok.Colony_ [ _val = PC_COLONY ]
            |    tok.BattleSpeed_ [ _val = PC_BATTLE_SPEED ]
            |    tok.StarlaneSpeed_ [ _val = PC_STARLANE_SPEED ]
            ;
        static bool once = true;
        if (once) {
            retval.name("ShipPartClass");
#if DEBUG_PARSERS
            debug(retval);
#endif
            once = false;
        }
        return retval;
    }

    template <>
    enum_parser_rule<CombatFighterType>::type& enum_parser<CombatFighterType>()
    {
        const parse::lexer& tok = parse::lexer::instance();
        static enum_parser_rule<CombatFighterType>::type retval
            =    tok.Interceptor_ [ _val = INTERCEPTOR ]
            |    tok.Bomber_ [ _val = BOMBER ]
            ;
        static bool once = true;
        if (once) {
            retval.name("CombatFighterType");
#if DEBUG_PARSERS
            debug(retval);
#endif
            once = false;
        }
        return retval;
    }

    template <>
    enum_parser_rule<CaptureResult>::type& enum_parser<CaptureResult>()
    {
        const parse::lexer& tok = parse::lexer::instance();
        static enum_parser_rule<CaptureResult>::type retval
            =    tok.Capture_ [ _val = CR_CAPTURE ]
            |    tok.Retain_ [ _val = CR_RETAIN ]
            |    tok.Destroy_ [ _val = CR_DESTROY ]
            ;
        static bool once = true;
        if (once) {
            retval.name("CaptureResult");
#if DEBUG_PARSERS
            debug(retval);
#endif
            once = false;
        }
        return retval;
    }

    template <>
    enum_parser_rule<ValueRef::StatisticType>::type& enum_parser<ValueRef::StatisticType>()
    {
        const parse::lexer& tok = parse::lexer::instance();
        static enum_parser_rule<ValueRef::StatisticType>::type retval
            =    tok.Count_ [ _val = ValueRef::COUNT ]
            |    tok.Sum_ [ _val = ValueRef::SUM ]
            |    tok.Mean_ [ _val = ValueRef::MEAN ]
            |    tok.RMS_ [ _val = ValueRef::RMS ]
            |    tok.Mode_ [ _val = ValueRef::MODE ]
            |    tok.Max_ [ _val = ValueRef::MAX ]
            |    tok.Min_ [ _val = ValueRef::MIN ]
            |    tok.Spread_ [ _val = ValueRef::SPREAD ]
            |    tok.StDev_ [ _val = ValueRef::STDEV ]
            |    tok.Product_ [ _val = ValueRef::PRODUCT ]
            ;
        static bool once = true;
        if (once) {
            retval.name("StatisticType");
#if DEBUG_PARSERS
            debug(retval);
#endif
            once = false;
        }
        return retval;
    }

    enum_parser_rule<MeterType>::type& non_ship_part_meter_type_enum() {
        const parse::lexer& tok = parse::lexer::instance();
        static enum_parser_rule<MeterType>::type retval
            =    tok.TargetConstruction_ [ _val = METER_TARGET_CONSTRUCTION ]
            |    tok.TargetIndustry_ [ _val = METER_TARGET_INDUSTRY ]
            |    tok.TargetMining_ [ _val = METER_TARGET_MINING ]
            |    tok.TargetPopulation_ [ _val = METER_TARGET_POPULATION ]
            |    tok.TargetResearch_ [ _val = METER_TARGET_RESEARCH ]
            |    tok.TargetTrade_ [ _val = METER_TARGET_TRADE ]

            |    tok.MaxDefense_ [ _val = METER_MAX_DEFENSE ]
            |    tok.MaxFuel_ [ _val = METER_MAX_FUEL ]
            |    tok.MaxShield_ [ _val = METER_MAX_SHIELD ]
            |    tok.MaxStructure_ [ _val = METER_MAX_STRUCTURE ]
            |    tok.MaxTroops_ [ _val = METER_MAX_TROOPS ]

            |    tok.Construction_ [ _val = METER_CONSTRUCTION ]
            |    tok.Industry_ [ _val = METER_INDUSTRY ]
            |    tok.Mining_ [ _val = METER_MINING ]
            |    tok.Population_ [ _val = METER_POPULATION ]
            |    tok.Research_ [ _val = METER_RESEARCH ]
            |    tok.Trade_ [ _val = METER_TRADE ]

            |    tok.Defense_ [ _val = METER_DEFENSE ]
            |    tok.Fuel_ [ _val = METER_FUEL ]
            |    tok.Shield_ [ _val = METER_SHIELD ]
            |    tok.Structure_ [ _val = METER_STRUCTURE ]
            |    tok.Troops_ [ _val = METER_TROOPS ]

            |    tok.Supply_ [ _val = METER_SUPPLY ]
            |    tok.Stealth_ [ _val = METER_STEALTH ]
            |    tok.Detection_ [ _val = METER_DETECTION ]
            |    tok.BattleSpeed_ [ _val = METER_BATTLE_SPEED ]
            |    tok.StarlaneSpeed_ [ _val = METER_STARLANE_SPEED ]
            ;
        static bool once = true;
        if (once) {
            retval.name("non-ship-part MeterType");
#if DEBUG_PARSERS
            debug(retval);
#endif
            once = false;
        }
        return retval;
    }

    enum_parser_rule<MeterType>::type& set_non_ship_part_meter_type_enum() {
        const parse::lexer& tok = parse::lexer::instance();
        static enum_parser_rule<MeterType>::type retval
            =    tok.SetTargetConstruction_ [ _val = METER_TARGET_CONSTRUCTION ]
            |    tok.SetTargetIndustry_ [ _val = METER_TARGET_INDUSTRY ]
            |    tok.SetTargetMining_ [ _val = METER_TARGET_MINING ]
            |    tok.SetTargetPopulation_ [ _val = METER_TARGET_POPULATION ]
            |    tok.SetTargetResearch_ [ _val = METER_TARGET_RESEARCH ]
            |    tok.SetTargetTrade_ [ _val = METER_TARGET_TRADE ]

            |    tok.SetMaxDefense_ [ _val = METER_MAX_DEFENSE ]
            |    tok.SetMaxFuel_ [ _val = METER_MAX_FUEL ]
            |    tok.SetMaxShield_ [ _val = METER_MAX_SHIELD ]
            |    tok.SetMaxStructure_ [ _val = METER_MAX_STRUCTURE ]
            |    tok.SetMaxTroops_ [ _val = METER_MAX_TROOPS ]

            |    tok.SetConstruction_ [ _val = METER_CONSTRUCTION ]
            |    tok.SetIndustry_ [ _val = METER_INDUSTRY ]
            |    tok.SetMining_ [ _val = METER_MINING ]
            |    tok.SetPopulation_ [ _val = METER_POPULATION ]
            |    tok.SetResearch_ [ _val = METER_RESEARCH ]
            |    tok.SetTrade_ [ _val = METER_TRADE ]

            |    tok.SetDefense_ [ _val = METER_DEFENSE ]
            |    tok.SetFuel_ [ _val = METER_FUEL ]
            |    tok.SetShield_ [ _val = METER_SHIELD ]
            |    tok.SetStructure_ [ _val = METER_STRUCTURE ]
            |    tok.SetTroops_ [ _val = METER_TROOPS ]

            |    tok.SetSupply_ [ _val = METER_SUPPLY ]
            |    tok.SetStealth_ [ _val = METER_STEALTH ]
            |    tok.SetDetection_ [ _val = METER_DETECTION ]
            |    tok.SetBattleSpeed_ [ _val = METER_BATTLE_SPEED ]
            |    tok.SetStarlaneSpeed_ [ _val = METER_STARLANE_SPEED ]
            ;
        static bool once = true;
        if (once) {
            retval.name("non-ship-part MeterType");
#if DEBUG_PARSERS
            debug(retval);
#endif
            once = false;
        }
        return retval;
    }

    enum_parser_rule<MeterType>::type& set_ship_part_meter_type_enum()
    {
        const parse::lexer& tok = parse::lexer::instance();
        static enum_parser_rule<MeterType>::type retval
            =    tok.SetDamage_ [ _val = METER_DAMAGE ]
            |    tok.SetROF_ [ _val = METER_ROF ]
            |    tok.SetRange_ [ _val = METER_RANGE ]
            |    tok.SetSpeed_ [ _val = METER_SPEED ]
            |    tok.SetCapacity_ [ _val = METER_CAPACITY ]
            |    tok.SetAntiShipDamage_ [ _val = METER_ANTI_SHIP_DAMAGE ]
            |    tok.SetAntiFighterDamage_ [ _val = METER_ANTI_FIGHTER_DAMAGE ]
            |    tok.SetLaunchRate_ [ _val = METER_LAUNCH_RATE ]
            |    tok.SetFighterWeaponRange_ [ _val = METER_FIGHTER_WEAPON_RANGE ]
            |    tok.SetStealth_ [ _val = METER_STEALTH ]
            |    tok.SetDetection_ [ _val = METER_DETECTION ]
            |    tok.SetStructure_ [ _val = METER_STRUCTURE ]
            ;
        static bool once = true;
        if (once) {
            retval.name("ship-part MeterType");
#if DEBUG_PARSERS
            debug(retval);
#endif
            once = false;
        }
        return retval;
    }
}
