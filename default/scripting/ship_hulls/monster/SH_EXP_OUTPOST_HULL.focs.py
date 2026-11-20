from focs._conditions import All, InSystem, IsSource, NumberOf, OwnedBy, Planet, Ship, Turn
from focs._effects import (
    CreateBuilding,
    EffectsGroup,
    SetDamage,
    SetMaxSecondaryStat,
    SetPopulation,
    SetSecondaryStat,
    SetShield,
    SetSpecies,
)
from focs._enums import AnyEmpire, AsteroidsType, Core, GasGiantType, Internal
from focs._ship_hulls import Hull, Slot
from focs._sources import Source, Target
from focs._value_refs import Value
from macros.misc_pre import SHIP_SHIELD_FACTOR, SHIP_WEAPON_DAMAGE_FACTOR
from macros.priorities import POPULATION_OVERRIDE_PRIORITY

Hull(
    name="SH_EXP_OUTPOST_HULL",
    description="SH_EXP_OUTPOST_HULL_DESC",
    speed=0,
    fuel=1,
    NoDefaultFuelEffect=True,
    stealth=55,
    structure=1700,
    slots=[
        Slot(type=Core, position=(0.60, 0.45)),
        Slot(type=Internal, position=(0.40, 0.55)),
    ],
    buildcost=1,
    buildtime=1,
    producible=False,
    tags=["CAN_ALTER_STARLANES", "PEDIA_HULL_MONSTER_GUARD"],
    location=All,
    effectsgroups=[
        EffectsGroup(
            scope=NumberOf(
                number=1,
                condition=(Planet() & ~Planet(type=[AsteroidsType, GasGiantType]) & InSystem(id=Source.SystemID)),
            ),
            activation=Turn(high=0),
            priority=POPULATION_OVERRIDE_PRIORITY,
            effects=[
                SetSpecies(name="SP_EXPERIMENTOR"),
                SetPopulation(value=Target.TargetPopulation),
                CreateBuilding(type="BLD_EXPERIMENTOR_OUTPOST"),
            ],
        ),
        EffectsGroup(  # Render most shields and most of the weapons useless
            scope=Ship & OwnedBy(affiliation=AnyEmpire) & InSystem(id=Source.SystemID),
            effects=[
                SetShield(value=Value - (15 * SHIP_SHIELD_FACTOR)),
                SetDamage(partname="SR_WEAPON_1_1", value=Value - (1 * SHIP_WEAPON_DAMAGE_FACTOR)),
                SetDamage(partname="SR_WEAPON_2_1", value=Value - (3 * SHIP_WEAPON_DAMAGE_FACTOR)),
                SetDamage(partname="SR_WEAPON_3_1", value=Value - (4 * SHIP_WEAPON_DAMAGE_FACTOR)),
                SetDamage(partname="SR_WEAPON_4_1", value=Value - (5 * SHIP_WEAPON_DAMAGE_FACTOR)),
            ],
        ),
        EffectsGroup(  # Upgrade fighter defense
            scope=IsSource,
            effects=[
                SetMaxSecondaryStat(partname="SR_GRAV_PULSE", value=60),
                SetSecondaryStat(partname="SR_GRAV_PULSE", value=60),
            ],
        ),
    ],
    icon="",
    graphic="icons/monsters/experiment0.png",
)
