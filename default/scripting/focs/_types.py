# mypy: disable-error-code="empty-body"
from functools import total_ordering
from typing import Generic, TypeVar

from typing_extensions import Self


class _Effect:
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


class _Agregator:
    ...


class _ID:
    ...


class _SystemID(_ID):
    ...


class _PlanetId(_ID):
    ...


_T = TypeVar("_T")


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


class _FloatValue(_Value[_T]):
    ...


class _IntValue(_Value[_T]):
    ...


class _Scope:
    def __and__(self, other) -> Self:
        ...

    def __or__(self, other) -> Self:
        ...

    def __invert__(self) -> Self:
        ...


@total_ordering
class _Turn(_Scope):
    def __lt__(self, other) -> _Scope:
        ...

    def __eq__(self, other) -> _Scope:  # type: ignore[override]
        ...


@total_ordering
class _IntComparableScope(_Scope):
    def __eq__(self, other) -> _Scope:  # type: ignore[override]
        ...

    def __lt__(self, other) -> _Scope:
        ...
