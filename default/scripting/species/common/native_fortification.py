from species.common.planet_defense import AVERAGE_PLANETARY_DEFENSE
from species.common.planet_shields import AVERAGE_PLANETARY_SHIELDS, NATIVE_PLANETARY_SHIELDS

FORTICATION_PROBABILITY = 0.1 * MaxOf(float, 0.2 * Source.TargetPopulation, Source.Research + Source.Industry)

NATIVE_FORTIFICATION_MINIMAL = NATIVE_PLANETARY_SHIELDS(1)


DEFAULT_NATIVE_DEFENSE = [
    *AVERAGE_PLANETARY_SHIELDS,
    *AVERAGE_PLANETARY_DEFENSE,
    EffectsGroup(
        scope=IsSource & Turn(low=1, high=1) & (GalaxyMaxAIAggression >= 1),
        activation=Planet()
        & Random(probability=FORTICATION_PROBABILITY)
        & ~ContainedBy(
            Contains(
                Planet() & HasSpecial(name="MODERATE_TECH_NATIVES_SPECIAL")
                | Planet() & HasSpecial(name="HIGH_TECH_NATIVES_SPECIAL")
                | HasSpecies() & HasTag(name="PRIMITIVE")
            )
        ),
        effects=[
            Conditional(
                condition=~Random(probability=FORTICATION_PROBABILITY),
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
