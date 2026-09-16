from focs._conditions import EmpireHasAdoptedPolicy, IsSource
from focs._enums import ArmourClass, FighterHangarClass, ShortRangeClass, TroopsClass
from focs._sources import Source
from focs._value_refs import (
    GameRule,
    NumPartClassesInShipDesign,
    PartOfClassInShipDesign,
    ShipDesignsInProduction,
    ShipDesignsOwned,
    ShipPartsOwned,
    StatisticIf,
    UsedInDesignID,
    Vif,
)

FLEET_UPKEEP_MULTIPLICATOR = (
    1
    +
    # reduce by half if policy adopted
    (
        1
        - 0.5
        * StatisticIf(float, condition=IsSource & EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_ENGINEERING"))
    )
    * (
        # increase cost dependent on number of ships or number of ship parts
        (1 - GameRule(type=float, name="RULE_SHIP_PART_BASED_UPKEEP"))
        * ((0.01 * ShipDesignsOwned(empire=Source.Owner)) + (0.01 * ShipDesignsInProduction(empire=Source.Owner)))
        + GameRule(type=float, name="RULE_SHIP_PART_BASED_UPKEEP")
        * (
            (0.002 * ShipPartsOwned(empire=Source.Owner, class_=ShortRangeClass))
            + (0.002 * ShipPartsOwned(empire=Source.Owner, class_=FighterHangarClass))
            + (0.002 * ShipPartsOwned(empire=Source.Owner, class_=ArmourClass))
            + (0.002 * ShipPartsOwned(empire=Source.Owner, class_=TroopsClass))
            + (0.002 * ShipDesignsOwned(empire=Source.Owner))
            + (0.01 * ShipDesignsInProduction(empire=Source.Owner))
        )
    )
)

# ///////////////////////////
# // PLC_DESIGN_SIMPLICITY //

# gets registered in named_values.py as necessary vrefs are not implemented for the legacy focs.txt parser
DESIGN_SIMPLICITY_SOURCE_COMPLEXITY_COUNT_VREF = NumPartClassesInShipDesign(
    design=UsedInDesignID
) + PartOfClassInShipDesign(name="Colony", design=UsedInDesignID)


# The formula defining the cost factor depending on the complexity
# note this gets calculated every time complexity is checked
def DESIGN_SIMPLICITY_COMPLEXITY_FACTOR_FOR_ARG1_VREF(vref):
    return Vif(float, vref > 4.0, 1, Vif(float, vref > 3.0, 0.95, Vif(float, vref > 2.0, 0.9, 0.8)))


DESIGN_SIMPLICITY_POLICY_MULTIPLIER = 1.0 - (
    StatisticIf(float, condition=IsSource & EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_DESIGN_SIMPLICITY"))
    * (1.0 - DESIGN_SIMPLICITY_COMPLEXITY_FACTOR_FOR_ARG1_VREF(DESIGN_SIMPLICITY_SOURCE_COMPLEXITY_COUNT_VREF))
)
# ///////////////////////////

# used within a production cost calculation, in which the location at which the production happens
# is the Target object and another object owned by the producing empire is the source object
SHIP_HULL_COST_MULTIPLIER = (
    GameRule(type=float, name="RULE_SHIP_HULL_COST_FACTOR") * DESIGN_SIMPLICITY_POLICY_MULTIPLIER
)

SHIP_PART_COST_MULTIPLIER = (
    GameRule(type=float, name="RULE_SHIP_PART_COST_FACTOR") * DESIGN_SIMPLICITY_POLICY_MULTIPLIER
)
