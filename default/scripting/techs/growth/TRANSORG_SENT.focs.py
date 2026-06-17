from focs._conditions import OwnedBy, Planet
from focs._effects import Conditional, EffectsGroup, Item, SetInfluence
from focs._enums import UnlockPolicy
from focs._sources import LocalCandidate, Source, Target
from focs._techs import Tech
from focs._value_refs import (
    MaxOf,
    MinOf,
    NamedReal,
    NamedRealLookup,
    Value,
)
from macros.base_prod import TECH_COST_MULTIPLIER
from macros.priorities import AFTER_ALL_TARGET_MAX_METERS_PRIORITY

Tech(
    name="GRO_TRANSORG_SENT",
    description="GRO_TRANSORG_SENT_DESC",
    short_description="POLICY_UNLOCK_SHORT_DESC",
    category="GROWTH_CATEGORY",
    researchcost=126 * TECH_COST_MULTIPLIER,
    researchturns=7,
    tags=["PEDIA_GROWTH_CATEGORY", "THEORY"],
    prerequisites=["GRO_TERRAFORM"],
    unlock=Item(type=UnlockPolicy, name="PLC_TERRAFORMING"),
    effectsgroups=[
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner),
            priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
            effects=Conditional(
                condition=(Value(LocalCandidate.Influence) <= Value(LocalCandidate.TargetInfluence)),
                effects=[
                    SetInfluence(
                        value=MinOf(
                            float,
                            Value + NamedReal(name="GRO_TRANSORG_INFLUENCE_RATE", value=1.0),
                            Value(Target.TargetInfluence),
                        )
                    )
                ],
                else_=[
                    SetInfluence(
                        value=MaxOf(
                            float,
                            Value - NamedRealLookup(name="GRO_TRANSORG_INFLUENCE_RATE"),
                            Value(Target.TargetInfluence),
                        )
                    )
                ],
            ),
        )
    ],
    graphic="icons/tech/trans_organic_sentience.png",
)
