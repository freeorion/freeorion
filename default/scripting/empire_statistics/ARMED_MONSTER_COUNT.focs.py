from focs._effects import Armed, Monster, Ship, StatisticCount, Unowned
from focs._empire_statistics import EmpireStatistic

EmpireStatistic(name="ARMED_MONSTER_COUNT", value=StatisticCount(float, condition=Ship & Monster & Armed & Unowned))
