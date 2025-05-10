try:
    from focs._empire_statistics import *
except ModuleNotFoundError:
    pass

from empire_statistic_ import EmpireStatisticModule

EmpireStatisticModule(
    name="ARMED_MONSTER_COUNT", value=StatisticCount(float, condition=Ship & Monster & Armed & Unowned)
)
