from focs._effects import EffectsGroup, IsSource, Large, Medium, Planet, SetPlanetSize, Small, Tiny, Turn

# TODO convert to list
LARGE_PLANET = EffectsGroup(scope=IsSource, activation=Planet() & Turn(high=0), effects=SetPlanetSize(planetsize=Large))


NOT_LARGE_PLANET = [
    EffectsGroup(
        scope=IsSource,
        activation=Planet() & Turn(high=0) & ~Planet(size=[Tiny, Medium]),
        effects=SetPlanetSize(planetsize=Small),
    )
]
