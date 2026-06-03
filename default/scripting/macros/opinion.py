# Policy Liberty doubles the stability malus from disliked buildings, policies and planetary foci
# Policy Conformance halves it
# Values are declared in common/named_values.focs.txt
from focs._conditions import EmpireHasAdoptedPolicy, IsSource
from focs._effects import Target
from focs._value_refs import (
    NamedRealLookup,
    StatisticIf,
)

POLICY_DISLIKE_SCALING = (
    1.0
    * NamedRealLookup(name="PLC_LIBERTY_DISLIKE_FACTOR")
    ** (StatisticIf(float, condition=IsSource & EmpireHasAdoptedPolicy(empire=Target.Owner, name="PLC_LIBERTY")))
    * NamedRealLookup(name="PLC_CONFORMANCE_DISLIKE_FACTOR")
    ** (StatisticIf(float, condition=IsSource & EmpireHasAdoptedPolicy(empire=Target.Owner, name="PLC_CONFORMANCE")))
)
