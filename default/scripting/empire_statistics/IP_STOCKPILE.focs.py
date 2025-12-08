from focs._effects import EmpireStockpile, ResourceInfluence, Source
from focs._empire_statistics import EmpireStatistic

EmpireStatistic(name="IP_STOCKPILE", value=EmpireStockpile(empire=Source.Owner, resource=ResourceInfluence))
