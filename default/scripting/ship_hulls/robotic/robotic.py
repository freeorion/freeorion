from focs._conditions import InSystem, IsSource, Structure, Turn
from focs._effects import EffectsGroup, SetStructure
from focs._sources import LocalCandidate
from focs._value_refs import NamedRealLookup, Value

# macros needed only for robotic hulls
ROBOTIC_REPAIR = EffectsGroup(
    scope=IsSource
    & (~InSystem() | (InSystem() & Turn(low=LocalCandidate.System.LastTurnBattleHere + 1)))
    & Structure(high=LocalCandidate.MaxStructure - 0.001),
    effects=SetStructure(value=Value + NamedRealLookup(name="SHP_ROBOTIC_REPAIR")),
)
