from focs._effects import (
    AddSpecial,
    Conditional,
    ContainedBy,
    Contains,
    DesignHasPart,
    Destroy,
    DirectDistanceBetween,
    EffectsGroup,
    EnemyOf,
    Fleet,
    Focus,
    GenerateSitRepMessage,
    InSystem,
    IsBuilding,
    IsSource,
    LocalCandidate,
    MoveTo,
    MoveTowards,
    Number,
    Object,
    OwnedBy,
    Planet,
    Random,
    RootCandidate,
    SetMaxSupply,
    SetPopulation,
    SetStealth,
    SetTargetPopulation,
    Ship,
    Source,
    Stationary,
    System,
    Target,
    Turn,
    Unowned,
    Value,
    WithinDistance,
    WithinStarlaneJumps,
)
from macros.priorities import (
    POPULATION_DEFAULT_PRIORITY,
    POPULATION_FIRST_PRIORITY,
    TARGET_POPULATION_AFTER_SCALING_PRIORITY,
)

PLANETARY_DRIVE_ACTIVATION = (
    Planet()
    & Focus(type=["FOCUS_PLANET_DRIVE"])
    & WithinStarlaneJumps(
        jumps=1,
        condition=System
        & Contains(
            (
                IsBuilding(name=["BLD_PLANET_BEACON"])
                | (
                    Ship
                    & DesignHasPart(low=1, high=999, name="SP_PLANET_BEACON")
                    & Turn(low=LocalCandidate.ArrivedOnTurn + 1)
                )
            )
            & OwnedBy(empire=Source.Owner)
        )
        & ~Contains(IsSource),
    )
)

ADVANCED_FOCUS_EFFECTS = [
    EffectsGroup(
        scope=IsSource, activation=Planet() & Focus(type=["FOCUS_STEALTH"]), effects=SetStealth(value=Value + 15)
    ),
    EffectsGroup(
        scope=Planet()
        & OwnedBy(affiliation=EnemyOf, empire=Source.Owner)
        & WithinStarlaneJumps(jumps=4, condition=IsSource)
        & ~Number(low=1, condition=IsBuilding(name=["BLD_GENOME_BANK"]) & OwnedBy(empire=RootCandidate.Owner)),
        activation=Focus(type=["FOCUS_BIOTERROR"]),
        priority=TARGET_POPULATION_AFTER_SCALING_PRIORITY,
        effects=SetTargetPopulation(value=Value - 4),
    ),
    EffectsGroup(
        scope=Fleet
        & OwnedBy(empire=Source.Owner)
        & ContainedBy(System & Contains(Planet() & OwnedBy(empire=Source.Owner) & Focus(type=["FOCUS_STARGATE_SEND"]))),
        activation=Planet() & Focus(type=["FOCUS_STARGATE_RECEIVE"]),
        stackinggroup="STARGATE_STACK",
        effects=[
            GenerateSitRepMessage(
                message="EFFECT_STARGATE",
                label="EFFECT_STARGATE_LABEL",
                icon="icons/focus/stargate_receive.png",
                parameters={"fleet": Target.ID, "system": Source.SystemID},
                empire=Source.Owner,
            ),
            MoveTo(destination=Contains(IsSource) & System),
        ],
    ),
    EffectsGroup(
        scope=IsSource,
        activation=PLANETARY_DRIVE_ACTIVATION,
        # add the special with higher priority, so it can trigger with the same
        # priotity as the effect below
        priority=POPULATION_FIRST_PRIORITY,
        effects=AddSpecial(name="STARLANE_DRIVE_ACTIVATED_SPECIAL"),
    ),
    EffectsGroup(
        scope=IsSource,
        activation=PLANETARY_DRIVE_ACTIVATION,
        priority=POPULATION_DEFAULT_PRIORITY,
        effects=[
            MoveTo(
                destination=System
                & WithinStarlaneJumps(jumps=1, condition=IsSource)
                & Contains(
                    (
                        IsBuilding(name=["BLD_PLANET_BEACON"])
                        | (
                            Ship
                            & DesignHasPart(low=1, high=999, name="SP_PLANET_BEACON")
                            & Turn(low=LocalCandidate.ArrivedOnTurn + 1)
                        )
                    )
                    & OwnedBy(empire=Source.Owner)
                )
                & ~Contains(IsSource)
            ),
            GenerateSitRepMessage(
                message="EFFECT_PLANET_DRIVE",
                label="EFFECT_PLANET_DRIVE_LABEL",
                icon="icons/building/planetary_stardrive.png",
                parameters={"planet": Source.ID, "system": Source.SystemID},
                empire=Source.Owner,
            ),
            SetPopulation(value=Value / 2),
        ],
    ),
    EffectsGroup(
        scope=IsSource,
        activation=Random(probability=0.5)
        & Planet()
        & Focus(type=["FOCUS_PLANET_DRIVE"])
        & WithinStarlaneJumps(
            jumps=1,
            condition=System
            & Contains(
                (
                    IsBuilding(name=["BLD_PLANET_BEACON"])
                    | (
                        Ship
                        & DesignHasPart(low=1, high=999, name="SP_PLANET_BEACON")
                        & Turn(low=LocalCandidate.ArrivedOnTurn + 1)
                    )
                )
                & OwnedBy(empire=Source.Owner)
            )
            & ~Contains(IsSource),
        )
        & ~WithinDistance(distance=200, condition=IsBuilding(name=["BLD_LIGHTHOUSE"])),
        effects=[
            GenerateSitRepMessage(
                message="SITREP_PLANET_DRIVE_FAILURE",
                label="SITREP_PLANET_DRIVE_FAILURE_LABEL",
                icon="icons/sitrep/colony_destroyed.png",
                parameters={"planet": Source.ID, "system": Source.SystemID},
                empire=Source.Owner,
            ),
            Destroy,
        ],
    ),
    EffectsGroup(
        scope=Fleet
        & ~Stationary
        & ~InSystem()
        & (OwnedBy(affiliation=EnemyOf, empire=Source.Owner) | Unowned)
        & (Source.System.ID == LocalCandidate.NextSystemID),
        activation=Planet() & Focus(type=["FOCUS_DISTORTION"]),
        stackinggroup="DISTORTION_MOVEMENT_STACK",
        effects=Conditional(
            condition=WithinDistance(distance=40, condition=Object(id=RootCandidate.PreviousSystemID)),
            effects=[
                GenerateSitRepMessage(
                    message="EFFECT_FLEET_MOVED_TO",
                    label="EFFECT_FLEET_MOVED_TO_LABEL",
                    parameters={
                        "fleet": Target.ID,
                        "system": Target.PreviousSystemID,
                        "rawtext": DirectDistanceBetween(Target.ID, Target.PreviousSystemID),
                        "planet": Source.ID,
                    },
                    empire=Source.Owner,
                ),
                MoveTo(destination=Object(id=Target.PreviousSystemID)),
            ],
            else_=[
                GenerateSitRepMessage(
                    message="EFFECT_FLEET_MOVED_TOWARDS",
                    label="EFFECT_FLEET_MOVED_TOWARDS_LABEL",
                    parameters={
                        "fleet": Target.ID,
                        "system": Target.PreviousSystemID,
                        "rawtext": DirectDistanceBetween(Target.ID, Target.PreviousSystemID),
                        "planet": Source.ID,
                    },
                    empire=Source.Owner,
                ),
                MoveTowards(speed=40, target=Object(id=Target.PreviousSystemID)),
            ],
        ),
    ),
    EffectsGroup(
        scope=IsSource,
        activation=Focus(type=["FOCUS_LOGISTICS"]),
        accountinglabel="SHP_INTSTEL_LOG",
        effects=SetMaxSupply(value=Value + 3),
    ),
]
