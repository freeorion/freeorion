from focs._effects import (
    AddStarlanes,
    ContainedBy,
    Contains,
    DirectDistanceBetween,
    EffectsGroup,
    HasStarlane,
    InSystem,
    IsBuilding,
    IsRootCandidate,
    IsSource,
    LocalCandidate,
    MaxOf,
    MinimumNumberOf,
    NamedReal,
    NamedRealLookup,
    Number,
    Object,
    OwnedBy,
    Planet,
    Population,
    SetTargetHappiness,
    Source,
    SpeciesDislikes,
    SpeciesLikes,
    StarlaneToWouldBeAngularlyCloseToExistingStarlane,
    StarlaneToWouldBeCloseToObject,
    StarlaneToWouldCrossExistingStarlane,
    StatisticCount,
    System,
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


CAN_ADD_STARLANE_TO_SOURCE = (
    System
    & ~Contains(IsSource)
    & ~HasStarlane(from_=Object(id=Source.SystemID))
    # specify the system, not the source object. the condition will consider multiple lanes
    # connecting to a system as not crossing,
    # but if the object is not itself a system but is in a system,
    # then the system may be considered as crossing a lane from the object's location
    & ~StarlaneToWouldCrossExistingStarlane(from_=Object(id=Source.SystemID))
    & ~StarlaneToWouldBeAngularlyCloseToExistingStarlane(from_=Object(id=Source.SystemID), maxdotprod=0.87)
    & ~StarlaneToWouldBeCloseToObject(
        distance=20.0,
        from_=Object(id=Source.SystemID),
        closeto=(
            System  # don't go near other systems
            & ~IsSource  # source is probably a building or planet, exclude any planet or system at the location of the source, but don't assume that the source is in a system or is on a planet when doing so
            & ~Contains(IsSource)
            & ~ContainedBy(Contains(IsSource))
            & ~IsRootCandidate
            & ~Contains(IsRootCandidate)
            & ~ContainedBy(Contains(IsRootCandidate))
        ),
    )
)


DO_STARLANE_BORE = AddStarlanes(
    endpoint=MinimumNumberOf(
        number=1, sortkey=DirectDistanceBetween(Source.ID, LocalCandidate.ID), condition=CAN_ADD_STARLANE_TO_SOURCE
    )
)


BORE_POSSIBLE = Number(low=1, condition=CAN_ADD_STARLANE_TO_SOURCE)
