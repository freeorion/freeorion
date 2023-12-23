from focs._effects import (
    Destroy,
    EffectsGroup,
    GenerateSitRepMessage,
    IsBuilding,
    IsSource,
    LocalCandidate,
    MaxOf,
    Object,
    OwnedBy,
    Partial,
    Planet,
    Population,
    SetOwner,
    SetVisibility,
    Source,
    Target,
    ThisBuilding,
    Turn,
    ValueVisibility,
)
from macros.misc import UNOWNED_EMPIRE_ID
from macros.priorities import POPULATION_OVERRIDE_PRIORITY

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # type: ignore[reportUnboundVariable]
    name="BLD_ABANDON_OUTPOST",
    description="BLD_ABANDON_OUTPOST_DESC",
    captureresult=DestroyOnCapture,  # type: ignore[reportUnboundVariable]
    buildcost=20,
    buildtime=1,
    location=Planet() & OwnedBy(empire=Source.Owner) & Population(high=0),
    effectsgroups=[
        # PREPARE FOR TWO TURNS
        EffectsGroup(
            scope=IsSource,
            activation=Turn(high=Source.CreationTurn + 2),
            stackinggroup="ABANDON_OUTPOST_STACK",
            effects=[
                GenerateSitRepMessage(
                    message="SITREP_OUTPOST_ABANDONED_PREPARATION",
                    label="SITREP_OUTPOST_ABANDONED_PREPARATION_LABEL",
                    icon="icons/tech/environmental_encapsulation.png",
                    parameters={"planet": Source.PlanetID, "rawtext": Source.CreationTurn + 4},
                    empire=Source.Owner,
                )
            ],
        ),
        # THEN CANT ABANDON IF BATTLE
        EffectsGroup(
            scope=IsSource,
            activation=Turn(low=Source.CreationTurn + 3, high=Source.System.LastTurnBattleHere),
            stackinggroup="ABANDON_OUTPOST_STACK",
            effects=[
                GenerateSitRepMessage(
                    message="SITREP_OUTPOST_ABANDONED_PREVENTED_BY_BATTLE",
                    label="SITREP_OUTPOST_ABANDONED_LABEL",
                    icon="icons/tech/environmental_encapsulation.png",
                    parameters={"planet": Target.PlanetID, "system": Target.SystemID},
                    empire=Source.Owner,
                )
            ],
        ),
        # ELSE ABANDON OUTPOST
        EffectsGroup(
            scope=Object(id=Source.PlanetID) & Planet(),
            activation=Turn(low=MaxOf(int, Source.System.LastTurnBattleHere + 1, Source.CreationTurn + 3)),
            stackinggroup="ABANDON_OUTPOST_STACK",
            priority=POPULATION_OVERRIDE_PRIORITY,
            effects=[
                SetOwner(empire=UNOWNED_EMPIRE_ID),
                GenerateSitRepMessage(
                    message="SITREP_OUTPOST_ABANDONED",
                    label="SITREP_OUTPOST_ABANDONED_LABEL",
                    icon="icons/tech/environmental_encapsulation.png",
                    parameters={"planet": Target.ID},
                    empire=Source.Owner,
                ),
                SetVisibility(empire=Source.Owner, visibility=MaxOf("Visibility", ValueVisibility, Partial)),
            ],
        ),
        EffectsGroup(
            scope=(LocalCandidate.PlanetID == Source.PlanetID) & IsBuilding() & (~IsBuilding(name=[ThisBuilding])),
            activation=Turn(low=MaxOf(int, Source.System.LastTurnBattleHere + 1, Source.CreationTurn + 3)),
            priority=POPULATION_OVERRIDE_PRIORITY,
            effects=[SetOwner(empire=UNOWNED_EMPIRE_ID)],
        ),
        EffectsGroup(
            scope=IsSource,
            activation=Turn(low=MaxOf(int, Source.System.LastTurnBattleHere + 1, Source.CreationTurn + 3)),
            effects=Destroy,
        ),
    ],
    icon="icons/tech/environmental_encapsulation.png",
)
