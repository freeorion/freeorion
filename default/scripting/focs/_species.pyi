from focs._types import _Condition, _EffectGroup, _FloatParam, _IntParam, _PlanetEnvironment, _PlanetType

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
    spawnrate: _IntParam | None = None,
    spawnlimit: _IntParam | None = None,
    annexation_condition: _Condition | None = None,
    annexation_cost: _FloatParam = 0.0,
): ...
