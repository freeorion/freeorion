from focs._types import _Condition, _EffectGroup

class _CaptureResult: ...

DefaultCaptureResult = _CaptureResult()  # TODO get a better name
DestroyOnCapture = _CaptureResult()

def BuildingType(
    *,
    name: str,
    description: str,
    buildcost: int | float,
    buildtime: int | float,  # https://github.com/freeorion/freeorion/pull/5160#discussion_r2020142743
    location: _Condition,
    effectsgroups: list[_EffectGroup],
    icon: str,
    captureresult: _CaptureResult = DefaultCaptureResult,
    enqueuelocation=_Condition(),
    tags: list[str] | None = None,
): ...
