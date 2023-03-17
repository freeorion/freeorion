# Policy Liberty doubles the stability malus from disliked buildings, policies and planetary foci
# Policy Conformance halves it
# Values are declared in common/named_values.focs.txt
POLICY_DISLIKE_SCALING = (
    1.0
    * (
        2.0
        ** (StatisticIf(float, condition=IsSource & EmpireHasAdoptedPolicy(empire=Target.Owner, name="PLC_LIBERTY")))
    )
    * (
        0.5
        ** (
            StatisticIf(float, condition=IsSource & EmpireHasAdoptedPolicy(empire=Target.Owner, name="PLC_CONFORMANCE"))
        )
    )
)
