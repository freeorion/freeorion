from focs._conditions import IsAnyObject, IsSource
from focs._effects_new import Destroy, EffectsGroup
from macros.base_prod import BUILDING_COST_MULTIPLIER

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_NOVA_BOMB_ACTIVATOR",
    description="BLD_NOVA_BOMB_ACTIVATOR_DESC",
    buildcost=25 * BUILDING_COST_MULTIPLIER,
    buildtime=1,
    location=(IsAnyObject),
    effectsgroups=[
        EffectsGroup(
            scope=IsSource,
            effects=[Destroy],
        ),
    ],
    icon="icons/building/nova-bomb-activator.png",
)
