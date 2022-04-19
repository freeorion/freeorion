import freeOrionAIInterface as fo
from enum import Enum
from typing import List

from common.fo_typing import PlanetId
from freeorion_tools.caching import cache_for_current_turn, cache_for_session


class Building(Enum):
    """
    Enum to represent fo building template.

    Pros:
    - Shorten code a bit
    - Allow to have shorter names for ids,
      we could get read of BLD prefix, since it always use with Building. prefix.
    - Possible to add automated check that buildings exists and all building are matched.

    Cons:
    - fo code for getting id returns strings, probably will need to make an adapters.

    Thoughts:
    - We could replace it in iterative way one constant at the time,
      this will require to use `value` a lot. When all ids are converted we could get rid of `value` usage.
    - Maybe we should create multiple enums, one for ship yard, etc.
     So Buildong.SHIPYARD_ENRG_COMP will be Shipyard.ENRG_COMP
    - fo_typing.BuildingId represent the same piece of data, need to join them somehow
    """

    SHIPYARD_ENRG_COMP = "BLD_SHIPYARD_ENRG_COMP"
    SHIPYARD_AST = "BLD_SHIPYARD_AST"
    ...

    def enqueue(self, pid: PlanetId):
        return fo.issueEnqueueBuildingProductionOrder(self.name, pid)

    @cache_for_session
    def get_type(self) -> "fo.buildingType":
        return fo.getBuildingType(self.value)

    @cache_for_current_turn
    def available(self, empire: "fo.empire" = None) -> bool:
        if empire is None:
            empire = fo.getEmpire()
        return empire.buildingTypeAvailable(self.value)

    def queued_in(self) -> List[PlanetId]:
        return [element.locationID for element in fo.getEmpire().productionQueue if (element.name == self.value)]
