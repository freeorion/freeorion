import sys
import freeOrionAIInterface as fo  # pylint: disable=import-error


class UniverseObject(object):
    """Stores information about AI target - its id and type."""

    def __init__(self, target_id):
        self.id = target_id
        if not self:
            print >> sys.stderr, "Target is invalid %s" % self

    def __cmp__(self, other):
        return type(self) == type(other) and cmp(self.id, other.id)

    def __str__(self):
        target = self.get_object()
        if target is None:
            target_name = "%4d" % self.id
        else:
            target_name = target.name
        return "{ %7s : [%4d] %9s}" % (
            self.object_name, self.id, target_name
        )

    def get_object(self):
        """
        Returns UniverseObject or None.
        """
        return None

    def __nonzero__(self):
        return self.id is not None and self.id >= 0

    def get_system(self):
        """Returns all system AITargets required to visit in this object."""
        raise NotImplementedError()


class Planet(UniverseObject):
    object_name = 'planet'

    def get_system(self):
        universe = fo.getUniverse()
        planet = universe.getPlanet(self.id)
        return System(planet.systemID)

    def get_object(self):
        return fo.getUniverse().getPlanet(self.id)


class System(UniverseObject):
    object_name = 'system'

    def get_system(self):
        return self

    def get_object(self):
        return fo.getUniverse().getSystem(self.id)


class Fleet(UniverseObject):
    object_name = 'fleet'

    def get_system(self):
        # Fleet systemID is where is fleet going.
        # If fleet is going nowhere, then it is location of fleet
        universe = fo.getUniverse()
        fleet = universe.getFleet(self.id)
        system_id = fleet.nextSystemID
        if system_id == -1:
            system_id = fleet.systemID
        return System(system_id)

    def get_object(self):
        return fo.getUniverse().getFleet(self.id)
