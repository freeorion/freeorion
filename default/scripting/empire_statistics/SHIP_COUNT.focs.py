from focs._effects import Source
from focs._empire_statistics import EmpireStatistic
from focs._value_refs import ShipDesignsOwned

EmpireStatistic(name="SHIP_COUNT", value=ShipDesignsOwned(empire=Source.Owner))
