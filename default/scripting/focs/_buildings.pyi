from focs._types import _Condition, _EffectGroup

class _DestroyAction: ...

DestroyOnCapture = _DestroyAction()

def BuildingType(
    *,
    name: str,
    description: str,
    buildcost: int | float,
    buildtime: int,
    location: _Condition,
    effectsgroups: list[_EffectGroup],
    icon: str,
    captureresult=_DestroyAction,
    enqueuelocation=_Condition(),
): ...
