import freeOrionAIInterface as fo
from enum import Enum
from typing import List

from common.fo_typing import EmpireId, PlanetId

# Default value when AI empire should be used.
THIS_EMPIRE = None


class BuildingType(Enum):
    """
    Enum to represent fo building template.

    This class encapsulate FOCS names and fo API usage exposing convenient API for AI.
    It should not contain any decision logic, only provide information that is easy to handle.

    It should not accept or expose any fo.* object.

    Note: It does not required to have exactly the same name and value.
    """

    SHIPYARD_ENRG_COMP = "BLD_SHIPYARD_ENRG_COMP"
    SHIPYARD_AST = "BLD_SHIPYARD_AST"

    def enqueue(self, pid: PlanetId) -> bool:
        """
        Add building to production queue and return True if succeeded.
        """
        return bool(fo.issueEnqueueBuildingProductionOrder(self.name, pid))

    def available(self, eid: EmpireId = THIS_EMPIRE) -> bool:
        """
        Return true if this building is available for empire.
        """
        if eid is THIS_EMPIRE:
            empire_object = fo.getEmpire()
        else:
            empire_object = fo.getEmpire(eid)
        return empire_object.buildingTypeAvailable(self.value)

    def queued_in(self) -> List[PlanetId]:
        """
        Return list of planet ids where this building is queued.
        """
        return [element.locationID for element in fo.getEmpire().productionQueue if (element.name == self.value)]

    def can_be_enqueued(self, planet: PlanetId, empire: EmpireId = THIS_EMPIRE) -> bool:
        if empire is THIS_EMPIRE:
            empire = fo.empireID()
        return fo.getBuildingType(self.value).canBeEnqueued(empire, planet)

    def can_be_produced(self, planet: PlanetId, empire: EmpireId = THIS_EMPIRE) -> bool:
        if empire is THIS_EMPIRE:
            empire = fo.empireID()
        return fo.getBuildingType(self.value).canBeProduced(empire, planet)

    def production_cost(self, planet: PlanetId, empire: EmpireId = THIS_EMPIRE) -> float:
        if empire is THIS_EMPIRE:
            empire = fo.empireID()
        return fo.getBuildingType(self.value).productionCost(empire, planet)

    def production_time(self, planet: PlanetId, empire: EmpireId = THIS_EMPIRE) -> int:
        if empire is THIS_EMPIRE:
            empire = fo.empireID()
        return fo.getBuildingType(self.value).productionTime(empire, planet)
