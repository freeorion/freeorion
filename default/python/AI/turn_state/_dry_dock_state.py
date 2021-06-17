import freeOrionAIInterface as fo
from typing import Mapping, Tuple

import AIDependencies
from common.fo_typing import PlanetId, SystemId
from freeorion_tools import ReadOnlyDict
from freeorion_tools.caching import cache_for_current_turn


@cache_for_current_turn
def get_empire_drydocks() -> Mapping[SystemId, Tuple[PlanetId]]:
    """
    Return a map from system ids to planet ids where empire drydocks are located.
    """
    universe = fo.getUniverse()
    empire_id = fo.empireID()
    drydocks = {}
    for building_id in universe.buildingIDs:
        building = universe.getBuilding(building_id)
        if not building:
            continue
        if building.buildingTypeName == AIDependencies.BLD_SHIPYARD_ORBITAL_DRYDOCK and building.ownedBy(empire_id):
            drydocks.setdefault(building.systemID, []).append(building.planetID)
    return ReadOnlyDict({k: tuple(v) for k, v in drydocks.items()})
