# mypy: disable-error-code="empty-body"
from typing import Generic, TypeVar, Union

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


class _DesignID(_ID):
    ...


_T = TypeVar("_T", str, int, float)


class _Condition:
    def __and__(self, other) -> Self:
        ...

    def __or__(self, other) -> Self:
        ...

    def __invert__(self) -> Self:
        ...

    def __mul__(self, other: _T) -> _T:
        """
        Support for boolean multiplication.
        """
        return other

    def __rmul__(self, other: _T) -> _T:
        """
        Support for boolean multiplication.
        """
        return other


class _ConditionalComposition:
    """
    Base class to type that return conditions when used in comparison operations.
    """

    def __eq__(self, other) -> _Condition:  # type: ignore[override]
        ...

    def __ne__(self, other) -> _Condition:  # type: ignore[override]
        ...

    def __lt__(self, other) -> _Condition:
        ...

    def __gt__(self, other) -> _Condition:
        ...

    def __le__(self, other) -> _Condition:
        ...

    def __ge__(self, other) -> _Condition:
        ...


class _Value(Generic[_T], _ConditionalComposition):
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


class _FloatValue(_Value[float], float):  # type: ignore[misc]
    ...


class _IntValue(_Value[float], int):  # type: ignore[misc]
    ...


class _TurnValue(_Value[float], int):  # type: ignore[misc]
    ...


class _SpeciesValue(_Value[float], str):  # type: ignore[misc]
    ...


# Type hints for function arguments.  This could be a literal or _Value returned by other function.
_StringParam = Union[_Value[str], str]
_IntParam = Union[_Value[int], int]
# We assume that we accept int anywhere we could accept float
_FloatParam = Union[_Value[int], int, _Value[float], float]
_ValueParam = Union[_Value[int], int, _Value[float], float, _Value[str], str]


class _BuildingType:
    ...
