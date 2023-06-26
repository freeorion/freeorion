import freeOrionAIInterface as fo
from logging import warning
from typing import Optional

from AIDependencies import INVALID_ID
from common.fo_typing import FleetId, ObjectId, PlanetId, SystemId


class Target:
    """
    Stores information about AI target - its id and type.
    """

    object_name = "target"
    id: ObjectId

    def __init__(self):
        if not self:
            warning("Target is invalid %s" % self)

    def __eq__(self, other):
        return type(self) == type(other) and self.id == other.id

    def __hash__(self):
        return hash(self.id)

    def __str__(self):
        target = self.get_object()
        if target is None:
            target = f"inaccessible {self.object_name}_{self.id}<>"
        return str(target)

    def get_object(self) -> Optional["fo.universeObject"]:
        return None

    def __bool__(self):
        return self.id is not None and self.id >= 0

    __nonzero__ = __bool__

    def get_system(self) -> Optional["TargetSystem"]:
        """
        Returns the system that contains this object, or None.
        """
        raise NotImplementedError()


class TargetPlanet(Target):
    object_name = "planet"

    def __init__(self, target_id: PlanetId):
        self.id: PlanetId = target_id
        super().__init__()

    def get_system(self) -> Optional["TargetSystem"]:
        """
        Returns the system that contains this object, or None.
        """
        universe = fo.getUniverse()
        planet = universe.getPlanet(self.id)
        return TargetSystem(planet.systemID)

    def get_object(self) -> "fo.planet":
        """
        :rtype fo.planet:
        """
        return fo.getUniverse().getPlanet(self.id)


class TargetSystem(Target):
    object_name = "system"

    def __init__(self, target_id: SystemId):
        self.id: SystemId = target_id
        super().__init__()

    def get_system(self) -> "TargetSystem":
        """
        Returns this object.
        """
        return self

    def get_object(self) -> "fo.system":
        return fo.getUniverse().getSystem(self.id)


class TargetFleet(Target):
    object_name = "fleet"

    def __init__(self, target_id: FleetId):
        self.id: FleetId = target_id
        super().__init__()

    def get_current_system_id(self):
        """
        Get current systemID (or INVALID_ID if on a starlane).
        """
        universe = fo.getUniverse()
        fleet = universe.getFleet(self.id)
        # will also return INVALID_ID if somehow the fleet cannot be retrieved
        return fleet.systemID if fleet else INVALID_ID

    def get_system(self) -> Optional[TargetSystem]:
        """
        Get current fleet location or target system if currently on starlane.
        """
        universe = fo.getUniverse()
        fleet = universe.getFleet(self.id)
        system_id = fleet.nextSystemID
        if system_id == INVALID_ID:  # fleet is not moving
            system_id = fleet.systemID
        return TargetSystem(system_id)

    def get_object(self) -> "fo.fleet":
        return fo.getUniverse().getFleet(self.id)
