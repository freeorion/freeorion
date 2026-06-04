from focs._conditions import (
    Armed,
    Monster,
    Ship,
    Unowned,
)
from focs._empire_statistics import EmpireStatistic
from focs._value_refs import StatisticCount

EmpireStatistic(name="ARMED_MONSTER_COUNT", value=StatisticCount(float, condition=Ship & Monster & Armed & Unowned))
