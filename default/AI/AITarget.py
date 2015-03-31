import EnumsAI
import freeOrionAIInterface as fo  # pylint: disable=import-error


class AITarget(object):
    """Stores information about AI target - its id and type."""

    def __init__(self, target_id):
        self.target_id = target_id

    def __cmp__(self, other):
       return cmp(self.target_id, other.target_id)

    def __str__(self):
        target = self.target_obj
        if target is None:
            target_name = "%4d" % self.target_id
        else:
            target_name = target.name
        return "{ %7s : [%4d] %9s}" % (
            self.name, self.target_id, target_name
        )

    @property
    def target_obj(self):
        """
        Returns target UniverseObject or None.
        """
        return None

    def valid(self):
        """Returns if this object is valid."""
        return self.target_type is not None and EnumsAI.check_validity(self.target_id)

    def get_required_system_ai_targets(self):
        """Returns all system AITargets required to visit in this object."""
        raise NotImplementedError()


class TargetPlanet(AITarget):
    name = 'planet'

    def get_required_system_ai_targets(self):
        result = []
        universe = fo.getUniverse()
        planet = universe.getPlanet(self.target_id)
        result.append(TargetSystem(planet.systemID))

    @property
    def target_obj(self):
        universe = fo.getUniverse()
        return universe.getPlanet(self.target_id)


class TargetSystem(AITarget):
    name = 'system'

    def get_required_system_ai_targets(self):
        return [self]

    @property
    def target_obj(self):
       universe = fo.getUniverse()
       return universe.getSystem(self.target_id)


class TargetFleet(AITarget):
    name = 'fleet'

    def get_required_system_ai_targets(self):
        # Fleet systemID is where is fleet going.
        # If fleet is going nowhere, then it is location of fleet
        universe = fo.getUniverse()
        fleet = universe.getFleet(self.target_id)
        system_id = fleet.nextSystemID
        if system_id == -1:
            system_id = fleet.systemID
        return [TargetSystem(system_id)]


    @property
    def target_obj(self):
        universe = fo.getUniverse()
        return universe.getFleet(self.target_id)


# Old unused targets
# TARGET_BUILDING = 0
# TARGET_TECHNOLOGY = 1
# TARGET_SHIP = 4
# TARGET_EMPIRE = 6
# TARGET_ALL_OTHER_EMPIRES = 7

