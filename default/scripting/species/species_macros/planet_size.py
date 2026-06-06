from focs._conditions import IsSource, Planet, Turn
from focs._effects_new import EffectsGroup, SetPlanetSize
from focs._enums import Huge, Large, Medium, Small, Tiny

LARGE_PLANET = [
    EffectsGroup(scope=IsSource, activation=Planet() & Turn(high=0), effects=SetPlanetSize(planetsize=Large)),
]

NOT_LARGE_PLANET = [
    EffectsGroup(
        scope=IsSource,
        activation=Planet() & Turn(high=0) & ~Planet(size=[Tiny, Medium]),
        effects=SetPlanetSize(planetsize=Small),
    )
]

HUGE_PLANET = [
    EffectsGroup(
        scope=IsSource,
        activation=Planet() & Turn(high=0),
        effects=SetPlanetSize(planetsize=Huge),
    )
]
