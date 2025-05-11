from focs._effects import Armed, Monster, Ship, StatisticCount, Unowned
from focs.empire_statistic_ import EmpireStatisticModule

EmpireStatisticModule(
    name="ARMED_MONSTER_COUNT", value=StatisticCount(float, condition=Ship & Monster & Armed & Unowned)
)
