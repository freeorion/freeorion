from focs._conditions import IsSource
from focs._effects import EffectsGroup, SetShield
from focs._sources import Source, Target
from focs._value_refs import CurrentTurn
from macros.priorities import AFTER_ALL_TARGET_MAX_METERS_PRIORITY

MONSTER_SHIELD_REGENERATION = EffectsGroup(  # Increase shields to max when not in battle
    scope=IsSource,
    activation=(Source.LastTurnActiveInBattle < CurrentTurn),
    stackinggroup="SHIELD_REGENERATION_EFFECT",
    priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
    effects=SetShield(value=Target.MaxShield),
)
