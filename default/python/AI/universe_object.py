import freeOrionAIInterface as fo  # pylint: disable=import-error
from AIDependencies import INVALID_ID

from common.configure_logging import convenience_function_references_for_logger
(debug, info, warn, error, fatal) = convenience_function_references_for_logger(__name__)


class UniverseObject(object):
    """
    Stores information about AI target - its id and type.
    :type id: int
    """
    object_name = 'universe_object'

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
        Returns fo.universeObject or None.
        """
        return None

    def __nonzero__(self):
        return self.id is not None and self.id >= 0

    def get_system(self):
        """
        Returns the system that contains this object, or None.
        :rtype: System | None
        """
        raise NotImplementedError()


class Planet(UniverseObject):
    object_name = 'planet'

    def get_system(self):
        """
        Returns the system that contains this object, or None.
        :rtype: System | None
        """
        universe = fo.getUniverse()
        planet = universe.getPlanet(self.id)
        return System(planet.systemID)

    def get_object(self):
        """
        :rtype fo.planet:
        """
        return fo.getUniverse().getPlanet(self.id)


class System(UniverseObject):
    object_name = 'system'

    def get_system(self):
        """
        Returns this object.
        :rtype: System
        """
        return self

    def get_object(self):
        return fo.getUniverse().getSystem(self.id)


class Fleet(UniverseObject):
    object_name = 'fleet'

    def get_system(self):
        """
        Get current fleet location or target system if currently on starlane.
        :rtype: System | None
        """
        universe = fo.getUniverse()
        fleet = universe.getFleet(self.id)
        system_id = fleet.nextSystemID
        if system_id == INVALID_ID:  # fleet is not moving
            system_id = fleet.systemID
        return System(system_id)

    def get_object(self):
        return fo.getUniverse().getFleet(self.id)
