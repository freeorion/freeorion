from logging import debug

import freeOrionAIInterface as fo  # pylint: disable=import-error
import AIstate
import CombatRatingsAI
import EspionageAI
import FleetUtilsAI
import InvasionAI
import PlanetUtilsAI
import PriorityAI
import ProductionAI
from AIDependencies import INVALID_ID
from aistate_interface import get_aistate
from CombatRatingsAI import combine_ratings, combine_ratings_list, rating_difference
from EnumsAI import MissionType
from freeorion_tools import cache_by_turn
from target import TargetSystem
from turn_state import state

MinThreat = 10  # the minimum threat level that will be ascribed to an unknown threat capable of killing scouts
_military_allocations = []
_verbose_mil_reporting = False
_best_ship_rating_cache = {}  # indexed by turn, value is rating of that turn


def cur_best_mil_ship_rating(include_designs=False):
    """Find the best military ship we have available in this turn and return its rating.

    :param include_designs: toggles if available designs are considered or only existing ships
    :return: float: rating of the best ship
    """
    current_turn = fo.currentTurn()
    if current_turn in _best_ship_rating_cache:
        best_rating = _best_ship_rating_cache[current_turn]
        if include_designs:
            best_design_rating = ProductionAI.cur_best_military_design_rating()
            best_rating = max(best_rating, best_design_rating)
        return best_rating
    best_rating = 0.001
    universe = fo.getUniverse()
    aistate = get_aistate()
    for fleet_id in FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.MILITARY):
        fleet = universe.getFleet(fleet_id)
        for ship_id in fleet.shipIDs:
            ship_rating = CombatRatingsAI.ShipCombatStats(ship_id).get_rating(enemy_stats=aistate.get_standard_enemy())
            best_rating = max(best_rating, ship_rating)
    _best_ship_rating_cache[current_turn] = best_rating
    if include_designs:
        best_design_rating = ProductionAI.cur_best_military_design_rating()
        best_rating = max(best_rating, best_design_rating)
    return max(best_rating, 0.001)


def get_preferred_max_military_portion_for_single_battle():
    """
    Determine and return the preferred max portion of military to be allocated to a single battle.

    May be used to downgrade various possible actions requiring military support if they would require an excessive
    allocation of military forces.  At the beginning of the game this max portion starts as 1.0, then is slightly
    reduced to account for desire to reserve some defenses for other locations, and then in mid to late game, as the
    size of the the military grows, this portion is further reduced to promote pursuit of multiple battlefronts in
    parallel as opposed to single battlefronts against heavily defended positions.

    :return: a number in range (0:1] for preferred max portion of miltary to be allocated to a single battle
    :rtype: float
    """
    # TODO: this is a roughcut first pass, needs plenty of refinement
    if fo.currentTurn() < 40:
        return 1.0
    best_ship_equivalents = (get_concentrated_tot_mil_rating() / cur_best_mil_ship_rating())**0.5
    _MAX_SHIPS_BEFORE_PREFERRING_LESS_THAN_FULL_ENGAGEMENT = 3
    if best_ship_equivalents <= _MAX_SHIPS_BEFORE_PREFERRING_LESS_THAN_FULL_ENGAGEMENT:
        return 1.0
    # the below ratio_exponent is still very much a work in progress.  It should probably be somewhere in the range of
    # 0.2 to 0.5.  Values at the larger end will create a smaller expected battle size threshold that would
    # cause the respective opportunity (invasion, colonization) scores to be discounted, thereby more quickly creating
    # pressure for the AI to pursue multiple small/medium resistance fronts rather than pursuing a smaller number fronts
    # facing larger resistance.  The AI will start facing some scoring pressure to not need to throw 100% of its
    # military at a target as soon as its max military rating surpasses the equvalent of
    # _MAX_SHIPS_BEFORE_PREFERRING_LESS_THAN_FULL_ENGAGEMENT of its best ships.  That starts simply as some scoring
    # pressure to be able to hold back some small portion of its ships from the engagement, in order to be able to use
    # them for defense or for other targets.  With an exponent value of 0.25, this would start creating substantial
    # pressure against devoting more than half the military to a single target once the total military is somewhere
    # above 18 best-ship equivalents, and pressure against deovting more than a third once the total is above about 80
    # best-ship equivalents.  With an exponent value of 0.5, those thresholds would be 6 ships and 11 ships.  With the
    # initial value of 0.35, those thresholds are about 10 ships and 25 ships.  Depending on how this return value is
    # used, it should not prevent the more heavily fortified targets (and therefore discounted) from being taken
    # if there are no more remaining easier targets available.
    ratio_exponent = 0.35
    return 1.0 / (best_ship_equivalents + 1 - _MAX_SHIPS_BEFORE_PREFERRING_LESS_THAN_FULL_ENGAGEMENT)**ratio_exponent


def try_again(mil_fleet_ids, try_reset=False, thisround=""):
    """Clear targets and orders for all specified fleets then call get_military_fleets again."""
    aistate = get_aistate()
    for fid in mil_fleet_ids:
        mission = aistate.get_fleet_mission(fid)
        mission.clear_fleet_orders()
        mission.clear_target()
    get_military_fleets(try_reset=try_reset, thisround=thisround)


def avail_mil_needing_repair(mil_fleet_ids, split_ships=False, on_mission=False, repair_limit=0.70):
    """Returns tuple of lists: (ids_needing_repair, ids_not)."""
    fleet_buckets = [[], []]
    universe = fo.getUniverse()
    cutoff = [repair_limit, 0.25][on_mission]
    aistate = get_aistate()
    for fleet_id in mil_fleet_ids:
        fleet = universe.getFleet(fleet_id)
        ship_buckets = [[], []]
        ships_cur_health = [0, 0]
        ships_max_health = [0, 0]
        for ship_id in fleet.shipIDs:
            this_ship = universe.getShip(ship_id)
            cur_struc = this_ship.initialMeterValue(fo.meterType.structure)
            max_struc = this_ship.initialMeterValue(fo.meterType.maxStructure)
            ship_ok = cur_struc >= cutoff * max_struc
            ship_buckets[ship_ok].append(ship_id)
            ships_cur_health[ship_ok] += cur_struc
            ships_max_health[ship_ok] += max_struc
        this_sys_id = fleet.systemID if fleet.nextSystemID == INVALID_ID else fleet.nextSystemID
        fleet_ok = (sum(ships_cur_health) >= cutoff * sum(ships_max_health))
        local_status = aistate.systemStatus.get(this_sys_id, {})
        my_local_rating = combine_ratings(local_status.get('mydefenses', {}).get('overall', 0), local_status.get('myFleetRating', 0))
        my_local_rating_vs_planets = local_status.get('myFleetRatingVsPlanets', 0)
        combat_trigger = bool(local_status.get('fleetThreat', 0) or local_status.get('monsterThreat', 0))
        if not combat_trigger and local_status.get('planetThreat', 0):
            universe = fo.getUniverse()
            system = universe.getSystem(this_sys_id)
            for planet_id in system.planetIDs:
                planet = universe.getPlanet(planet_id)
                if planet.ownedBy(fo.empireID()):  # TODO: also exclude at-peace planets
                    continue
                if planet.unowned and not EspionageAI.colony_detectable_by_empire(planet_id, empire=fo.empireID()):
                    continue
                if sum([planet.currentMeterValue(meter_type) for meter_type in
                        [fo.meterType.defense, fo.meterType.shield, fo.meterType.construction]]):
                    combat_trigger = True
                    break
        needed_here = combat_trigger and local_status.get('totalThreat', 0) > 0  # TODO: assess if remaining other forces are sufficient
        safely_needed = needed_here and my_local_rating > local_status.get('totalThreat', 0) and my_local_rating_vs_planets > local_status.get('planetThreat', 0)  # TODO: improve both assessment prongs
        if not fleet_ok:
            if safely_needed:
                debug("Fleet %d at %s needs repair but deemed safely needed to remain for defense" % (fleet_id, universe.getSystem(fleet.systemID)))
            else:
                if needed_here:
                    debug("Fleet %d at %s needed present for combat, but is damaged and deemed unsafe to remain." % (fleet_id, universe.getSystem(fleet.systemID)))
                    debug("\t my_local_rating: %.1f ; threat: %.1f" % (my_local_rating, local_status.get('totalThreat', 0)))
                debug("Selecting fleet %d at %s for repair" % (fleet_id, universe.getSystem(fleet.systemID)))
        fleet_buckets[fleet_ok or bool(safely_needed)].append(fleet_id)
    return fleet_buckets


# TODO Move relevant initialization code from get_military_fleets into this class
class AllocationHelper(object):

    def __init__(self, already_assigned_rating, already_assigned_rating_vs_planets, available_rating, try_reset):
        """
        :param dict already_assigned_rating:
        :param float available_rating:
        """
        self.try_reset = try_reset
        self.allocations = []
        self.allocation_by_groups = {}

        self.available_rating = available_rating
        self._remaining_rating = available_rating

        self.threat_bias = 0.
        self.safety_factor = get_aistate().character.military_safety_factor()

        self.already_assigned_rating = dict(already_assigned_rating)
        self.already_assigned_rating_vs_planets = dict(already_assigned_rating_vs_planets)
        # store the number of empires which have supply or have supply within 2 jumps of the system
        self.enemy_supply = {sys_id: min(2, len(enemies_nearly_supplying_system(sys_id)))
                             for sys_id in fo.getUniverse().systemIDs}

    @property
    def remaining_rating(self):
        return self._remaining_rating

    @remaining_rating.setter
    def remaining_rating(self, value):
        self._remaining_rating = max(0, value)

    def allocate(self, group, sys_id, min_rating, min_rating_vs_planets, take_any, max_rating):
        tup = (sys_id, min_rating, min_rating_vs_planets, take_any, max_rating)
        self.allocations.append(tup)
        self.allocation_by_groups.setdefault(group, []).append(tup)
        if self._remaining_rating <= min_rating:
            self._remaining_rating = 0
        else:
            self._remaining_rating = rating_difference(self._remaining_rating, min_rating)


class Allocator(object):
    """
    Base class for Military allocation for a single system.

    The Allocator class and its subclasses are used to allocate
    military resources for a single system. First, a minimum
    and a maximum military rating are calculated which are
    required / desired based on e.g. threat in the system.

    An Allocator class defines if military resources should
    be allocated even if the minimum requirements are not met
    or if military resources are only allocated if the threshold
    is passed.

    The information is then passed to an AllocationHelper
    instance. Allocating military resources by an Allocator
    does not necessarily mean that military ships are actually
    assigned to that system. It should be understood as a request
    instead. If allocations of higher priority already use all the
    available military resources, no ships can be sent.


    Public methods:
        :allocate(): Calculate the required/desired military resources
                    for the system and enqueue the allocation info
                    to the AllocationHelper.


    Public attributes:
        :ivar sys_id: ID of the system for which military resources are allocated


    Example usage:
        CapitelDefenseAllocator(capital_sys_id, allocation_helper).allocate()
    """
    _min_alloc_factor = 1.
    _max_alloc_factor = 2.
    _potential_threat_factor = 1.
    _allocation_group = ''
    _military_reset_ratio = -1  # if ratio of available to needed rating is smaller than this, then reset allocations

    def __init__(self, sys_id, allocation_helper):
        """
        :param int sys_id: System for which military resources are allocated
        :param AllocationHelper allocation_helper: The allocation helper where the information is to be stored.
        """
        self.sys_id = sys_id
        self._allocation_helper = allocation_helper

    def allocate(self):
        """Calculate the desired allocation for this system and enqueue it in the allocation_helper."""
        threat = self._calculate_threat()
        min_alloc = self._minimum_allocation(threat)
        max_alloc = self._maximum_allocation(threat)
        alloc_vs_planets = self._allocation_vs_planets()
        if min_alloc <= 0 and alloc_vs_planets <= 0:
            # nothing to allocate here...
            return
        min_alloc = max(min_alloc, alloc_vs_planets)
        max_alloc = max(max_alloc, alloc_vs_planets)

        ratio = self._allocation_helper.remaining_rating / float(min_alloc)
        if self._allocation_helper.remaining_rating > min_alloc or self._take_any():
            self._allocation_helper.allocate(
                group=self._allocation_group,
                sys_id=self.sys_id,
                min_rating=min(min_alloc, self._allocation_helper.remaining_rating),
                min_rating_vs_planets=min(alloc_vs_planets, self._allocation_helper.remaining_rating),
                take_any=self._take_any(),
                max_rating=max_alloc,
            )
        if ratio < 1:
            self._handle_not_enough_resources(ratio)

    def _calculate_threat(self):
        """
        Calculate the required military rating in the system.

        The value calculated does not have to represent a tangible
        threat / enemy force. It only provides a measurement how much
        military should be sent to the system. Deriving further conditions
        for military presence and translating them into an equivalent
        military rating is strongly encouraged.
        It is implied however, that the value calculated here should
        be greater than or at least equal to the actual visible strength
        of enemy forces within the system so that a subsequent military
        mission can be successful.

        :return: Equivalent military rating required in the system
        :rtype: float
        """
        raise NotImplementedError

    def _minimum_allocation(self, threat):
        """
        Calculate the minimum allocation for the system.

        The default minimum allocation is the missing forces
        to obtain a rating given by the threat weighted with
        the subclass' *min_alloc_factor*.
        Existing military missions are considered.

        Subclasses may choose to override this method and
        implement a different logic.

        :param float threat: threat as calculated by _calculate_threat()
        :rtype: float
        """
        return CombatRatingsAI.rating_needed(
            self._min_alloc_factor * threat,
            self.assigned_rating)

    def _maximum_allocation(self, threat):
        """
        Calculate the maximum allocation for the system.

        The default maximum allocation is the missing forces
        to obtain a rating given by the threat weighted with
        the subclass' *max_alloc_factor*.
        Existing military missions are considered.

        Subclasses may choose to override this method and
        implement a different logic.

        :param float threat:
        :rtype: float
        """
        return CombatRatingsAI.rating_needed(
                self._max_alloc_factor * threat,
                self.assigned_rating)

    def _allocation_vs_planets(self):
        return CombatRatingsAI.rating_needed(
            self.safety_factor * self._planet_threat(),
            self.assigned_rating_vs_planets)

    def _take_any(self):
        """
        If true, forces smaller than the minimum allocation are accepted.

        :rtype: bool
        """
        raise NotImplementedError

    def _handle_not_enough_resources(self, ratio):
        """Called if minimum allocation is larget than available resources.

        High priority subclasses are expected to throw a ReleaseMilitaryException
        which should be caught from the caller of allocate() and trigger the
        release of all military resources so they can be reassigned to higher
        priority targets.

        :param float ratio: ratio of available resources to minimum allocation
        """
        if ratio < self._military_reset_ratio and self._allocation_helper.try_reset:
            raise ReleaseMilitaryException

    @property
    def nearby_empire_count(self):
        """The number of enemy empires within at most 2 jumps."""
        return self._allocation_helper.enemy_supply.get(self.sys_id, 0)

    @property
    def threat_bias(self):
        """A constant threat biases added additively to the calculated threat."""
        return self._allocation_helper.threat_bias

    @property
    def safety_factor(self):
        """A multiplicative factor for threat calculations"""
        return self._allocation_helper.safety_factor

    @property
    def assigned_rating(self):
        """The combined rating of existing missions assigned to the system."""
        return self._allocation_helper.already_assigned_rating.get(self.sys_id, 0)

    @property
    def assigned_rating_vs_planets(self):
        return self._allocation_helper.already_assigned_rating_vs_planets.get(self.sys_id, 0)

    def _local_threat(self):
        """Military rating of enemies present in the system."""
        return get_system_local_threat(self.sys_id)

    def _neighbor_threat(self):
        """Military rating of enemies present in neighboring system."""
        return get_system_neighbor_threat(self.sys_id)

    def _jump2_threat(self):
        """Military rating of enemies present 2 jumps away from the system."""
        return get_system_jump2_threat(self.sys_id)

    def _potential_threat(self):
        """Number of nearby enemies times the average enemy rating weighted by _potential_threat_factor"""
        return self.nearby_empire_count * enemy_rating() * self._potential_threat_factor

    def _regional_threat(self):
        """Threat derived from enemy supply lanes."""
        return get_system_regional_threat(self.sys_id)

    def _potential_support(self):
        """Military rating of our forces in neighboring systems."""
        return get_system_neighbor_support(self.sys_id)

    def _planet_threat(self):
        return get_system_planetary_threat(self.sys_id)

    def _enemy_ship_count(self):
        return get_aistate().systemStatus.get(self.sys_id, {}).get('enemy_ship_count', 0.)


class CapitalDefenseAllocator(Allocator):

    _allocation_group = 'capitol'
    _military_reset_ratio = 0.5

    def _minimum_allocation(self, threat):
        nearby_forces = CombatRatingsAI.combine_ratings(
                self.assigned_rating, self._potential_support())
        return max(
                CombatRatingsAI.rating_needed(self._regional_threat(), nearby_forces),
                CombatRatingsAI.rating_needed(1.4*threat, self.assigned_rating))

    def _maximum_allocation(self, threat):
        return max(
                CombatRatingsAI.rating_needed(1.5*self._regional_threat(), self.assigned_rating),
                CombatRatingsAI.rating_needed(2*threat, self.assigned_rating))

    def _calculate_threat(self):
        potential_threat = max(self._potential_threat() - self._potential_support(), 0)
        actual_threat = self.safety_factor * (
            2*self.threat_bias +
            + CombatRatingsAI.combine_ratings(self._local_threat(), self._neighbor_threat()))
        return potential_threat + actual_threat

    def _take_any(self):
        return True


class PlanetDefenseAllocator(Allocator):

    _allocation_group = 'occupied'
    _min_alloc_factor = 1.1
    _max_alloc_factor = 1.5
    _potential_threat_factor = 0.5
    _military_reset_ratio = 0.8

    def allocate(self):
        remaining_rating = self._allocation_helper.remaining_rating
        if remaining_rating > 0:
            super(PlanetDefenseAllocator, self).allocate()
            return
        if self._minimum_allocation(self._calculate_threat()):
            pass  # raise ReleaseMilitaryException TODO

    def _minimum_allocation(self, threat):
        super_call = super(PlanetDefenseAllocator, self)._minimum_allocation(threat)
        restriction = 0.5 * self._allocation_helper.available_rating
        return min(super_call, restriction)

    def _calculate_threat(self):
        nearby_forces = CombatRatingsAI.combine_ratings(
            self._potential_support(), self.assigned_rating)
        return (
            self.threat_bias +
            + self.safety_factor * CombatRatingsAI.combine_ratings(self._local_threat(),
                                                                   self._neighbor_threat()) +
            + max(0., self._potential_threat() + self._jump2_threat() - nearby_forces))

    def _take_any(self):
        return True


class TargetAllocator(Allocator):

    _allocation_group = 'otherTargets'
    _min_alloc_factor = 1.3
    _max_alloc_factor = 2.5
    _potential_threat_factor = 0.5

    def _calculate_threat(self):
        return (
            self.threat_bias +
            + self.safety_factor * CombatRatingsAI.combine_ratings_list([
                self._local_threat(),
                .75 * self._neighbor_threat(),
                .5 * self._jump2_threat()])
            + self._potential_threat())

    def _take_any(self):
        return self.assigned_rating > 0

    def _planet_threat_multiplier(self):
        # to the extent that enemy planetary defenses are bolstered by fleet defenses, a smaller portion of our
        # attacks will land on the planet and hence we need a greater portion of planet-effective attacks.  One
        # desired characteristic of the following planet_threat_multiplier is that if the entire local threat is
        # due to planetary threat then we want the multiplier to be unity.  Furthermore, the more enemy ships are
        # present, the smaller proportion of our attacks would be directed against the enemy planet.  The following is
        # just one of many forms of calculation that might work reasonably.
        # TODO: assess and revamp the planet_threat_multiplier calculation
        return ((self._enemy_ship_count() + self._local_threat()/self._planet_threat())**0.5
                if self._planet_threat() > 0 else 1.0)

    def _allocation_vs_planets(self):
        return CombatRatingsAI.rating_needed(
            self.safety_factor*self._planet_threat_multiplier()*self._planet_threat(),
            self.assigned_rating_vs_planets)


class TopTargetAllocator(TargetAllocator):
    _allocation_group = 'topTargets'
    _max_alloc_factor = 3


class OutpostTargetAllocator(TargetAllocator):
    _max_alloc_factor = 3


class BlockadeAllocator(TargetAllocator):
    _potential_threat_factor = 0.25
    _max_alloc_factor = 1.5

    def _maximum_allocation(self, threat):
        return min(self._minimum_allocation(threat), self._allocation_helper.remaining_rating) * self._max_alloc_factor


class LocalThreatAllocator(Allocator):
    _potential_threat_factor = 0
    _min_alloc_factor = 1.3
    _max_alloc_factor = 2
    _allocation_group = 'otherTargets'

    def _calculate_threat(self):

        systems_status = get_aistate().systemStatus.get(self.sys_id, {})
        threat = self.safety_factor * CombatRatingsAI.combine_ratings(systems_status.get('fleetThreat', 0),
                                                                      systems_status.get('monsterThreat', 0) +
                                                                      + systems_status.get('planetThreat', 0))

        return self.threat_bias + threat

    def _take_any(self):
        return False


class InteriorTargetsAllocator(LocalThreatAllocator):
    _max_alloc_factor = 2.5
    _min_alloc_factor = 1.3

    def _calculate_threat(self):
        return self.threat_bias + self.safety_factor * self._local_threat()

    def _maximum_allocation(self, threat):
        return self._max_alloc_factor * min(self._minimum_allocation(threat), self._allocation_helper.remaining_rating)

    def _take_any(self):
        return self.assigned_rating > 0


class ExplorationTargetAllocator(LocalThreatAllocator):
    _potential_threat_factor = 0.25
    _max_alloc_factor = 2.0
    _allocation_group = 'exploreTargets'

    def _calculate_threat(self):
        return self.safety_factor * self._local_threat() + self._potential_threat()

    def _take_any(self):
        return False


class BorderSecurityAllocator(LocalThreatAllocator):
    _min_alloc_factor = 1.2
    _max_alloc_factor = 2
    _allocation_group = 'accessibleTargets'

    def __init__(self, sys_id, allocation_helper):
        super(BorderSecurityAllocator, self).__init__(sys_id, allocation_helper)

    def _maximum_allocation(self, threat):
        return self._max_alloc_factor * self.safety_factor * max(self._local_threat(), self._neighbor_threat())


class ReleaseMilitaryException(Exception):
    pass


# TODO: May want to move these functions into AIstate class
def get_system_local_threat(sys_id):
    return get_aistate().systemStatus.get(sys_id, {}).get('totalThreat', 0.)


def get_system_jump2_threat(sys_id):
    return get_aistate().systemStatus.get(sys_id, {}).get('jump2_threat', 0.)


def get_system_neighbor_support(sys_id):
    return get_aistate().systemStatus.get(sys_id, {}).get('my_neighbor_rating', 0.)


def get_system_neighbor_threat(sys_id):
    return get_aistate().systemStatus.get(sys_id, {}).get('neighborThreat', 0.)


def get_system_regional_threat(sys_id):
    return get_aistate().systemStatus.get(sys_id, {}).get('regional_threat', 0.)


def get_system_planetary_threat(sys_id):
    return get_aistate().systemStatus.get(sys_id, {}).get('planetThreat', 0.)


def enemy_rating():
    """:rtype: float"""
    return get_aistate().empire_standard_enemy_rating


def get_my_defense_rating_in_system(sys_id):
    return get_aistate().systemStatus.get(sys_id, {}).get('mydefenses', {}).get('overall')


def enemies_nearly_supplying_system(sys_id):
    return get_aistate().systemStatus.get(sys_id, {}).get('enemies_nearly_supplied', [])


def get_military_fleets(mil_fleets_ids=None, try_reset=True, thisround="Main"):
    """Get armed military fleets."""
    global _military_allocations

    universe = fo.getUniverse()
    empire_id = fo.empireID()
    home_system_id = PlanetUtilsAI.get_capital_sys_id()

    all_military_fleet_ids = (mil_fleets_ids if mil_fleets_ids is not None
                              else FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.MILITARY))

    # Todo: This block had been originally added to address situations where fleet missions were not properly
    #  terminating, leaving fleets stuck in stale deployments. Assess if this block is still needed at all; delete
    #  if not, otherwise restructure the following code so that in event a reset is occurring greater priority is given
    #  to providing military support to locations where a necessary Secure mission might have just been released (i.e.,
    #  at invasion and colony/outpost targets where the troopships and colony ships are on their way), or else allow
    #  only a partial reset which does not reset Secure missions.
    enable_periodic_mission_reset = False
    if enable_periodic_mission_reset and try_reset and (fo.currentTurn() + empire_id) % 30 == 0 and thisround == "Main":
        debug("Resetting all Military missions as part of an automatic periodic reset to clear stale missions.")
        try_again(all_military_fleet_ids, try_reset=False, thisround=thisround + " Reset")
        return

    mil_fleets_ids = list(FleetUtilsAI.extract_fleet_ids_without_mission_types(all_military_fleet_ids))
    mil_needing_repair_ids, mil_fleets_ids = avail_mil_needing_repair(mil_fleets_ids, split_ships=True)
    avail_mil_rating = combine_ratings_list(map(CombatRatingsAI.get_fleet_rating, mil_fleets_ids))

    if not mil_fleets_ids:
        if "Main" in thisround:
            _military_allocations = []
        return []

    # for each system, get total rating of fleets assigned to it
    already_assigned_rating = {}
    already_assigned_rating_vs_planets = {}
    aistate = get_aistate()
    systems_status = aistate.systemStatus
    enemy_sup_factor = {}  # enemy supply
    for sys_id in universe.systemIDs:
        already_assigned_rating[sys_id] = 0
        already_assigned_rating_vs_planets[sys_id] = 0
        enemy_sup_factor[sys_id] = min(2, len(systems_status.get(sys_id, {}).get('enemies_nearly_supplied', [])))
    for fleet_id in [fid for fid in all_military_fleet_ids if fid not in mil_fleets_ids]:
        ai_fleet_mission = aistate.get_fleet_mission(fleet_id)
        if not ai_fleet_mission.target:  # shouldn't really be possible
            continue
        last_sys = ai_fleet_mission.target.get_system().id  # will count this fleet as assigned to last system in target list  # TODO last_sys or target sys?
        this_rating = CombatRatingsAI.get_fleet_rating(fleet_id)
        this_rating_vs_planets = CombatRatingsAI.get_fleet_rating_against_planets(fleet_id)
        already_assigned_rating[last_sys] = CombatRatingsAI.combine_ratings(
                already_assigned_rating.get(last_sys, 0), this_rating)
        already_assigned_rating_vs_planets[last_sys] = CombatRatingsAI.combine_ratings(
                already_assigned_rating_vs_planets.get(last_sys, 0), this_rating_vs_planets)
    for sys_id in universe.systemIDs:
        my_defense_rating = systems_status.get(sys_id, {}).get('mydefenses', {}).get('overall', 0)
        already_assigned_rating[sys_id] = CombatRatingsAI.combine_ratings(my_defense_rating, already_assigned_rating[sys_id])
        if _verbose_mil_reporting and already_assigned_rating[sys_id]:
            debug("\t System %s already assigned rating %.1f" % (
                universe.getSystem(sys_id), already_assigned_rating[sys_id]))

    # get systems to defend
    capital_id = PlanetUtilsAI.get_capital()
    if capital_id is not None:
        capital_planet = universe.getPlanet(capital_id)
    else:
        capital_planet = None
    # TODO: if no owned planets try to capture one!
    if capital_planet:
        capital_sys_id = capital_planet.systemID
    else:  # should be rare, but so as to not break code below, pick a randomish mil-centroid system
        capital_sys_id = None  # unless we can find one to use
        system_dict = {}
        for fleet_id in all_military_fleet_ids:
            status = aistate.fleetStatus.get(fleet_id, None)
            if status is not None:
                system_id = status['sysID']
                if not list(universe.getSystem(system_id).planetIDs):
                    continue
                system_dict[system_id] = system_dict.get(system_id, 0) + status.get('rating', 0)
        ranked_systems = sorted([(val, sys_id) for sys_id, val in system_dict.items()])
        if ranked_systems:
            capital_sys_id = ranked_systems[-1][-1]
        else:
            try:
                capital_sys_id = aistate.fleetStatus.items()[0][1]['sysID']
            except:
                pass

    num_targets = max(10, PriorityAI.allotted_outpost_targets)
    top_target_planets = ([pid for pid, pscore, trp in AIstate.invasionTargets[:PriorityAI.allotted_invasion_targets()]
                           if pscore > InvasionAI.MIN_INVASION_SCORE] +
                          [pid for pid, (pscore, spec) in aistate.colonisableOutpostIDs.items()[:num_targets]
                           if pscore > InvasionAI.MIN_INVASION_SCORE] +
                          [pid for pid, (pscore, spec) in aistate.colonisablePlanetIDs.items()[:num_targets]
                           if pscore > InvasionAI.MIN_INVASION_SCORE])
    top_target_planets.extend(aistate.qualifyingTroopBaseTargets.keys())

    base_col_target_systems = PlanetUtilsAI.get_systems(top_target_planets)
    top_target_systems = []
    for sys_id in AIstate.invasionTargetedSystemIDs + base_col_target_systems:
        if sys_id not in top_target_systems:
            if aistate.systemStatus[sys_id]['totalThreat'] > get_tot_mil_rating():
                continue
            top_target_systems.append(sys_id)  # doing this rather than set, to preserve order

    try:
        # capital defense
        allocation_helper = AllocationHelper(already_assigned_rating, already_assigned_rating_vs_planets, avail_mil_rating, try_reset)
        if capital_sys_id is not None:
            CapitalDefenseAllocator(capital_sys_id, allocation_helper).allocate()

        # defend other planets
        empire_planet_ids = PlanetUtilsAI.get_owned_planets_by_empire(universe.planetIDs)
        empire_occupied_system_ids = list(set(PlanetUtilsAI.get_systems(empire_planet_ids)) - {capital_sys_id})
        for sys_id in empire_occupied_system_ids:
            PlanetDefenseAllocator(sys_id, allocation_helper).allocate()

        # attack / protect high priority targets
        for sys_id in top_target_systems:
            TopTargetAllocator(sys_id, allocation_helper).allocate()

        # enemy planets
        other_targeted_system_ids = [sys_id for sys_id in set(PlanetUtilsAI.get_systems(AIstate.opponentPlanetIDs)) if
                                     sys_id not in top_target_systems]
        for sys_id in other_targeted_system_ids:
            TargetAllocator(sys_id, allocation_helper).allocate()

        # colony / outpost targets
        other_targeted_system_ids = [sys_id for sys_id in
                                     list(set(AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs)) if
                                     sys_id not in top_target_systems]
        for sys_id in other_targeted_system_ids:
            OutpostTargetAllocator(sys_id, allocation_helper).allocate()

        # TODO blockade enemy systems

        # interior systems
        targetable_ids = set(state.get_systems_by_supply_tier(0))
        current_mil_systems = [sid for sid, _, _, _, _ in allocation_helper.allocations]
        interior_targets1 = targetable_ids.difference(current_mil_systems)
        interior_targets = [sid for sid in interior_targets1 if (
            allocation_helper.threat_bias + systems_status.get(sid, {}).get('totalThreat', 0) > 0.8 * allocation_helper.already_assigned_rating[sid])]
        for sys_id in interior_targets:
            InteriorTargetsAllocator(sys_id, allocation_helper).allocate()

        # TODO Exploration targets

        # border protections
        visible_system_ids = aistate.visInteriorSystemIDs | aistate.visBorderSystemIDs
        accessible_system_ids = ([sys_id for sys_id in visible_system_ids if
                                 universe.systemsConnected(sys_id, home_system_id, empire_id)]
                                 if home_system_id != INVALID_ID else [])
        current_mil_systems = [sid for sid, alloc, rvp, take_any, _ in allocation_helper.allocations if alloc > 0]
        border_targets1 = [sid for sid in accessible_system_ids if sid not in current_mil_systems]
        border_targets = [sid for sid in border_targets1 if (
            allocation_helper.threat_bias + systems_status.get(sid, {}).get('fleetThreat', 0) + systems_status.get(sid, {}).get(
                    'planetThreat', 0) > 0.8 * allocation_helper.already_assigned_rating[sid])]
        for sys_id in border_targets:
            BorderSecurityAllocator(sys_id, allocation_helper).allocate()
    except ReleaseMilitaryException:
        try_again(all_military_fleet_ids)
        return

    new_allocations = []
    remaining_mil_rating = avail_mil_rating
    # for top categories assign max_alloc right away as available
    for cat in ['capitol', 'occupied', 'topTargets']:
        for sid, alloc, rvp, take_any, max_alloc in allocation_helper.allocation_by_groups.get(cat, []):
            if remaining_mil_rating <= 0:
                break
            this_alloc = min(remaining_mil_rating, max_alloc)
            new_allocations.append((sid, this_alloc, alloc, rvp, take_any))
            remaining_mil_rating = rating_difference(remaining_mil_rating,  this_alloc)

    base_allocs = set()
    # for lower priority categories, first assign base_alloc around to all, then top up as available
    for cat in ['otherTargets', 'accessibleTargets', 'exploreTargets']:
        for sid, alloc, rvp, take_any, max_alloc in allocation_helper.allocation_by_groups.get(cat, []):
            if remaining_mil_rating <= 0:
                break
            alloc = min(remaining_mil_rating, alloc)
            base_allocs.add(sid)
            remaining_mil_rating = rating_difference(remaining_mil_rating,  alloc)
    for cat in ['otherTargets', 'accessibleTargets', 'exploreTargets']:
        for sid, alloc, rvp, take_any, max_alloc in allocation_helper.allocation_by_groups.get(cat, []):
            if sid not in base_allocs:
                break
            if remaining_mil_rating <= 0:
                new_allocations.append((sid, alloc, alloc, rvp, take_any))
            else:
                local_max_avail = combine_ratings(remaining_mil_rating, alloc)
                new_rating = min(local_max_avail, max_alloc)
                new_allocations.append((sid, new_rating, alloc, rvp, take_any))
                remaining_mil_rating = rating_difference(local_max_avail, new_rating)

    if "Main" in thisround:
        _military_allocations = new_allocations
    if _verbose_mil_reporting or "Main" in thisround:
        debug("------------------------------\nFinal %s Round Military Allocations: %s \n-----------------------" % (thisround, dict([(sid, alloc) for sid, alloc, _, _, _ in new_allocations])))
        debug("(Apparently) remaining military rating: %.1f" % remaining_mil_rating)

    return new_allocations


def assign_military_fleets_to_systems(use_fleet_id_list=None, allocations=None, round=1):
    # assign military fleets to military theater systems
    global _military_allocations
    universe = fo.getUniverse()
    if allocations is None:
        allocations = []

    doing_main = (use_fleet_id_list is None)
    aistate = get_aistate()
    if doing_main:
        aistate.misc['ReassignedFleetMissions'] = []
        base_defense_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.ORBITAL_DEFENSE)
        unassigned_base_defense_ids = FleetUtilsAI.extract_fleet_ids_without_mission_types(base_defense_ids)
        for fleet_id in unassigned_base_defense_ids:
            fleet = universe.getFleet(fleet_id)
            if not fleet:
                continue
            sys_id = fleet.systemID
            target = TargetSystem(sys_id)
            fleet_mission = aistate.get_fleet_mission(fleet_id)
            fleet_mission.clear_fleet_orders()
            fleet_mission.clear_target()
            mission_type = MissionType.ORBITAL_DEFENSE
            fleet_mission.set_target(mission_type, target)

        all_military_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.MILITARY)
        if not all_military_fleet_ids:
            _military_allocations = []
            return
        avail_mil_fleet_ids = list(FleetUtilsAI.extract_fleet_ids_without_mission_types(all_military_fleet_ids))
        mil_needing_repair_ids, avail_mil_fleet_ids = avail_mil_needing_repair(avail_mil_fleet_ids)
        these_allocations = _military_allocations
        debug("==================================================")
        debug("Assigning military fleets")
        debug("---------------------------------")
    else:
        avail_mil_fleet_ids = list(use_fleet_id_list)
        mil_needing_repair_ids, avail_mil_fleet_ids = avail_mil_needing_repair(avail_mil_fleet_ids)
        these_allocations = allocations

    # send_for_repair(mil_needing_repair_ids) #currently, let get taken care of by AIFleetMission.generate_fleet_orders()

    # get systems to defend

    avail_mil_fleet_ids = set(avail_mil_fleet_ids)
    for sys_id, alloc, minalloc, rvp, takeAny in these_allocations:
        if not doing_main and not avail_mil_fleet_ids:
            break
        found_fleets = []
        found_stats = {}
        ensure_return = sys_id not in set(AIstate.colonyTargetedSystemIDs
                                          + AIstate.outpostTargetedSystemIDs
                                          + AIstate.invasionTargetedSystemIDs)
        these_fleets = FleetUtilsAI.get_fleets_for_mission(
            target_stats={'rating': alloc, 'ratingVsPlanets': rvp, 'target_system': TargetSystem(sys_id)},
            min_stats={'rating': minalloc, 'ratingVsPlanets': rvp, 'target_system': TargetSystem(sys_id)},
            cur_stats=found_stats, starting_system=sys_id, fleet_pool_set=avail_mil_fleet_ids,
            fleet_list=found_fleets, ensure_return=ensure_return)
        if not these_fleets:
            if not found_fleets or not (FleetUtilsAI.stats_meet_reqs(found_stats, {'rating': minalloc}) or takeAny):
                if doing_main:
                    if _verbose_mil_reporting:
                        debug("NO available/suitable military allocation for system %d ( %s ) -- requested allocation %8d, found available rating %8d in fleets %s" % (sys_id, universe.getSystem(sys_id).name, minalloc, found_stats.get('rating', 0), found_fleets))
                avail_mil_fleet_ids.update(found_fleets)
                continue
            else:
                these_fleets = found_fleets
        elif doing_main and _verbose_mil_reporting:
            debug("FULL+ military allocation for system %d ( %s ) -- requested allocation %8d, got %8d with fleets %s"
                  % (sys_id, universe.getSystem(sys_id).name, alloc, found_stats.get('rating', 0), these_fleets))
        target = TargetSystem(sys_id)
        for fleet_id in these_fleets:
            fo.issueAggressionOrder(fleet_id, True)
            fleet_mission = aistate.get_fleet_mission(fleet_id)
            fleet_mission.clear_fleet_orders()
            fleet_mission.clear_target()
            if sys_id in set(AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs + AIstate.invasionTargetedSystemIDs):
                mission_type = MissionType.SECURE
            else:
                mission_type = MissionType.MILITARY
            fleet_mission.set_target(mission_type, target)
            fleet_mission.generate_fleet_orders()
            if not doing_main:
                aistate.misc.setdefault('ReassignedFleetMissions', []).append(fleet_mission)

    if doing_main:
        debug("---------------------------------")
    last_round = 3
    last_round_name = "LastRound"
    if round <= last_round:
        # check if any fleets remain unassigned
        all_military_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.MILITARY)
        avail_mil_fleet_ids = list(FleetUtilsAI.extract_fleet_ids_without_mission_types(all_military_fleet_ids))
        allocations = []
        round += 1
        thisround = "Extras Remaining Round %d" % round if round < last_round else last_round_name
        if avail_mil_fleet_ids:
            debug("Still have available military fleets: %s" % avail_mil_fleet_ids)
            allocations = get_military_fleets(mil_fleets_ids=avail_mil_fleet_ids, try_reset=False, thisround=thisround)
        if allocations:
            assign_military_fleets_to_systems(use_fleet_id_list=avail_mil_fleet_ids, allocations=allocations, round=round)


@cache_by_turn
def get_tot_mil_rating():
    """
    Give an assessment of total miltary rating considering all fleets as if distributed to separate systems.

    :return: a military rating value
    :rtype: float
    """
    return sum(CombatRatingsAI.get_fleet_rating(fleet_id)
               for fleet_id in FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.MILITARY))


@cache_by_turn
def get_concentrated_tot_mil_rating():
    """
    Give an assessment of total miltary rating as if all fleets were merged into a single mega-fleet.

    :return: a military rating value
    :rtype: float
    """
    return CombatRatingsAI.combine_ratings_list([CombatRatingsAI.get_fleet_rating(fleet_id) for fleet_id in
                                                 FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.MILITARY)])


@cache_by_turn
def get_num_military_ships():
    fleet_status = get_aistate().fleetStatus
    return sum(fleet_status.get(fid, {}).get('nships', 0)
               for fid in FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.MILITARY))


def get_military_fleets_with_target_system(target_system_id):
    military_mission_types = [MissionType.MILITARY,  MissionType.SECURE]
    found_fleets = []
    for fleet_mission in get_aistate().get_fleet_missions_with_any_mission_types(military_mission_types):
        if fleet_mission.target and fleet_mission.target.id == target_system_id:
            found_fleets.append(fleet_mission.fleet.id)
    return found_fleets
