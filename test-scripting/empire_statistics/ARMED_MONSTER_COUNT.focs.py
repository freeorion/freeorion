try:
    from focs._effects import *
except ModuleNotFoundError:
    pass

from focs._empire_statistics import EmpireStatistic

EmpireStatistic(name="ARMED_MONSTER_COUNT", value=StatisticCount(float, condition=Ship & Monster & Armed & Unowned))
