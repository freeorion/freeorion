from focs._conditions import (
    Contains,
    IsBuilding,
    OwnedBy,
    Planet,
)
from focs._enums import Core, External, Internal
from focs._ship_hulls import Hull, Slot
from focs._sources import Source
from macros.upkeep import FLEET_UPKEEP_MULTIPLICATOR, SHIP_HULL_COST_MULTIPLIER
from ship_hulls.ship_hulls import (
    ADD_HULL_FUEL_TO_MAX_FUEL_METER,
    AVERAGE_BASE_FUEL_REGEN,
    GOOD_FUEL_EFFICIENCY,
    REGULAR_HULL_DETECTION,
    SCAVANGE_FUEL_UNOWNED,
    UNOWNED_GOOD_VISION,
    UNOWNED_MOVE,
)
from ship_hulls.spatial_flux.spatial_flux import (
    SPATIAL_FLUX_STEALTH_EFFECTS,
)

Hull(
    name="SH_SPACE_FLUX_COMPOSITE",
    description="SH_SPACE_FLUX_COMPOSITE_DESC",
    speed=80,
    fuel=4,
    NoDefaultFuelEffect=True,
    stealth=15,
    structure=50,
    slots=[
        Slot(type=Internal, position=(0.40, 0.50)),
        Slot(type=Core, position=(0.55, 0.50)),
        Slot(type=External, position=(0.70, 0.50)),
    ],
    buildcost=40 * FLEET_UPKEEP_MULTIPLICATOR * SHIP_HULL_COST_MULTIPLIER,
    buildtime=2,
    tags=["PEDIA_HULL_LINE_SPATIAL_FLUX", "GOOD_FUEL_EFFICIENCY"],
    location=(
        Planet()
        & OwnedBy(empire=Source.Owner)
        & Contains(IsBuilding(name="BLD_SHIPYARD_BASE") & OwnedBy(empire=Source.Owner))
        & Contains(IsBuilding(name="BLD_SHIPYARD_ORBITAL_DRYDOCK") & OwnedBy(empire=Source.Owner))
    ),
    effectsgroups=[
        *SPATIAL_FLUX_STEALTH_EFFECTS,
        *GOOD_FUEL_EFFICIENCY,
        ADD_HULL_FUEL_TO_MAX_FUEL_METER,
        *AVERAGE_BASE_FUEL_REGEN,
        REGULAR_HULL_DETECTION,
        SCAVANGE_FUEL_UNOWNED,
        UNOWNED_GOOD_VISION,
        UNOWNED_MOVE,
    ],
    # FIXME graphic and icon
    icon="icons/ship_hulls/space_flux_composite_hull_small.png",
    graphic="hulls_design/space_flux_composite_hull.png",
)
