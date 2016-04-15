import copy
from collections import OrderedDict as odict
from time import time
import sys
import freeOrionAIInterface as fo  # pylint: disable=import-error

import AIFleetMission
import ExplorationAI
import FleetUtilsAI
import ResourcesAI
from EnumsAI import MissionType, ExplorableSystemType, get_ship_roles_types, ShipRoleType, check_validity
import CombatRatingsAI
import MilitaryAI
import PlanetUtilsAI
from freeorion_tools import dict_from_map
from universe_object import System


# moving ALL or NEARLY ALL 'global' variables into AIState object rather than module
# in general, leaving items as a module attribute if they are recalculated each turn without reference to prior values
# global variables
colonyTargetedSystemIDs = []
outpostTargetedSystemIDs = []
opponentPlanetIDs = []
opponentSystemIDs = []  # TODO: Currently never filled but some (uncommented) code in MilitaryAI refers to this...
invasionTargets = []
invasionTargetedSystemIDs = []
blockadeTargetedSystemIDs = []  # TODO also never filled atm... either implement this or remove redundant code
militarySystemIDs = []
colonyFleetIDs = []
outpostFleetIDs = []
invasionFleetIDs = []
militaryFleetIDs = []
fleetsLostBySystem = {}  # keys are system_ids, values are ratings for the fleets lost
popCtrSystemIDs = []
colonizedSystems = {}
empireStars = {}
popCtrIDs = []
outpostIDs = []
outpostSystemIDs = []


# AIstate class
class AIstate(object):
    """Stores AI game state."""
    def __init__(self, aggression=fo.aggression.typical):
        # Debug info
        # unique id for game
        self.uid = self.generate_uid(first=True)
        # unique ids for turns.  {turn: uid}
        self.turn_uids = {}

        # 'global' (?) variables
        self.colonisablePlanetIDs = odict()
        self.colonisableOutpostIDs = odict()  #
        self.__aiMissionsByFleetID = {}
        self.__shipRoleByDesignID = {}
        self.__fleetRoleByID = {}
        self.design_rating_adjustments = {}
        self.diplomatic_logs = {}
        self.__priorityByType = {}

        # initialize home system knowledge
        universe = fo.getUniverse()
        empire = fo.getEmpire()
        self.empireID = empire.empireID
        self.origHomeworldID = empire.capitalID
        homeworld = universe.getPlanet(self.origHomeworldID)
        self.origSpeciesName = (homeworld and homeworld.speciesName) or ""
        if homeworld:
            self.origHomeSystemID = homeworld.systemID
        else:
            self.origHomeSystemID = -1
        self.visBorderSystemIDs = {self.origHomeSystemID: 1}
        self.visInteriorSystemIDs = {}
        self.expBorderSystemIDs = {self.origHomeSystemID: 1}
        self.expInteriorSystemIDs = {}
        self.exploredSystemIDs = {}
        self.unexploredSystemIDs = {self.origHomeSystemID: 1}
        self.fleetStatus = {}  # keys: 'sysID', 'nships', 'rating'
        # systemStatus keys: 'name', 'neighbors' (sysIDs), '2jump_ring' (sysIDs), '3jump_ring', '4jump_ring'
        # 'fleetThreat', 'planetThreat', 'monsterThreat' (specifically, immobile nonplanet threat), 'totalThreat', 'localEnemyFleetIDs',
        # 'neighborThreat', 'max_neighbor_threat', 'jump2_threat' (up to 2 jumps away), 'jump3_threat', 'jump4_threat', 'regional_threat'
        # 'myDefenses' (planet rating), 'myfleets', 'myFleetsAccessible'(not just next desitination), 'myFleetRating'
        # 'my_neighbor_rating' (up to 1 jump away), 'my_jump2_rating', 'my_jump3_rating', my_jump4_rating'
        # 'local_fleet_threats', 'regional_fleet_threats' <== these are only for mobile fleet threats
        self.systemStatus = {}
        self.planet_status = {}
        self.needsEmergencyExploration = []
        self.newlySplitFleets = {}
        self.aggression = aggression
        self.militaryRating = 0
        self.shipCount = 4
        self.misc = {}
        self.qualifyingColonyBaseTargets = {}
        self.qualifyingOutpostBaseTargets = {}
        self.qualifyingTroopBaseTargets = {}
        self.__empire_standard_enemy = CombatRatingsAI.default_ship_stats().get_stats(hashable=True)  # TODO: track on a per-empire basis
        self.empire_standard_enemy_rating = 0  # TODO: track on a per-empire basis

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

    def refresh(self):
        """Turn start AIstate cleanup/refresh."""
        universe = fo.getUniverse()
        # checks exploration border & clears roles/missions of missing fleets & updates fleet locs & threats
        fleetsLostBySystem.clear()
        invasionTargets[:] = []
        exploration_center = PlanetUtilsAI.get_capital_sys_id()
        if exploration_center == -1:  # a bad state probably from an old savegame, or else empire has lost (or almost has)
            exploration_center = self.origHomeSystemID

        # check if planets in cache is still present. Remove destroyed.
        for system_id, info in sorted(self.systemStatus.items()):
            planet_dict = info.get('planets', {})
            cache_planet_set = set(planet_dict)
            system_planet_set = set(universe.getSystem(system_id).planetIDs)
            diff = cache_planet_set - system_planet_set
            if diff:
                print "Removing destroyed planets from systemStatus for system %s: planets to be removed: %s" % (system_id, sorted(diff))
                for key in diff:
                    del planet_dict[key]

        ExplorationAI.graphFlags.clear()
        if fo.currentTurn() < 50:
            print "-------------------------------------------------"
            print "Border Exploration Update (relative to %s" % (PlanetUtilsAI.sys_name_ids([exploration_center, -1])[0])
            print "-------------------------------------------------"
        if self.visBorderSystemIDs.keys() == [-1]:
            self.visBorderSystemIDs.clear()
            self.visBorderSystemIDs[exploration_center] = 1
        for sys_id in self.visBorderSystemIDs.keys():  # This dict modified during iteration.
            if fo.currentTurn() < 50:
                print "Considering border system %s" % (PlanetUtilsAI.sys_name_ids([sys_id, -1])[0])
            ExplorationAI.follow_vis_system_connections(sys_id, exploration_center)
        newly_explored = ExplorationAI.update_explored_systems()
        nametags = []
        for sys_id in newly_explored:
            newsys = universe.getSystem(sys_id)
            nametags.append("ID:%4d -- %-20s" % (sys_id, (newsys and newsys.name) or "name unknown"))  # an explored system *should* always be able to be gotten
        if newly_explored:
            print "-------------------------------------------------"
            print "Newly explored systems:\n"+"\n".join(nametags)
            print "-------------------------------------------------"
        # cleanup fleet roles
        # self.update_fleet_locs()
        self.__clean_fleet_roles()
        self.__clean_fleet_missions(FleetUtilsAI.get_empire_fleet_ids())
        print "Fleets lost by system: %s" % fleetsLostBySystem
        self.update_system_status()

    def delete_fleet_info(self, fleet_id):
        if fleet_id in self.__aiMissionsByFleetID:
            del self.__aiMissionsByFleetID[fleet_id]
        if fleet_id in self.fleetStatus:
            del self.fleetStatus[fleet_id]
        if fleet_id in self.__fleetRoleByID:
            del self.__fleetRoleByID[fleet_id]

    def report_system_threats(self):
        if fo.currentTurn() >= 100:
            return
        universe = fo.getUniverse()
        sys_id_list = sorted(universe.systemIDs)  # will normally look at this, the list of all known systems
        # assess fleet and planet threats
        print "----------------------------------------------"
        print "System Threat Assessments"
        for sys_id in sys_id_list:
            sys_status = self.systemStatus.get(sys_id, {})
            system = universe.getSystem(sys_id)
            print "%-20s: %s" % (system, sys_status)

    def assess_planet_threat(self, pid, sighting_age=0):
        sighting_age += 1  # play it safe
        universe = fo.getUniverse()
        planet = universe.getPlanet(pid)
        if not planet:
            return {'overall': 0, 'attack': 0, 'health': 0}
        current_shields = planet.currentMeterValue(fo.meterType.shield)
        max_shields = planet.currentMeterValue(fo.meterType.maxShield)
        current_defense = planet.currentMeterValue(fo.meterType.defense)
        max_defense = planet.currentMeterValue(fo.meterType.maxDefense)
        shields = min(max_shields, current_shields + 2 * sighting_age)  # TODO: base off regen tech
        defense = min(max_defense, current_defense + 2 * sighting_age)  # TODO: base off regen tech
        return {'overall': defense * (defense + shields), 'attack': defense, 'health': (defense + shields)}

    def assess_enemy_supply(self):
        """
        Assesses where enemy empires have Supply
        :return:a tuple of 2 dicts, each of which is keyed by system id, and each of which is a list of empire ids
        1st dict-- enemies that actually have supply at this system
        2nd dict-- enemies that have supply within 2 jumps from this system (if they clear obstructions)
        """
        enemy_ids = [_id for _id in fo.allEmpireIDs() if _id != fo.empireID()]
        actual_supply = {}
        near_supply = {}
        for enemy_id in enemy_ids:
            this_enemy = fo.getEmpire(enemy_id)
            if not this_enemy:
                print "Could not retrieve empire for empire id %d" % enemy_id  # do not spam chat_error with this
                continue
            for sys_id in this_enemy.fleetSupplyableSystemIDs:
                actual_supply.setdefault(sys_id, []).append(enemy_id)
            for sys_id, supply_val in this_enemy.supplyProjections().items():
                if supply_val >= -2:
                    near_supply.setdefault(sys_id, []).append(enemy_id)
        return actual_supply, near_supply

    def update_system_status(self):
        print"-------\nUpdating System Threats\n---------"
        universe = fo.getUniverse()
        empire = fo.getEmpire()
        empire_id = fo.empireID()
        destroyed_object_ids = universe.destroyedObjectIDs(empire_id)
        supply_unobstructed_systems = set(empire.supplyUnobstructedSystems)
        min_hidden_attack = 4
        min_hidden_health = 8
        system_id_list = universe.systemIDs  # will normally look at this, the list of all known systems

        # for use in debugging
        verbose = False

        # assess enemy fleets that may have been momentarily visible
        cur_e_fighters = {CombatRatingsAI.default_ship_stats().get_stats(hashable=True): [0]}  # start with a dummy entry
        old_e_fighters = {CombatRatingsAI.default_ship_stats().get_stats(hashable=True): [0]}  # start with a dummy entry
        enemy_fleet_ids = []
        enemies_by_system = {}
        my_fleets_by_system = {}
        fleet_spot_position = {}
        saw_enemies_at_system = {}
        my_milship_rating = MilitaryAI.cur_best_mil_ship_rating()
        current_turn = fo.currentTurn()
        for fleet_id in universe.fleetIDs:
            fleet = universe.getFleet(fleet_id)
            if fleet is None:
                continue
            if not fleet.empty:
                # TODO: check if currently in system and blockaded before accepting destination as location
                this_system_id = (fleet.nextSystemID != -1 and fleet.nextSystemID) or fleet.systemID
                if fleet.ownedBy(empire_id):
                    if fleet_id not in destroyed_object_ids:
                        my_fleets_by_system.setdefault(this_system_id, []).append(fleet_id)
                        fleet_spot_position.setdefault(fleet.systemID, []).append(fleet_id)
                else:
                    dead_fleet = fleet_id in destroyed_object_ids
                    if not fleet.ownedBy(-1) and fleet.hasArmedShips:
                        ship_stats = CombatRatingsAI.FleetCombatStats(fleet_id).get_ship_stats(hashable=True)
                        e_f_dict = [cur_e_fighters, old_e_fighters][dead_fleet]  # track old/dead enemy fighters for rating assessments in case not enough current info
                        for stats in ship_stats:
                            attacks = stats[0]
                            if attacks:
                                e_f_dict.setdefault(stats, [0])[0] += 1
                    partial_vis_turn = universe.getVisibilityTurnsMap(fleet_id, empire_id).get(fo.visibility.partial, -9999)
                    if partial_vis_turn >= current_turn - 1:  # only interested in immediately recent data
                        if not dead_fleet:
                            saw_enemies_at_system[fleet.systemID] = True
                            enemy_fleet_ids.append(fleet_id)
                            enemies_by_system.setdefault(this_system_id, []).append(fleet_id)
                            if not fleet.ownedBy(-1):
                                self.misc.setdefault('enemies_sighted', {}).setdefault(current_turn, []).append(fleet_id)
                                rating = CombatRatingsAI.get_fleet_rating(fleet_id, enemy_stats=CombatRatingsAI.get_empire_standard_fighter())
                                if rating > 0.25 * my_milship_rating:
                                    self.misc.setdefault('dangerous_enemies_sighted', {}).setdefault(current_turn, []).append(fleet_id)
        e_f_dict = [cur_e_fighters, old_e_fighters][len(cur_e_fighters) == 1]
        std_fighter = sorted([(v, k) for k, v in e_f_dict.items()])[-1][1]
        self.__empire_standard_enemy = std_fighter
        self.empire_standard_enemy_rating = self.get_standard_enemy().get_rating()
        # TODO: If no current information available, rate against own fighters

        # assess fleet and planet threats & my local fleets
        for sys_id in system_id_list:
            sys_status = self.systemStatus.setdefault(sys_id, {})
            system = universe.getSystem(sys_id)
            if verbose:
                print "AIState threat evaluation for %s" % system
            # update fleets
            sys_status['myfleets'] = my_fleets_by_system.get(sys_id, [])
            sys_status['myFleetsAccessible'] = fleet_spot_position.get(sys_id, [])
            local_enemy_fleet_ids = enemies_by_system.get(sys_id, [])
            sys_status['localEnemyFleetIDs'] = local_enemy_fleet_ids
            if system:
                sys_status['name'] = system.name
                for fid in system.fleetIDs:
                    if fid in destroyed_object_ids:  # TODO: double check are these checks/deletes necessary?
                        self.delete_fleet_info(fid)  # this is safe even if fleet wasn't mine
                        continue
                    fleet = universe.getFleet(fid)
                    if not fleet or fleet.empty:
                        self.delete_fleet_info(fid)  # this is safe even if fleet wasn't mine
                        continue

            # update threats
            sys_vis_dict = universe.getVisibilityTurnsMap(sys_id, fo.empireID())
            partial_vis_turn = sys_vis_dict.get(fo.visibility.partial, -9999)
            mob_ratings = []  # for mobile unowned monster fleets
            lost_fleet_rating = 0
            enemy_ratings = []
            monster_ratings = []
            mobile_fleets = []
            for fid in local_enemy_fleet_ids:
                fleet = universe.getFleet(fid)
                if not fleet:
                    continue
                fleet_rating = CombatRatingsAI.get_fleet_rating(fid, enemy_stats=CombatRatingsAI.get_empire_standard_fighter())
                if fleet.speed == 0:
                    monster_ratings.append(fleet_rating)
                    if verbose:
                        print "\t immobile enemy fleet %s has rating %.1f" % (fleet, fleet_rating)
                else:
                    if verbose:
                        print "\t mobile enemy fleet %s has rating %.1f" % (fleet, fleet_rating)
                    mobile_fleets.append(fid)
                    if fleet.unowned:
                        mob_ratings.append(fleet_rating)
                    else:
                        enemy_ratings.append(fleet_rating)
            enemy_rating = CombatRatingsAI.combine_ratings_list(enemy_ratings)
            monster_rating = CombatRatingsAI.combine_ratings_list(monster_ratings)
            mob_rating = CombatRatingsAI.combine_ratings_list(mob_ratings)
            if fleetsLostBySystem.get(sys_id, []):
                lost_fleet_rating = CombatRatingsAI.combine_ratings_list(fleetsLostBySystem[sys_id])
            if not system or partial_vis_turn == -9999:  # under current visibility rules should not be possible to have any losses or other info here, but just in case...
                if verbose:
                    print "Have never had partial vis for system %d ( %s ) -- basing threat assessment on old info and lost ships" % (sys_id, sys_status.get('name', "name unknown"))
                sys_status.setdefault('local_fleet_threats', set())
                sys_status['planetThreat'] = 0
                sys_status['fleetThreat'] = int(max(CombatRatingsAI.combine_ratings(enemy_rating, mob_rating), 0.98 * sys_status.get('fleetThreat', 0), 1.1 * lost_fleet_rating - monster_rating))
                sys_status['monsterThreat'] = int(max(monster_rating, 0.98 * sys_status.get('monsterThreat', 0), 1.1 * lost_fleet_rating - enemy_rating - mob_rating))
                sys_status['enemy_threat'] = int(max(enemy_rating, 0.98 * sys_status.get('enemy_threat', 0), 1.1 * lost_fleet_rating - monster_rating - mob_rating))
                sys_status['mydefenses'] = {'overall': 0, 'attack': 0, 'health': 0}
                sys_status['totalThreat'] = sys_status['fleetThreat']
                sys_status['regional_fleet_threats'] = sys_status['local_fleet_threats'].copy()
                continue

            # have either stale or current info
            pattack = 0
            phealth = 0
            mypattack, myphealth = 0, 0
            for pid in system.planetIDs:
                prating = self.assess_planet_threat(pid, sighting_age=current_turn-partial_vis_turn)
                planet = universe.getPlanet(pid)
                if not planet:
                    continue
                if planet.owner == self.empireID:  # TODO: check for diplomatic status
                    mypattack += prating['attack']
                    myphealth += prating['health']
                else:
                    if [special for special in planet.specials if "_NEST_" in special]:
                        sys_status['nest_threat'] = 100
                    pattack += prating['attack']
                    phealth += prating['health']
            sys_status['planetThreat'] = pattack * phealth
            sys_status['mydefenses'] = {'overall': mypattack * myphealth, 'attack': mypattack, 'health': myphealth}

            if max(sys_status.get('totalThreat', 0), pattack*phealth) >= 0.6 * lost_fleet_rating:  # previous threat assessment could account for losses, ignore the losses now
                lost_fleet_rating = 0

            # TODO use sitrep combat info rather than estimating stealthed enemies by fleets lost to them
            # TODO also only consider past stealthed fleet threat to still be present if the system is still obstructed
            # TODO: track visibility across turns in order to distinguish the blip of visibility in (losing) combat,
            # which FO currently treats as being for the previous turn, partially superseding the previous visibility for that turn
            if not partial_vis_turn == current_turn:  # (universe.getVisibility(sys_id, self.empire_id) >= fo.visibility.partial):
                print "System %s currently not visible" % system
                sys_status.setdefault('local_fleet_threats', set())
                # print "Stale visibility for system %d ( %s ) -- last seen %d, current Turn %d -- basing threat assessment on old info and lost ships"%(sys_id, sys_status.get('name', "name unknown"), partial_vis_turn, currentTurn)
                sys_status['fleetThreat'] = int(max(CombatRatingsAI.combine_ratings(enemy_rating, mob_rating), 0.98*sys_status.get('fleetThreat', 0), 2.0 * lost_fleet_rating - max(sys_status.get('monsterThreat', 0), monster_rating)))
                sys_status['enemy_threat'] = int(max(enemy_rating, 0.98*sys_status.get('enemy_threat', 0), 1.1 * lost_fleet_rating - max(sys_status.get('monsterThreat', 0), monster_rating)))
                sys_status['monsterThreat'] = int(max(monster_rating, 0.98*sys_status.get('monsterThreat', 0)))
                # sys_status['totalThreat'] = ((pattack + enemy_attack + monster_attack) ** 0.8) * ((phealth + enemy_health + monster_health)** 0.6)  # reevaluate this
                sys_status['totalThreat'] = max(CombatRatingsAI.combine_ratings_list([enemy_rating, mob_rating, monster_rating, pattack * phealth]), 2*lost_fleet_rating, 0.98*sys_status.get('totalThreat', 0))
            else:  # system considered visible #TODO: reevaluate as visibility rules change
                print "System %s currently visible" % system
                sys_status['local_fleet_threats'] = set(mobile_fleets)
                sys_status['fleetThreat'] = int(max(CombatRatingsAI.combine_ratings(enemy_rating, mob_rating), 2*lost_fleet_rating - monster_rating))  # includes mobile monsters
                if verbose:
                    print "enemy threat calc parts: enemy rating %.1f, lost fleet rating %.1f, monster_rating %.1f" % (enemy_rating, lost_fleet_rating, monster_rating)
                sys_status['enemy_threat'] = int(max(enemy_rating, 2*lost_fleet_rating - monster_rating))  # does NOT include mobile monsters
                sys_status['monsterThreat'] = monster_rating
                sys_status['totalThreat'] = CombatRatingsAI.combine_ratings_list([enemy_rating, mob_rating, monster_rating, pattack * phealth])
            sys_status['regional_fleet_threats'] = sys_status['local_fleet_threats'].copy()
            sys_status['fleetThreat'] = max(sys_status['fleetThreat'], sys_status.get('nest_threat', 0))
            sys_status['totalThreat'] = max(sys_status['totalThreat'], sys_status.get('nest_threat', 0))

            if partial_vis_turn > 0 and sys_id not in supply_unobstructed_systems:  # has been seen with Partial Vis, but is currently supply-blocked
                sys_status['fleetThreat'] = max(sys_status['fleetThreat'], min_hidden_attack * min_hidden_health)
                sys_status['totalThreat'] = max(sys_status['totalThreat'], ((pattack + min_hidden_attack) ** 0.8) * ((phealth + min_hidden_health) ** 0.6))
            if verbose and sys_status['fleetThreat'] > 0:
                print "%s intermediate status: %s" % (system, sys_status)

        enemy_supply, enemy_near_supply = self.assess_enemy_supply()  # TODO: assess change in enemy supply over time
        # assess secondary threats (threats of surrounding systems) and update my fleet rating
        for sys_id in system_id_list:
            sys_status = self.systemStatus[sys_id]
            sys_status['enemies_supplied'] = enemy_supply.get(sys_id, [])
            sys_status['enemies_nearly_supplied'] = enemy_near_supply.get(sys_id, [])
            my_ratings_list = []
            for fid in sys_status['myfleets']:
                this_rating = self.get_rating(fid, True, self.get_standard_enemy())  # DONE!
                my_ratings_list.append(this_rating)
            if sys_id != -1:
                sys_status['myFleetRating'] = my_ratings_list and CombatRatingsAI.combine_ratings_list(my_ratings_list) or 0
                sys_status['all_local_defenses'] = CombatRatingsAI.combine_ratings(sys_status['myFleetRating'], sys_status['mydefenses']['overall'])
            sys_status['neighbors'] = set(dict_from_map(universe.getSystemNeighborsMap(sys_id, self.empireID)))
            
        for sys_id in system_id_list:
            sys_status = self.systemStatus[sys_id]
            neighbors = sys_status.get('neighbors', set())
            this_system = fo.getUniverse().getSystem(sys_id)
            if verbose:
                print "Regional Assessment for %s with local fleet threat %.1f" % (this_system, sys_status.get('fleetThreat', 0))
            jumps2 = set()
            jumps3 = set()
            jumps4 = set()
            for seta, setb in [(neighbors, jumps2), (jumps2, jumps3), (jumps3, jumps4)]:
                for sys2id in seta:
                    setb.update(self.systemStatus.get(sys2id, {}).get('neighbors', set()))
            jump2ring = jumps2 - neighbors - {sys_id}
            jump3ring = jumps3 - jumps2
            jump4ring = jumps4 - jumps3
            sys_status['2jump_ring'] = jump2ring
            sys_status['3jump_ring'] = jump3ring
            sys_status['4jump_ring'] = jump4ring
            threat, max_threat, myrating, j1_threats = self.area_ratings(neighbors,
                         ref_sys_name="neighbors " + str(this_system)) if verbose else self.area_ratings(neighbors)
            sys_status['neighborThreat'] = threat
            sys_status['max_neighbor_threat'] = max_threat
            sys_status['my_neighbor_rating'] = myrating
            threat, max_threat, myrating, j2_threats = self.area_ratings(jump2ring,
                         ref_sys_name="jump2 " + str(this_system)) if verbose else self.area_ratings(jump2ring)
            sys_status['jump2_threat'] = threat
            sys_status['my_jump2_rating'] = myrating
            threat, max_threat, myrating, j3_threats = self.area_ratings(jump3ring)
            sys_status['jump3_threat'] = threat
            sys_status['my_jump3_rating'] = myrating
            threat_keys = ['fleetThreat', 'neighborThreat', 'jump2_threat']  # for local system includes both enemies and mobs
            sys_status['regional_threat'] = CombatRatingsAI.combine_ratings_list(map(lambda x: sys_status.get(x, 0), threat_keys))
            # TODO: investigate cases where regional_threat has been nonzero but no regional_threat_fleets
            # (probably due to attenuating history of past threats)
            sys_status.setdefault('regional_fleet_threats', set()).update(j1_threats, j2_threats)
            # threat, max_threat, myrating, j4_threats = self.area_ratings(jump4ring)
            # sys_status['jump4_threat'] = threat
            # sys_status['my_jump4_rating'] = myrating

    def area_ratings(self, system_ids):
        """Returns (fleet_threat, max_threat, myFleetRating, threat_fleets) compiled over a group of systems."""
        max_threat = 0
        threat = 0
        myrating = 0
        threat_fleets = set()
        threat_detail = []
        for sys_id in system_ids:
            sys_status = self.systemStatus.get(sys_id, {})
            # TODO: have distinct treatment for both enemy_threat and fleetThreat, respectively
            fthreat = sys_status.get('enemy_threat', 0)
            if fthreat > max_threat:
                max_threat = fthreat
            threat = CombatRatingsAI.combine_ratings(threat, fthreat)
            myrating = CombatRatingsAI.combine_ratings(myrating, sys_status.get('myFleetRating', 0))
            # myrating = FleetUtilsAI.combine_ratings(myrating, sys_status.get('all_local_defenses', 0))
            threat_detail.append((sys_id, fthreat, sys_status.get('local_fleet_threats', [])))
            threat_fleets.update(sys_status.get('local_fleet_threats', []))
        return threat, max_threat, myrating, threat_fleets

    def after_turn_cleanup(self):
        """Removes not required information to save from AI state after AI complete its turn."""
        # some ships in fleet can be destroyed between turns and then fleet may have have different roles
        # self.__fleetRoleByID = {}
        pass

    def __has_fleet_mission(self, fleet_id):
        """Returns True if fleetID has AIFleetMission."""
        return fleet_id in self.__aiMissionsByFleetID

    def get_fleet_mission(self, fleet_id):
        """Returns AIFleetMission with fleetID."""
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
        """Add new AIFleetMission with fleetID if it already not exists."""
        if self.get_fleet_mission(fleet_id) is None:
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

    def __clean_fleet_missions(self, fleet_ids):
        """Cleanup of AIFleetMissions."""
        for fleet_id in fleet_ids:
            if self.get_fleet_mission(fleet_id) is None:
                self.__add_fleet_mission(fleet_id)

        deleted_fleet_ids = []
        for mission in self.get_all_fleet_missions():
            if mission.fleet.id not in fleet_ids:
                deleted_fleet_ids.append(mission.fleet.id)
        for deleted_fleet_id in deleted_fleet_ids:
            self.__remove_fleet_mission(deleted_fleet_id)

        for mission in self.get_all_fleet_missions():
            mission.clean_invalid_targets()

    def has_target(self, mission_type, target):
        for mission in self.get_fleet_missions_with_any_mission_types([mission_type]):
            if mission.has_target(mission_type, target):
                return True
        return False

    def get_rating(self, fleet_id, force_new=False, enemy_stats=None):
        """Returns a dict with various rating info."""
        if fleet_id in self.fleetStatus and not force_new and enemy_stats is None:
            return self.fleetStatus[fleet_id].get('rating', {})
        else:
            fleet = fo.getUniverse().getFleet(fleet_id)
            if not fleet:
                return {}  # TODO: also ensure any info for that fleet is deleted
            status = {'rating': CombatRatingsAI.get_fleet_rating(fleet_id, enemy_stats),  # TODO
                      'sysID': fleet.systemID, 'nships': len(fleet.shipIDs)}
            self.fleetStatus[fleet_id] = status
            return status['rating']

    def update_fleet_rating(self, fleet_id):
        self.get_rating(fleet_id, force_new=True)

    def get_ship_role(self, ship_design_id):
        """Returns ship role for given designID, assesses and adds as needed."""

        if ship_design_id in self.__shipRoleByDesignID and self.__shipRoleByDesignID[ship_design_id] != ShipRoleType.INVALID:  # if thought was invalid, recheck to be sure
            return self.__shipRoleByDesignID[ship_design_id]
        else:
            role = FleetUtilsAI.assess_ship_design_role(fo.getShipDesign(ship_design_id))
            self.__shipRoleByDesignID[ship_design_id] = role
            return role

    def add_ship_role(self, ship_design_id, ship_role):
        """Adds a ship designID/role pair."""
        if not (ship_role in get_ship_roles_types()):
            print "Invalid shipRole: " + str(ship_role)
            return
        elif ship_design_id in self.__shipRoleByDesignID:
            return
        else:
            self.__shipRoleByDesignID[ship_design_id] = ship_role

    def remove_ship_role(self, ship_design_id):
        """Removes a ship designID/role pair."""

        if ship_design_id in self.__shipRoleByDesignID:
            print "Removed role for ship design %d named: %s" % (ship_design_id, fo.getShipDesign(ship_design_id).name)
            del self.__shipRoleByDesignID[ship_design_id]
            return
        else:
            print "No existing role found for ship design %d named: %s" % (ship_design_id, fo.getShipDesign(ship_design_id).name)

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

    def add_fleet_role(self, fleet_id, mission_type):
        """Adds a fleet ID/role pair."""

        if not check_validity(mission_type):
            return
        if fleet_id in self.__fleetRoleByID:
            return
        else:
            self.__fleetRoleByID[fleet_id] = mission_type

    def remove_fleet_role(self, fleet_id):
        """Removes a fleet ID/role pair."""
        if fleet_id in self.__fleetRoleByID:
            del self.__fleetRoleByID[fleet_id]
            return

    def session_start_cleanup(self):
        ResourcesAI.newTargets.clear()
        self.newlySplitFleets = {}
        for fleetID in FleetUtilsAI.get_empire_fleet_ids():
            self.get_fleet_role(fleetID)
            self.update_fleet_rating(fleetID)
            self.ensure_have_fleet_missions([fleetID])
        self.__clean_fleet_roles(just_resumed=True)
        fleetsLostBySystem.clear()
        popCtrSystemIDs[:] = []  # resets without detroying existing references
        colonizedSystems.clear()
        empireStars.clear()
        popCtrIDs[:] = []
        outpostIDs[:] = []
        outpostSystemIDs[:] = []
        ResourcesAI.lastFociCheck[0] = 0
        self.qualifyingColonyBaseTargets.clear()
        self.qualifyingOutpostBaseTargets.clear()
        self.qualifyingTroopBaseTargets.clear()
        # self.reset_invasions()
        
    def reset_invasions(self):
        """Useful when testing changes to invasion planning."""
        invasion_missions = self.get_fleet_missions_with_any_mission_types([MissionType.INVASION])
        for mission in invasion_missions:
            mission.clear_fleet_orders()
            mission.clear_target()

    def __clean_fleet_roles(self, just_resumed=False):
        """Removes fleetRoles if a fleet has been lost, and update fleet Ratings."""
        for sys_id in self.systemStatus:
            self.systemStatus[sys_id]['myFleetRating'] = 0

        universe = fo.getUniverse()
        ok_fleets = FleetUtilsAI.get_empire_fleet_ids()
        fleet_list = sorted(list(self.__fleetRoleByID))
        unaccounted_fleets = set(ok_fleets) - set(fleet_list)
        ship_count = 0
        print "----------------------------------------------------------------------------------"
        print "In CleanFleetRoles %s" % fleet_list
        print "-----------"
        print "Empire-owned fleet_list : %s" % ok_fleets
        print "-----------"
        if unaccounted_fleets:
            print "Fleets unaccounted for in Empire Records:", unaccounted_fleets
        print "----------------------------------------------------------------------------------"
        destroyed_object_ids = universe.destroyedObjectIDs(fo.empireID())
        for fleet_id in fleet_list:
            status = self.fleetStatus.setdefault(fleet_id, {})
            old_rating = status.get('rating', 0)
            new_rating = CombatRatingsAI.get_fleet_rating(fleet_id, self.get_standard_enemy())
            old_sys_id = status.get('sysID', -2)
            fleet = universe.getFleet(fleet_id)
            if fleet:
                sys_id = fleet.systemID
                if old_sys_id in [-2, -1]:
                    old_sys_id = sys_id
                status['nships'] = len(fleet.shipIDs)
                ship_count += status['nships']
            else:
                sys_id = old_sys_id  # can still retrieve a fleet object even if fleet was just destroyed, so shouldn't get here
                # however,this has been observed happening, and is the reason a fleet check was added a few lines below.
                # Not at all sure how this came about, but was throwing off threat assessments
            if fleet_id not in ok_fleets:  # or fleet.empty:
                if (fleet and self.__fleetRoleByID.get(fleet_id, -1) != -1 and fleet_id not in destroyed_object_ids and
                                [ship_id for ship_id in fleet.shipIDs if ship_id not in destroyed_object_ids]):
                    if not just_resumed:
                        fleetsLostBySystem.setdefault(old_sys_id, []).append(max(old_rating, MilitaryAI.MinThreat))
                if fleet_id in self.__fleetRoleByID:
                    del self.__fleetRoleByID[fleet_id]
                if fleet_id in self.__aiMissionsByFleetID:
                    del self.__aiMissionsByFleetID[fleet_id]
                if fleet_id in self.fleetStatus:
                    del self.fleetStatus[fleet_id]
                continue
            else:  # fleet in ok fleets
                sys1 = universe.getSystem(sys_id)
                if sys_id == -1:
                    sys1_name = 'starlane'
                else:
                    sys1_name = (sys1 and sys1.name) or "unknown"
                next_sys_id = fleet.nextSystemID
                sys2 = universe.getSystem(next_sys_id)
                if next_sys_id == -1:
                    sys2_name = 'starlane'
                else:
                    sys2_name = (sys2 and sys2.name) or "unknown"
                print "Fleet %d (%s) oldRating: %6d | new_rating %6d | at system %d (%s) | next system %d (%s)" % (fleet_id, fleet.name, old_rating, new_rating,
                                                                                                                   fleet.systemID, sys1_name, fleet.nextSystemID, sys2_name)
                print "Fleet %d (%s) summary: %s" % (fleet_id, fleet.name, CombatRatingsAI.FleetCombatStats(fleet_id).get_ship_stats())
                status['rating'] = new_rating
                if next_sys_id != -1:
                    status['sysID'] = next_sys_id
                elif sys_id != -1:
                    status['sysID'] = sys_id
                else:
                    main_missin = self.get_fleet_mission(fleet_id)
                    main_mission_type = (main_missin.getAIMissionTypes() + [-1])[0]
                    if main_mission_type != -1:
                        targets = main_missin.getAITargets(main_mission_type)
                        if targets:
                            mMT0 = targets[0]
                            if isinstance(mMT0.target_type, System):
                                status['sysID'] = mMT0.target.id  # hmm, but might still be a fair ways from here
        self.shipCount = ship_count
        print "------------------------"
        # Next string used in charts. Don't modify it!
        print "Empire Ship Count: ", ship_count
        print "Empire standard fighter summary: ", CombatRatingsAI.get_empire_standard_fighter().get_stats()
        print "------------------------"

    def get_explorable_systems(self, explorable_systems_type):
        """Get all explorable systems determined by type."""
        # return copy.deepcopy(self.__explorableSystemByType[explorableSystemsType])
        if explorable_systems_type == ExplorableSystemType.EXPLORED:
            return list(self.exploredSystemIDs)
        elif explorable_systems_type == ExplorableSystemType.UNEXPLORED:
            return list(self.unexploredSystemIDs)
        else:
            print "Error -- unexpected explorableSystemsType (value %s ) submited to AIState.get_explorable_systems "
            return {}

    def set_priority(self, priority_type, value):
        """Sets a priority of the specified type."""
        self.__priorityByType[priority_type] = value

    def get_priority(self, priority_type):
        """Returns the priority value of the specified type."""

        if priority_type in self.__priorityByType:
            return copy.deepcopy(self.__priorityByType[priority_type])
        return 0

    def get_all_priorities(self):
        """Returns a dictionary with all priority values."""
        return copy.deepcopy(self.__priorityByType)

    def print_priorities(self):
        """Prints all priorities."""
        print "all priorities:"
        for priority in self.__priorityByType:
            print "    %s: %s" % (priority, self.__priorityByType[priority])
        print

    def split_new_fleets(self):
        """Split any new fleets (at new game creation, can have unplanned mix of ship roles)."""

        print "Review of current Fleet Role/Mission records:"
        print "--------------------"
        print "Map of Roles keyed by Fleet ID: %s" % self.get_fleet_roles_map()
        print "--------------------"
        print "Map of Missions keyed by ID:"
        for item in self.get_fleet_missions_map().items():
            print "%-5d: %s" % item
        print "--------------------"
        # TODO: check length of fleets for losses or do in AIstat.__cleanRoles
        known_fleets = self.get_fleet_roles_map()
        self.newlySplitFleets.clear()

        fleets_to_split = [fleet_id for fleet_id in FleetUtilsAI.get_empire_fleet_ids() if fleet_id not in known_fleets]

        if fleets_to_split:
            universe = fo.getUniverse()
            print "Splitting new fleets"
            for fleet_id in fleets_to_split:
                fleet = universe.getFleet(fleet_id)
                if not fleet:
                    print "Error splitting new fleets; resulting fleet ID %d appears to not exist" % fleet_id
                    continue
                fleet_len = len(list(fleet.shipIDs))
                if fleet_len == 1:
                    continue
                new_fleets = FleetUtilsAI.split_fleet(fleet_id)  # try splitting fleet
                print "\t from splitting fleet ID %4d with %d ships, got %d new fleets:" % (fleet_id, fleet_len, len(new_fleets))
                # old fleet may have different role after split, later will be again identified
                # self.remove_fleet_role(fleet_id)  # in current system, orig new fleet will not yet have been assigned a role

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
