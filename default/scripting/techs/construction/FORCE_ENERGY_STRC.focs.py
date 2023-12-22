from focs._effects import (
    Conditional,
    EffectsGroup,
    LocalCandidate,
    MaxOf,
    MinOf,
    NamedReal,
    NamedRealLookup,
    OwnedBy,
    Planet,
    SetConstruction,
    SetIndustry,
    SetResearch,
    SetStockpile,
    Source,
    Target,
    Value,
)
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER
from macros.priorities import AFTER_ALL_TARGET_MAX_METERS_PRIORITY

Tech(
    name="CON_FRC_ENRG_STRC",
    description="CON_FRC_ENRG_STRC_DESC",
    short_description="METER_GROWTH_SHORT_DESC",
    category="CONSTRUCTION_CATEGORY",
    researchcost=350 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_CONSTRUCTION_CATEGORY"],
    prerequisites=["LRN_FORCE_FIELD", "CON_ARCH_PSYCH"],
    effectsgroups=[
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner),
            accountinglabel="CON_TECH_ACCOUNTING_LABEL",
            priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
            effects=[
                Conditional(
                    condition=(Value(LocalCandidate.Industry) <= Value(LocalCandidate.TargetIndustry)),
                    effects=[
                        SetIndustry(
                            value=MinOf(
                                float,
                                Value + NamedReal(name="FORCE_ENERGY_RATE_INCREASE", value=3.0),
                                Value(Target.TargetIndustry),
                            )
                        )
                    ],
                    else_=[
                        SetIndustry(
                            value=MaxOf(
                                float,
                                Value - NamedRealLookup(name="FORCE_ENERGY_RATE_INCREASE"),
                                Value(Target.TargetIndustry),
                            )
                        )
                    ],
                ),
                Conditional(
                    condition=(Value(LocalCandidate.Research) <= Value(LocalCandidate.TargetResearch)),
                    effects=[
                        SetResearch(
                            value=MinOf(
                                float,
                                Value + NamedRealLookup(name="FORCE_ENERGY_RATE_INCREASE"),
                                Value(Target.TargetResearch),
                            )
                        )
                    ],
                    else_=[
                        SetResearch(
                            value=MaxOf(
                                float,
                                Value - NamedRealLookup(name="FORCE_ENERGY_RATE_INCREASE"),
                                Value(Target.TargetResearch),
                            )
                        )
                    ],
                ),
                Conditional(
                    condition=(Value(LocalCandidate.Construction) <= Value(LocalCandidate.TargetConstruction)),
                    effects=[
                        SetConstruction(
                            value=MinOf(
                                float,
                                Value + NamedRealLookup(name="FORCE_ENERGY_RATE_INCREASE"),
                                Value(Target.TargetConstruction),
                            )
                        )
                    ],
                    else_=[
                        SetConstruction(
                            value=MaxOf(
                                float,
                                Value - NamedRealLookup(name="FORCE_ENERGY_RATE_INCREASE"),
                                Value(Target.TargetConstruction),
                            )
                        )
                    ],
                ),
                Conditional(
                    condition=(Value(LocalCandidate.Stockpile) <= Value(LocalCandidate.MaxStockpile)),
                    effects=[
                        SetStockpile(
                            value=MinOf(
                                float,
                                Value + NamedRealLookup(name="FORCE_ENERGY_RATE_INCREASE"),
                                Value(Target.MaxStockpile),
                            )
                        )
                    ],
                    else_=[
                        SetStockpile(
                            value=MaxOf(
                                float,
                                Value - NamedRealLookup(name="FORCE_ENERGY_RATE_INCREASE"),
                                Value(Target.MaxStockpile),
                            )
                        )
                    ],
                ),
            ],
        )
    ],
    graphic="icons/tech/force_energy_structures.png",
)
