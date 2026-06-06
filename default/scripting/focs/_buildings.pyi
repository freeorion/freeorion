from focs._enums import DefaultCaptureResult
from focs._types import _CaptureResult, _Condition, _EffectGroup, _FloatParam

def BuildingType(
    *,
    name: str,
    description: str,
    buildcost: _FloatParam,
    buildtime: _FloatParam,  # https://github.com/freeorion/freeorion/pull/5160#discussion_r2020142743
    colony: bool = False,
    shipyard: bool = False,
    species: str = "",
    location: _Condition,
    effectsgroups: list[_EffectGroup],
    icon: str,
    captureresult: _CaptureResult = DefaultCaptureResult,
    enqueuelocation=_Condition(),
    tags: list[str] | None = None,
): ...
