try:
    from focs._empire_statistics import *
except ModuleNotFoundError:
    pass

EmpireStatistic(name="ARMED_MONSTER_COUNT", value=StatisticCount(float, condition=Ship & Monster & Armed & Unowned))
