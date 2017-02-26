import freeOrionAIInterface as fo  # pylint: disable=import-error
import FreeOrionAI as foAI
import AIstate
import universe_object
from EnumsAI import MissionType
import FleetUtilsAI
from CombatRatingsAI import combine_ratings, combine_ratings_list, rating_needed
import PlanetUtilsAI
import PriorityAI
import ColonisationAI
import ProductionAI
import CombatRatingsAI
from freeorion_tools import ppstring, chat_human, print_error
from AIDependencies import INVALID_ID

MinThreat = 10  # the minimum threat level that will be ascribed to an unknown threat capable of killing scouts
_military_allocations = []
_min_mil_allocations = {}
totMilRating = 0
num_milships = 0
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
    for fleet_id in FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.MILITARY):
        fleet = universe.getFleet(fleet_id)
        for ship_id in fleet.shipIDs:
            ship_rating = CombatRatingsAI.ShipCombatStats(ship_id).get_rating(enemy_stats=foAI.foAIstate.get_standard_enemy())
            best_rating = max(best_rating, ship_rating)
    _best_ship_rating_cache[current_turn] = best_rating
    if include_designs:
        best_design_rating = ProductionAI.cur_best_military_design_rating()
        best_rating = max(best_rating, best_design_rating)
    return max(best_rating, 0.001)


def try_again(mil_fleet_ids, try_reset=False, thisround=""):
    """Clear targets and orders for all specified fleets then call get_military_fleets again."""
    for fid in mil_fleet_ids:
        mission = foAI.foAIstate.get_fleet_mission(fid)
        mission.clear_fleet_orders()
        mission.clear_target()
    get_military_fleets(try_reset=try_reset, thisround=thisround)


def avail_mil_needing_repair(mil_fleet_ids, split_ships=False, on_mission=False, repair_limit=0.70):
    """Returns tuple of lists: (ids_needing_repair, ids_not)."""
    fleet_buckets = [[], []]
    universe = fo.getUniverse()
    cutoff = [repair_limit, 0.25][on_mission]
    for fleet_id in mil_fleet_ids:
        fleet = universe.getFleet(fleet_id)
        ship_buckets = [[], []]
        ships_cur_health = [0, 0]
        ships_max_health = [0, 0]
        for ship_id in fleet.shipIDs:
            this_ship = universe.getShip(ship_id)
            cur_struc = this_ship.currentMeterValue(fo.meterType.structure)
            max_struc = this_ship.currentMeterValue(fo.meterType.maxStructure)
            ship_ok = cur_struc >= cutoff * max_struc
            ship_buckets[ship_ok].append(ship_id)
            ships_cur_health[ship_ok] += cur_struc
            ships_max_health[ship_ok] += max_struc
        this_sys_id = (fleet.nextSystemID != INVALID_ID and fleet.nextSystemID) or fleet.systemID
        fleet_ok = (sum(ships_cur_health) >= cutoff * sum(ships_max_health))
        local_status = foAI.foAIstate.systemStatus.get(this_sys_id, {})
        my_local_rating = combine_ratings(local_status.get('mydefenses', {}).get('overall', 0), local_status.get('myFleetRating', 0))
        needed_here = local_status.get('totalThreat', 0) > 0  # TODO: assess if remaining other forces are sufficient
        safely_needed = needed_here and my_local_rating > local_status.get('totalThreat', 0)  # TODO: improve both assessment prongs
        if not fleet_ok:
            if safely_needed:
                print "Fleet %d at %s needs repair but deemed safely needed to remain for defense" % (fleet_id, ppstring(PlanetUtilsAI.sys_name_ids([fleet.systemID])))
            else:
                if needed_here:
                    print "Fleet %d at %s needed present for combat, but is damaged and deemed unsafe to remain." % (fleet_id, ppstring(PlanetUtilsAI.sys_name_ids([fleet.systemID])))
                    print "\t my_local_rating: %.1f ; threat: %.1f" % (my_local_rating, local_status.get('totalThreat', 0))
                print "Selecting fleet %d at %s for repair" % (fleet_id, ppstring(PlanetUtilsAI.sys_name_ids([fleet.systemID])))
        fleet_buckets[fleet_ok or safely_needed].append(fleet_id)
    return fleet_buckets


# TODO Move relevant initialization code from get_military_fleets into this class
class AllocationHelper(object):

    def __init__(self, already_assigned_rating, available_rating):
        """
        :param dict already_assigned_rating:
        :param float available_rating:
        """
        #
        self._allocations = []
        self._allocation_by_groups = {}

        #
        self.available_rating = available_rating
        self._remaining_rating = available_rating

        #
        self.threat_bias = 0.
        self.safety_factor = foAI.foAIstate.character.military_safety_factor()

        #
        self.already_assigned_rating = dict(already_assigned_rating)
        # store the number of empires which have supply or have supply within 2 jumps of the system
        self.enemy_supply = {sys_id: min(2, len(enemies_nearly_supplying_system(sys_id)))
                             for sys_id in fo.getUniverse().systemIDs}

    @property
    def remaining_rating(self):
        return self._remaining_rating

    @remaining_rating.setter
    def remaining_rating(self, value):
        self._remaining_rating = max(0, value)

    def allocate(self, group, sys_id, min_rating, take_any, max_rating):
        tup = (sys_id, min_rating, take_any, max_rating)
        self._allocations.append(tup)
        self._allocation_by_groups.setdefault(group, []).append(tup)
        self._remaining_rating -= min_rating


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
        if min_alloc > 0:
            ratio = self._allocation_helper.remaining_rating / float(min_alloc)
            if ratio < 1:
                self._handle_not_enough_resources(ratio)
        if (min_alloc > 0 and  # TODO only if remaining rating
                (self._allocation_helper.remaining_rating > min_alloc or self._take_any())):
            self._allocation_helper.allocate(
                group=self._allocation_group,
                sys_id=self.sys_id,
                min_rating=min(min_alloc, self._allocation_helper.remaining_rating),
                take_any=self._take_any(),
                max_rating=max_alloc,
            )
        else:
            return

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
        if ratio < self._military_reset_ratio:
            pass  # TODO raise ReleaseMilitaryException

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
    _min_alloc_factor = 1.3
    _max_alloc_factor = 2
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
    _min_alloc_factor = 1.4
    _max_alloc_factor = 2
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
    _min_alloc_factor = 1.4
    _max_alloc_factor = 2
    _allocation_group = 'otherTargets'

    def _calculate_threat(self):

        systems_status = foAI.foAIstate.systemStatus.get(self.sys_id, {})
        threat = self.safety_factor * CombatRatingsAI.combine_ratings(systems_status.get('fleetThreat', 0),
                                                                      systems_status.get('monsterThreat', 0) +
                                                                      + systems_status.get('planetThreat', 0))

        return self.threat_bias + threat

    def _take_any(self):
        return False


class InteriorTargetsAllocator(LocalThreatAllocator):
    _max_alloc_factor = 3
    _min_alloc_factor = 1.5

    def _calculate_threat(self):
        return self.threat_bias + self.safety_factor * self._local_threat()

    def _maximum_allocation(self, threat):
        return self._max_alloc_factor * min(self._minimum_allocation(threat), self._allocation_helper.remaining_rating)

    def _take_any(self):
        return self.assigned_rating > 0


class ExplorationTargetAllocator(LocalThreatAllocator):
    _potential_threat_factor = 0.25
    _max_alloc_factor = 2 * 1.4
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
    return foAI.foAIstate.systemStatus.get(sys_id, {}).get('totalThreat', 0.)


def get_system_jump2_threat(sys_id):
    return foAI.foAIstate.systemStatus.get(sys_id, {}).get('jump2_threat', 0.)


def get_system_neighbor_support(sys_id):
    return foAI.foAIstate.systemStatus.get(sys_id, {}).get('my_neighbor_rating', 0.)


def get_system_neighbor_threat(sys_id):
    return foAI.foAIstate.systemStatus.get(sys_id, {}).get('neighborThreat', 0.)


def get_system_regional_threat(sys_id):
    return foAI.foAIstate.systemStatus.get(sys_id, {}).get('regional_threat', 0.)


def enemy_rating():
    """:rtype: float"""
    return foAI.foAIstate.empire_standard_enemy_rating


def get_my_defense_rating_in_system(sys_id):
    return foAI.foAIstate.systemStatus.get(sys_id, {}).get('mydefenses', {}).get('overall')


def enemies_nearly_supplying_system(sys_id):
    return foAI.foAIstate.systemStatus.get(sys_id, {}).get('enemies_nearly_supplied', [])


def get_military_fleets(mil_fleets_ids=None, try_reset=True, thisround="Main"):
    """Get armed military fleets."""
    global _military_allocations, totMilRating, num_milships

    universe = fo.getUniverse()
    empire_id = fo.empireID()
    home_system_id = PlanetUtilsAI.get_capital_sys_id()

    all_military_fleet_ids = (mil_fleets_ids if mil_fleets_ids is not None
                              else FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.MILITARY))

    if try_reset and (fo.currentTurn() + empire_id) % 30 == 0 and thisround == "Main":
        try_again(all_military_fleet_ids, try_reset=False, thisround=thisround + " Reset")
        return

    num_milships = sum(foAI.foAIstate.fleetStatus.get(fid, {}).get('nships', 0) for fid in all_military_fleet_ids)

    if "Main" in thisround:
        totMilRating = sum(CombatRatingsAI.get_fleet_rating(fid) for fid in all_military_fleet_ids)

    enemy_rating = foAI.foAIstate.empire_standard_enemy_rating

    mil_fleets_ids = list(FleetUtilsAI.extract_fleet_ids_without_mission_types(all_military_fleet_ids))
    mil_needing_repair_ids, mil_fleets_ids = avail_mil_needing_repair(mil_fleets_ids, split_ships=True)
    avail_mil_rating = sum(map(CombatRatingsAI.get_fleet_rating, mil_fleets_ids))
    if "Main" in thisround:
        print "=================================================="
        print "%s Round Available Military Rating: %d" % (thisround, avail_mil_rating)
        print "---------------------------------"
    remaining_mil_rating = avail_mil_rating
    allocations = []
    allocation_groups = {}

    if not mil_fleets_ids:
        if "Main" in thisround:
            _military_allocations = []
        return []

    # for each system, get total rating of fleets assigned to it
    already_assigned_rating = {}
    systems_status = foAI.foAIstate.systemStatus
    enemy_sup_factor = {}  # enemy supply
    for sys_id in universe.systemIDs:
        already_assigned_rating[sys_id] = 0
        enemy_sup_factor[sys_id] = min(2, len(systems_status.get(sys_id, {}).get('enemies_nearly_supplied', [])))
    for fleet_id in [fid for fid in all_military_fleet_ids if fid not in mil_fleets_ids]:
        ai_fleet_mission = foAI.foAIstate.get_fleet_mission(fleet_id)
        if not ai_fleet_mission.target:  # shouldn't really be possible
            continue
        last_sys = ai_fleet_mission.target.get_system().id  # will count this fleet as assigned to last system in target list  # TODO last_sys or target sys?
        this_rating = CombatRatingsAI.get_fleet_rating(fleet_id)
        already_assigned_rating[last_sys] = CombatRatingsAI.combine_ratings(already_assigned_rating.get(last_sys, 0), this_rating)
    for sys_id in universe.systemIDs:
        my_defense_rating = systems_status.get(sys_id, {}).get('mydefenses', {}).get('overall', 0)
        already_assigned_rating[sys_id] = CombatRatingsAI.combine_ratings(my_defense_rating, already_assigned_rating[sys_id])
        if _verbose_mil_reporting and already_assigned_rating[sys_id]:
            print "\t System %s already assigned rating %.1f" % (
                universe.getSystem(sys_id), already_assigned_rating[sys_id])

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
            status = foAI.foAIstate.fleetStatus.get(fleet_id, None)
            if status is not None:
                sys_id = status['sysID']
                if not list(universe.getSystem(sys_id).planetIDs):
                    continue
                system_dict[sys_id] = system_dict.get(sys_id, 0) + status.get('rating', 0)
        ranked_systems = sorted([(val, sys_id) for sys_id, val in system_dict.items()])
        if ranked_systems:
            capital_sys_id = ranked_systems[-1][-1]
        else:
            try:
                capital_sys_id = foAI.foAIstate.fleetStatus.items()[0][1]['sysID']
            except:
                pass

    if False:
        if fo.currentTurn() < 20:
            threat_bias = 0
        elif fo.currentTurn() < 40:
            threat_bias = 10
        elif fo.currentTurn() < 60:
            threat_bias = 80
        elif fo.currentTurn() < 80:
            threat_bias = 200
        else:
            threat_bias = 400
    else:
        threat_bias = 0

    safety_factor = foAI.foAIstate.character.military_safety_factor()

    num_targets = max(10, PriorityAI.allotted_outpost_targets)
    top_target_planets = ([pid for pid, pscore, trp in AIstate.invasionTargets[:PriorityAI.allottedInvasionTargets] if pscore > 20] +
                          [pid for pid, (pscore, spec) in foAI.foAIstate.colonisableOutpostIDs.items()[:num_targets] if pscore > 20] +
                          [pid for pid, (pscore, spec) in foAI.foAIstate.colonisablePlanetIDs.items()[:num_targets] if pscore > 20])
    top_target_planets.extend(foAI.foAIstate.qualifyingTroopBaseTargets.keys())
    base_col_target_systems = PlanetUtilsAI.get_systems(top_target_planets)
    top_target_systems = []
    for sys_id in AIstate.invasionTargetedSystemIDs + base_col_target_systems:
        if sys_id not in top_target_systems:
            if foAI.foAIstate.systemStatus[sys_id]['totalThreat'] > totMilRating:
                continue
            top_target_systems.append(sys_id)  # doing this rather than set, to preserve order


    allocation_helper = AllocationHelper(already_assigned_rating, avail_mil_rating)
    if _verbose_mil_reporting:
        print "----------------------------"
    # <editor-fold description="Capital Defense">
    if capital_sys_id is not None:
        capital_sys_status = systems_status[capital_sys_id]
        capital_threat = safety_factor*(2 * threat_bias + combine_ratings_list([capital_sys_status[thrt_key] for thrt_key in ['totalThreat', 'neighborThreat']]))
        capital_threat += max(0, enemy_sup_factor[capital_sys_id]*enemy_rating - capital_sys_status.get('my_neighbor_rating', 0))
        local_support = combine_ratings(already_assigned_rating[capital_sys_id], capital_sys_status['my_neighbor_rating'])
        base_needed_rating = rating_needed(capital_sys_status['regional_threat'], local_support)
        needed_rating = max(base_needed_rating, rating_needed(1.4 * capital_threat, already_assigned_rating[capital_sys_id]))
        max_alloc = max(rating_needed(1.5 * capital_sys_status['regional_threat'], already_assigned_rating[capital_sys_id]),
                        rating_needed(2 * capital_threat, already_assigned_rating[capital_sys_id]))
        new_alloc = 0
        if try_reset:
            if needed_rating > 0.5*avail_mil_rating:
                try_again(all_military_fleet_ids)
                return
        if needed_rating > 0:
            new_alloc = min(remaining_mil_rating, needed_rating)
            allocations.append((capital_sys_id, new_alloc, True, max_alloc))
            allocation_groups.setdefault('capitol', []).append((capital_sys_id, new_alloc, True, max_alloc))
            if _verbose_mil_reporting:
                report_format = ("\tAt Capital system %s, local threat %.1f, regional threat %.1f, local support %.1f, "
                                 "base_needed_rating %.1f, needed_rating %.1f, new allocation %.1f")
                print report_format % (universe.getSystem(capital_sys_id), capital_threat,
                                       capital_sys_status['regional_threat'], local_support,
                                       base_needed_rating, needed_rating, new_alloc)
            remaining_mil_rating -= new_alloc
        if "Main" in thisround or new_alloc > 0:
            if _verbose_mil_reporting:
                print "Empire Capital System: (%d) %s -- threat : %d, military allocation: existing: %d ; new: %d" % (capital_sys_id, universe.getSystem(capital_sys_id).name, capital_threat, already_assigned_rating[capital_sys_id], new_alloc)
                print "-----------------"

        CapitalDefenseAllocator(capital_sys_id, allocation_helper).allocate()
    print "Original Code Allocations:", allocation_groups
    print "Rebased Code Allocations: ", allocation_helper._allocation_by_groups
    # </editor-fold>

    # <editor-fold description="Empire Occupied Systems">
    empire_planet_ids = PlanetUtilsAI.get_owned_planets_by_empire(universe.planetIDs)
    empire_occupied_system_ids = list(set(PlanetUtilsAI.get_systems(empire_planet_ids)) - {capital_sys_id})
    if "Main" in thisround:
        if _verbose_mil_reporting:
            print "Empire-Occupied Systems: %s" % (["| %d %s |" % (eo_sys_id, universe.getSystem(eo_sys_id).name) for eo_sys_id in empire_occupied_system_ids])
            print "-----------------"
    new_alloc = 0
    if empire_occupied_system_ids:
        oc_sys_tot_threat_v1 = [(o_s_id, threat_bias + safety_factor*combine_ratings_list(
                                [systems_status.get(o_s_id, {}).get(thrt_key, 0) for thrt_key in ['totalThreat', 'neighborThreat']]))
                                                                                            for o_s_id in empire_occupied_system_ids]
        tot_oc_sys_threat = sum([thrt for _, thrt in oc_sys_tot_threat_v1])
        tot_cur_alloc = sum([already_assigned_rating[sid] for sid, _ in oc_sys_tot_threat_v1])
        # intentionally after tallies, but perhaps should be before
        oc_sys_tot_threat = []
        threat_details = {}
        for sys_id, sys_threat in oc_sys_tot_threat_v1:
            j2_threat = systems_status.get(sys_id, {}).get('jump2_threat', 0)
            local_defenses = combine_ratings_list([systems_status.get(sys_id, {}).get('my_neighbor_rating', 0),
                                                   already_assigned_rating[sys_id]])
            threat_details[sys_id] = (sys_threat, enemy_sup_factor[sys_id] * 0.5 * enemy_rating,
                                      j2_threat, local_defenses)
            oc_sys_tot_threat.append((sys_id, sys_threat + max(0,
                                                               enemy_sup_factor[sys_id] * 0.5 * enemy_rating +
                                                               j2_threat - local_defenses
                                                               )))

        oc_sys_alloc = 0
        for sid, thrt in oc_sys_tot_threat:
            cur_alloc = already_assigned_rating[sid]
            needed_rating = rating_needed(1.3 * thrt, cur_alloc)
            max_alloc = rating_needed(2*thrt, cur_alloc)
            if (needed_rating > 0.8 * remaining_mil_rating) and try_reset:
                try_again(all_military_fleet_ids)
                return
            this_alloc = 0
            if needed_rating > 0 and remaining_mil_rating > 0:
                this_alloc = max(0, min(needed_rating, 0.5 * avail_mil_rating, remaining_mil_rating))
                new_alloc += this_alloc
                allocations.append((sid, this_alloc, True, max_alloc))
                allocation_groups.setdefault('occupied', []).append((sid, this_alloc, True, max_alloc))
                remaining_mil_rating -= this_alloc
                oc_sys_alloc += this_alloc
            if "Main" in thisround or this_alloc > 0:
                if _verbose_mil_reporting:
                    print "Provincial Occupied system %4d ( %10s ) has local threat %8d ; existing military allocation %d and new allocation %8d" % (sid, universe.getSystem(sid).name, thrt, cur_alloc, this_alloc)
                    print "\t base threat was %.1f, supply_threat %.1f, jump2_threat %.1f, and local defenses %.1f" % (
                        threat_details[sid])
        if "Main" in thisround or new_alloc > 0:
            if _verbose_mil_reporting:
                print "Provincial Empire-Occupied Sytems under total threat: %d -- total mil allocation: existing %d ; new: %d" % (tot_oc_sys_threat, tot_cur_alloc, oc_sys_alloc)
                print "-----------------"
        for sys_id, _ in oc_sys_tot_threat:
            PlanetDefenseAllocator(sys_id, allocation_helper).allocate()
        print "Original Code Allocations:", allocation_groups
        print "Rebased Code Allocations: ", allocation_helper._allocation_by_groups
    # </editor-fold>
    # <editor-fold description="Top Targeted Systems">
    # TODO: do native invasions highest priority
    other_targeted_system_ids = top_target_systems
    if "Main" in thisround:
        if _verbose_mil_reporting:
            print "Top Colony and Invasion Targeted Systems : %s" % (["| %d %s |" % (sys_id, universe.getSystem(sys_id).name) for sys_id in other_targeted_system_ids])
            print "-----------------"
    new_alloc = 0
    if other_targeted_system_ids:
        ot_sys_alloc = 0
        ot_sys_threat = [(o_s_id, threat_bias + safety_factor * combine_ratings_list([
                                systems_status.get(o_s_id, {}).get('totalThreat', 0),
                                0.75 * systems_status.get(o_s_id, {}).get('neighborThreat', 0),
                                0.5 * systems_status.get(o_s_id, {}).get('jump2_threat', 0)]))
                         for o_s_id in other_targeted_system_ids]
        totot_sys_threat = sum([thrt for sid, thrt in ot_sys_threat])
        tot_cur_alloc = sum([already_assigned_rating[sid] for sid, thrt in ot_sys_threat])
        # intentionally after tallies, but perhaps should be before
        ot_sys_threat = [(sys_id, sys_threat + enemy_sup_factor[sys_id] * 0.5 * enemy_rating)
                         for sys_id, sys_threat in ot_sys_threat]
        for sid, thrt in ot_sys_threat:
            cur_alloc = already_assigned_rating[sid]
            needed_rating = rating_needed(1.4*thrt, cur_alloc)
            this_alloc = 0
            # only record more allocation for this invasion if we already started or have enough rating available
            take_any = already_assigned_rating[sid] > 0
            if needed_rating > 0 and (remaining_mil_rating > needed_rating or take_any):
                this_alloc = max(0, min(needed_rating, remaining_mil_rating))
                max_alloc = rating_needed(3 * thrt, cur_alloc)
                new_alloc += this_alloc
                allocations.append((sid, this_alloc, take_any, max_alloc))
                allocation_groups.setdefault('topTargets', []).append((sid, this_alloc, take_any, max_alloc))
                remaining_mil_rating -= this_alloc
                ot_sys_alloc += this_alloc
            if "Main" in thisround or this_alloc > 0:
                if _verbose_mil_reporting:
                    print "Targeted system %4d ( %10s ) has local threat %8d ; existing military allocation %d and new allocation %8d" % (sid, universe.getSystem(sid).name, thrt, cur_alloc, this_alloc)
        if "Main" in thisround or new_alloc > 0:
            if _verbose_mil_reporting:
                print "-----------------"
                print "Top Colony and Invasion Targeted Systems under total threat: %d -- total mil allocation-- existing: %d ; new: %d" % (totot_sys_threat, tot_cur_alloc, ot_sys_alloc)
                print "-----------------"
        for sys_id, _ in ot_sys_threat:
            TopTargetAllocator(sys_id, allocation_helper).allocate()
    # </editor-fold>
    # <editor-fold description="Targeted Systems">
    # TODO: do native invasions highest priority
    other_targeted_system_ids = [sys_id for sys_id in set(PlanetUtilsAI.get_systems(AIstate.opponentPlanetIDs)) if sys_id not in top_target_systems]
    if "Main" in thisround:
        if _verbose_mil_reporting:
            print "Other Invasion Targeted Systems : %s" % (["| %d %s |" % (sys_id, universe.getSystem(sys_id).name) for sys_id in other_targeted_system_ids])
            print "-----------------"
    # for these, calc local threat only, no neighbor threat, but use a multiplier for fleet safety
    new_alloc = 0
    if other_targeted_system_ids:
        ot_sys_alloc = 0
        ot_sys_threat = [(o_s_id, threat_bias + safety_factor*combine_ratings_list([
                                systems_status.get(o_s_id, {}).get('totalThreat', 0),
                                0.75*systems_status.get(o_s_id, {}).get('neighborThreat', 0),
                                0.5*systems_status.get(o_s_id, {}).get('jump2_threat', 0)]))
                         for o_s_id in other_targeted_system_ids]
        totot_sys_threat = sum([thrt for sid, thrt in ot_sys_threat])
        tot_cur_alloc = sum([already_assigned_rating[sid] for sid, thrt in ot_sys_threat])
        # intentionally after tallies, but perhaps should be before
        ot_sys_threat = [(sys_id, sys_threat + enemy_sup_factor[sys_id] * 0.5 * enemy_rating) for sys_id, sys_threat in ot_sys_threat]
        for sid, thrt in ot_sys_threat:
            cur_alloc = already_assigned_rating[sid]
            needed_rating = rating_needed(1.4 * thrt, cur_alloc)
            this_alloc = 0
            # only record more allocation for this invasion if we already started or have enough rating available
            take_any = already_assigned_rating[sid] > 0
            if needed_rating > 0 and (remaining_mil_rating > needed_rating or take_any):
                this_alloc = max(0, min(needed_rating, remaining_mil_rating))
                new_alloc += this_alloc
                max_alloc = rating_needed(2 * thrt, cur_alloc)
                allocations.append((sid, this_alloc, take_any, max_alloc))
                allocation_groups.setdefault('otherTargets', []).append((sid, this_alloc, take_any, max_alloc))
                remaining_mil_rating -= this_alloc
                ot_sys_alloc += this_alloc
            if "Main" in thisround or this_alloc > 0:
                if _verbose_mil_reporting:
                    print "Targeted system %4d ( %10s ) has local threat %8d ; existing military allocation %d and new allocation %8d" % (sid, universe.getSystem(sid).name, thrt, cur_alloc, this_alloc)
        if "Main" in thisround or new_alloc > 0:
            if _verbose_mil_reporting:
                print "-----------------"
                print "Invasion Targeted Systems under total threat: %d -- total mil allocation-- existing: %d ; new: %d" % (totot_sys_threat, tot_cur_alloc, ot_sys_alloc)
                print "-----------------"
        for sys_id, _ in ot_sys_threat:
            TargetAllocator(sys_id, allocation_helper).allocate()
    # </editor-fold>
    # <editor-fold description="Colo+Outpost">
    other_targeted_system_ids = [sys_id for sys_id in list(set(AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs)) if sys_id not in top_target_systems]
    if "Main" in thisround:
        if _verbose_mil_reporting:
            print "Other Targeted Systems : %s" % (["| %d %s |" % (sys_id, universe.getSystem(sys_id).name) for sys_id in other_targeted_system_ids])
            print "-----------------"
    if other_targeted_system_ids:
        ot_sys_alloc = 0
        ot_sys_threat = [(o_s_id, threat_bias + safety_factor*combine_ratings_list([
                                systems_status.get(o_s_id, {}).get('totalThreat', 0),
                                0.75*systems_status.get(o_s_id, {}).get('neighborThreat', 0),
                                0.5*systems_status.get(o_s_id, {}).get('jump2_threat', 0)]))
                         for o_s_id in other_targeted_system_ids]
        totot_sys_threat = sum([thrt for sid, thrt in ot_sys_threat])
        # intentionally after tallies, but perhaps should be before
        ot_sys_threat = [(sys_id, sys_threat + enemy_sup_factor[sys_id] * 0.5 * enemy_rating) for sys_id, sys_threat in ot_sys_threat]
        tot_cur_alloc = sum([already_assigned_rating[sid] for sid, thrt in ot_sys_threat])
        new_alloc = 0
        for sid, thrt in ot_sys_threat:
            cur_alloc = already_assigned_rating[sid]
            needed_rating = rating_needed(1.4 * thrt, cur_alloc)
            this_alloc = 0
            # only record more allocation for this invasion if we already started or have enough rating available
            take_any = already_assigned_rating[sid] > 0
            if needed_rating > 0 and (remaining_mil_rating > needed_rating or take_any):
                this_alloc = max(0, min(needed_rating, remaining_mil_rating))
                new_alloc += this_alloc
                max_alloc = rating_needed(3*thrt, cur_alloc)
                allocations.append((sid, this_alloc, take_any, max_alloc))
                allocation_groups.setdefault('otherTargets', []).append((sid, this_alloc, take_any, max_alloc))
                remaining_mil_rating -= this_alloc
                ot_sys_alloc += this_alloc
            if "Main" in thisround or this_alloc > 0:
                if _verbose_mil_reporting:
                    print "Targeted system %4d ( %10s ) has local threat %8d ; existing military allocation %d and new allocation %8d" % (sid, universe.getSystem(sid).name, thrt, cur_alloc, this_alloc)
        if "Main" in thisround or new_alloc > 0:
            if _verbose_mil_reporting:
                print "-----------------"
                print "Other Targeted Systems under total threat: %d -- total mil allocation-- existing: %d ; new: %d" % (totot_sys_threat, tot_cur_alloc, ot_sys_alloc)
                print "-----------------"
        for sys_id, _ in ot_sys_threat:
            OutpostTargetAllocator(sys_id, allocation_helper).allocate()
    # </editor-fold>
    # TODO The entire code block below is effectively not used as AIstate.opponentSystemIDs is never filled...
    other_targeted_system_ids = []
    # targetable_ids = ColonisationAI.annexableSystemIDs.union(empire.fleetSupplyableSystemIDs)
    targetable_ids = set(ColonisationAI.systems_by_supply_tier.get(0, []) + ColonisationAI.systems_by_supply_tier.get(1, []))
    for sys_id in AIstate.opponentSystemIDs:
        if sys_id in targetable_ids:
            other_targeted_system_ids.append(sys_id)
        else:
            for nID in universe.getImmediateNeighbors(sys_id, empire_id):
                if nID in targetable_ids:
                    other_targeted_system_ids.append(sys_id)
                    break

    if "Main" in thisround:
        if _verbose_mil_reporting:
            print "Blockade Targeted Systems : %s" % (["| %d %s |" % (sys_id, universe.getSystem(sys_id).name) for sys_id in other_targeted_system_ids])
            print "-----------------"
    if other_targeted_system_ids:
        ot_sys_alloc = 0
        ot_sys_threat = [(o_s_id, threat_bias + safety_factor*combine_ratings_list([
                                systems_status.get(o_s_id, {}).get('totalThreat', 0),
                                0.75*systems_status.get(o_s_id, {}).get('neighborThreat', 0),
                                0.5*systems_status.get(o_s_id, {}).get('jump2_threat', 0)]))
                         for o_s_id in other_targeted_system_ids]
        totot_sys_threat = sum([thrt for sid, thrt in ot_sys_threat])
        tot_cur_alloc = sum([already_assigned_rating[sid] for sid, thrt in ot_sys_threat])
        # intentionally after tallies, but perhaps should be before
        # this supply threat calc intentionally uses a lower multiplier 0.25
        ot_sys_threat = [(sys_id, sys_threat + enemy_sup_factor[sys_id] * 0.25 * enemy_rating) for sys_id, sys_threat in ot_sys_threat]
        new_alloc = 0
        for sid, thrt in ot_sys_threat:
            cur_alloc = already_assigned_rating[sid]
            needed_rating = rating_needed(1.4*thrt, cur_alloc)
            this_alloc = 0
            # only record more allocation for this invasion if we already started or have enough rating available
            take_any = already_assigned_rating[sid] > 0
            if needed_rating > 0 and (remaining_mil_rating > needed_rating or take_any):
                this_alloc = max(0, min(needed_rating, remaining_mil_rating))
                new_alloc += this_alloc
                max_alloc = 1.5 * this_alloc
                allocations.append((sid, this_alloc, take_any, max_alloc))
                allocation_groups.setdefault('otherTargets', []).append((sid, this_alloc, take_any, max_alloc))
                remaining_mil_rating -= this_alloc
                ot_sys_alloc += this_alloc
            if "Main" in thisround or this_alloc > 0:
                if _verbose_mil_reporting:
                    print "Blockade Targeted system %4d ( %10s ) has local threat %8d ; existing military allocation %d and new allocation %8d" % (sid, universe.getSystem(sid).name, thrt, cur_alloc, this_alloc)
        if "Main" in thisround or new_alloc > 0:
            if _verbose_mil_reporting:
                print "-----------------"
                print "Blockade Targeted Systems under total threat: %d -- total mil allocation-- existing: %d ; new: %d" % (totot_sys_threat, tot_cur_alloc, ot_sys_alloc)
                print "-----------------"
        for sys_id, _ in ot_sys_threat:
            BlockadeAllocator(sys_id, allocation_helper).allocate()

    current_mil_systems = [sid for sid, alloc, take_any, _ in allocations]
    interior_targets1 = targetable_ids.difference(current_mil_systems)
    interior_targets = [sid for sid in interior_targets1 if (
        threat_bias + systems_status.get(sid, {}).get('totalThreat', 0) > 0.8 * already_assigned_rating[sid])]
    if "Main" in thisround:
        if _verbose_mil_reporting:
            print
            print "Other Empire-Proximal Systems : %s" % (["| %d %s |" % (sys_id, universe.getSystem(sys_id).name) for sys_id in interior_targets1])
            print "-----------------"
    # for these, calc local threat only, no neighbor threat, but use a multiplier for fleet safety
    new_alloc = 0
    if interior_targets:
        ot_sys_alloc = 0
        ot_sys_threat = [(o_s_id, threat_bias + safety_factor*(systems_status.get(o_s_id, {}).get('totalThreat', 0))) for o_s_id in interior_targets]
        totot_sys_threat = sum([thrt for sid, thrt in ot_sys_threat])
        tot_cur_alloc = sum([already_assigned_rating[sid] for sid, thrt in ot_sys_threat])
        # do not add enemy supply threat here
        for sid, thrt in ot_sys_threat:
            cur_alloc = already_assigned_rating[sid]
            needed_rating = rating_needed(1.5 * thrt, cur_alloc)
            this_alloc = 0
            # only record more allocation for this invasion if we already started or have enough rating available
            take_any = already_assigned_rating[sid] > 0
            if needed_rating > 0 and (remaining_mil_rating > needed_rating or take_any):
                this_alloc = max(0, min(needed_rating, remaining_mil_rating))
                new_alloc += this_alloc
                max_alloc = 3 * this_alloc
                allocations.append((sid, this_alloc, take_any, max_alloc))
                allocation_groups.setdefault('otherTargets', []).append((sid, this_alloc, take_any, max_alloc))
                remaining_mil_rating -= this_alloc
                ot_sys_alloc += this_alloc
            if "Main" in thisround or this_alloc > 0:
                if _verbose_mil_reporting:
                    print "Other interior system %4d ( %10s ) has local threat %8d ; existing military allocation %d and new allocation %8d" % (sid, universe.getSystem(sid).name, thrt, cur_alloc, this_alloc)
        if "Main" in thisround or new_alloc > 0:
            if _verbose_mil_reporting:
                print "-----------------"
                print "Other Interior Systems under total threat: %d -- total mil allocation-- existing: %d ; new: %d" % (totot_sys_threat, tot_cur_alloc, ot_sys_alloc)
                print "-----------------"
        for sys_id, _ in ot_sys_threat:
            InteriorTargetsAllocator(sys_id, allocation_helper).allocate()
    elif "Main" in thisround:
        if _verbose_mil_reporting:
            print "-----------------"
            print "No Other Interior Systems with fleet threat "
            print "-----------------"

    # explo_target_ids, _ = ExplorationAI.get_current_exploration_info(verbose=False)
    explo_target_ids = []
    if "Main" in thisround:
        if _verbose_mil_reporting:
            print
            print "Exploration-targeted Systems: %s" % (["| %d %s |" % (sys_id, universe.getSystem(sys_id).name) for sys_id in explo_target_ids])
            print "-----------------"
    # for these, calc fleet threat only, no neighbor threat, but use a multiplier for fleet safety
    new_alloc = 0
    if explo_target_ids:
        ot_sys_alloc = 0
        ot_sys_threat = [(o_s_id, safety_factor*combine_ratings(systems_status.get(o_s_id, {}).get('fleetThreat', 0), systems_status.get(o_s_id, {}).get('monsterThreat', 0) + systems_status.get(o_s_id, {}).get('planetThreat', 0))) for o_s_id in explo_target_ids]
        totot_sys_threat = sum([thrt for sid, thrt in ot_sys_threat])
        tot_cur_alloc = sum([0.8*already_assigned_rating[sid] for sid, thrt in ot_sys_threat])
        # intentionally after tallies, but perhaps should be before
        # this supply threat calc intentionally uses a lower multiplier 0.25
        ot_sys_threat = [(sys_id, sys_threat + enemy_sup_factor[sys_id] * 0.25 * enemy_rating) for sys_id, sys_threat in ot_sys_threat]
        for sid, thrt in ot_sys_threat:
            cur_alloc = already_assigned_rating[sid]
            needed_rating = rating_needed(1.4 * thrt, cur_alloc)
            this_alloc = 0
            # only record more allocation for this invasion if we already started or have enough rating available
            take_any = False
            if needed_rating > 0 and (remaining_mil_rating > needed_rating or take_any):
                this_alloc = max(0, min(needed_rating, remaining_mil_rating))
                new_alloc += this_alloc
                max_alloc = 2 * this_alloc
                allocations.append((sid, this_alloc, take_any, max_alloc))
                allocation_groups.setdefault('exploreTargets', []).append((sid, this_alloc, take_any, max_alloc))
                remaining_mil_rating -= this_alloc
                ot_sys_alloc += this_alloc
            if "Main" in thisround or this_alloc > 0:
                if _verbose_mil_reporting:
                    print "Exploration-targeted %4d ( %10s ) has local threat %8d ; existing military allocation %d and new allocation %8d" % (sid, universe.getSystem(sid).name, thrt, cur_alloc, this_alloc)
        for sys_id, _ in ot_sys_threat:
            ExplorationTargetAllocator(sys_id, allocation_helper).allocate()
        if "Main" in thisround or new_alloc > 0:
            if _verbose_mil_reporting:
                print "-----------------"
                print "Exploration-targeted s under total threat: %d -- total mil allocation-- existing: %d ; new: %d" % (totot_sys_threat, tot_cur_alloc, ot_sys_alloc)
                print "-----------------"

    visible_system_ids = foAI.foAIstate.visInteriorSystemIDs.keys() + foAI.foAIstate. visBorderSystemIDs.keys()
    accessible_system_ids = [sys_id for sys_id in visible_system_ids if universe.systemsConnected(sys_id, home_system_id, empire_id)]
    current_mil_systems = [sid for sid, alloc, take_any, _ in allocations if alloc > 0]
    border_targets1 = [sid for sid in accessible_system_ids if sid not in current_mil_systems]
    border_targets = [sid for sid in border_targets1 if (threat_bias + systems_status.get(sid, {}).get('fleetThreat', 0) + systems_status.get(sid, {}).get('planetThreat', 0) > 0.8*already_assigned_rating[sid])]
    if "Main" in thisround:
        if _verbose_mil_reporting:
            print
            print "Empire-Accessible Systems not yet allocated military: %s" % (["| %d %s |" % (sys_id, universe.getSystem(sys_id) and universe.getSystem(sys_id).name) for sys_id in border_targets1])
            print "-----------------"
    # for these, calc fleet threat only, no neighbor threat, but use a multiplier for fleet safety
    new_alloc = 0
    if border_targets:
        ot_sys_alloc = 0
        ot_sys_threat = [(o_s_id, threat_bias + safety_factor*combine_ratings(systems_status.get(o_s_id, {}).get('fleetThreat', 0), systems_status.get(o_s_id, {}).get('monsterThreat', 0) + systems_status.get(o_s_id, {}).get('planetThreat', 0))) for o_s_id in border_targets]
        totot_sys_threat = sum([thrt for sid, thrt in ot_sys_threat])
        tot_cur_alloc = sum([0.8*already_assigned_rating[sid] for sid, thrt in ot_sys_threat])
        # do not add enemy supply threat here
        for sid, thrt in ot_sys_threat:
            cur_alloc = already_assigned_rating[sid]
            needed_rating = rating_needed(1.2 * thrt, cur_alloc)
            this_alloc = 0
            # only record more allocation for this invasion if we already started or have enough rating available
            take_any = False
            if needed_rating > 0 and (remaining_mil_rating > needed_rating or take_any):
                this_alloc = max(0, min(needed_rating, remaining_mil_rating))
                new_alloc += this_alloc
                max_alloc = safety_factor*2*max(systems_status.get(sid, {}).get('totalThreat', 0), systems_status.get(sid, {}).get('neighborThreat', 0))
                allocations.append((sid, this_alloc, take_any, max_alloc))
                allocation_groups.setdefault('accessibleTargets', []).append((sid, this_alloc, take_any, max_alloc))
                remaining_mil_rating -= this_alloc
                ot_sys_alloc += this_alloc
            if "Main" in thisround or this_alloc > 0:
                if _verbose_mil_reporting:
                    print "Other Empire-Accessible system %4d ( %10s ) has local biased threat %8d ; existing military allocation %d and new allocation %8d" % (sid, universe.getSystem(sid).name, thrt, cur_alloc, this_alloc)
        if "Main" in thisround or new_alloc > 0:
            if _verbose_mil_reporting:
                print "-----------------"
                print "Other Empire-Accessible Systems under total biased threat: %d -- total mil allocation-- existing: %d ; new: %d" % (totot_sys_threat, tot_cur_alloc, ot_sys_alloc)
                print "-----------------"
        for sys_id, _ in ot_sys_threat:
            BorderSecurityAllocator(sys_id, allocation_helper).allocate()
    elif "Main" in thisround:
        if _verbose_mil_reporting:
            print "-----------------"
            print "No Other Empire-Accessible Systems with biased local threat "
            print "-----------------"

    if allocation_groups != allocation_helper._allocation_by_groups:
        print_error("Refactored allocation code yields differing results!")
        chat_human("New allocs: %s" % allocation_helper._allocation_by_groups)
        chat_human("Old allocs: %s" % allocation_groups)

    new_allocations = []
    remaining_mil_rating = avail_mil_rating
    # for top categories assign max_alloc right away as available
    for cat in ['capitol', 'occupied', 'topTargets']:
        for sid, alloc, take_any, max_alloc in allocation_groups.get(cat, []):
            if remaining_mil_rating <= 0:
                break
            this_alloc = min(remaining_mil_rating, max_alloc)
            new_allocations.append((sid, this_alloc, alloc, take_any))
            remaining_mil_rating -= this_alloc

    base_allocs = set()
    # for lower priority categories, first assign base_alloc around to all, then top up as available
    for cat in ['otherTargets', 'accessibleTargets', 'exploreTargets']:
        for sid, alloc, take_any, max_alloc in allocation_groups.get(cat, []):
            if remaining_mil_rating <= 0:
                break
            base_allocs.add(sid)
            remaining_mil_rating -= alloc
    for cat in ['otherTargets', 'accessibleTargets', 'exploreTargets']:
        for sid, alloc, take_any, max_alloc in allocation_groups.get(cat, []):
            if sid not in base_allocs:
                break
            if remaining_mil_rating <= 0:
                new_allocations.append((sid, alloc, alloc, take_any))
            else:
                new_rating = min(remaining_mil_rating + alloc, max_alloc)
                new_allocations.append((sid, new_rating, alloc, take_any))
                remaining_mil_rating -= (new_rating - alloc)

    if "Main" in thisround:
        _military_allocations = new_allocations
    _min_mil_allocations.clear()
    _min_mil_allocations.update([(sid, alloc) for sid, alloc, take_any, _ in allocations])
    if _verbose_mil_reporting or "Main" in thisround:
        print "------------------------------\nFinal %s Round Military Allocations: %s \n-----------------------" % (thisround, dict([(sid, alloc) for sid, alloc, minalloc, take_any in new_allocations]))
        print "(Apparently) remaining military rating: %.1f" % remaining_mil_rating

    # export military systems for other AI modules
    if "Main" in thisround:
        AIstate.militarySystemIDs = list(set([sid for sid, alloc, minalloc, take_any in new_allocations]).union([sid for sid in already_assigned_rating if already_assigned_rating[sid] > 0]))
    else:
        AIstate.militarySystemIDs = list(set([sid for sid, alloc, minalloc, take_any in new_allocations]).union(AIstate.militarySystemIDs))
    return new_allocations


def assign_military_fleets_to_systems(use_fleet_id_list=None, allocations=None, round=1):
    # assign military fleets to military theater systems
    global _military_allocations
    universe = fo.getUniverse()
    if allocations is None:
        allocations = []

    doing_main = (use_fleet_id_list is None)
    if doing_main:
        foAI.foAIstate.misc['ReassignedFleetMissions'] = []
        base_defense_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.ORBITAL_DEFENSE)
        unassigned_base_defense_ids = FleetUtilsAI.extract_fleet_ids_without_mission_types(base_defense_ids)
        for fleet_id in unassigned_base_defense_ids:
            fleet = universe.getFleet(fleet_id)
            if not fleet:
                continue
            sys_id = fleet.systemID
            target = universe_object.System(sys_id)
            fleet_mission = foAI.foAIstate.get_fleet_mission(fleet_id)
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
        print "=================================================="
        print "Assigning military fleets"
        print "---------------------------------"
    else:
        avail_mil_fleet_ids = list(use_fleet_id_list)
        mil_needing_repair_ids, avail_mil_fleet_ids = avail_mil_needing_repair(avail_mil_fleet_ids)
        these_allocations = allocations

    # send_for_repair(mil_needing_repair_ids) #currently, let get taken care of by AIFleetMission.generate_fleet_orders()
    
    # get systems to defend

    avail_mil_fleet_ids = set(avail_mil_fleet_ids)
    for sys_id, alloc, minalloc, takeAny in these_allocations:
        if not doing_main and not avail_mil_fleet_ids:
            break
        found_fleets = []
        found_stats = {}
        these_fleets = FleetUtilsAI.get_fleets_for_mission({'rating': alloc}, {'rating': minalloc}, found_stats,
                                                           starting_system=sys_id, fleet_pool_set=avail_mil_fleet_ids,
                                                           fleet_list=found_fleets)
        if not these_fleets:
            if not found_fleets or not (FleetUtilsAI.stats_meet_reqs(found_stats, {'rating': minalloc}) or takeAny):
                if doing_main:
                    if _verbose_mil_reporting:
                        print "NO available/suitable military allocation for system %d ( %s ) -- requested allocation %8d, found available rating %8d in fleets %s" % (sys_id, universe.getSystem(sys_id).name, minalloc, found_stats.get('rating', 0), found_fleets)
                avail_mil_fleet_ids.update(found_fleets)
                continue
            else:
                these_fleets = found_fleets
                rating = CombatRatingsAI.combine_ratings_list(map(CombatRatingsAI.get_fleet_rating, found_fleets))
                if doing_main and _verbose_mil_reporting:
                    if rating < _min_mil_allocations.get(sys_id, 0):
                        print "PARTIAL military allocation for system %d ( %s ) -- requested allocation %8d -- got %8d with fleets %s" % (sys_id, universe.getSystem(sys_id).name, minalloc, rating, these_fleets)
                    else:
                        print "FULL MIN military allocation for system %d ( %s ) -- requested allocation %8d -- got %8d with fleets %s" % (sys_id, universe.getSystem(sys_id).name, _min_mil_allocations.get(sys_id, 0), rating, these_fleets)
        elif doing_main and _verbose_mil_reporting:
            print "FULL+ military allocation for system %d ( %s ) -- requested allocation %8d, got %8d with fleets %s" % (sys_id, universe.getSystem(sys_id).name, alloc, found_stats.get('rating', 0), these_fleets)
        target = universe_object.System(sys_id)
        for fleet_id in these_fleets:
            fo.issueAggressionOrder(fleet_id, True)
            fleet_mission = foAI.foAIstate.get_fleet_mission(fleet_id)
            fleet_mission.clear_fleet_orders()
            fleet_mission.clear_target()
            if sys_id in list(set(AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs + AIstate.invasionTargetedSystemIDs + AIstate.blockadeTargetedSystemIDs)):
                mission_type = MissionType.SECURE
            else:
                mission_type = MissionType.MILITARY
            fleet_mission.set_target(mission_type, target)
            fleet_mission.generate_fleet_orders()
            if not doing_main:
                foAI.foAIstate.misc.setdefault('ReassignedFleetMissions', []).append(fleet_mission)

    if doing_main:
        print "---------------------------------"
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
            print "Still have available military fleets: %s" % avail_mil_fleet_ids
            allocations = get_military_fleets(mil_fleets_ids=avail_mil_fleet_ids, try_reset=False, thisround=thisround)
        if allocations:
            assign_military_fleets_to_systems(use_fleet_id_list=avail_mil_fleet_ids, allocations=allocations, round=round)

