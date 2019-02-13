import copy
from collections import Counter, OrderedDict as odict
from logging import error, info, warn, debug
from operator import itemgetter
from time import time

import freeOrionAIInterface as fo  # pylint: disable=import-error
from common.print_utils import Table, Text, Float

import AIFleetMission
import ColonisationAI
import ExplorationAI
import FleetUtilsAI
from EnumsAI import MissionType, ShipRoleType
import CombatRatingsAI
import MilitaryAI
import PlanetUtilsAI
from freeorion_tools import get_partial_visibility_turn
from AIDependencies import INVALID_ID, TECH_NATIVE_SPECIALS
from character.character_module import create_character, Aggression

# moving ALL or NEARLY ALL 'global' variables into AIState object rather than module
# in general, leaving items as a module attribute if they are recalculated each turn without reference to prior values
# global variables
colonyTargetedSystemIDs = []
outpostTargetedSystemIDs = []
opponentPlanetIDs = []
invasionTargets = []
invasionTargetedSystemIDs = []
fleetsLostBySystem = {}  # keys are system_ids, values are ratings for the fleets lost
empireStars = {}


class ConversionError(Exception):
    """Exception to be raised if the conversion of a savegame state fails.

    Automatically logs and chats to the host if raised.
    """

    def __init__(self, msg=""):
        error(msg, exc_info=True)


def convert_to_version(state, version):
    """Convert a savegame AIstate to the next version.

    :param dict state: savegame state, modified in function
    :param int version: Version to convert to
    """
    debug("Trying to convert savegame state to version %d..." % version)
    current_version = state.get("version", -1)
    debug("  Current version: %d" % current_version)
    if current_version == version:
        raise ConversionError("Can't convert AI savegame to the same compatibility version.")

    if current_version > version:
        raise ConversionError("Can't convert AI savegame to an older compatibility version.")

    if version != current_version + 1:
        raise ConversionError("Can't skip a compatibility version when converting AI savegame.")

    # Starting with version 3, we switched from pickle to json-style encoding
    # Do not try to load an older savegame even if it magically passed the encoder.
    if version <= 3:
        raise ConversionError("The AI savegame version is no longer supported.")

    if version == 4:
        del state['qualifyingOutpostBaseTargets']
        del state['qualifyingColonyBaseTargets']
        state['orbital_colonization_manager'] = ColonisationAI.OrbitalColonizationManager()

    if version == 5:
        state['last_turn_played'] = 0

    #   state["some_new_member"] = some_default_value
    #   del state["some_removed_member"]
    #   state["list_changed_to_set"] = set(state["list_changed_to_set"])

    debug("  All updates set. Setting new version number.")
    state["version"] = version


class AIstate(object):
    """Stores AI game state.

    IMPORTANT:
    (i) If class members are redefined, added or deleted, then the
    version number must be increased by 1 and the convert_to_version()
    function must be updated so a saved state from the previous version
    is playable with this AIstate version, i.e. new members must be added
    and outdated members must be modified and / or deleted.

    (ii) The AIstate is stored as an encoded string in save game files
    (currently via the pickle module). The attributes of the AIstate must
    therefore be compatible with the encoding method, which currently generally
    means that they must be native python data types (or other data types the
    encoder is augmented to handle), not objects such as UniverseObject
    instances or C++ enum values brought over from the C++ side
    via boost. If desiring to store a reference to a UniverseObject store its
    object id instead; for enum values store their int conversion value.
    """
    version = 5

    def __init__(self, aggression):
        # Do not allow to create AIstate instances with an invalid version number.
        if not hasattr(AIstate, 'version'):
            raise ConversionError("AIstate must have an integer version attribute for savegame compatibility")
        if not isinstance(AIstate.version, int):
            raise ConversionError("Version attribute of AIstate must be an integer!")
        if AIstate.version < 0:
            raise ConversionError("AIstate savegame compatibility version must be a positive integer!")

        # need to store the version explicitly as the class variable "version" is only stored in the
        # self.__class__.__dict__ while we only pickle the object (i.e. self.__dict__ )
        self.version = AIstate.version

        # Debug info
        # unique id for game
        self.uid = self.generate_uid(first=True)
        # unique ids for turns.  {turn: uid}
        self.turn_uids = {}

        # see AIstate docstring re importance of int cast for aggression
        self._aggression = int(aggression)

        # 'global' (?) variables
        self.colonisablePlanetIDs = odict()
        self.colonisableOutpostIDs = odict()  #
        self.__aiMissionsByFleetID = {}
        self.__shipRoleByDesignID = {}
        self.__fleetRoleByID = {}
        self.diplomatic_logs = {}
        self.__priorityByType = {}

        # initialize home system knowledge
        universe = fo.getUniverse()
        empire = fo.getEmpire()
        self.empireID = empire.empireID
        homeworld = universe.getPlanet(empire.capitalID)
        self.__origin_home_system_id = homeworld.systemID if homeworld else INVALID_ID
        self.visBorderSystemIDs = {self.__origin_home_system_id}
        self.visInteriorSystemIDs = set()
        self.exploredSystemIDs = set()
        self.unexploredSystemIDs = {self.__origin_home_system_id}
        self.fleetStatus = {}  # keys: 'sysID', 'nships', 'rating'
        # systemStatus keys:
        # 'name', 'neighbors' (sysIDs), '2jump_ring' (sysIDs), '3jump_ring', '4jump_ring', 'enemy_ship_count',
        # 'fleetThreat', 'planetThreat', 'monsterThreat' (specifically, immobile nonplanet threat), 'totalThreat',
        # 'localEnemyFleetIDs', 'neighborThreat', 'max_neighbor_threat', 'jump2_threat' (up to 2 jumps away),
        # 'jump3_threat', 'jump4_threat', 'regional_threat', 'myDefenses' (planet rating), 'myfleets',
        # 'myFleetsAccessible'(not just next desitination), 'myFleetRating', 'my_neighbor_rating' (up to 1 jump away),
        # 'my_jump2_rating', 'my_jump3_rating', my_jump4_rating', 'local_fleet_threats',
        # 'regional_fleet_threats' <== these are only for mobile fleet threats
        self.systemStatus = {}
        self.needsEmergencyExploration = []
        self.newlySplitFleets = {}
        self.militaryRating = 0
        self.shipCount = 4
        self.misc = {}  # Keys: "enemies_sighted" (dict[turn: list[fleetIDs]]),
        #                       "observed_empires" (set[enemy empire IDs]),
        #                       "ReassignedFleetMissions" (list[FleetMissions])
        self.orbital_colonization_manager = ColonisationAI.OrbitalColonizationManager()
        self.qualifyingTroopBaseTargets = {}
        # TODO: track on a per-empire basis
        self.__empire_standard_enemy = CombatRatingsAI.default_ship_stats().get_stats(hashable=True)
        self.empire_standard_enemy_rating = 0  # TODO: track on a per-empire basis
        self.character = create_character(aggression, self.empireID)
        self.last_turn_played = 0

    def __setstate__(self, state):
        try:
            for v in range(state.get("version", -1), AIstate.version):
                convert_to_version(state, v+1)
        except ConversionError:
            if '_aggression' in state:
                aggression = state['_aggression']
            else:
                try:
                    aggression = state['character'].get_trait(Aggression).key
                except Exception:
                    error("Could not find the aggression level of the AI, defaulting to typical.", exc_info=True)
                    aggression = fo.aggression.typical
            self.__init__(aggression)
            return

        # build the ordered dict with sorted entries from the (unsorted) dict
        # that is contained in the savegame state.
        for content in ("colonisablePlanetIDs", "colonisableOutpostIDs"):
            sorted_planets = sorted(state[content].items(),
                                    key=itemgetter(1), reverse=True)
            state[content] = odict(sorted_planets)

        self.__dict__ = state

    def generate_uid(self, first=False):
        """
        Generates unique identifier.
        It is hexed number of milliseconds.
        To set self.uid use flag first=True result will be
        number of mils between current time and some recent date
        For turn result is mils between uid and current time
        """
        time_delta = (time() - 1433809768) * 1000
        if not first:
            time_delta - int(self.uid, 16)
        res = hex(int(time_delta))[2:].strip('L')
        return res

    def set_turn_uid(self):
        """
        Set turn uid. Should be called once per generateOrders.
        When game loaded same turn can be evaluated once again. We force change id for it.
        """
        uid = self.generate_uid()
        self.turn_uids[fo.currentTurn()] = uid
        return uid

    def get_current_turn_uid(self):
        """
        Return uid of current turn.
        """
        return self.turn_uids.setdefault(fo.currentTurn(), self.generate_uid())

    def get_prev_turn_uid(self):
        """
        Return uid of previous turn.
        If called during the first turn after loading a saved game that had an AI version not yet using uids
        will return default value.
        """
        return self.turn_uids.get(fo.currentTurn() - 1, '0')

    def __refresh(self):
        """Turn start AIstate cleanup/refresh."""
        fleetsLostBySystem.clear()
        invasionTargets[:] = []

    def __border_exploration_update(self):
        universe = fo.getUniverse()
        exploration_center = PlanetUtilsAI.get_capital_sys_id()
        # a bad state probably from an old savegame, or else empire has lost (or almost has)
        if exploration_center == INVALID_ID:
            exploration_center = self.__origin_home_system_id
        ExplorationAI.graph_flags.clear()
        if fo.currentTurn() < 50:
            debug("-------------------------------------------------")
            debug("Border Exploration Update (relative to %s)" % universe.getSystem(exploration_center))
            debug("-------------------------------------------------")
        if self.visBorderSystemIDs == {INVALID_ID}:
            self.visBorderSystemIDs.clear()
            self.visBorderSystemIDs.add(exploration_center)
        for sys_id in list(self.visBorderSystemIDs):  # This set is modified during iteration.
            if fo.currentTurn() < 50:
                debug("Considering border system %s" % universe.getSystem(sys_id))
            ExplorationAI.follow_vis_system_connections(sys_id, exploration_center)
        newly_explored = ExplorationAI.update_explored_systems()
        nametags = []
        for sys_id in newly_explored:
            newsys = universe.getSystem(sys_id)
            # an explored system *should* always be able to be gotten
            nametags.append("ID:%4d -- %-20s" % (sys_id, (newsys and newsys.name) or "name unknown"))
        if newly_explored:
            debug("-------------------------------------------------")
            debug("Newly explored systems:\n%s" % "\n".join(nametags))
            debug("-------------------------------------------------")

    def delete_fleet_info(self, fleet_id):
        if fleet_id in self.__aiMissionsByFleetID:
            del self.__aiMissionsByFleetID[fleet_id]
        if fleet_id in self.fleetStatus:
            del self.fleetStatus[fleet_id]
        if fleet_id in self.__fleetRoleByID:
            del self.__fleetRoleByID[fleet_id]
        for sys_status in self.systemStatus.values():
            for fleet_list in [sys_status.get('myfleets', []), sys_status.get('myFleetsAccessible', [])]:
                if fleet_id in fleet_list:
                    fleet_list.remove(fleet_id)

    def __report_system_threats(self):
        """Print a table with system threats to the logfile."""
        current_turn = fo.currentTurn()
        if current_turn >= 100:
            return
        threat_table = Table([
            Text('System'), Text('Vis.'), Float('Total'), Float('by Monsters'), Float('by Fleets'),
            Float('by Planets'), Float('1 jump away'), Float('2 jumps'), Float('3 jumps')],
            table_name="System Threat Turn %d" % current_turn
        )
        universe = fo.getUniverse()
        for sys_id in universe.systemIDs:
            sys_status = self.systemStatus.get(sys_id, {})
            system = universe.getSystem(sys_id)
            threat_table.add_row([
                system,
                "Yes" if sys_status.get('currently_visible', False) else "No",
                sys_status.get('totalThreat', 0),
                sys_status.get('monsterThreat', 0),
                sys_status.get('fleetThreat', 0),
                sys_status.get('planetThreat', 0),
                sys_status.get('neighborThreat', 0.0),
                sys_status.get('jump2_threat', 0.0),
                sys_status.get('jump3_threat', 0.0),
            ])
        info(threat_table)

    def __report_system_defenses(self):
        """Print a table with system defenses to the logfile."""
        current_turn = fo.currentTurn()
        if current_turn >= 100:
            return
        defense_table = Table([
            Text('System Defenses'), Float('Total'), Float('by Planets'), Float('by Fleets'),
            Float('Fleets 1 jump away'), Float('2 jumps'), Float('3 jumps')],
                table_name="System Defenses Turn %d" % current_turn
        )
        universe = fo.getUniverse()
        for sys_id in universe.systemIDs:
            sys_status = self.systemStatus.get(sys_id, {})
            system = universe.getSystem(sys_id)
            defense_table.add_row([
                system,
                sys_status.get('all_local_defenses', 0.0),
                sys_status.get('mydefenses', {}).get('overall', 0.0),
                sys_status.get('myFleetRating', 0.0),
                sys_status.get('my_neighbor_rating', 0.0),
                sys_status.get('my_jump2_rating', 0.0),
                sys_status.get('my_jump3_rating', 0.0),
            ])
        info(defense_table)

    def assess_planet_threat(self, pid, sighting_age=0):
        if sighting_age > 5:
            sighting_age += 1  # play it safe
        universe = fo.getUniverse()
        planet = universe.getPlanet(pid)
        if not planet:
            return {'overall': 0, 'attack': 0, 'health': 0}
        init_shields = planet.initialMeterValue(fo.meterType.shield)
        next_shields = planet.currentMeterValue(fo.meterType.shield)  # always assumes regen will occur
        max_shields = planet.currentMeterValue(fo.meterType.maxShield)
        init_defense = planet.initialMeterValue(fo.meterType.defense)
        next_defense = planet.currentMeterValue(fo.meterType.defense)  # always assumes regen will occur
        max_defense = planet.currentMeterValue(fo.meterType.maxDefense)
        for special, bonuses in TECH_NATIVE_SPECIALS.items():
            if special in planet.specials and sighting_age > 0:
                shield_bonus = bonuses.get('shields', 0)
                defense_bonus = bonuses.get('defense', 0)
                max_shields = max(max_shields, shield_bonus)
                max_defense = max(max_defense, defense_bonus)
                next_shields, init_shields = max(next_shields, shield_bonus), max(init_shields, shield_bonus)
                next_defense, init_defense = max(next_defense, defense_bonus), max(init_defense, defense_bonus)
        # TODO: get regens from knowledge of possessed tech
        # note the max below is because sometimes the next value will be less than init
        # (e.g. shields just after invasion)
        shield_regen = max(1, next_shields - init_shields)
        defense_regen = max(1, next_defense - init_defense)
        shields = min(max_shields, init_shields + sighting_age * shield_regen)
        defense = min(max_defense, init_defense + sighting_age * defense_regen)
        return {'overall': defense * (defense + shields), 'attack': defense, 'health': (defense + shields)}

    def assess_enemy_supply(self):
        """
        Assesses where enemy empires have Supply
        :return: a tuple of 2 dicts, each of which is keyed by system id, and each of which is a list of empire ids
        1st dict -- enemies that actually have supply at this system
        2nd dict -- enemies that have supply within 2 jumps from this system (if they clear obstructions)
        :rtype: (dict[int, list[int]], dict[int, list[int]])
        """
        enemy_ids = [_id for _id in fo.allEmpireIDs() if _id != fo.empireID()]
        actual_supply = {}
        near_supply = {}
        for enemy_id in enemy_ids:
            this_enemy = fo.getEmpire(enemy_id)
            if not this_enemy:
                debug("Could not retrieve empire for empire id %d" % enemy_id)  # do not spam chat_error with this
                continue
            for sys_id in this_enemy.fleetSupplyableSystemIDs:
                actual_supply.setdefault(sys_id, []).append(enemy_id)
            for sys_id, supply_val in this_enemy.supplyProjections().items():
                if supply_val >= -2:
                    near_supply.setdefault(sys_id, []).append(enemy_id)
        return actual_supply, near_supply

    def __update_empire_standard_enemy(self):
        """Update the empire's standard enemy.

        The standard enemy is the enemy that is most often seen.
        """
        # TODO: If no current information available, rate against own fighters
        universe = fo.getUniverse()
        empire_id = fo.empireID()

        # assess enemy fleets that may have been momentarily visible (start with dummy entries)
        dummy_stats = CombatRatingsAI.default_ship_stats().get_stats(hashable=True)
        cur_e_fighters = Counter()  # actual visible enemies
        old_e_fighters = Counter({dummy_stats: 0})  # destroyed enemies TODO: consider seen but out of sight enemies

        for fleet_id in universe.fleetIDs:
            fleet = universe.getFleet(fleet_id)
            if (not fleet or fleet.empty or fleet.ownedBy(empire_id) or fleet.unowned or
                    not (fleet.hasArmedShips or fleet.hasFighterShips)):
                continue

            # track old/dead enemy fighters for rating assessments in case not enough current info
            ship_stats = CombatRatingsAI.FleetCombatStats(fleet_id).get_ship_stats(hashable=True)
            dead_fleet = fleet_id in universe.destroyedObjectIDs(empire_id)
            e_f_dict = old_e_fighters if dead_fleet else cur_e_fighters
            for stats in ship_stats:
                # log only ships that are armed
                if stats[0]:
                    e_f_dict[stats] += 1

        e_f_dict = cur_e_fighters or old_e_fighters
        self.__empire_standard_enemy = sorted([(v, k) for k, v in e_f_dict.items()])[-1][1]
        self.empire_standard_enemy_rating = self.get_standard_enemy().get_rating()

    def __update_system_status(self):
        debug('{0} Updating System Threats {0}'.format(10 * "="))
        universe = fo.getUniverse()
        empire = fo.getEmpire()
        empire_id = fo.empireID()
        destroyed_object_ids = universe.destroyedObjectIDs(empire_id)
        supply_unobstructed_systems = set(empire.supplyUnobstructedSystems)
        min_hidden_attack = 4
        min_hidden_health = 8
        observed_empires = self.misc.setdefault("observed_empires", set())

        # TODO: Variables that are recalculated each turn from scratch should not be stored in AIstate
        # clear previous game state
        for sys_id in self.systemStatus:
            self.systemStatus[sys_id]['enemy_ship_count'] = 0
            self.systemStatus[sys_id]['myFleetRating'] = 0
            self.systemStatus[sys_id]['myFleetRatingVsPlanets'] = 0

        # for use in debugging
        verbose = False

        # assess enemy fleets that may have been momentarily visible
        enemies_by_system = {}
        my_fleets_by_system = {}
        fleet_spot_position = {}
        current_turn = fo.currentTurn()
        for fleet_id in universe.fleetIDs:
            fleet = universe.getFleet(fleet_id)
            if not fleet or fleet.empty:
                self.delete_fleet_info(fleet_id)  # this is safe even if fleet wasn't mine
                continue
            # TODO: check if currently in system and blockaded before accepting destination as location
            this_system_id = fleet.nextSystemID if fleet.nextSystemID != INVALID_ID else fleet.systemID
            dead_fleet = fleet_id in destroyed_object_ids
            if dead_fleet:
                self.delete_fleet_info(fleet_id)

            if fleet.ownedBy(empire_id):
                if not dead_fleet:
                    my_fleets_by_system.setdefault(this_system_id, []).append(fleet_id)
                    fleet_spot_position.setdefault(fleet.systemID, []).append(fleet_id)
                continue

            # TODO: consider checking death of individual ships.  If ships had been moved from this fleet
            # into another fleet, we might have witnessed their death in that other fleet but if this fleet
            # had not been seen since before that transfer then the ships might also still be listed here.
            if dead_fleet:
                continue

            # we are only interested in immediately recent data
            if get_partial_visibility_turn(fleet_id) < (current_turn - 1):
                continue

            sys_status = self.systemStatus.setdefault(this_system_id, {})
            sys_status['enemy_ship_count'] = sys_status.get('enemy_ship_count', 0) + len(fleet.shipIDs)
            enemies_by_system.setdefault(this_system_id, []).append(fleet_id)

            if not fleet.unowned:
                self.misc.setdefault('enemies_sighted', {}).setdefault(current_turn, []).append(fleet_id)
                observed_empires.add(fleet.owner)

        # assess fleet and planet threats & my local fleets
        for sys_id in universe.systemIDs:
            sys_status = self.systemStatus.setdefault(sys_id, {})
            system = universe.getSystem(sys_id)
            if verbose:
                debug("AIState threat evaluation for %s" % system)
            # update fleets
            sys_status['myfleets'] = my_fleets_by_system.get(sys_id, [])
            sys_status['myFleetsAccessible'] = fleet_spot_position.get(sys_id, [])
            local_enemy_fleet_ids = enemies_by_system.get(sys_id, [])
            sys_status['localEnemyFleetIDs'] = local_enemy_fleet_ids
            if system:
                sys_status['name'] = system.name

            # update my fleet rating versus planets so that planet ratings can be more accurate
            my_ratings_against_planets_list = []
            for fid in sys_status['myfleets']:
                my_ratings_against_planets_list.append(self.get_rating(fid, against_planets=True))
                sys_status['myFleetRatingVsPlanets'] = CombatRatingsAI.combine_ratings_list(
                    my_ratings_against_planets_list)

            # update threats
            monster_ratings = []  # immobile
            enemy_ratings = []  # owned & mobile
            mob_ratings = []  # mobile & unowned
            mobile_fleets = []  # mobile and either owned or unowned
            for fid in local_enemy_fleet_ids:
                fleet = universe.getFleet(fid)  # ensured to exist
                fleet_rating = CombatRatingsAI.get_fleet_rating(
                    fid, enemy_stats=CombatRatingsAI.get_empire_standard_fighter())
                if fleet.speed == 0:
                    monster_ratings.append(fleet_rating)
                    if verbose:
                        debug("\t immobile enemy fleet %s has rating %.1f" % (fleet, fleet_rating))
                    continue

                if verbose:
                    debug("\t mobile enemy fleet %s has rating %.1f" % (fleet, fleet_rating))
                mobile_fleets.append(fid)
                if fleet.unowned:
                    mob_ratings.append(fleet_rating)
                else:
                    enemy_ratings.append(fleet_rating)

            enemy_rating = CombatRatingsAI.combine_ratings_list(enemy_ratings)
            monster_rating = CombatRatingsAI.combine_ratings_list(monster_ratings)
            mob_rating = CombatRatingsAI.combine_ratings_list(mob_ratings)
            lost_fleets = fleetsLostBySystem.get(sys_id, [])
            lost_fleet_rating = CombatRatingsAI.combine_ratings_list(lost_fleets)
            if lost_fleet_rating:
                debug("Just lost fleet rating %.1f in system %s", lost_fleet_rating, system)

            # under current visibility rules should not be possible to have any losses or other info here,
            # but just in case...
            partial_vis_turn = get_partial_visibility_turn(sys_id)
            if not system or partial_vis_turn < 0:
                if verbose:
                    debug("Never had partial vis for %s - basing threat assessment on old info and lost ships" % system)
                sys_status.setdefault('local_fleet_threats', set())
                sys_status['planetThreat'] = 0
                sys_status['fleetThreat'] = max(
                    CombatRatingsAI.combine_ratings(enemy_rating, mob_rating),
                    0.98 * sys_status.get('fleetThreat', 0),
                    1.1*lost_fleet_rating - monster_rating)
                sys_status['monsterThreat'] = max(
                    monster_rating,
                    0.98 * sys_status.get('monsterThreat', 0),
                    1.1*lost_fleet_rating - enemy_rating - mob_rating)
                sys_status['enemy_threat'] = max(
                    enemy_rating,
                    0.98 * sys_status.get('enemy_threat', 0),
                    1.1*lost_fleet_rating - monster_rating - mob_rating)
                sys_status['mydefenses'] = {'overall': 0, 'attack': 0, 'health': 0}
                sys_status['totalThreat'] = sys_status['fleetThreat']
                sys_status['regional_fleet_threats'] = sys_status['local_fleet_threats'].copy()
                continue

            # have either stale or current info
            pattack = phealth = 0
            mypattack = myphealth = 0
            for pid in system.planetIDs:
                planet = universe.getPlanet(pid)
                if not planet:
                    continue
                sighting_age = current_turn - get_partial_visibility_turn(pid)
                prating = self.assess_planet_threat(pid, sighting_age)
                if planet.ownedBy(empire_id):  # TODO: check for diplomatic status
                    mypattack += prating['attack']
                    myphealth += prating['health']
                else:
                    pattack += prating['attack']
                    phealth += prating['health']
                    if any("_NEST_" in special for special in planet.specials):
                        sys_status['nest_threat'] = 100
            sys_status['planetThreat'] = pattack * phealth
            sys_status['mydefenses'] = {'overall': mypattack * myphealth, 'attack': mypattack, 'health': myphealth}

            # previous threat assessment could account for losses, ignore the losses now
            if (lost_fleet_rating and
                    lost_fleet_rating < max(sys_status.get('totalThreat', 0), pattack * phealth)):
                debug("In system %s: Ignoring lost fleets since known threats could cause it.", system)
                lost_fleet_rating = 0

            # TODO use sitrep combat info rather than estimating stealthed enemies by fleets lost to them
            # TODO also only consider past stealthed fleet threat to still be present if the system is still obstructed
            # TODO: track visibility across turns in order to distinguish the blip of visibility in (losing) combat,
            #       which FO currently treats as being for the previous turn,
            #       partially superseding the previous visibility for that turn

            if not partial_vis_turn == current_turn:
                sys_status.setdefault('local_fleet_threats', set())
                sys_status['currently_visible'] = False
                # print ("Stale visibility for system %d ( %s ) -- last seen %d, "
                #        "current Turn %d -- basing threat assessment on old info and lost ships") % (
                #     sys_id, sys_status.get('name', "name unknown"), partial_vis_turn, currentTurn)
                sys_status['fleetThreat'] = max(
                    CombatRatingsAI.combine_ratings(enemy_rating, mob_rating),
                    0.98 * sys_status.get('fleetThreat', 0),
                    2.0 * lost_fleet_rating - max(sys_status.get('monsterThreat', 0), monster_rating))
                sys_status['enemy_threat'] = max(
                    enemy_rating,
                    0.98 * sys_status.get('enemy_threat', 0),
                    1.1*lost_fleet_rating - max(sys_status.get('monsterThreat', 0), monster_rating))
                sys_status['monsterThreat'] = max(monster_rating, 0.98 * sys_status.get('monsterThreat', 0))
                # sys_status['totalThreat'] = ((pattack + enemy_attack + monster_attack) ** 0.8)\
                #                             * ((phealth + enemy_health + monster_health)** 0.6)  # reevaluate this
                sys_status['totalThreat'] = max(
                    CombatRatingsAI.combine_ratings_list([enemy_rating, mob_rating, monster_rating, pattack * phealth]),
                    2 * lost_fleet_rating,
                    0.98 * sys_status.get('totalThreat', 0))
            else:  # system considered visible
                sys_status['currently_visible'] = True
                sys_status['local_fleet_threats'] = set(mobile_fleets)
                # includes mobile monsters
                sys_status['fleetThreat'] = max(
                    CombatRatingsAI.combine_ratings(enemy_rating, mob_rating), 2*lost_fleet_rating - monster_rating)
                if verbose:
                    debug("enemy threat calc parts: enemy rating %.1f, lost fleet rating %.1f, monster_rating %.1f" % (
                        enemy_rating, lost_fleet_rating, monster_rating))
                # does NOT include mobile monsters
                sys_status['enemy_threat'] = max(enemy_rating, 2*lost_fleet_rating - monster_rating)
                sys_status['monsterThreat'] = monster_rating
                sys_status['totalThreat'] = CombatRatingsAI.combine_ratings_list([
                    sys_status['fleetThreat'],
                    sys_status['monsterThreat'],
                    pattack * phealth,
                ])
            sys_status['regional_fleet_threats'] = sys_status['local_fleet_threats'].copy()
            sys_status['fleetThreat'] = max(sys_status['fleetThreat'], sys_status.get('nest_threat', 0))
            sys_status['totalThreat'] = max(sys_status['totalThreat'], sys_status.get('nest_threat', 0))

            # has been seen with Partial Vis, but is currently supply-blocked
            if partial_vis_turn > 0 and sys_id not in supply_unobstructed_systems:
                sys_status['fleetThreat'] = max(sys_status['fleetThreat'], min_hidden_attack * min_hidden_health)
                sys_status['totalThreat'] = max(sys_status['totalThreat'],
                                                CombatRatingsAI.combine_ratings(sys_status.get('planetThreat', 0),
                                                                                (min_hidden_attack*min_hidden_health)))
            if verbose and sys_status['fleetThreat'] > 0:
                debug("%s intermediate status: %s" % (system, sys_status))

        enemy_supply, enemy_near_supply = self.assess_enemy_supply()  # TODO: assess change in enemy supply over time
        # assess secondary threats (threats of surrounding systems) and update my fleet rating
        for sys_id in universe.systemIDs:
            sys_status = self.systemStatus[sys_id]
            sys_status['enemies_supplied'] = enemy_supply.get(sys_id, [])
            observed_empires.update(enemy_supply.get(sys_id, []))
            sys_status['enemies_nearly_supplied'] = enemy_near_supply.get(sys_id, [])
            my_ratings_list = []
            my_ratings_against_planets_list = []
            for fid in sys_status['myfleets']:
                this_rating = self.get_rating(fid, True, self.get_standard_enemy())
                my_ratings_list.append(this_rating)
                my_ratings_against_planets_list.append(self.get_rating(fid, against_planets=True))
            if sys_id != INVALID_ID:
                sys_status['myFleetRating'] = CombatRatingsAI.combine_ratings_list(my_ratings_list)
                sys_status['myFleetRatingVsPlanets'] = CombatRatingsAI.combine_ratings_list(
                    my_ratings_against_planets_list)
                sys_status['all_local_defenses'] = CombatRatingsAI.combine_ratings(
                    sys_status['myFleetRating'], sys_status['mydefenses']['overall'])
            sys_status['neighbors'] = set(universe.getImmediateNeighbors(sys_id, self.empireID))

        for sys_id in universe.systemIDs:
            sys_status = self.systemStatus[sys_id]
            neighbors = sys_status.get('neighbors', set())
            this_system = universe.getSystem(sys_id)
            if verbose:
                debug("Regional Assessment for %s with local fleet threat %.1f" % (
                    this_system, sys_status.get('fleetThreat', 0)))
            jumps2 = set()
            jumps3 = set()
            jumps4 = set()
            for seta, setb in [(neighbors, jumps2), (jumps2, jumps3), (jumps3, jumps4)]:
                for sys2id in seta:
                    setb.update(self.systemStatus.get(sys2id, {}).get('neighbors', set()))
            jump2ring = jumps2 - neighbors - {sys_id}
            jump3ring = jumps3 - jumps2 - neighbors - {sys_id}
            jump4ring = jumps4 - jumps3 - jumps2 - neighbors - {sys_id}
            sys_status['2jump_ring'] = jump2ring
            sys_status['3jump_ring'] = jump3ring
            sys_status['4jump_ring'] = jump4ring
            threat, max_threat, myrating, j1_threats = self.area_ratings(neighbors)
            sys_status['neighborThreat'] = threat
            sys_status['max_neighbor_threat'] = max_threat
            sys_status['my_neighbor_rating'] = myrating
            threat, max_threat, myrating, j2_threats = self.area_ratings(jump2ring)
            sys_status['jump2_threat'] = threat
            sys_status['my_jump2_rating'] = myrating
            threat, max_threat, myrating, j3_threats = self.area_ratings(jump3ring)
            sys_status['jump3_threat'] = threat
            sys_status['my_jump3_rating'] = myrating
            # for local system includes both enemies and mobs
            threat_keys = ['fleetThreat', 'neighborThreat', 'jump2_threat']
            sys_status['regional_threat'] = CombatRatingsAI.combine_ratings_list(
                [sys_status.get(x, 0) for x in threat_keys])
            # TODO: investigate cases where regional_threat has been nonzero but no regional_threat_fleets
            # (probably due to attenuating history of past threats)
            sys_status.setdefault('regional_fleet_threats', set()).update(j1_threats, j2_threats)

    def area_ratings(self, system_ids):
        """Returns (fleet_threat, max_threat, myFleetRating, threat_fleets) compiled over a group of systems."""
        myrating = threat = max_threat = 0
        threat_fleets = set()
        for sys_id in system_ids:
            sys_status = self.systemStatus.get(sys_id, {})
            # TODO: have distinct treatment for both enemy_threat and fleetThreat, respectively
            fthreat = sys_status.get('enemy_threat', 0)
            max_threat = max(max_threat, fthreat)
            threat = CombatRatingsAI.combine_ratings(threat, fthreat)
            myrating = CombatRatingsAI.combine_ratings(myrating, sys_status.get('myFleetRating', 0))
            # myrating = FleetUtilsAI.combine_ratings(myrating, sys_status.get('all_local_defenses', 0))
            threat_fleets.update(sys_status.get('local_fleet_threats', []))
        return threat, max_threat, myrating, threat_fleets

    def get_fleet_mission(self, fleet_id):
        """
        Returns AIFleetMission with fleetID.
        :rtype: AIFleetMission.AIFleetMission
        """
        if fleet_id in self.__aiMissionsByFleetID:
            return self.__aiMissionsByFleetID[fleet_id]
        else:
            return None

    def get_all_fleet_missions(self):
        """Returns all AIFleetMissions."""
        return self.__aiMissionsByFleetID.values()

    def get_fleet_missions_map(self):
        return self.__aiMissionsByFleetID

    def get_fleet_missions_with_any_mission_types(self, mission_types):
        """Returns all AIFleetMissions which contains any of fleetMissionTypes."""
        result = []
        for mission in self.get_all_fleet_missions():
            if mission.type in mission_types:
                result.append(mission)
        return result

    def __add_fleet_mission(self, fleet_id):
        """Add a new dummy AIFleetMission for the passed fleet_id if it has no mission yet."""
        if self.get_fleet_mission(fleet_id) is not None:
            warn("Tried to add a new fleet mission for fleet that already had a mission.")
            return
        self.__aiMissionsByFleetID[fleet_id] = AIFleetMission.AIFleetMission(fleet_id)

    def __remove_fleet_mission(self, fleet_id):
        """Remove invalid AIFleetMission with fleetID if it exists."""
        if self.get_fleet_mission(fleet_id) is not None:
            self.__aiMissionsByFleetID[fleet_id] = None
            del self.__aiMissionsByFleetID[fleet_id]

    def ensure_have_fleet_missions(self, fleet_ids):
        for fleet_id in fleet_ids:
            if self.get_fleet_mission(fleet_id) is None:
                self.__add_fleet_mission(fleet_id)

    def __clean_fleet_missions(self):
        """Assign a new dummy mission to new fleets and clean up existing, now invalid missions."""
        current_empire_fleets = FleetUtilsAI.get_empire_fleet_ids()

        # assign a new (dummy) mission to new fleets
        for fleet_id in current_empire_fleets:
            if self.get_fleet_mission(fleet_id) is None:
                self.__add_fleet_mission(fleet_id)

        # Check all fleet missions for validity and clear invalid targets.
        # If a fleet does not exist anymore, mark mission for deletion.
        # Deleting only after the loop allows us to avoid an expensive copy.
        deleted_fleet_ids = []
        for mission in self.get_all_fleet_missions():
            if mission.fleet.id not in current_empire_fleets:
                deleted_fleet_ids.append(mission.fleet.id)
            else:
                mission.clean_invalid_targets()
        for deleted_fleet_id in deleted_fleet_ids:
            self.__remove_fleet_mission(deleted_fleet_id)

    def has_target(self, mission_type, target):
        for mission in self.get_fleet_missions_with_any_mission_types([mission_type]):
            if mission.has_target(mission_type, target):
                return True
        return False

    def get_rating(self, fleet_id, force_new=False, enemy_stats=None, against_planets=False):
        """Returns a dict with various rating info."""
        if fleet_id in self.fleetStatus and not force_new and enemy_stats is None:
            return self.fleetStatus[fleet_id].get('rating', 0)
        else:
            fleet = fo.getUniverse().getFleet(fleet_id)
            if not fleet:
                return {}  # TODO: also ensure any info for that fleet is deleted
            status = {'rating': CombatRatingsAI.get_fleet_rating(fleet_id, enemy_stats),
                      'ratingVsPlanets': CombatRatingsAI.get_fleet_rating_against_planets(fleet_id),
                      'sysID': fleet.systemID, 'nships': len(fleet.shipIDs)}
            self.fleetStatus[fleet_id] = status
            return status['rating'] if not against_planets else status['ratingVsPlanets']

    def update_fleet_rating(self, fleet_id):
        self.get_rating(fleet_id, force_new=True)

    def get_ship_role(self, ship_design_id):
        """Returns ship role for given designID, assesses and adds as needed."""

        # if thought was invalid, recheck to be sure
        if (ship_design_id in self.__shipRoleByDesignID and
                self.__shipRoleByDesignID[ship_design_id] != ShipRoleType.INVALID):
            return self.__shipRoleByDesignID[ship_design_id]
        else:
            role = FleetUtilsAI.assess_ship_design_role(fo.getShipDesign(ship_design_id))
            self.__shipRoleByDesignID[ship_design_id] = role
            return role

    def get_fleet_roles_map(self):
        return self.__fleetRoleByID

    def get_fleet_role(self, fleet_id, force_new=False):
        """Returns fleet role by ID."""

        if not force_new and fleet_id in self.__fleetRoleByID:
            return self.__fleetRoleByID[fleet_id]
        else:
            role = FleetUtilsAI.assess_fleet_role(fleet_id)
            self.__fleetRoleByID[fleet_id] = role
            make_aggressive = False
            if role in [MissionType.COLONISATION,
                        MissionType.OUTPOST,
                        MissionType.ORBITAL_INVASION,
                        MissionType.ORBITAL_OUTPOST
                        ]:
                pass
            elif role in [MissionType.EXPLORATION,
                          MissionType.INVASION
                          ]:
                this_rating = self.get_rating(fleet_id)  # Done!
                n_ships = self.fleetStatus.get(fleet_id, {}).get('nships', 1)  # entry sould exist due to above line
                if float(this_rating) / n_ships >= 0.5 * MilitaryAI.cur_best_mil_ship_rating():
                    make_aggressive = True
            else:
                make_aggressive = True
            fo.issueAggressionOrder(fleet_id, make_aggressive)
            return role

    def session_start_cleanup(self):
        self.newlySplitFleets = {}
        for fleetID in FleetUtilsAI.get_empire_fleet_ids():
            self.get_fleet_role(fleetID)
            self.update_fleet_rating(fleetID)
            self.ensure_have_fleet_missions([fleetID])
        self.__clean_fleet_roles(just_resumed=True)
        fleetsLostBySystem.clear()
        empireStars.clear()
        self.qualifyingTroopBaseTargets.clear()

    def __clean_fleet_roles(self, just_resumed=False):
        """Removes fleetRoles if a fleet has been lost, and update fleet Ratings."""
        universe = fo.getUniverse()
        current_empire_fleets = FleetUtilsAI.get_empire_fleet_ids()
        self.shipCount = 0

        fleet_table = Table([
            Text('Fleet'), Float('Rating'), Float('Troops'),
            Text('Location'), Text('Destination')],
            table_name="Fleet Summary Turn %d" % fo.currentTurn()
        )
        # need to loop over a copy as entries are deleted in loop
        for fleet_id in list(self.__fleetRoleByID):
            fleet_status = self.fleetStatus.setdefault(fleet_id, {})
            rating = CombatRatingsAI.get_fleet_rating(fleet_id, self.get_standard_enemy())
            old_sys_id = fleet_status.get('sysID', -2)  # TODO: Introduce helper function instead
            fleet = universe.getFleet(fleet_id)
            if fleet:
                sys_id = fleet.systemID
                if old_sys_id in [-2, -1]:
                    old_sys_id = sys_id
                fleet_status['nships'] = len(fleet.shipIDs)  # TODO: Introduce helper function instead
                self.shipCount += fleet_status['nships']
            else:
                # can still retrieve a fleet object even if fleet was just destroyed, so shouldn't get here
                # however,this has been observed happening, and is the reason a fleet check was added a few lines below.
                # Not at all sure how this came about, but was throwing off threat assessments
                sys_id = old_sys_id

            # check if fleet is destroyed and if so, delete stored information
            if fleet_id not in current_empire_fleets:  # or fleet.empty:
                debug("Just lost %s", fleet)
                if not just_resumed:
                    fleetsLostBySystem.setdefault(old_sys_id, []).append(
                        max(rating, fleet_status.get('rating', 0.), MilitaryAI.MinThreat))

                self.delete_fleet_info(fleet_id)
                continue

            # if reached here, the fleet does still exist
            this_sys = universe.getSystem(sys_id)
            next_sys = universe.getSystem(fleet.nextSystemID)

            fleet_table.add_row([
                    fleet,
                    rating,
                    FleetUtilsAI.count_troops_in_fleet(fleet_id),
                    this_sys or 'starlane',
                    next_sys or '-',
                ])

            fleet_status['rating'] = rating
            if next_sys:
                fleet_status['sysID'] = next_sys.id
            elif this_sys:
                fleet_status['sysID'] = this_sys.id
            else:
                error("Fleet %s has no valid system." % fleet)
        info(fleet_table)
        # Next string used in charts. Don't modify it!
        debug("Empire Ship Count: %s" % self.shipCount)
        debug("Empire standard fighter summary: %s", (CombatRatingsAI.get_empire_standard_fighter().get_stats(), ))
        debug("------------------------")

    def get_explored_system_ids(self):
        return list(self.exploredSystemIDs)

    def get_unexplored_system_ids(self):
        return list(self.unexploredSystemIDs)

    def set_priority(self, priority_type, value):
        """Sets a priority of the specified type."""
        self.__priorityByType[priority_type] = value

    def get_priority(self, priority_type):
        """Returns the priority value of the specified type."""

        if priority_type in self.__priorityByType:
            return copy.deepcopy(self.__priorityByType[priority_type])
        return 0

    def __report_last_turn_fleet_missions(self):
        """Print a table reviewing last turn fleet missions to the log file."""
        universe = fo.getUniverse()
        mission_table = Table(
                [Text('Fleet'), Text('Mission'), Text('Ships'), Float('Rating'), Float('Troops'), Text('Target')],
                table_name="Turn %d: Fleet Mission Review from Last Turn" % fo.currentTurn())
        for fleet_id, mission in self.get_fleet_missions_map().items():
            fleet = universe.getFleet(fleet_id)
            if not fleet:
                continue
            if not mission:
                mission_table.add_row([fleet])
            else:
                mission_table.add_row([
                    fleet,
                    mission.type or "None",
                    len(fleet.shipIDs),
                    CombatRatingsAI.get_fleet_rating(fleet_id),
                    FleetUtilsAI.count_troops_in_fleet(fleet_id),
                    mission.target or "-"
                ])
        info(mission_table)

    def __split_new_fleets(self):
        """Split any new fleets.

        This function is supposed to be called once at the beginning of the turn.
        Splitting the auto generated fleets at game start or those created by
        recently built ships allows the AI to assign correct roles to all ships.
        """
        # TODO: check length of fleets for losses or do in AIstate.__cleanRoles
        universe = fo.getUniverse()
        known_fleets = self.get_fleet_roles_map()
        self.newlySplitFleets.clear()

        fleets_to_split = [fleet_id for fleet_id in FleetUtilsAI.get_empire_fleet_ids() if fleet_id not in known_fleets]
        if fleets_to_split:
            debug("Trying to split %d new fleets" % len(fleets_to_split))
        for fleet_id in fleets_to_split:
            fleet = universe.getFleet(fleet_id)
            if not fleet:
                warn("Trying to split fleet %d but seemingly does not exist" % fleet_id)
                continue
            fleet_len = len(fleet.shipIDs)
            if fleet_len == 1:
                continue
            new_fleets = FleetUtilsAI.split_fleet(fleet_id)
            debug("Split fleet %d with %d ships into %d new fleets:" % (fleet_id, fleet_len, len(new_fleets)))
            # old fleet may have different role after split, later will be again identified
            # in current system, orig new fleet will not yet have been assigned a role
            # self.remove_fleet_role(fleet_id)

    def __cleanup_qualifiying_base_targets(self):
        """Cleanup invalid entries in qualifying base targets."""
        universe = fo.getUniverse()
        empire_id = fo.empireID()
        for dct in [self.qualifyingTroopBaseTargets]:
            for pid in dct.keys():
                planet = universe.getPlanet(pid)
                if planet and planet.ownedBy(empire_id):
                    del dct[pid]

    def prepare_for_new_turn(self):
        self.__report_last_turn_fleet_missions()
        self.__split_new_fleets()
        self.__refresh()  # TODO: Use turn_state instead
        self.__border_exploration_update()
        self.__cleanup_qualifiying_base_targets()
        self.orbital_colonization_manager.turn_start_cleanup()
        self.__clean_fleet_roles()
        self.__clean_fleet_missions()
        debug("Fleets lost by system: %s" % fleetsLostBySystem)
        self.__update_empire_standard_enemy()
        self.__update_system_status()
        self.__report_system_threats()
        self.__report_system_defenses()
        self.__report_exploration_status()

    def __report_exploration_status(self):
        universe = fo.getUniverse()
        explored_system_ids = self.get_explored_system_ids()
        debug("Unexplored Systems: %s " % map(universe.getSystem, self.get_unexplored_system_ids()))
        debug("Explored SystemIDs: %s" % map(universe.getSystem, explored_system_ids))
        debug("Explored PlanetIDs: %s" % PlanetUtilsAI.get_planets_in__systems_ids(explored_system_ids))

    def log_peace_request(self, initiating_empire_id, recipient_empire_id):
        """Keep a record of peace requests made or received by this empire."""

        peace_requests = self.diplomatic_logs.setdefault('peace_requests', {})
        log_index = (initiating_empire_id, recipient_empire_id)
        peace_requests.setdefault(log_index, []).append(fo.currentTurn())

    def log_war_declaration(self, initiating_empire_id, recipient_empire_id):
        """Keep a record of war declarations made or received by this empire."""

        # if war declaration is made on turn 1, don't hold it against them
        if fo.currentTurn() == 1:
            return
        war_declarations = self.diplomatic_logs.setdefault('war_declarations', {})
        log_index = (initiating_empire_id, recipient_empire_id)
        war_declarations.setdefault(log_index, []).append(fo.currentTurn())

    def get_standard_enemy(self):
        return CombatRatingsAI.ShipCombatStats(stats=self.__empire_standard_enemy)
