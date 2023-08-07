# mypy: disable-error-code="empty-body"
from functools import total_ordering
from typing import Generic, TypeVar

from typing_extensions import Self


class _Effect:
    ...


class _EffectGroup:
    ...


class _StarType:
    ...


class _Visibility:
    ...


class _PlanetSize:
    ...


class _PlanetType:
    ...


class _PlanetEnvironment:
    ...


class _Empire:
    ...


class _Species(str):
    ...


class _Focus:
    ...


class _Resource:
    ...


class _Aggregator:
    ...


class _ID:
    ...


class _SystemID(_ID):
    ...


class _PlanetId(_ID):
    ...


_T = TypeVar("_T", bound=[int, float, str])


@total_ordering
class _Value(Generic[_T]):
    def __add__(self, other) -> Self:
        ...

    def __radd__(self, other) -> Self:
        ...

    def __sub__(self, other) -> Self:
        ...

    def __rsub__(self, other) -> Self:
        ...

    def __mul__(self, other) -> Self:
        ...

    def __rmul__(self, other) -> Self:
        ...

    def __floordiv__(self, other) -> Self:
        ...

    def __rfloordiv__(self, other) -> Self:
        ...

    def __truediv__(self, other) -> Self:
        ...

    def __rtruediv__(self, other) -> Self:
        ...

    def __pow__(self, other) -> Self:
        ...

    def __rpow__(self, other) -> Self:
        ...

    def __gt__(self, other) -> bool:
        ...

    def __eq__(self, other) -> bool:
        ...


_FloatValue = _Value[float]
_IntValue = _Value[int]


class _Condition:
    def __and__(self, other) -> Self:
        ...

    def __or__(self, other) -> Self:
        ...

    def __invert__(self) -> Self:
        ...


class _FocusType:
    ...


@total_ordering
class _Turn(_Condition):
    def __lt__(self, other) -> _Condition:
        ...

    def __eq__(self, other) -> _Condition:  # type: ignore[override]
        ...


@total_ordering
class _IntComparableCondition(_Condition):
    def __eq__(self, other) -> _Condition:  # type: ignore[override]
        ...

    def __lt__(self, other) -> _Condition:
        ...
