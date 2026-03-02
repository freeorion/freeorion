from focs._types import (
    _Condition,
    _IntParam,
    _ShipPartClass,
)

def DesignHasPartClass(name: _ShipPartClass, low: _IntParam = ..., high: _IntParam = ...) -> _Condition: ...
