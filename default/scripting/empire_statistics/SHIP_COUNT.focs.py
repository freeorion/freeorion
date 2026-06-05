from focs._empire_statistics import EmpireStatistic
from focs._sources import Source
from focs._value_refs import ShipDesignsOwned

EmpireStatistic(name="SHIP_COUNT", value=ShipDesignsOwned(empire=Source.Owner))
