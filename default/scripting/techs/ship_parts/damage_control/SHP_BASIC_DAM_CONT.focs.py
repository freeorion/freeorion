from common.base_prod import TECH_COST_MULTIPLIER
from common.misc import SHIP_STRUCTURE_FACTOR

Tech(
    name="SHP_BASIC_DAM_CONT",
    description="SHP_BASIC_DAM_CONT_DESC",
    short_description="STRUCTURE_SHORT_DESC",
    category="SHIP_PARTS_CATEGORY",
    researchcost=40 * TECH_COST_MULTIPLIER,
    researchturns=4,
    tags=["PEDIA_DAMAGE_CONTROL_PART_TECHS"],
    prerequisites="SHP_MIL_ROBO_CONT",
    unlock=Item(type=UnlockPolicy, name="PLC_ENGINEERING"),
    effectsgroups=EffectsGroup(
        scope=Ship
        & OwnedBy(empire=Source.Owner)
        & (~InSystem() | InSystem() & Turn(low=LocalCandidate.System.LastTurnBattleHere + 1))
        & Structure(high=LocalCandidate.MaxStructure - 0.001),
        effects=SetStructure(value=Value + SHIP_STRUCTURE_FACTOR),
    ),
)
