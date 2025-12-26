from focs._effects import (
    BuildBuilding,
    Contains,
    Destroy,
    EffectsGroup,
    Enqueued,
    GenerateSitRepMessage,
    HasSpecial,
    IsBuilding,
    IsSource,
    Object,
    OwnedBy,
    Planet,
    RemoveSpecial,
    Source,
    Target,
)
from macros.priorities import (
    SPECIAL_REMOVAL_PRIORITY,
)

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass


def EG_NEST_REMOVAL(monster_name):
    special_name = f"{monster_name}_NEST_SPECIAL"

    return EffectsGroup(
        scope=(Object(id=Source.PlanetID) & Planet() & HasSpecial(name=special_name)),
        # Remove the special at very late priority so meter changes based on it
        # (e.g. stability likes/dislikes) are allowed to happen beforehand.
        # This is to consistent with meter prediction in UI (which does not take special removal into account)
        # NB: only works as intended if we do not do a second full effect application round for setting meters of newly created objects)
        #     else the special will be already removed when the second application happens and the meter effects wont happen
        priority=SPECIAL_REMOVAL_PRIORITY,
        effects=[
            RemoveSpecial(name=special_name),
            GenerateSitRepMessage(
                message="EFFECT_NEST_REMOVAL",
                label="EFFECT_NEST_REMOVAL_LABEL",
                icon="icons/building/nest_eradicator.png",
                parameters={"planet": Target.ID},
                empire=Source.Owner,
            ),
        ],
    )


BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_NEST_ERADICATOR",
    description="BLD_NEST_ERADICATOR_DESC",
    buildcost=80,
    buildtime=5,
    location=(
        Planet()
        & OwnedBy(empire=Source.Owner)
        & ~Contains(IsBuilding(name=["BLD_NEST_ERADICATOR"]))
        & (
            HasSpecial(name="JUGGERNAUT_NEST_SPECIAL")
            | HasSpecial(name="KRAKEN_NEST_SPECIAL")
            | HasSpecial(name="SNOWFLAKE_NEST_SPECIAL")
        )
    ),
    enqueuelocation=~Enqueued(
        type=BuildBuilding,
        name="BLD_NEST_ERADICATOR",
    ),
    effectsgroups=[
        EG_NEST_REMOVAL("JUGGERNAUT"),
        EG_NEST_REMOVAL("KRAKEN"),
        EG_NEST_REMOVAL("SNOWFLAKE"),
        EffectsGroup(
            scope=IsSource,
            effects=[Destroy],
        ),
    ],
    icon="icons/building/nest_eradicator.png",
)
