from typing import TypeVar

class _GameRuleDefinition: ...

R = TypeVar("R", int, float, str)

def GameRuleDefinition(
    *,
    type: type[R],
    name: str,
    description="",
    category="",
    default: R = ...,
    min: float = 0.0,
    max: float = 0.0,
    allowed: list[str] = ...,
) -> _GameRuleDefinition: ...
