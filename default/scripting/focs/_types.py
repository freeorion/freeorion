# Module for typing checks in sceleton files
# It should be real py file to be importable, but this file is not actually used in runtime.
#
# Game still try to import it and find the FOCS objects, so we use try catch to supress an error in logs.
# Python do not have typing library included in FOCS parsing environment.
# See discussion here: https://github.com/freeorion/freeorion/issues/4844

try:
    from typing import Literal, TypeVar, overload

    class _Condition:
        """
        This value represent an expression that is resolved to boolean.

        We need to have a special "class" because we use integer operands with it (& | ~),
        which return int type, that breaks linter logic.
        """

        def __and__(self, other) -> "_Condition": ...  # type: ignore

        def __or__(self, other) -> "_Condition": ...  # type: ignore

        def __invert__(self) -> "_Condition": ...  # type: ignore
        def __gt__(self, other) -> "_Condition": ...  # type: ignore
        def __ge__(self, other) -> "_Condition": ...  # type: ignore
        def __lt__(self, other) -> "_Condition": ...  # type: ignore
        def __le__(self, other) -> "_Condition": ...  # type: ignore
        def __mul__(self, other) -> int:
            return 0

        def __rmul__(self, other) -> int:
            return 0

        def __eq__(self, other) -> "_Condition": ...  # type: ignore
        def __ne__(self, other) -> "_Condition": ...  # type: ignore

    class _ConditionalComposition:
        """
        Base class to type that return conditions when used in comparison operations.
        """

        def __eq__(self, other) -> _Condition:  # type: ignore
            ...

        def __ne__(self, other) -> _Condition:  # type: ignore
            ...

        def __lt__(self, other) -> _Condition: ...  # type: ignore[invalid-return-type]

        def __gt__(self, other) -> _Condition: ...  # type: ignore[invalid-return-type]

        def __le__(self, other) -> _Condition: ...  # type: ignore[invalid-return-type]

        def __ge__(self, other) -> _Condition: ...  # type: ignore[invalid-return-type]

    class _Int(_ConditionalComposition):  # pyrefly: ignore[inconsistent-inheritance]
        @overload
        def __mul__(self, other: int) -> "_Int": ...  # pyrefly: ignore[invalid-overload]
        @overload
        def __mul__(self, other) -> "_Float": ...  # type: ignore[invalid-return-type]
        @overload
        def __rmul__(self, other: int) -> "_Int": ...  # pyrefly: ignore[invalid-overload]
        @overload
        def __rmul__(self, other) -> "_Float": ...  # type: ignore[invalid-return-type]

        def __truediv__(self, other) -> "_Float": ...  # type: ignore[invalid-return-type]
        def __rtruediv__(self, other) -> "_Float": ...  # type: ignore[invalid-return-type]

        def __sub__(self, other) -> "_Int": ...  # type: ignore[invalid-return-type]
        def __rsub__(self, other) -> "_Int": ...  # type: ignore[invalid-return-type]

        def __add__(self, other) -> "_Int": ...  # type: ignore[invalid-return-type]
        def __radd__(self, other) -> "_Int": ...  # type: ignore[invalid-return-type]

        def __pow__(self, other) -> "_Int": ...  # type: ignore[invalid-return-type]
        def __rpow__(self, other) -> "_Int": ...  # type: ignore[invalid-return-type]

        def __neg__(self) -> "_Int": ...  # type: ignore[invalid-return-type]

    class _Float(_ConditionalComposition):  # pyrefly: ignore[inconsistent-inheritance]
        def __mul__(self, other) -> "_Float": ...  # type: ignore[invalid-return-type]
        def __rmul__(self, other) -> "_Float": ...  # type: ignore[invalid-return-type]

        def __truediv__(self, other) -> "_Float": ...  # type: ignore[invalid-return-type]
        def __rtruediv__(self, other) -> "_Float": ...  # type: ignore[invalid-return-type]

        def __sub__(self, other) -> "_Float": ...  # type: ignore[invalid-return-type]
        def __rsub__(self, other) -> "_Float": ...  # type: ignore[invalid-return-type]

        def __add__(self, other) -> "_Float": ...  # type: ignore[invalid-return-type]
        def __radd__(self, other) -> "_Float": ...  # type: ignore[invalid-return-type]

        def __pow__(self, other) -> "_Float": ...  # type: ignore[invalid-return-type]
        def __rpow__(self, other) -> "_Float": ...  # type: ignore[invalid-return-type]

        def __neg__(self) -> "_Float": ...  # type: ignore[invalid-return-type]

    class _MeterType: ...

    class _Effect: ...

    class _EffectGroup: ...

    class _StarType: ...

    class _Visibility: ...

    class _PlanetSize: ...

    class _PlanetType: ...

    class _PlanetEnvironment: ...

    # Type for return
    class _Empire: ...

    # Type for arguments, since invalid Empire is -1
    _EmpireId = _Empire | Literal[-1]

    class _Focus: ...

    class _Resource: ...

    class _Aggregator: ...

    _T = TypeVar("_T", str, int, float)
    _N = TypeVar("_N", int, float)
    # For some generic functions like Statistic
    # we need to specify type of processed argument
    # and type of output separately
    _T_IN = TypeVar("_T_IN", str, int, float, _Int)
    _T_OUT = TypeVar("_T_OUT", str, int, float, _Int)

    class _ID:
        def __eq__(self, other) -> _Condition: ...  # type: ignore

        def __ne__(self, other) -> _Condition: ...  # type: ignore

    class _FleetID(_ID): ...

    class _SystemID(_ID): ...

    class _PlanetId(_ID): ...

    class _DesignID(_ID): ...

    _SpeciesValue = str

    # Type hints for function arguments.  This could be a literal or _Int, _Float returned by other function.
    _StringParam = str
    _IntParam = int | _Int
    # We assume that we accept int anywhere we could accept float
    _FloatParam = int | float | _Int | _Float
    _ValueParam = int | float | str | _Int | _Float

    class _BuildingType: ...

except ModuleNotFoundError:
    pass
