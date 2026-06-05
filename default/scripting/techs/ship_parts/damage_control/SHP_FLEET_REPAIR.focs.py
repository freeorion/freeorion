from focs._conditions import (
    EmpireHasAdoptedPolicy,
    InSystem,
    IsSource,
    OwnedBy,
    ResourceSupplyConnected,
    Ship,
    Stationary,
    Structure,
    Turn,
)
from focs._effects import AllyOf
from focs._effects_new import EffectsGroup, SetStructure
from focs._sources import LocalCandidate, Source, Target
from focs._techs import Tech
from focs._value_refs import (
    Value,
)
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_FLEET_REPAIR",
    description="SHP_FLEET_REPAIR_DESC",
    short_description="SHIP_REPAIR_DESC",
    category="SHIP_PARTS_CATEGORY",
    researchcost=130 * TECH_COST_MULTIPLIER,
    researchturns=10,
    tags=["PEDIA_DAMAGE_CONTROL_PART_TECHS"],
    prerequisites=["SHP_INTSTEL_LOG", "SHP_BASIC_DAM_CONT"],
    effectsgroups=[
        EffectsGroup(
            scope=Ship
            & InSystem()
            & Stationary
            & (
                OwnedBy(empire=Source.Owner)
                |
                # either ally providing repair or owner of ship beign repaired must adopt policy to share repairs
                (
                    EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_ALLIED_REPAIR")
                    | EmpireHasAdoptedPolicy(empire=LocalCandidate.Owner, name="PLC_ALLIED_REPAIR")
                )
                & OwnedBy(affiliation=AllyOf, empire=Source.Owner)
            )
            & Turn(low=LocalCandidate.System.LastTurnBattleHere + 1)
            & Structure(high=LocalCandidate.MaxStructure - 0.001)
            & ResourceSupplyConnected(empire=Source.Owner, condition=IsSource),
            stackinggroup="FLEET_REPAIR",
            effects=SetStructure(value=Value + (Target.MaxStructure / 10)),
        )
    ],
    graphic="icons/tech/fleet_field_repair.png",
)
