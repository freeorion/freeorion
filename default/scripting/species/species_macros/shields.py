from focs._effects import (
    CurrentTurn,
    DesignHasPart,
    EffectsGroup,
    GalaxyMaxAIAggression,
    IsHuman,
    IsSource,
    LocalCandidate,
    SetMaxShield,
    SetShield,
    Ship,
    StatisticIf,
    Target,
    Value,
)
from macros.misc import SHIP_SHIELD_FACTOR
from macros.priorities import AFTER_ALL_TARGET_MAX_METERS_PRIORITY, DEFAULT_PRIORITY

STANDARD_SHIP_SHIELDS = [
    EffectsGroup(  # gives human bonuses when AI Aggression set to Beginner
        scope=IsSource,
        activation=IsHuman & (GalaxyMaxAIAggression == 0) & Ship,  # human player, not human species
        accountinglabel="DIFFICULTY",
        priority=DEFAULT_PRIORITY,
        effects=SetMaxShield(value=Value + (1 * SHIP_SHIELD_FACTOR)),
    ),
    EffectsGroup(  # increase to max when not in battle
        scope=IsSource,
        activation=Ship & (LocalCandidate.LastTurnActiveInBattle < CurrentTurn),
        stackinggroup="SHIELD_REGENERATION_EFFECT",
        priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
        effects=SetShield(value=Target.MaxShield),
    ),
]


def _shield_type_base_value(shield_tag):
    return SHIP_SHIELD_FACTOR * StatisticIf(float, condition=IsSource & DesignHasPart(name=shield_tag))


def _ship_shields(
    *, tag: str, defence_grid: float, deflector: float, plasma: float, multispec: float, black: float
) -> list:
    return [
        *STANDARD_SHIP_SHIELDS,
        EffectsGroup(
            description=f"{tag}_SHIP_SHIELD_DESC",
            scope=IsSource & Ship,
            effects=SetMaxShield(
                value=(
                    Value
                    + defence_grid * _shield_type_base_value("SH_DEFENSE_GRID")
                    + deflector * _shield_type_base_value("SH_DEFLECTOR")
                    + plasma * _shield_type_base_value("SH_PLASMA")
                    + multispec * _shield_type_base_value("SH_MULTISPEC")
                    + black * _shield_type_base_value("SH_BLACK")
                )
            ),
        ),
    ]


BAD_SHIP_SHIELDS = _ship_shields(tag="BAD", defence_grid=-0.5, deflector=-1.0, plasma=-1.5, multispec=-2.0, black=-2.5)
GOOD_SHIP_SHIELDS = _ship_shields(tag="GOOD", defence_grid=0.5, deflector=1.0, plasma=1.5, multispec=2.0, black=2.5)
GREAT_SHIP_SHIELDS = _ship_shields(tag="GREAT", defence_grid=1.0, deflector=2.0, plasma=3.0, multispec=4.0, black=5.0)
ULTIMATE_SHIP_SHIELDS = _ship_shields(
    tag="ULTIMATE", defence_grid=1.5, deflector=3.0, plasma=4.5, multispec=6.0, black=7.5
)
