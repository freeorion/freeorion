from focs._types import _Condition, _EffectGroup

class _DestroyAction: ...

DestroyOnCapture = _DestroyAction()

def BuildingType(
    *,
    name: str,
    description: str,
    captureresult: _DestroyAction,
    buildcost: int,
    buildtime: int,
    location: _Condition,
    effectsgroups: list[_EffectGroup],
    icon: str,
): ...
