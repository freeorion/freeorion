from common.misc import PLANET_DEFENSE_FACTOR, PLANET_SHIELD_FACTOR
from species.common.env import RADIATED_STANDARD_EP
from species.common.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.common.happiness import AVERAGE_HAPPINESS
from species.common.industry import GREAT_INDUSTRY
from species.common.influence import GOOD_INFLUENCE
from species.common.planet_defense import AVERAGE_PLANETARY_DEFENSE
from species.common.planet_shields import AVERAGE_PLANETARY_SHIELDS
from species.common.population import AVERAGE_POPULATION
from species.common.research import GOOD_RESEARCH
from species.common.shields import ULTIMATE_SHIP_SHIELDS
from species.common.stockpile import GREAT_STOCKPILE
from species.common.supply import GREAT_SUPPLY
from species.common.troops import AVERAGE_DEFENSE_TROOPS

Species(
    name="SP_ACIREMA",
    description="SP_ACIREMA_DESC",
    gameplay_description="SP_ACIREMA_GAMEPLAY_DESC",
    native=True,
    can_produce_ships=True,
    tags=[
        "SELF_SUSTAINING",
        "GREAT_INDUSTRY",
        "GOOD_RESEARCH",
        "GOOD_INFLUENCE",
        "GOOD_PLANETARY_SHIELDS",
        "ULTIMATE_SHIP_SHIELDS",
        "GREAT_SUPPLY",
        "PEDIA_SELF_SUSTAINING_SPECIES_CLASS",
    ],
    foci=[
        HAS_INDUSTRY_FOCUS,
        HAS_RESEARCH_FOCUS,
        HAS_INFLUENCE_FOCUS,
        HAS_GROWTH_FOCUS,
        *HAS_ADVANCED_FOCI,
    ],
    defaultfocus="FOCUS_INDUSTRY",
    likes=[
        "PLC_EXPLORATION",
        "PLC_INDUSTRIALISM",
        "PLC_CENTRALIZATION",
        "PLC_MARTIAL_LAW",
        "PLC_RACIAL_PURITY",
    ],
    dislikes=[
        "PLC_DIVERSITY",
        "PLC_ENVIRONMENTALISM",
        "PLC_INTERSTELLAR_INFRA",
        "PLC_SYSTEM_INFRA",
        "PLC_ALLIED_REPAIR",
    ],
    effectsgroups=[
        *GREAT_INDUSTRY,
        *GOOD_RESEARCH,
        *GOOD_INFLUENCE,
        *GREAT_STOCKPILE,
        *AVERAGE_POPULATION,
        *AVERAGE_HAPPINESS,
        *GREAT_SUPPLY,
        *AVERAGE_DEFENSE_TROOPS,
        # not for description
        EffectsGroup(
            scope=IsSource,
            activation=Planet()
            & (
                Turn(low=10, high=10)
                | Turn(low=80, high=80)
                | Turn(low=150, high=150)
                | Turn(low=220, high=220)
                | Turn(low=290, high=290)
            )
            & ~WithinStarlaneJumps(jumps=1, condition=System & Contains(Planet() & OwnedBy(affiliation=AnyEmpire))),
            effects=CreateShip(designname="SM_ACIREMA_GUARD", species="SP_ACIREMA"),
        ),
        EffectsGroup(
            scope=Object(id=Source.SystemID) & System, activation=Turn(high=0), effects=SetStarType(type=Blue)
        ),
        *ULTIMATE_SHIP_SHIELDS,
        EffectsGroup(
            description="GOOD_PLANETARY_DEFENSE_DESC",
            scope=IsSource,
            activation=Planet(),
            effects=SetMaxDefense(value=Value + (5 * PLANET_DEFENSE_FACTOR)),
        ),
        EffectsGroup(
            description="GOOD_PLANETARY_SHIELD_DESC",
            scope=IsSource,
            activation=Planet(),
            effects=SetMaxShield(value=Value + (5 * PLANET_SHIELD_FACTOR)),
        ),
        *AVERAGE_PLANETARY_SHIELDS,
        *AVERAGE_PLANETARY_DEFENSE,
    ],
    **RADIATED_STANDARD_EP,
    graphic="icons/species/acirema.png",
)
