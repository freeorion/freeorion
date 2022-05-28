from enum import Enum


class DumpKey(Enum):
    EmpireColors = "EmpireColors"
    EmpireID = "EmpireID"
    CapitalID = "CapitalID"
    Output = "CurrentOutput"
    SHIP_CONT = "ShipCount"


class DumpType(Enum):
    str = "str"
    int = "int"
