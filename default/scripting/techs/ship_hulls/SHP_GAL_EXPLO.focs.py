from focs._effects import (
    CanColonize,
    EffectsGroup,
    GasGiantType,
    GenerateSitRepMessage,
    HasSpecies,
    Huge,
    IsSource,
    Large,
    LocalCandidate,
    MinOf,
    OwnedBy,
    Planet,
    SetMaxSupply,
    SetSupply,
    Small,
    Source,
    Target,
    Tiny,
    Turn,
    Value,
)
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER
from macros.misc import GROWTH_RATE_FACTOR, MIN_RECOLONIZING_HAPPINESS, MIN_RECOLONIZING_SIZE
from macros.priorities import AFTER_ALL_TARGET_MAX_METERS_PRIORITY

Tech(
    name="SHP_GAL_EXPLO",
    description="SHP_GAL_EXPLO_DESC",
    short_description="POLICY_UNLOCK_SHORT_DESC",
    category="SHIP_PARTS_CATEGORY",
    researchcost=1 * TECH_COST_MULTIPLIER,
    researchturns=1,
    tags=["PEDIA_SHIP_PARTS_CATEGORY", "THEORY"],
    unlock=Item(type=UnlockPolicy, name="PLC_EXPLORATION"),
    effectsgroups=[
        EffectsGroup(
            scope=Planet(size=[Tiny]) & OwnedBy(empire=Source.Owner),
            accountinglabel="TINY_PLANET_LABEL",
            effects=SetMaxSupply(value=Value + 2),
        ),
        EffectsGroup(
            scope=Planet(size=[Small]) & OwnedBy(empire=Source.Owner),
            accountinglabel="SMALL_PLANET_LABEL",
            effects=SetMaxSupply(value=Value + 1),
        ),
        EffectsGroup(
            scope=Planet(size=[Large]) & OwnedBy(empire=Source.Owner),
            accountinglabel="LARGE_PLANET_LABEL",
            effects=SetMaxSupply(value=Value - 1),
        ),
        EffectsGroup(
            scope=Planet(size=[Huge]) & OwnedBy(empire=Source.Owner),
            accountinglabel="HUGE_PLANET_LABEL",
            effects=SetMaxSupply(value=Value - 2),
        ),
        EffectsGroup(
            scope=Planet(type=[GasGiantType]) & OwnedBy(empire=Source.Owner),
            accountinglabel="GAS_GIANT_LABEL",
            effects=SetMaxSupply(value=Value - 1),
        ),
        EffectsGroup(  # outpost supply increases 1 per turn up to max
            scope=Planet() & OwnedBy(empire=Source.Owner) & ~HasSpecies(),
            priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
            effects=SetSupply(value=MinOf(float, Value(Target.MaxSupply), Value + 1)),
        ),
        # generate sitrep for any planet that is about to increase to above the
        # recolonizing population, while also having sufficient happiness to do
        # do so
        EffectsGroup(
            scope=Planet()
            & OwnedBy(empire=Source.Owner)
            & CanColonize
            & (LocalCandidate.Population < LocalCandidate.TargetPopulation)
            & (
                LocalCandidate.Population
                + LocalCandidate.Population
                * GROWTH_RATE_FACTOR
                * (LocalCandidate.TargetPopulation + 1 - LocalCandidate.Population)
                >= MIN_RECOLONIZING_SIZE
            )
            & (LocalCandidate.TargetHappiness >= MIN_RECOLONIZING_HAPPINESS)
            & (LocalCandidate.Happiness >= MIN_RECOLONIZING_HAPPINESS - 1)
            & (
                (0.1 <= LocalCandidate.Population) & (LocalCandidate.Population <= (MIN_RECOLONIZING_SIZE - 0.001))
                | (0.1 <= LocalCandidate.Happiness) & (LocalCandidate.Happiness <= (MIN_RECOLONIZING_HAPPINESS - 0.001))
            ),
            effects=[
                GenerateSitRepMessage(
                    message="SITREP_POP_THRESHOLD",
                    label="SITREP_POP_THRESHOLD_LABEL",
                    icon="icons/sitrep/colony_growth.png",
                    parameters={"planet": Target.ID},
                    empire=Source.Owner,
                )
            ],
        ),
        EffectsGroup(
            scope=IsSource,
            activation=Turn(low=0, high=0),
            effects=[
                GenerateSitRepMessage(
                    message="SITREP_WELCOME",
                    label="SITREP_WELCOME_LABEL",  # explicitly provided so that the custom SitRep intro message can share the same SitRepPanel filter
                    icon="icons/sitrep/fo_logo.png",
                    empire=Source.Owner,
                )
            ],
        ),
    ],
    graphic="icons/tech/galactic_exploration.png",
)
