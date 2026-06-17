from focs._buildings import BuildingType
from focs._conditions import Contains, Enqueued, HasSpecial, IsBuilding, IsSource, Object, OwnedBy, Planet
from focs._effects import AddSpecial, Destroy, EffectsGroup, GenerateSitRepMessage
from focs._enums import AsteroidsType, BuildBuilding, GasGiantType, Good
from focs._sources import Source, Target
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.enqueue import DO_NOT_CONTAIN_FOR_ALL_TERRAFORM_PLANET_TYPES, ENQUEUE_BUILD_ONE_PER_PLANET

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_GAIA_TRANS",
    description="BLD_GAIA_TRANS_DESC",
    buildcost=200 * Target.HabitableSize * BUILDING_COST_MULTIPLIER,
    buildtime=12,
    location=(
        Planet()
        & OwnedBy(empire=Source.Owner)
        & Planet(environment=[Good])
        & ~Planet(type=[AsteroidsType, GasGiantType])
        & ~HasSpecial(name="GAIA_SPECIAL")
        # No terraforming buildings contained or enqueued (neither popular nor any of the targeted ones)
        & ~Contains(IsBuilding(name=["BLD_TERRAFORM_BEST"]))
        & ~Enqueued(type=BuildBuilding, name="BLD_TERRAFORM_BEST")
        & DO_NOT_CONTAIN_FOR_ALL_TERRAFORM_PLANET_TYPES()
    ),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        EffectsGroup(
            scope=(Object(id=Source.PlanetID) & Planet()),
            effects=[
                AddSpecial(name="GAIA_SPECIAL"),
                GenerateSitRepMessage(
                    message="EFFECT_GAIA",
                    label="EFFECT_GAIA_LABEL",
                    icon="icons/specials_huge/gaia.png",
                    parameters={"planet": Target.ID},
                    empire=Source.Owner,
                ),
            ],
        ),
        EffectsGroup(
            scope=IsSource,
            effects=[Destroy],
        ),
    ],
    icon="icons/specials_huge/gaia.png",
)
