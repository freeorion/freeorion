from focs._effects import (
    EmpireHasAdoptedPolicy,
    GameRule,
    IsSource,
    NamedInteger,
    NamedIntegerLookup,
    NamedReal,
    NamedRealLookup,
    Source,
    StatisticIf,
    UsedInDesignID,
)
from focs._value_refs import (
    IfIntRefLessEqualIntThenDoubleOrDouble,
    NumPartClassesInShipDesign,
    PartOfClassInShipDesign
)

# Proposed naming convention: <EFFECT_NAME> _ <METER_NAME> _ FLAT/PERPOP/<others>
# Examples:
# - PLC_CONFEDERATION_TARGET_HAPPINESS_FLAT (flat bonus/malus to TargetHappiness from policy PLC_CONFEDERATION)
# - GARRISON_2_TROOPREGEN_FLAT (flat bonus/malus to troop regen from tech Garrison_2
# - GRO_CYBORGS_MAX_TROOPS_PERPOP (bonus/malus per population to MaxTroops from tech GRO_CYBORGS)

# from macros.base_prod import BUILDING_COST_MULTIPLIER

NamedInteger(name="XENOPHOBIC_MAX_JUMPS", value=4)

NamedReal(name="PROTECION_FOCUS_STABILITY_BONUS", value=GameRule(type=float, name="RULE_PROTECTION_FOCUS_STABILITY"))

NamedReal(name="FU_RAMSCOOP_REFUEL", value=0.4)
NamedReal(name="BLD_NEST_RESERVE_WILD_SPAWN_FACTOR", value=0.3)

# For reduced/increased ship cost
# value is used in upkeep.focs.txt, can be migrated to upkeep.focs.py as soon as that gets used when parsing ships
# Note: colony part class increases complexity by 4; every other classe by 1
colony_extra_complexity = 3
NamedInteger(name="DESIGN_SIMPLICITY_SOURCE_COMPLEXITY_PC_COLONY", value=1 + colony_extra_complexity)
NamedInteger(
    name="DESIGN_SIMPLICITY_SOURCE_COMPLEXITY_COUNT_VREF",
    value=NumPartClassesInShipDesign(design=UsedInDesignID)
    + (PartOfClassInShipDesign(name="PC_COLONY", design=UsedInDesignID) * colony_extra_complexity)
)
NamedReal(name="PLC_DESIGN_SIMPLICITY_COMPLEXITY_FACTOR_AAA", value=0.8)
NamedReal(name="PLC_DESIGN_SIMPLICITY_COMPLEXITY_FACTOR_AA", value=0.9)
NamedReal(name="PLC_DESIGN_SIMPLICITY_COMPLEXITY_FACTOR_A", value=0.95)
NamedReal(name="PLC_DESIGN_SIMPLICITY_COMPLEXITY_FACTOR", value=1.0)
#NamedReal(name="PLC_DESIGN_SIMPLICITY_MAX_REDUCTION_PERCENT", value=20)
# round(( 1.0 - NamedRealLookup name = "PLC_DESIGN_SIMPLICITY_COMPLEXITY_FACTOR_AAA" ) * 100)
NamedReal(name="PLC_DESIGN_SIMPLICITY_MAX_REDUCTION_PERCENT", value=(1 - NamedRealLookup(name="PLC_DESIGN_SIMPLICITY_COMPLEXITY_FACTOR_AAA")) * 100)

#def less_equal_range(vref, dict, default: float):
    
# // DESIGN_SIMPLICITY_SOURCE_COMPLEXITY_COUNT_VREF is defined in python focs, this should be executed afterwards but is not
NamedReal(
    name="PLC_DESIGN_SIMPLICITY_COMPLEXITY_FACTOR_FOR_ARG1_VREF",
    value=IfIntRefLessEqualIntThenDoubleOrDouble(
        lhs=NamedIntegerLookup(name="DESIGN_SIMPLICITY_SOURCE_COMPLEXITY_COUNT_VREF"),
        rhs=2,
        iff=0.8,
        ellse=IfIntRefLessEqualIntThenDoubleOrDouble(
            lhs=NamedIntegerLookup(name="DESIGN_SIMPLICITY_SOURCE_COMPLEXITY_COUNT_VREF"),
            rhs=3,
            iff=0.9,
            ellse=IfIntRefLessEqualIntThenDoubleOrDouble(
                lhs=NamedIntegerLookup(name="DESIGN_SIMPLICITY_SOURCE_COMPLEXITY_COUNT_VREF"),
                rhs=4,
                iff=0.95,
                ellse=1.0
            )
        )
    )
)
# NamedIntegerLookup name="DESIGN_SIMPLICITY_SOURCE_COMPLEXITY_COUNT_VREF" > 4 ? 1.0 :
# ( NamedIntegerLookup name="DESIGN_SIMPLICITY_SOURCE_COMPLEXITY_COUNT_VREF" > 3 ? 0.95 :
#   ( NamedIntegerLookup name="DESIGN_SIMPLICITY_SOURCE_COMPLEXITY_COUNT_VREF" > 2 ? 0.9 : 0.8 )
# )
# )

# FIXME unsupported operand type(s) for *: 'ValueRefInt' and 'float'

NamedReal(
    name="PLC_DESIGN_SIMPLICITY_POLICY_MULTIPLIER",
    value=1.0
    - (
        (1.0 - NamedRealLookup(name="PLC_DESIGN_SIMPLICITY_COMPLEXITY_FACTOR_FOR_ARG1_VREF"))
        * StatisticIf(
            float, condition=IsSource & EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_DESIGN_SIMPLICITY")
        )
    ),
)
