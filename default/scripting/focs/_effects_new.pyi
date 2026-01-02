from focs._types import _Condition, _Effect, _FloatParam

def MoveInOrbit(
    speed: _FloatParam,
    focus: _Condition = ...,
    x: _FloatParam = ...,
    y: _FloatParam = ...,
) -> _Effect: ...
