from focs._effects import (
    EmpireHasAdoptedPolicy,
    IsSource,
    Source,
    SpeciesColoniesOwned,
    StatisticIf,
)

COLONY_UPKEEP_MULTIPLICATOR = 1 + 0.06 * SpeciesColoniesOwned(empire=Source.Owner)

COLONIZATION_POLICY_MULTIPLIER = (
    1
    - (StatisticIf(float, condition=IsSource & EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_COLONIZATION")))
    / 3
    + (StatisticIf(float, condition=IsSource & EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_CENTRALIZATION")))
    / 3
)
