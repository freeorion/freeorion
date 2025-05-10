from empire_statistic_ import EmpireStatisticModule
from focs._effects import Armed, Monster, Ship, StatisticCount, Unowned

EmpireStatisticModule(
    name="ARMED_MONSTER_COUNT", value=StatisticCount(float, condition=Ship & Monster & Armed & Unowned)
)
