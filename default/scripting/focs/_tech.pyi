from typing_extensions import TypeAlias

from focs._types import _EffectGroup, _FloatParam

class _Item: ...
class _ItemType: ...

UnlockPolicy = _ItemType()
UnlockShipPart = _ItemType()
UnlockBuilding = _ItemType()
UnlockShipHull = _ItemType()

def Item(type: _ItemType, name: str) -> _Item: ...

_Color: TypeAlias = tuple[int, int, int] | tuple[int, int, int, int]

class _Category: ...

def Category(
    *,
    name: str,
    graphic: str,
    colour: _Color,
) -> _Category: ...
def Tech(
    name: str,
    description: str,
    short_description: str,
    category: str,
    researchcost: _FloatParam,
    researchturns: int,
    tags: list[str],
    prerequisites: list[str] = [],
    effectsgroups: list[_EffectGroup] = [],
    graphic: str = "",
    researchable=True,
    unlock=_Item(),
): ...
