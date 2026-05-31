from typing import TypeAlias

_Color: TypeAlias = tuple[int, int, int] | tuple[int, int, int, int]

def Category(
    *,
    name: str,
    graphic: str,
    colour: _Color,
) -> None: ...
