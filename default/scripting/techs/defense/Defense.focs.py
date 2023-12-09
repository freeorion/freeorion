from focs._effects import (
    CurrentTurn,
    EffectsGroup,
    LocalCandidate,
    MinOf,
    NamedRealLookup,
    OwnedBy,
    Planet,
    SetDefense,
    SetMaxShield,
    SetShield,
    SetTroops,
    Source,
    Target,
    Value,
)
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER
from macros.misc import PLANET_DEFENSE_FACTOR, PLANET_SHIELD_FACTOR
from macros.priorities import AFTER_ALL_TARGET_MAX_METERS_PRIORITY
from techs.defense.mines import EG_SYSTEM_MINES

Tech(
    name="DEF_ROOT_DEFENSE",
    description="DEF_ROOT_DEFENSE_DESC",
    short_description="TROOPS_SHORT_DESC",
    category="DEFENSE_CATEGORY",
    researchcost=1,
    researchturns=1,
    tags=["PEDIA_DEFENSE_CATEGORY"],
    effectsgroups=[
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner),
            stackinggroup="PLANET_SHIELDS_STACK_ROOT",
            effects=SetMaxShield(value=Value + PLANET_SHIELD_FACTOR, accountinglabel="DEF_ROOT_DEFENSE"),
        ),
        EffectsGroup(  # base regeneration of troops, defense and shields if not attacked
            scope=Planet()
            & OwnedBy(empire=Source.Owner)
            & (LocalCandidate.LastTurnAttackedByShip < CurrentTurn)
            & (LocalCandidate.LastTurnConquered < CurrentTurn),
            priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
            effects=[
                SetShield(value=MinOf(float, Value + PLANET_SHIELD_FACTOR, Value(Target.MaxShield))),
                SetDefense(value=MinOf(float, Value + PLANET_DEFENSE_FACTOR, Value(Target.MaxDefense))),
                SetTroops(value=MinOf(float, Value + 1, Value(Target.MaxTroops))),
            ],
        ),
        EffectsGroup(  # set minimum troops for just-colonized planets
            scope=Planet() & OwnedBy(empire=Source.Owner) & (LocalCandidate.LastTurnColonized == CurrentTurn),
            priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
            effects=SetTroops(
                value=MinOf(float, NamedRealLookup(name="IMPERIAL_GARRISON_MAX_TROOPS_FLAT"), Value(Target.MaxTroops))
            ),
        ),
    ],
    graphic="",
)

Tech(
    name="DEF_PLANET_CLOAK",
    description="DEF_PLANET_CLOAK_DESC",
    short_description="BUILDING_UNLOCK_SHORT_DESC",
    category="DEFENSE_CATEGORY",
    researchcost=9999 * TECH_COST_MULTIPLIER,
    researchturns=9999,
    researchable=False,
    tags=["PEDIA_DEFENSE_CATEGORY"],
    prerequisites=["SPY_STEALTH_3"],
    unlock=Item(type=UnlockBuilding, name="BLD_PLANET_CLOAK"),
)

Tech(
    name="DEF_SYST_DEF_MINE_1",
    description="DEF_SYST_DEF_MINE_1_DESC",
    short_description="DEFENSE_SHORT_DESC",
    category="DEFENSE_CATEGORY",
    researchcost=192 * TECH_COST_MULTIPLIER,
    researchturns=4,
    tags=["PEDIA_DEFENSE_CATEGORY"],
    prerequisites=["DEF_DEFENSE_NET_1"],
    effectsgroups=EG_SYSTEM_MINES(
        NamedRealLookup(name="DEF_SYST_DEF_MINE_1_DAMAGE"), 75, "EMPIRE"
    ),  # Priority deliberately not a macro and before all priority macros
    graphic="icons/tech/system_defense_mines.png",
)

Tech(
    name="DEF_SYST_DEF_MINE_2",
    description="DEF_SYST_DEF_MINE_2_DESC",
    short_description="DEFENSE_SHORT_DESC",
    category="DEFENSE_CATEGORY",
    researchcost=360 * TECH_COST_MULTIPLIER,
    researchturns=6,
    tags=["PEDIA_DEFENSE_CATEGORY"],
    prerequisites=["DEF_SYST_DEF_MINE_1"],
    effectsgroups=EG_SYSTEM_MINES(NamedRealLookup(name="DEF_SYST_DEF_MINE_2_DAMAGE"), 65, "EMPIRE"),
    graphic="icons/tech/system_defense_mines.png",
)

Tech(
    name="DEF_SYST_DEF_MINE_3",
    description="DEF_SYST_DEF_MINE_3_DESC",
    short_description="DEFENSE_SHORT_DESC",
    category="DEFENSE_CATEGORY",
    researchcost=600 * TECH_COST_MULTIPLIER,
    researchturns=8,
    tags=["PEDIA_DEFENSE_CATEGORY"],
    prerequisites=["DEF_SYST_DEF_MINE_2"],
    effectsgroups=EG_SYSTEM_MINES(NamedRealLookup(name="DEF_SYST_DEF_MINE_3_DAMAGE"), 60, "EMPIRE"),
    graphic="icons/tech/system_defense_mines.png",
)
