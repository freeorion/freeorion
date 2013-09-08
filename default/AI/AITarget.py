import EnumsAI
from EnumsAI import AITargetType
import freeOrionAIInterface as fo # pylint: disable=import-error

AI_TARGET_TYPE_NAMES = AITargetType()

class AITarget(object):
    "stores information about AI target - its id and type"

    def __init__(self, target_type, target_id):
        "constructor"

        self.target_type = target_type
        self.target_id = target_id

    def __cmp__(self, other):
        "compares AITargets"

        if self.target_id < other.target_id:
            return - 1
        elif self.target_id == other.target_id:
            if self.target_type < other.target_type:
                return - 1
            elif self.target_type == other.target_type:
                return 0
            return 1
        return 1

    def __eq__(self, other):
        "returns equal to other object"

        if other == None:
            return False
        if isinstance(other, AITarget):
            return self.__cmp__(other) == 0
        return NotImplemented

    def __ne__(self, other):
        "returns not equal to other object"

        result = self.__eq__(other)
        if result is NotImplemented:
            return result
        return not result

    def __str__(self):
        "returns describing string"
        target = self.target_obj
        if target is None:
            target_name = "%4d" % self.target_id
        else:
            target_name = target.name
        return "{ %7s : [%4d] %9s}" % (
            AI_TARGET_TYPE_NAMES.name(self.target_type),
            self.target_id,
            target_name
        )

    @property
    def target_obj(self):
        """
        returns target UniverseObject for fleets, systems, planets, buildings;
        None for other targets
        """
        universe = fo.getUniverse()
        target = None
        if self.target_type == AITargetType.TARGET_FLEET:
            target = universe.getFleet(self.target_id)
        elif self.target_type == AITargetType.TARGET_SYSTEM:
            target = universe.getSystem(self.target_id)
        elif self.target_type == AITargetType.TARGET_PLANET:
            target = universe.getPlanet(self.target_id)
        elif self.target_type == AITargetType.TARGET_BUILDING:
            target = universe.getBuilding(self.target_id)
        return target

    def valid(self):
        "returns if this object is valid"

        if self.target_id == None or self.target_type == None or \
                EnumsAI.checkValidity(self.target_id) == False:
            return False

        if AITargetType.TARGET_EMPIRE == self.target_type:
            return self.target_id in fo.AllEmpireIDs()
        else:
            return None != self.target_obj

        return False

    def get_required_system_ai_targets(self):
        "returns all system AITargets required to visit in this object"

        # TODO: add parameter turn

        result = []
        if AITargetType.TARGET_SYSTEM == self.target_type:
            result.append(self)

        elif AITargetType.TARGET_PLANET == self.target_type:
            universe = fo.getUniverse()
            planet = universe.getPlanet(self.target_id)
            ai_target = AITarget(AITargetType.TARGET_SYSTEM, planet.systemID)

            result.append(ai_target)

        elif AITargetType.TARGET_FLEET == self.target_type:
            # Fleet systemID is where is fleet going.
            # If fleet is going nowhere, then it is location of fleet
            universe = fo.getUniverse()
            fleet = universe.getFleet(self.target_id)
            system_id = fleet.nextSystemID
            if (system_id == -1):
                system_id = fleet.systemID
            ai_target = AITarget(AITargetType.TARGET_SYSTEM, system_id)

            result.append(ai_target)

        return result
