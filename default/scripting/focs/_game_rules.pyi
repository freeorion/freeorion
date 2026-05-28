from focs._types import _T

def GameRule(
    name: str,
    description: str,
    category: str,
    type: type[_T],
    default: _T,
    rank: int,
    max: _T | None = None,
    min: _T | None = None,
    allowed: list[_T] | None = None,
) -> None: ...
