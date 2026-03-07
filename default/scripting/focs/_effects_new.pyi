from focs._types import _Condition, _Effect, _FloatParam, _PlanetSize, _PlanetType, _StringParam

def MoveInOrbit(
    speed: _FloatParam,
    focus: _Condition = ...,
    x: _FloatParam = ...,
    y: _FloatParam = ...,
) -> _Effect: ...
def CreatePlanet(
    type: _PlanetType,
    planetsize: _PlanetSize,
    name: _StringParam = ...,
    initial_effects: list[_Effect] = ...,
) -> _Effect: ...
