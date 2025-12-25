from typing import TypeVar

class _GameRuleDefinition: ...

R = TypeVar("R", int, float, str, bool)

def GameRule(
    *,
    type: type[R],
    name: str,
    description: str,
    category: str,
    default: R,
    rank: int,
    min: float = 0.0,
    max: float = 0.0,
    allowed: list[str] = ...,
) -> _GameRuleDefinition: ...
