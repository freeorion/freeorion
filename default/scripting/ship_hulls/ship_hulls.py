from focs._conditions import (
    ContainedBy,
    Contains,
    DesignHasHull,
    DesignHasPart,
    Fleet,
    HasSpecies,
    HasTag,
    IsBuilding,
    IsSource,
    Monster,
    Object,
    OwnedBy,
    Planet,
    Random,
    Ship,
    Stationary,
    System,
    Turn,
    Unowned,
    WithinStarlaneJumps,
)
from focs._effects import (
    Conditional,
    EffectsGroup,
    GenerateSitRepMessage,
    SetDestination,
    SetDetection,
    SetFuel,
    SetMaxFuel,
)
from focs._enums import EnemyOf, GasGiantType
from focs._sources import LocalCandidate, Source, Target
from focs._value_refs import HullFuel, MinOf, NamedRealLookup, StatisticIf, Value
from macros.priorities import (
    AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
    METER_OVERRIDE_PRIORITY,
    TARGET_AFTER_2ND_SCALING_PRIORITY,
    TARGET_LAST_BEFORE_OVERRIDE_PRIORITY,
)

SCAVANGE_FUEL_UNOWNED = EffectsGroup(
    scope=IsSource, activation=Stationary & Unowned & Random(probability=0.6), effects=SetFuel(value=Value + 1)
)

REGULAR_HULL_DETECTION = EffectsGroup(scope=IsSource, effects=SetDetection(value=Value + 25))

UNOWNED_GOOD_VISION = EffectsGroup(
    scope=IsSource, activation=Unowned, accountinglabel="GOOD_VISION_LABEL", effects=SetDetection(value=Value + 50)
)

UNOWNED_MOVE = EffectsGroup(
    scope=Object(id=Source.FleetID) & Fleet,
    activation=Turn(low=10)
    & Stationary
    & Unowned
    & Random(probability=0.9)
    & ~(DesignHasHull(name="SH_ASTEROID") & Monster),
    stackinggroup="MONSTER_FLEET_MOVE_STACK",
    effects=SetDestination(
        destination=System
        & ~Object(id=Source.SystemID)
        & WithinStarlaneJumps(jumps=1, condition=IsSource)
        & ~Contains(IsBuilding(name="BLD_EXPERIMENTOR_OUTPOST"))
    ),
)


# macro to tell player when a ship regenerates its fuel
# arg1: base regen rate
def REFUEL_MESSAGE(arg1: float):
    return EffectsGroup(
        scope=IsSource,
        activation=Stationary
        & (Source.Fuel < 1)
        & (Source.MaxFuel >= 1)
        & (
            Source.Fuel
            + arg1 * StatisticIf(float, condition=IsSource & Turn(low=Source.ArrivedOnTurn + 1))
            + NamedRealLookup(name="FU_RAMSCOOP_REFUEL")
            * StatisticIf(
                float,
                condition=IsSource & DesignHasPart(name="FU_RAMSCOOP") & IsSource & Turn(low=Source.ArrivedOnTurn + 1),
            )
            + 0.1
            * StatisticIf(
                float,
                condition=IsSource
                & HasSpecies(name="SP_SLY")
                & ContainedBy(
                    Object(id=Source.SystemID)
                    & Contains(Planet(type=[GasGiantType]) & ~OwnedBy(affiliation=EnemyOf, empire=Source.Owner))
                ),
            )
            >= 1
        ),
        effects=GenerateSitRepMessage(
            message="EFFECT_SHIP_REFUELED",
            label="EFFECT_SHIP_REFUELED_LABEL",
            icon="icons/meter/fuel.png",
            parameters={"system": Source.SystemID, "ship": Source.ID},
            empire=Source.Owner,
        ),
    )


AVERAGE_BASE_FUEL_REGEN = [
    EffectsGroup(
        description="AVERAGE_BASE_FUEL_REGEN_DESC",
        scope=IsSource,
        activation=Turn(low=LocalCandidate.ArrivedOnTurn + 1) & Stationary & (Source.Fuel < Source.MaxFuel),
        stackinggroup="BASE_FUEL_REGEN",
        accountinglabel="BASE_FUEL_REGEN_LABEL",
        priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
        effects=SetFuel(value=MinOf(float, Target.MaxFuel, Value + 0.1)),
    ),
    REFUEL_MESSAGE(0.1),
]


def HULL_FUEL_EFFICIENCY_EFFECTSGROUP(label: str, multiplier: float):
    return [
        EffectsGroup(
            description="HULL_FUEL_EFFICIENCY_DESC",
            scope=IsSource & Ship,
            accountinglabel=f"{label}_FUEL_EFFICIENCY_LABEL",
            priority=TARGET_AFTER_2ND_SCALING_PRIORITY,
            effects=SetMaxFuel(
                value=Value
                * multiplier
                * StatisticIf(float, condition=IsSource & HasTag(name=f"{label}_FUEL_EFFICIENCY"))
            ),
        ),
        # If a ship has less than one maximum fuel it will never be able to travel out of supply so refueling does not make sense.
        # In order to communicate that and to prevent wrong refuel sitreps, we set the maximum fuel for that case to zero
        # I need the final MaxFuel value to decide if we should zero it.
        # Scope and activation conditions are evaluated before the effects so using an If condition = Value() in effects expression instead
        EffectsGroup(
            description="MAX_FUEL_LESS_THAN_ONE_DESC",
            scope=IsSource & Ship,
            accountinglabel="MAX_FUEL_LESS_THAN_ONE_LABEL",
            priority=METER_OVERRIDE_PRIORITY,
            effects=Conditional(
                condition=Value(Source.MaxFuel) < 1,
                effects=[SetMaxFuel(value=0)],
            ),
        ),
    ]


GREAT_FUEL_EFFICIENCY = HULL_FUEL_EFFICIENCY_EFFECTSGROUP("GREAT", 4)

# This adds the hull's base fuel to the max fuel meter after applying the fuel efficiency multiplier.
# Note the use of the default accountinglabel.
ADD_HULL_FUEL_TO_MAX_FUEL_METER = EffectsGroup(
    scope=IsSource,
    accountinglabel="TT_SHIP_HULL",
    priority=TARGET_LAST_BEFORE_OVERRIDE_PRIORITY,
    effects=SetMaxFuel(value=Value + HullFuel(name=Source.Hull)),
)
