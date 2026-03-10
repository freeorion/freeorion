from focs._effects import (
    AsteroidsSize,
    AsteroidsType,
    Barren,
    Conditional,
    Desert,
    EffectsGroup,
    GalaxyPlanetDensity,
    GasGiantSize,
    GasGiantType,
    Inferno,
    Large,
    Medium,
    Object,
    Ocean,
    OneOf,
    Radiated,
    Random,
    Size,
    Small,
    Source,
    Swamp,
    System,
    Terran,
    Tiny,
    Toxic,
    Tundra,
)
from focs._effects_new import CreatePlanet

EFFECT_CREATE_PLANET = CreatePlanet(
    type=OneOf("PlanetType", Barren, Desert, Inferno, Ocean, Radiated, Swamp, Terran, Toxic, Tundra),
    planetsize=OneOf("PlanetSize", Tiny, Small, Medium, Large),
)

EFFECT_CREATE_ASTEROID = CreatePlanet(
    type=AsteroidsType,
    planetsize=AsteroidsSize,
)

EFFECT_CREATE_GASGIANT = CreatePlanet(
    type=GasGiantType,
    planetsize=GasGiantSize,
)

# Create 0-5 planet bodies dependent on planet density setting
CREATE_PLANETS = [
    EffectsGroup(  # Low density: max 3 bodies: 0-3 planet, 0-1 asteroid, 0-1 gasgiant (none: 24.84+%)
        scope=System & Object(id=Source.SystemID),
        activation=Size(high=5) & (GalaxyPlanetDensity == 1),
        effects=[
            # 8% planet = 92% none
            Conditional(
                condition=Random(probability=0.08),
                effects=[EFFECT_CREATE_PLANET],
            ),
            # 50% asteroid, 10% planet = 40% none
            Conditional(
                condition=Random(probability=0.5),
                effects=[EFFECT_CREATE_ASTEROID],
                else_=[
                    Conditional(
                        condition=Random(probability=0.2),
                        effects=[EFFECT_CREATE_PLANET],
                    )
                ],
            ),
            # 10% planet, 22.5% gasgiant = 67.5% none
            Conditional(
                condition=Random(probability=0.1),
                effects=[EFFECT_CREATE_PLANET],
                else_=[
                    Conditional(
                        condition=Random(probability=0.25),
                        effects=[EFFECT_CREATE_GASGIANT],
                    )
                ],
            ),
        ],
    ),
    EffectsGroup(  # Medium density: max 4 bodies: 0-3 planet, 0-2 asteroid, 0-2 gasgiant (none: 15.53+%)
        scope=System & Object(id=Source.SystemID),
        activation=Size(high=5) & (GalaxyPlanetDensity == 2),
        effects=[
            # 15% asteroid, 17% planet = 68% none
            Conditional(
                condition=Random(probability=0.15),
                effects=[EFFECT_CREATE_ASTEROID],
                else_=[
                    Conditional(
                        condition=Random(probability=0.2),
                        effects=[EFFECT_CREATE_PLANET],
                    )
                ],
            ),
            # 30% planet = 70% none
            Conditional(
                condition=Random(probability=0.3),
                effects=[EFFECT_CREATE_PLANET],
            ),
            # 20% planet, 25.6% gasgiant = 54.4% none
            Conditional(
                condition=Random(probability=0.2),
                effects=[EFFECT_CREATE_PLANET],
                else_=[
                    Conditional(
                        condition=Random(probability=0.32),
                        effects=[EFFECT_CREATE_GASGIANT],
                    )
                ],
            ),
            # 20% asteroid, 20% gasgiant = 60% none
            Conditional(
                condition=Random(probability=0.2),
                effects=[EFFECT_CREATE_ASTEROID],
                else_=[
                    Conditional(
                        condition=Random(probability=0.25),
                        effects=[EFFECT_CREATE_GASGIANT],
                    )
                ],
            ),
        ],
    ),
    EffectsGroup(  # High density: max 5 bodies: 0-5 planet, 0-3 asteroid, 0-2 gasgiant (none: 4.39+%)
        scope=System & Object(id=Source.SystemID),
        activation=Size(high=5) & (GalaxyPlanetDensity == 3),
        effects=[
            # 50% asteroid, 12.5% planet 37.5% none
            Conditional(
                condition=Random(probability=0.5),
                effects=[EFFECT_CREATE_ASTEROID],
                else_=[
                    Conditional(
                        condition=Random(probability=0.25),
                        effects=[EFFECT_CREATE_PLANET],
                    )
                ],
            ),
            # 30% 2xplanet, 14% planet+asteroid, 10% asteroid = 46% none
            Conditional(
                condition=Random(probability=0.5),
                effects=[
                    Conditional(
                        condition=Random(probability=0.6),
                        effects=[
                            EFFECT_CREATE_PLANET,
                            EFFECT_CREATE_PLANET,
                        ],
                        else_=[
                            Conditional(
                                condition=Random(probability=0.7),
                                effects=[
                                    EFFECT_CREATE_PLANET,
                                    EFFECT_CREATE_ASTEROID,
                                ],
                                else_=[
                                    Conditional(
                                        condition=Random(probability=0.2),
                                        effects=[EFFECT_CREATE_ASTEROID],
                                    )
                                ],
                            )
                        ],
                    )
                ],
            ),
            # 35% planet, 19.5% gasgiant = 45.5% none
            Conditional(
                condition=Random(probability=0.35),
                effects=[EFFECT_CREATE_PLANET],
                else_=[
                    Conditional(
                        condition=Random(probability=0.3),
                        effects=[EFFECT_CREATE_GASGIANT],
                    )
                ],
            ),
            # 20% planet, 24% gasgiant = 56% none
            Conditional(
                condition=Random(probability=0.2),
                effects=[EFFECT_CREATE_PLANET],
                else_=[
                    Conditional(
                        condition=Random(probability=0.3),
                        effects=[EFFECT_CREATE_GASGIANT],
                    )
                ],
            ),
        ],
    ),
]
