from typing import Mapping, Tuple

import AIDependencies
import freeOrionAIInterface as fo

from freeorion_tools import cache_for_current_turn, ReadOnlyDict


@cache_for_current_turn
def get_empire_drydocks() -> Mapping[int, Tuple[int]]:
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
