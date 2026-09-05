from focs._types import _Condition, _EffectGroup, _FloatParam, _ShipSlotType

class _ShipSlot: ...

def Slot(*, type: _ShipSlotType, position: tuple[float, float]) -> _ShipSlot: ...
def Hull(
    *,
    name: str,
    description: str,
    exclusions: list[str] | None = None,
    speed: float,
    NoDefaultSpeedEffect: bool = False,
    fuel: float,
    NoDefaultFuelEffect: bool = False,
    stealth: float,
    NoDefaultStealthEffect: bool = False,
    structure: float,
    NoDefaultStructureEffect: bool = False,
    slots: list[_ShipSlot],
    buildcost: _FloatParam,
    buildtime: _FloatParam,
    producible: bool = True,
    tags: list[str] | None = None,
    location: _Condition,
    enqueuelocation=_Condition(),
    effectsgroups: list[_EffectGroup] | None = None,
    icon: str,
    graphic: str,
) -> None: ...
