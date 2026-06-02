from focs._effects_new import _Item
from focs._types import _EffectGroup, _FloatParam

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
) -> None: ...
