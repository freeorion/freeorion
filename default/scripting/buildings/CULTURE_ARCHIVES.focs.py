from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._buildings import BuildingType
from focs._conditions import Capital, Described, Happiness, IsAnyObject, Object, OwnedBy, Planet, Population, Unowned
from focs._effects import EffectsGroup, SetEmpireMeter, SetTargetIndustry, SetTargetResearch
from focs._sources import Source, Target
from focs._value_refs import (
    NamedReal,
    Value,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER, INDUSTRY_PER_POP
from macros.priorities import TARGET_AFTER_2ND_SCALING_PRIORITY

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_CULTURE_ARCHIVES",
    description="BLD_CULTURE_ARCHIVES_DESC",
    buildcost=200 * BUILDING_COST_MULTIPLIER,
    buildtime=1,
    tags=["ANTIQUATED"],
    location=Described(description="NOWHERE", condition=~IsAnyObject),
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
        EffectsGroup(
            scope=(
                Object(id=Source.PlanetID)
                & Planet()
                & ~Population(high=0)
                & OwnedBy(empire=Source.Owner)
                & Happiness(low=NamedReal(name="BLD_CULTURE_ARCHIVES_MIN_STABILITY", value=5))
            ),
            priority=TARGET_AFTER_2ND_SCALING_PRIORITY,
            effects=[
                SetTargetResearch(value=Value + NamedReal(name="BLD_CULTURE_ARCHIVES_TARGET_RESEARCH_FLAT", value=5)),
                SetTargetIndustry(
                    value=Value
                    + Target.Population
                    * NamedReal(name="BLD_CULTURE_ARCHIVES_TARGET_INDUSTRY_PERPOP", value=2.5 * INDUSTRY_PER_POP)
                ),
            ],
        ),
        EffectsGroup(
            scope=Capital & OwnedBy(empire=Source.Owner),
            activation=~Unowned,
            stackinggroup="CULTURAL_ARCHIVES_POLICY_SLOT_STACK",
            effects=[SetEmpireMeter(empire=Source.Owner, meter="SOCIAL_CATEGORY_NUM_POLICY_SLOTS", value=Value + 1)],
        ),
    ],
    icon="icons/building/archive.png",
)
