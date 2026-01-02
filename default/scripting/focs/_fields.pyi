from focs._types import _EffectGroup

def FieldType(
    name: str,
    description: str,
    stealth: float,
    graphic: str,
    effectsgroups: list[_EffectGroup] | None = None,
    tags: list[str] | None = None,
) -> None: ...
