# This module should be imported with *, that means we should not import other things in it, otherwise things that should be imported from other modules will be imported from here.

from focs._types import _Condition, _EffectGroup, _PlanetEnvironment, _PlanetType

class _FocusType: ...

def FocusType(
    *,
    name: str,
    description: str,
    location: _Condition,
    graphic: str,
) -> _FocusType: ...
def Species(
    *,
    name: str,
    description: str,
    gameplay_description: str,
    tags: list[str],
    foci: list[_FocusType],
    defaultfocus: str,
    effectsgroups: list[_EffectGroup],
    graphic: str,
    environments: dict[_PlanetType, _PlanetEnvironment],
    likes: list[str] = [],
    dislikes: list[str] = [],
    can_produce_ships: bool = False,
    playable: bool = False,
    can_colonize: bool = False,
    native: bool = False,
    annexation_condition=None,
    spawnrate: int | None = None,
    spawnlimit: int | None = None,
    annexationcondition: _Condition | None = None,
    annexationcost: float = 0.0,
): ...
def SpeciesCensusOrdering(order: list[str]): ...
