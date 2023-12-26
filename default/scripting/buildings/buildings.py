from focs._effects import (
    EffectsGroup,
    InSystem,
    IsBuilding,
    MaxOf,
    NamedReal,
    NamedRealLookup,
    Object,
    OwnedBy,
    Planet,
    Population,
    SetTargetHappiness,
    Source,
    SpeciesDislikes,
    SpeciesLikes,
    StatisticCount,
    Target,
    ThisBuilding,
    Value,
)
from macros.opinion import POLICY_DISLIKE_SCALING

STABILITY_PER_LIKED_BUILDING_ON_PLANET = 4.0


STABILITY_PER_LIKED_BUILDING_IN_SYSTEM = 1.0


STABILITY_PER_DISLIKED_BUILDING_ON_PLANET = 4.0 * POLICY_DISLIKE_SCALING

STABILITY_PER_DISLIKED_BUILDING_IN_SYSTEM = 1.0 * POLICY_DISLIKE_SCALING


SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS = [
    # species like building on the same planet
    EffectsGroup(
        scope=(Object(id=Source.PlanetID) & Population(low=0.001) & SpeciesLikes(name=ThisBuilding)),
        accountinglabel="LIKES_BUILDING_LABEL",
        effects=[SetTargetHappiness(value=Value + STABILITY_PER_LIKED_BUILDING_ON_PLANET)],
    ),
    # species dislike building on the same planet
    EffectsGroup(
        scope=(Object(id=Source.PlanetID) & Population(low=0.001) & SpeciesDislikes(name=ThisBuilding)),
        accountinglabel="DISLIKES_BUILDING_LABEL",
        effects=[SetTargetHappiness(value=Value - STABILITY_PER_DISLIKED_BUILDING_ON_PLANET)],
    ),
    # species like building in the same system
    EffectsGroup(
        scope=(
            Planet()
            & InSystem(id=Source.SystemID)
            & Population(low=0.001)
            & ~Object(id=Source.PlanetID)  # but not on same planet, which is covered by above case
            & SpeciesLikes(name=ThisBuilding)
        ),
        accountinglabel="LIKES_BUILDING_LABEL",
        effects=[SetTargetHappiness(value=Value + STABILITY_PER_LIKED_BUILDING_IN_SYSTEM)],
    ),
    # species dislike building in the same system
    EffectsGroup(
        scope=(
            Planet()
            & InSystem(id=Source.SystemID)
            & Population(low=0.001)
            & ~Object(id=Source.PlanetID)
            & SpeciesDislikes(name=ThisBuilding)
        ),
        accountinglabel="DISLIKES_BUILDING_LABEL",
        effects=[SetTargetHappiness(value=Value - STABILITY_PER_DISLIKED_BUILDING_IN_SYSTEM)],
    ),
    # species like building in empire
    EffectsGroup(
        scope=(
            Planet()
            & OwnedBy(empire=Source.Owner)
            & Population(low=0.001)
            & ~InSystem(id=Source.SystemID)
            & SpeciesLikes(name=ThisBuilding)
        ),
        accountinglabel="LIKES_BUILDING_LABEL",
        effects=[
            SetTargetHappiness(
                value=Value
                + (
                    NamedReal(name="BUILDING_LIKE_EMPIRE_SQRT_SCALING", value=0.5)
                    * MaxOf(
                        float,
                        1.0,
                        StatisticCount(
                            float,
                            condition=IsBuilding()
                            & IsBuilding(name=[ThisBuilding])
                            & OwnedBy(empire=Source.Owner)
                            & ~InSystem(id=Target.SystemID),
                        ),
                    )
                    ** (-0.5)
                )
            )
        ],
    ),
    # species dislike building in empire
    EffectsGroup(
        scope=(
            Planet()
            & OwnedBy(empire=Source.Owner)
            & Population(low=0.001)
            & ~InSystem(id=Source.SystemID)
            & SpeciesDislikes(name=ThisBuilding)
        ),
        accountinglabel="DISLIKES_BUILDING_LABEL",
        effects=[
            SetTargetHappiness(
                value=Value
                - (
                    NamedRealLookup(name="BUILDING_LIKE_EMPIRE_SQRT_SCALING")
                    * MaxOf(
                        float,
                        1.0,
                        StatisticCount(
                            float,
                            condition=IsBuilding()
                            & IsBuilding(name=[ThisBuilding])
                            & OwnedBy(empire=Source.Owner)
                            & ~InSystem(id=Target.SystemID),
                        ),
                    )
                    ** (-0.5)
                )
                * POLICY_DISLIKE_SCALING
            )
        ],
    ),
]
