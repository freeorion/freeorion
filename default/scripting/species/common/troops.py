from common.base_prod import TROOPS_PER_POP
from common.priorities import (
    AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
    TARGET_AFTER_2ND_SCALING_PRIORITY,
    TARGET_EARLY_BEFORE_SCALING_PRIORITY,
    TARGET_SCALING_PRIORITY,
)

UNSTABLE_REBEL_TROOPS = [
    EffectsGroup(
        scope=Source,
        activation=~Unowned & ~Happiness(low=0),
        accountinglabel="INSTABILITY_REBELLION",
        priority=TARGET_EARLY_BEFORE_SCALING_PRIORITY,
        # more negative stability and more population generates more rebels each turn
        # TODO: scale with some species trait?
        effects=SetRebelTroops(value=Value + Abs(float, Target.Happiness) * (Target.Population / 5.0) ** 0.5),
    ),
    EffectsGroup(
        scope=Source,
        activation=Planet() & ~Unowned & ~HasEmpireStockpile(empire=Source.Owner, resource=Influence, low=0) & ~Capital
        # TODO: some species trait?
        ,
        accountinglabel="PARTISANS_REBELLION",
        priority=TARGET_EARLY_BEFORE_SCALING_PRIORITY,
        effects=SetTargetHappiness(
            value=Value - 0.5 * Abs(float, EmpireStockpile(empire=Source.Owner, resource=Influence)) ** 0.5
        ),
    ),
    EffectsGroup(
        scope=Source,
        activation=Planet() & ~Unowned & ~HasEmpireStockpile(empire=Source.Owner, resource=Influence, low=0) & ~Capital,
        accountinglabel="INFLUENCE_DEBT_DECAY",
        priority=TARGET_EARLY_BEFORE_SCALING_PRIORITY,
        effects=SetTargetInfluence(
            value=Value
            + 0.5
            * Abs(float, EmpireStockpile(empire=Source.Owner, resource=Influence)) ** 0.5
            / MaxOf(float, 1.0, StatisticCount(float, condition=Planet() & HasSpecies() & OwnedBy(empire=Source.Owner)))
        ),
    ),
    EffectsGroup(
        scope=Source,
        activation=Planet(environment=[Adequate]),
        accountinglabel="DEPENDENCE_ON_IMPERIAL_SUPPORT_DUE_TO_ENVIRONMENT",
        priority=TARGET_SCALING_PRIORITY,
        effects=SetRebelTroops(value=MaxOf(float, 0.0, Abs(float, Value / 1.5))),
    ),
    EffectsGroup(
        scope=Source,
        activation=Planet(environment=[Poor]),
        accountinglabel="DEPENDENCE_ON_IMPERIAL_SUPPORT_DUE_TO_ENVIRONMENT",
        priority=TARGET_SCALING_PRIORITY,
        effects=SetRebelTroops(value=MaxOf(float, 0.0, Abs(float, Value / 2.0))),
    ),
    EffectsGroup(
        scope=Source,
        activation=Planet(environment=[Hostile]),
        accountinglabel="DEPENDENCE_ON_IMPERIAL_SUPPORT_DUE_TO_ENVIRONMENT",
        priority=TARGET_SCALING_PRIORITY,
        effects=SetRebelTroops(value=MaxOf(float, 0.0, Abs(float, Value / 4.0))),
    ),
]

BASIC_DEFENSE_TROOPS = [
    EffectsGroup(
        scope=Source,
        activation=Homeworld() & ~Unowned,
        stackinggroup="HOMEWORLD_TROOPS_STACK",
        priority=TARGET_EARLY_BEFORE_SCALING_PRIORITY,
        effects=SetMaxTroops(value=Value + 4),
        # accountinglabel="HOMEWORLD_LABEL",
    ),
    EffectsGroup(
        scope=Source,
        activation=Planet() & ~Unowned,
        accountinglabel="IMPERIAL_GARRISON_LABEL",
        priority=TARGET_EARLY_BEFORE_SCALING_PRIORITY,
        effects=SetMaxTroops(value=Value + NamedRealLookup(name="IMPERIAL_GARRISON_MAX_TROOPS_FLAT")),
    ),
    EffectsGroup(
        scope=Source,
        activation=Planet() & Unowned,
        stackinggroup="BASIC_TROOPS_STACK",
        accountinglabel="INDEPENDENT_TROOP_LABEL",
        priority=TARGET_EARLY_BEFORE_SCALING_PRIORITY,
        effects=SetMaxTroops(value=Value + 10),
    ),
    EffectsGroup(
        scope=Source,
        activation=Planet(),
        stackinggroup="POPULATION_TROOPS_STACK",
        accountinglabel="DEF_ROOT_DEFENSE",
        priority=TARGET_EARLY_BEFORE_SCALING_PRIORITY,
        effects=SetMaxTroops(
            value=Value
            + Target.Population * NamedReal(name="BASIC_DEFENSE_TROOPS_MAX_TROOPS_PERPOP", value=1.0 * TROOPS_PER_POP)
        ),
    ),
    EffectsGroup(  # gives human bonuses when AI Aggression set to Beginner
        scope=Source,
        activation=IsHuman & (GalaxyMaxAIAggression == 0) & Planet(),  # human player, not human species
        accountinglabel="DIFFICULTY",
        effects=SetMaxTroops(value=MaxOf(float, 6, Value * 2)),
    ),
    EffectsGroup(  # base troops regeneration
        scope=Source,
        activation=Planet()
        & (LocalCandidate.LastTurnConquered < CurrentTurn)
        & (LocalCandidate.LastTurnAttackedByShip < CurrentTurn),
        priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
        effects=SetTroops(
            value=MinOf(
                float, Value(Target.MaxTroops), Value + NamedReal(name="BASIC_DEFENSE_TROOPS_TROOPREGEN_FLAT", value=1)
            )
        ),
    ),
]

PROTECTION_FOCUS_TROOPS = [
    EffectsGroup(  # double max troops
        scope=Source,
        activation=Planet() & Focus(type=["FOCUS_PROTECTION"]),
        stackinggroup="FOCUS_PROTECTION_TROOPS_STACK",
        priority=TARGET_AFTER_2ND_SCALING_PRIORITY,
        effects=SetMaxTroops(value=MaxOf(float, Value, Value * 2)),
        # accountinglabel="FOCUS_PROTECTION_LABEL",
    ),
    EffectsGroup(  # increase troop growth rate
        scope=Source,
        activation=Planet()
        & Focus(type=["FOCUS_PROTECTION"])
        & (LocalCandidate.LastTurnConquered < CurrentTurn)
        & (LocalCandidate.LastTurnAttackedByShip < CurrentTurn),
        accountinglabel="FOCUS_PROTECTION_LABEL",
        priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
        effects=SetTroops(
            value=MinOf(
                float, Value(Target.MaxTroops), Value + MaxOf(float, Target.MaxTroops, Target.Population) ** 0.5
            )
        ),
    ),
]

BAD_DEFENSE_TROOPS = [
    *BASIC_DEFENSE_TROOPS,
    EffectsGroup(
        description="BAD_DEFENSE_TROOPS_DESC",
        scope=Source,
        activation=Planet(),
        effects=SetMaxTroops(value=Value * 0.5),
        # accountinglabel="BAD_TROOPS_LABEL",
    ),
    *UNSTABLE_REBEL_TROOPS,
    *PROTECTION_FOCUS_TROOPS,
]
