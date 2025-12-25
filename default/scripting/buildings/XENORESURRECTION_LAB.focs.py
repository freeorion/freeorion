from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    ContainedBy,
    Contains,
    CreateShip,
    EffectsGroup,
    GenerateSitRepMessage,
    GiveEmpireTech,
    HasSpecial,
    IsBuilding,
    IsSource,
    Object,
    OwnerHasTech,
    Planet,
    RemoveSpecial,
    Source,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_XENORESURRECTION_LAB",
    description="BLD_XENORESURRECTION_LAB_DESC",
    buildcost=100 * BUILDING_COST_MULTIPLIER,
    buildtime=4,
    location=(
        Planet()
        & ~Contains(IsBuilding(name=["BLD_XENORESURRECTION_LAB"]))
        & (
            HasSpecial(name="EXTINCT_BANFORO_SPECIAL")
            | HasSpecial(name="EXTINCT_KILANDOW_SPECIAL")
            | HasSpecial(name="EXTINCT_MISIORLA_SPECIAL")
            | HasSpecial(name="KRAKEN_IN_THE_ICE_SPECIAL")
        )
    ),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
        EffectsGroup(
            scope=IsSource,
            activation=(ContainedBy(Object(id=Source.PlanetID) & HasSpecial(name="EXTINCT_BANFORO_SPECIAL"))),
            effects=[GiveEmpireTech(name="TECH_COL_BANFORO")],
        ),
        EffectsGroup(
            scope=IsSource,
            activation=(ContainedBy(Object(id=Source.PlanetID) & HasSpecial(name="EXTINCT_KILANDOW_SPECIAL"))),
            effects=[GiveEmpireTech(name="TECH_COL_KILANDOW")],
        ),
        EffectsGroup(
            scope=IsSource,
            activation=(ContainedBy(Object(id=Source.PlanetID) & HasSpecial(name="EXTINCT_MISIORLA_SPECIAL"))),
            effects=[GiveEmpireTech(name="TECH_COL_MISIORLA")],
        ),
        EffectsGroup(
            scope=(Object(id=Source.PlanetID) & Planet() & HasSpecial(name="KRAKEN_IN_THE_ICE_SPECIAL")),
            activation=(OwnerHasTech(name="SHP_DOMESTIC_MONSTER")),
            effects=[
                CreateShip(designname="SM_WHITE_KRAKEN", empire=Source.Owner),
                RemoveSpecial(name="KRAKEN_IN_THE_ICE_SPECIAL"),
                GenerateSitRepMessage(
                    message="EFFECT_WHITE_KRAKEN_RESURRECTED",
                    label="EFFECT_WHITE_KRAKEN_RESURRECTED_LABEL",
                    icon="icons/monsters/kraken-5.png",
                    parameters={"planet": Source.PlanetID},
                    empire=Source.Owner,
                ),
            ],
        ),
    ],
    icon="icons/building/the_caducean_institute.png",
)
