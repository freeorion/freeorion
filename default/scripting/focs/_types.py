# Placeholder for module.
# mypy: disable-error-code="empty-body"
from typing import TypeVar, Union

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


# Type for return
class _Empire:
    ...


# Type for arguments, since invalid Empire is -1
_EmpireId = Union[_Empire, int]


class _Focus:
    ...


class _Resource:
    ...


class _Aggregator:
    ...


_T = TypeVar("_T", str, int, float)


class _Condition(bool):  # type: ignore
    """
    This value represent an expression that is resolved to boolean.

    We need to have a special "class" because we use integer operands with it (& | ~),
    which return int type, that breaks linter logic.
    """

    def __and__(self, other) -> Self:
        ...

    def __or__(self, other) -> Self:
        ...

    def __invert__(self) -> Self:
        ...


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


class _ID:
    ...

    def __eq__(self, other) -> _Condition:
        ...

    def __ne__(self, other) -> _Condition:
        ...


class _SystemID(_ID):
    ...


class _PlanetId(_ID):
    ...


class _DesignID(_ID):
    ...


_SpeciesValue = str


# Type hints for function arguments.  This could be a literal or _Value returned by other function.
_StringParam = str
_IntParam = int
# We assume that we accept int anywhere we could accept float
_FloatParam = Union[int, float]
_ValueParam = Union[int, float, str]


class _BuildingType:
    ...
