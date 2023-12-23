from focs._effects import (
    AddSpecial,
    Conditional,
    ContainedBy,
    Contains,
    EffectsGroup,
    GalaxyMaxAIAggression,
    HasSpecial,
    HasSpecies,
    HasTag,
    IsSource,
    MaxOf,
    OneOf,
    Planet,
    Random,
    Source,
    Turn,
)
from macros.misc import PLANET_DEFENSE_FACTOR, PLANET_SHIELD_FACTOR
from species.species_macros.detection import NATIVE_PLANETARY_DETECTION
from species.species_macros.planet_defense import AVERAGE_PLANETARY_DEFENSE, NATIVE_PLANETARY_DEFENSE
from species.species_macros.planet_shields import AVERAGE_PLANETARY_SHIELDS, _native_planetary_shields

_FORTIFICATION_PROBABILITY = 0.1 * MaxOf(float, 0.2 * Source.TargetPopulation, Source.Research + Source.Industry)


DEFAULT_NATIVE_DEFENSE = [
    *AVERAGE_PLANETARY_SHIELDS,
    *AVERAGE_PLANETARY_DEFENSE,
    EffectsGroup(
        scope=IsSource & Turn(low=1, high=1) & (GalaxyMaxAIAggression >= 1),
        activation=Planet()
        & Random(probability=_FORTIFICATION_PROBABILITY)
        & ~ContainedBy(
            Contains(
                Planet() & HasSpecial(name="MODERATE_TECH_NATIVES_SPECIAL")
                | Planet() & HasSpecial(name="HIGH_TECH_NATIVES_SPECIAL")
                | HasSpecies() & HasTag(name="PRIMITIVE")
            )
        ),
        effects=[
            Conditional(
                condition=~Random(probability=_FORTIFICATION_PROBABILITY),
                effects=[
                    AddSpecial(
                        name=OneOf(
                            str,
                            "NATIVE_FORTIFICATION_MINIMAL",
                            "NATIVE_FORTIFICATION_LOW",
                            "CLOUD_COVER_MASTER_SPECIAL",
                        )
                    ),
                ],
                else_=[
                    AddSpecial(
                        name=OneOf(
                            str,
                            "NATIVE_FORTIFICATION_MEDIUM",
                            "NATIVE_FORTIFICATION_HIGH",
                            "VOLCANIC_ASH_MASTER_SPECIAL",
                        )
                    ),
                ],
            )
        ],
    ),
]

ADVANCED_NATIVE_DEFENSE = [
    *AVERAGE_PLANETARY_SHIELDS,
    *AVERAGE_PLANETARY_DEFENSE,
    NATIVE_PLANETARY_DETECTION(10),
    NATIVE_PLANETARY_DEFENSE(10 * PLANET_DEFENSE_FACTOR),
    _native_planetary_shields(10 * PLANET_SHIELD_FACTOR),
    EffectsGroup(
        scope=IsSource & Turn(low=1, high=1) & (GalaxyMaxAIAggression >= 1),
        activation=Planet(),
        effects=[
            Conditional(
                condition=~ContainedBy(
                    Contains(
                        Planet() & HasSpecial(name="MODERATE_TECH_NATIVES_SPECIAL")
                        | Planet() & HasSpecial(name="HIGH_TECH_NATIVES_SPECIAL")
                    ),
                ),
                effects=[
                    AddSpecial(
                        name=OneOf(
                            str,
                            "NATIVE_FORTIFICATION_LOW",
                            "NATIVE_FORTIFICATION_MEDIUM",
                            "CLOUD_COVER_MASTER_SPECIAL",
                            "VOLCANIC_ASH_MASTER_SPECIAL",
                        )
                    )
                ],
                else_=[],
            )
        ],
    ),
]
