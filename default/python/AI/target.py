from logging import warn

import freeOrionAIInterface as fo  # pylint: disable=import-error
from AIDependencies import INVALID_ID


class Target(object):
    """
    Stores information about AI target - its id and type.
    :type id: int
    """
    object_name = 'target'

    def __init__(self, target_id):
        self.id = target_id
        if not self:
            warn("Target is invalid %s" % self)

    def __cmp__(self, other):
        return type(self) == type(other) and cmp(self.id, other.id)

    def __str__(self):
        target = self.get_object()
        if target is None:
            target = "inaccessible %s_%s<>" % (self.object_name, self.id)
        return str(target)

    def get_object(self):
        """
        :rtype:  fo.universeObject | None
        """
        return None

    def __nonzero__(self):
        return self.id is not None and self.id >= 0

    def get_system(self):
        """
        Returns the system that contains this object, or None.
        :rtype: TargetSystem | None
        """
        raise NotImplementedError()


class TargetPlanet(Target):
    object_name = 'planet'

    def get_system(self):
        """
        Returns the system that contains this object, or None.
        :rtype: TargetSystem | None
        """
        universe = fo.getUniverse()
        planet = universe.getPlanet(self.id)
        return TargetSystem(planet.systemID)

    def get_object(self):
        """
        :rtype fo.planet:
        """
        return fo.getUniverse().getPlanet(self.id)


class TargetSystem(Target):
    object_name = 'system'

    def get_system(self):
        """
        Returns this object.
        :rtype: TargetSystem
        """
        return self

    def get_object(self):
        return fo.getUniverse().getSystem(self.id)


class TargetFleet(Target):
    object_name = 'fleet'

    def get_current_system_id(self):
        """
        Get current systemID (or INVALID_ID if on a starlane).

        :rtype: int
        """
        universe = fo.getUniverse()
        fleet = universe.getFleet(self.id)
        # will also return INVALID_ID if somehow the fleet cannot be retrieved
        return fleet.systemID if fleet else INVALID_ID

    def get_system(self):
        """
        Get current fleet location or target system if currently on starlane.
        :rtype: TargetSystem | None
        """
        universe = fo.getUniverse()
        fleet = universe.getFleet(self.id)
        system_id = fleet.nextSystemID
        if system_id == INVALID_ID:  # fleet is not moving
            system_id = fleet.systemID
        return TargetSystem(system_id)

    def get_object(self):
        """
        :rtype fo.fleet:
        """
        return fo.getUniverse().getFleet(self.id)
