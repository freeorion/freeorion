from macros.opinion import POLICY_DISLIKE_SCALING
from macros.priorities import AFTER_ALL_TARGET_MAX_METERS_PRIORITY

try:
    from focs._effects import (
        Abs,
        Adequate,
        Capital,
        Contains,
        CurrentTurn,
        EffectsGroup,
        EmpireHasAdoptedPolicy,
        GameRule,
        GasGiantType,
        Good,
        HasSpecies,
        HasTag,
        Homeworld,
        Hostile,
        Huge,
        InSystem,
        IsBuilding,
        IsSource,
        JumpsBetween,
        Large,
        LocalCandidate,
        MaxOf,
        Medium,
        Min,
        MinOf,
        NamedReal,
        OwnedBy,
        Planet,
        Poor,
        Population,
        ResourceSupplyConnected,
        SetHappiness,
        SetTargetHappiness,
        Small,
        Source,
        SpeciesDislikes,
        SpeciesLikes,
        Statistic,
        Target,
        Tiny,
        Uninhabitable,
        Unowned,
        Value,
    )
except ModuleNotFoundError:
    pass


STABILITY_PER_LIKED_FOCUS = 2.0

# Policy Liberty doubles de focus dislike effect, policy Conformance halves it, macro is in macros/opinion.macros
STABILITY_PER_DISLIKED_FOCUS = 2.0 * POLICY_DISLIKE_SCALING

ENVIRONMENT_STABILITY_MODIFIER = [
    EffectsGroup(
        scope=IsSource,
        activation=Planet()
        & Planet(environment=[Uninhabitable])
        & (GameRule(type=int, name="RULE_HOSTILE_ENVIRONMENT_STABILITY") != 0),
        accountinglabel="UNINHABTIABLE_ENVIRONMENT_STABILITY_LABEL",
        effects=SetTargetHappiness(value=Value + GameRule(type=float, name="RULE_HOSTILE_ENVIRONMENT_STABILITY")),
    ),
    EffectsGroup(
        scope=IsSource,
        activation=Planet()
        & Planet(environment=[Hostile])
        & (GameRule(type=int, name="RULE_HOSTILE_ENVIRONMENT_STABILITY") != 0),
        accountinglabel="HOSTILE_ENVIRONMENT_STABILITY_LABEL",
        effects=SetTargetHappiness(value=Value + GameRule(type=float, name="RULE_HOSTILE_ENVIRONMENT_STABILITY")),
    ),
    EffectsGroup(
        scope=IsSource,
        activation=Planet()
        & Planet(environment=[Poor])
        & (GameRule(type=int, name="RULE_POOR_ENVIRONMENT_STABILITY") != 0),
        accountinglabel="POOR_ENVIRONMENT_STABILITY_LABEL",
        effects=SetTargetHappiness(value=Value + GameRule(type=float, name="RULE_POOR_ENVIRONMENT_STABILITY")),
    ),
    EffectsGroup(
        scope=IsSource,
        activation=Planet()
        & Planet(environment=[Adequate])
        & (GameRule(type=int, name="RULE_ADEQUATE_ENVIRONMENT_STABILITY") != 0),
        accountinglabel="ADEQUATE_ENVIRONMENT_STABILITY_LABEL",
        effects=SetTargetHappiness(value=Value + GameRule(type=float, name="RULE_ADEQUATE_ENVIRONMENT_STABILITY")),
    ),
    EffectsGroup(
        scope=IsSource,
        activation=Planet()
        & Planet(environment=[Good])
        & (GameRule(type=int, name="RULE_GOOD_ENVIRONMENT_STABILITY") != 0),
        accountinglabel="GOOD_ENVIRONMENT_STABILITY_LABEL",
        effects=SetTargetHappiness(value=Value + GameRule(type=float, name="RULE_GOOD_ENVIRONMENT_STABILITY")),
    ),
]

PLANET_SIZE_STABILITY_MODIFIER = [
    EffectsGroup(
        scope=IsSource,
        activation=Planet() & Planet(size=[Tiny]) & (GameRule(type=int, name="RULE_TINY_SIZE_STABILITY") != 0),
        accountinglabel="TINY_PLANET_STABILITY_LABEL",
        effects=SetTargetHappiness(value=Value + GameRule(type=float, name="RULE_TINY_SIZE_STABILITY")),
    ),
    EffectsGroup(
        scope=IsSource,
        activation=Planet() & Planet(size=[Small]) & (GameRule(type=int, name="RULE_SMALL_SIZE_STABILITY") != 0),
        accountinglabel="SMALL_PLANET_STABILITY_LABEL",
        effects=SetTargetHappiness(value=Value + GameRule(type=float, name="RULE_SMALL_SIZE_STABILITY")),
    ),
    EffectsGroup(
        scope=IsSource,
        activation=Planet() & Planet(size=[Medium]) & (GameRule(type=int, name="RULE_MEDIUM_SIZE_STABILITY") != 0),
        accountinglabel="SMALL_MEDIUM_STABILITY_LABEL",
        effects=SetTargetHappiness(value=Value + GameRule(type=float, name="RULE_MEDIUM_SIZE_STABILITY")),
    ),
    EffectsGroup(
        scope=IsSource,
        activation=Planet() & Planet(size=[Large]) & (GameRule(type=int, name="RULE_LARGE_SIZE_STABILITY") != 0),
        accountinglabel="LARGE_PLANET_STABILITY_LABEL",
        effects=SetTargetHappiness(value=Value + GameRule(type=float, name="RULE_LARGE_SIZE_STABILITY")),
    ),
    EffectsGroup(
        scope=IsSource,
        activation=Planet() & Planet(size=[Huge]) & (GameRule(type=int, name="RULE_HUGE_SIZE_STABILITY") != 0),
        accountinglabel="HUGE_PLANET_STABILITY_LABEL",
        effects=SetTargetHappiness(value=Value + GameRule(type=float, name="RULE_HUGE_SIZE_STABILITY")),
    ),
    EffectsGroup(
        scope=IsSource,
        activation=Planet()
        & Planet(type=[GasGiantType])
        & (GameRule(type=int, name="RULE_GAS_GIANT_SIZE_STABILITY") != 0),
        accountinglabel="GAS_GIANT_STABILITY_LABEL",
        effects=SetTargetHappiness(value=Value + GameRule(type=float, name="RULE_GAS_GIANT_SIZE_STABILITY")),
    ),
]


COMMON_HAPPINESS_EFFECTS = [
    *ENVIRONMENT_STABILITY_MODIFIER,
    *PLANET_SIZE_STABILITY_MODIFIER,
    EffectsGroup(  # increase or decrease 1 per turn towards target
        scope=IsSource,
        activation=Planet() & (LocalCandidate.LastTurnConquered < CurrentTurn),
        priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
        effects=SetHappiness(
            value=Value
            + MinOf(float, Abs(float, Value(Target.TargetHappiness) - Value), 1)
            * (1 - 2 * (Value > Value(Target.TargetHappiness)))
        ),
    ),
    EffectsGroup(  # artistic species make other planets with different artistic species on them and in the same system system more stable
        scope=Planet()
        & InSystem(id=Source.SystemID)
        & ~IsSource
        & HasSpecies()
        & ~HasSpecies(name=[Source.Species])
        & HasTag(name="ARTISTIC")
        # (SpeciesSpeciesOpinion species = LocalCandidate.Species species = Source.Species > 0) # TODO: implement within an Or condition, once species-species opinions are working...
        ,
        activation=Planet() & HasSpecies() & HasTag(name="ARTISTIC"),
        accountinglabel="ARTISAN_APPRECIATION",
        effects=SetTargetHappiness(value=Value + 1.0),
    ),
    EffectsGroup(
        scope=IsSource,
        activation=(GameRule(type=int, name="RULE_BASELINE_PLANET_STABILITY") != 0),
        accountinglabel="RULE_BASELINE_PLANET_STABILITY",
        effects=SetTargetHappiness(value=Value + GameRule(type=float, name="RULE_BASELINE_PLANET_STABILITY")),
    ),
    EffectsGroup(  # more stable when liked focuses are adopted
        scope=IsSource,
        activation=Population(low=0.001) & SpeciesLikes(name=LocalCandidate.Focus),
        accountinglabel="LIKES_FOCUS_LABEL",
        effects=SetTargetHappiness(value=Value + STABILITY_PER_LIKED_FOCUS),
    ),
    EffectsGroup(  # less stable when disliked focuses are adopted
        scope=IsSource,
        activation=~Unowned & Population(low=0.001) & SpeciesDislikes(name=LocalCandidate.Focus),
        accountinglabel="DISLIKES_FOCUS_LABEL",
        effects=SetTargetHappiness(value=Value - STABILITY_PER_DISLIKED_FOCUS),
    ),
    EffectsGroup(  # species on their own homeworld are more stable
        scope=IsSource,
        activation=Homeworld(name=[Source.Species]),
        accountinglabel="HOMEWORLD_BONUS",
        effects=SetTargetHappiness(value=Value + 5),
    ),
    EffectsGroup(  # capital planet is more stable
        scope=IsSource,
        activation=Capital,
        accountinglabel="CAPITAL_LABEL",
        effects=SetTargetHappiness(value=Value + 10),
    ),
    EffectsGroup(  # concentration camp nullifies stability
        scope=IsSource,
        activation=Contains(IsBuilding(name=["BLD_CONC_CAMP"])),
        accountinglabel="CONCENTRATION_CAMPS_LABEL",
        priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
        effects=[SetHappiness(value=0), SetTargetHappiness(value=0)],
    ),
]

# this should ensure that there is a capital / admin for JUMPS_TO_CAPITAL_OR_REGAD
SUPPLY_CONNECTED_TO_CAPITAL_OR_REGAD = ResourceSupplyConnected(
    empire=Source.Owner, condition=(Capital | IsBuilding(name=["BLD_REGIONAL_ADMIN"])) & OwnedBy(empire=Source.Owner)
)


def JUMPS_TO_CAPITAL_OR_REGAD(type):
    return Statistic(
        type,
        Min,
        # Replace JumpsBetween by JumpsBetweenByEmpireSupplyConnections once it is implemented
        value=JumpsBetween(Source.ID, LocalCandidate.ID),
        condition=(
            (Capital | IsBuilding(name=["BLD_REGIONAL_ADMIN"]))
            & OwnedBy(empire=Source.Owner)
            & ResourceSupplyConnected(empire=Source.Owner, condition=IsSource)
        ),
    )


STANDARD_SPECIES_CAPITAL_SUPPLY_CONNECTION_STABILITY = [
    EffectsGroup(  # close supply connected to capital planet is more stable
        scope=IsSource,
        activation=Planet()
        & ~Unowned
        & ~Capital
        & SUPPLY_CONNECTED_TO_CAPITAL_OR_REGAD
        & (JUMPS_TO_CAPITAL_OR_REGAD(int) < 5),
        accountinglabel="CAPITAL_CONNECTION_LABEL",
        effects=SetTargetHappiness(value=Value + MinOf(float, 5, 5 - JUMPS_TO_CAPITAL_OR_REGAD(float))),
    ),
    EffectsGroup(  # far supply connected to capital planet is less
        scope=IsSource,
        activation=Planet()
        & ~Unowned
        & ~Capital
        & ~EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_CONFEDERATION")
        & SUPPLY_CONNECTED_TO_CAPITAL_OR_REGAD
        & (JUMPS_TO_CAPITAL_OR_REGAD(int) > 5),
        accountinglabel="CAPITAL_POOR_CONNECTION_LABEL",
        effects=SetTargetHappiness(value=Value + MaxOf(float, -5, 5 - JUMPS_TO_CAPITAL_OR_REGAD(float))),
    ),
    EffectsGroup(  # no connection is much less stable
        scope=IsSource,
        activation=Planet()
        & ~Unowned
        & ~Capital
        & ~EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_CONFEDERATION")
        & ~SUPPLY_CONNECTED_TO_CAPITAL_OR_REGAD,
        accountinglabel="CAPITAL_DISCONNECTION_LABEL",
        effects=SetTargetHappiness(
            value=Value - NamedReal(name="DISCONNECTED_FROM_CAPITAL_AND_REGIONAL_ADMIN_STABILITY_PENALTY", value=10)
        ),
    ),
]

# BAD_HAPPINESS
# '''
# [[COMMON_HAPPINESS_EFFECTS]]
# [[STANDARD_SPECIES_CAPITAL_SUPPLY_CONNECTION_STABILITY]]

#         EffectsGroup
#             description = "BAD_HAPPINESS_DESC"
#             scope = Source
#             activation = Planet
#             accountinglabel = "BAD_HAPPINESS_LABEL"
#             effects = SetTargetHappiness value = Value + (NamedReal name = "BAD_HAPPINESS_VAL" value = -2.5)
# '''

# AVERAGE_HAPPINESS
# '''
# [[COMMON_HAPPINESS_EFFECTS]]
# [[STANDARD_SPECIES_CAPITAL_SUPPLY_CONNECTION_STABILITY]]
# '''

GOOD_HAPPINESS = [
    *COMMON_HAPPINESS_EFFECTS,
    *STANDARD_SPECIES_CAPITAL_SUPPLY_CONNECTION_STABILITY,
    EffectsGroup(
        description="GOOD_HAPPINESS_DESC",
        scope=IsSource,
        activation=Planet(),
        accountinglabel="GOOD_HAPPINESS_LABEL",
        effects=SetTargetHappiness(value=Value + NamedReal(name="GOOD_HAPPINESS_VAL", value=2.5)),
    ),
]

# GREAT_HAPPINESS
# '''
# [[COMMON_HAPPINESS_EFFECTS]]
# [[STANDARD_SPECIES_CAPITAL_SUPPLY_CONNECTION_STABILITY]]

#         EffectsGroup
#             description = "GREAT_HAPPINESS_DESC"
#             scope = Source
#             activation = Planet
#             accountinglabel = "GREAT_HAPPINESS_LABEL"
#             effects = SetTargetHappiness value = Value + (NamedReal name = "GREAT_HAPPINESS_VAL" value = 5)
# '''

# ULTIMATE_HAPPINESS
# '''
# [[COMMON_HAPPINESS_EFFECTS]]
# [[STANDARD_SPECIES_CAPITAL_SUPPLY_CONNECTION_STABILITY]]

#         EffectsGroup
#             description = "ULTIMATE_HAPPINESS_DESC"
#             scope = Source
#             activation = Planet
#             accountinglabel = "ULTIMATE_HAPPINESS_LABEL"
#             effects = SetTargetHappiness value = Value + (NamedReal name = "ULTIMATE_HAPPINESS_VAL" value = 7.5)
# '''

# INDEPENDENT_HAPPINESS
# '''[[DESCRIPTION_EFFECTSGROUP_MACRO(INDEPENDENT_HAPPINESS_DESC)]]
# [[COMMON_HAPPINESS_EFFECTS]]
# '''


# #include "../macros/opinion.macros"
