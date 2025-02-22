from focs._effects import (
    Abs,
    EffectsGroup,
    EmpireHasAdoptedPolicy,
    HasEmpireStockpile,
    HasSpecies,
    IsSource,
    IsTarget,
    MaxOf,
    MinOf,
    NumPoliciesAdopted,
    OwnedBy,
    Planet,
    Population,
    ResourceInfluence,
    SetConstruction,
    SetEmpireStockpile,
    SetHappiness,
    SetMaxTroops,
    SetTargetConstruction,
    Source,
    StatisticIf,
    Target,
    Value,
)
from focs._tech import *
from macros.priorities import (
    AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
    METER_OVERRIDE_PRIORITY,
    TARGET_AFTER_2ND_SCALING_PRIORITY,
)

Tech(
    name="CON_OUTPOST",
    description="CON_OUTPOST_DESC",
    short_description="CON_OUTPOST",
    category="CONSTRUCTION_CATEGORY",
    researchcost=1,
    researchturns=1,
    tags=["PEDIA_CONSTRUCTION_CATEGORY"],
    unlock=Item(type=UnlockBuilding, name="BLD_ABANDON_OUTPOST"),
    # Effects for outposts
    effectsgroups=[
        # Outposts only have 50% of troops
        EffectsGroup(
            scope=Planet()
            & OwnedBy(empire=Source.Owner)
            & Population(high=0)
            & ~EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_MARINE_RECRUITMENT"),
            stackinggroup="OUTPOST_TROOPS_STACK",
            priority=TARGET_AFTER_2ND_SCALING_PRIORITY,
            effects=SetMaxTroops(value=Value * 0.5),
            accountinglabel="OUTPOST_TROOP_LABEL",
        ),
        # Ensure construction minimum value of one, as this is necessary for being attacked
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner),
            # has to happen after e.g. FORCE_ENERGY_STRC effects which also happens at AFTER_ALL_TARGET_MAX_METERS_PRIORITY
            priority=METER_OVERRIDE_PRIORITY,
            effects=[
                SetTargetConstruction(value=MaxOf(float, Value, 1)),
                SetConstruction(value=MaxOf(float, Value, 1)),
            ],
        ),
        # Influence growth / reduction towards target, since outposts have no species to get this effect from
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner) & (~HasSpecies()),
            priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
            effects=SetHappiness(
                value=Value
                + MinOf(float, Abs(float, Value(Target.TargetHappiness) - Value), 1)
                * (1 - 2 * (StatisticIf(float, condition=IsTarget & (Value > Value(Target.TargetHappiness)))))
            ),
        ),
        # Reset influence to 0 if no policies adopted. Not really relevant to Outposts, but I need somewhere to put this...
        EffectsGroup(
            scope=IsSource,
            activation=HasEmpireStockpile(empire=Source.Owner, resource=ResourceInfluence, high=0)
            & (NumPoliciesAdopted(empire=Source.Owner) == 0),
            priority=METER_OVERRIDE_PRIORITY,
            effects=SetEmpireStockpile(resource=ResourceInfluence, value=0.0),
        ),
    ],
)
