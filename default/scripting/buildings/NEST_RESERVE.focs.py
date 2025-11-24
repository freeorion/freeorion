from focs._effects import (
    BuildBuilding,
    Contains,
    Enqueued,
    HasSpecial,
    IsBuilding,
    OwnedBy,
    Planet,
    Source,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # type: ignore[reportUnboundVariable]
    name="BLD_NEST_RESERVE",
    description="BLD_NEST_RESERVE_DESC",
    buildcost=5 * BUILDING_COST_MULTIPLIER,
    buildtime=2,
    location=(
        Planet()
        & OwnedBy(empire=Source.Owner)
        & ~Contains(IsBuilding(name=["BLD_NEST_RESERVE"]))
        & (
            HasSpecial(name="JUGGERNAUT_NEST_SPECIAL")
            | HasSpecial(name="KRAKEN_NEST_SPECIAL")
            | HasSpecial(name="SNOWFLAKE_NEST_SPECIAL")
        )
    ),
    enqueuelocation=~Enqueued(
        type=BuildBuilding,
        name="BLD_NEST_RESERVE",
    ),
    effectsgroups=[],
    icon="planets/rings/rings01.png",
)
