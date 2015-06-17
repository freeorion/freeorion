import copy
from collections import OrderedDict as odict
from time import time
import sys
import freeOrionAIInterface as fo  # pylint: disable=import-error

import AIFleetMission
import EnumsAI
import ExplorationAI
import FleetUtilsAI
import ProductionAI
import ResourcesAI
from EnumsAI import AIFleetMissionType, AIExplorableSystemType, TargetType
import MilitaryAI
import PlanetUtilsAI
from freeorion_tools import dict_from_map, get_ai_tag_grade


# moving ALL or NEARLY ALL 'global' variables into AIState object rather than module
# in general, leaving items as a module attribute if they are recalculated each turn without reference to prior values
# global variables
# foodStockpileSize = 1  # food stored per population
minimalColoniseValue = 3  # minimal value for a planet to be colonised
colonyTargetedSystemIDs = []
outpostTargetedSystemIDs = []
opponentPlanetIDs = []
opponentSystemIDs = []
invasionTargets = []
invasionTargetedSystemIDs = []
blockadeTargetedSystemIDs = []
militarySystemIDs = []
militaryTargetedSystemIDs = []
colonyFleetIDs = []
outpostFleetIDs = []
invasionFleetIDs = []
militaryFleetIDs = []
fleetsLostBySystem = {}
fleetsLostByID = {}
popCtrSystemIDs = []
colonizedSystems = {}
empireStars = {}
popCtrIDs = []
outpostIDs = []
outpostSystemIDs = []
piloting_grades = {}


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
        # self.foodStockpileSize = 1    # food stored per population
        self.minimalColoniseValue = 3  # minimal value for a planet to be colonised
        self.colonisablePlanetIDs = odict()
        self.colonisableOutpostIDs = odict()  #
        self.__aiMissionsByFleetID = {}
        self.__shipRoleByDesignID = {}
        self.__fleetRoleByID = {}
        self.designStats = {}
        self.design_rating_adjustments = {}
        self.diplomatic_logs = {}
        self.__priorityByType = {}

        # self.__explorableSystemByType = {}
        # for explorableSystemsType in EnumsAI.get_explorable_system_types():
        # self.__explorableSystemByType[explorableSystemsType] = {}

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
        self.empire_standard_fighter = (4, ((4, 1),), 0.0, 10.0)
        self.empire_standard_enemy = (4, ((4, 1),), 0.0, 10.0)  # TODO: track on a per-empire basis
        self.empire_standard_enemy_rating = 40  # TODO: track on a per-empire basis

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

    def __setstate__(self, state_dict):
        self.__dict__.update(state_dict)  # update attributes
        for dict_attrib in ['qualifyingColonyBaseTargets',
                            'qualifyingOutpostBaseTargets',
                            'qualifyingTroopBaseTargets',
                            'planet_status',
                            'diplomatic_logs']:
            if dict_attrib not in state_dict:
                self.__dict__[dict_attrib] = {}
        for std_attrib in ['empire_standard_fighter', 'empire_standard_enemy']:
            if std_attrib not in state_dict:
                self.__dict__[std_attrib] = (4, ((4, 1),), 0.0, 10.0)
        for odict_attrib in ['colonisablePlanetIDs', 'colonisableOutpostIDs']:
            if dict_attrib not in state_dict:
                self.__dict__[odict_attrib] = odict()
        if 'uid' not in state_dict:
            self.uid = self.generate_uid(first=True)
        if 'turn_uids' not in state_dict:
            self.turn_uids = {}
        self.__dict__.setdefault('empire_standard_enemy_rating', 40)

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
        fleetsLostByID.clear()
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
            nametags.append("ID:%4d -- %20s" % (sys_id, (newsys and newsys.name) or"name unknown"))  # an explored system *should* always be able to be gotten
        if newly_explored:
            print "-------------------------------------------------"
            print "newly explored systems: \n"+"\n".join(nametags)
            print "-------------------------------------------------"
        # cleanup fleet roles
        # self.update_fleet_locs()
        self.__clean_fleet_roles()
        self.__clean_fleet_missions(FleetUtilsAI.get_empire_fleet_ids())
        print "Fleets lost by system: %s" % fleetsLostBySystem
        self.update_system_status()

    def fleet_sum_tups_to_estat_dicts(self, summary):
        if not summary:
            return None
        # print "converting summary: ", summary
        estats = []
        try:
            for count, ship_sum in summary:
                estats.append((count, {'attacks': dict(ship_sum[1]), 'shields': ship_sum[2], 'structure': ship_sum[3]}))
            return estats
        except Exception as e:
            sys.stderr.write("Error converting fleet summary %s: %s\n" % (summary, e))
            return None
            
    # TODO not used
    def update_fleet_locs(self):
        universe = fo.getUniverse()
        moved_fleets = []
        for fleet_id in self.fleetStatus:
            old_location = self.fleetStatus[fleet_id]['sysID']
            fleet = universe.getFleet(fleet_id)
            if not fleet:
                print "can't retrieve fleet %4d to update location" % fleet_id
                continue  # TODO: update elsewhere?
            new_location = fleet.systemID
            if new_location != old_location:
                moved_fleets.append((fleet_id, old_location, new_location))
                self.fleetStatus[fleet_id]['sysID'] = new_location
                self.fleetStatus[fleet_id]['nships'] = len(fleet.shipIDs)

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
            print "%-20s: %s\n" % (system, sys_status)

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
        return {'overall': defense*(defense + shields), 'attack': defense, 'health': (defense + shields)}

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
            for sys_id, supply_val in this_enemy.supplyProjections(-3, False).items():
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

        # assess enemy fleets that may have been momentarily visible
        cur_e_fighters = {(0, ((0, 0),), 0.0, 5.0): [0]}  # start with a dummy entry
        old_e_fighters = {(0, ((0, 0),), 0.0, 5.0): [0]}  # start with a dummy entry
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
                this_system_id = (fleet.nextSystemID != -1 and fleet.nextSystemID) or fleet.systemID
                if fleet.ownedBy(empire_id):
                    if fleet_id not in destroyed_object_ids:
                        my_fleets_by_system.setdefault(this_system_id, []).append(fleet_id)
                        fleet_spot_position.setdefault(fleet.systemID, []).append(fleet_id)
                else:
                    dead_fleet = fleet_id in destroyed_object_ids
                    if not fleet.ownedBy(-1):
                        e_rating = self.rate_fleet(fleet_id)
                        e_f_dict = [cur_e_fighters, old_e_fighters][dead_fleet]  # track old/dead enemy fighters for rating assessments in case not enough current info
                        for count, sum_stats in e_rating['summary']:
                            if sum_stats[0] > 0:
                                e_f_dict.setdefault(sum_stats, [0])[0] += count
                    partial_vis_turn = universe.getVisibilityTurnsMap(fleet_id, empire_id).get(fo.visibility.partial, -9999)
                    if partial_vis_turn >= current_turn - 1:  # only interested in immediately recent data
                        if not dead_fleet:
                            saw_enemies_at_system[fleet.systemID] = True
                            enemy_fleet_ids.append(fleet_id)
                            enemies_by_system.setdefault(this_system_id, []).append(fleet_id)
                            if not fleet.ownedBy(-1):
                                self.misc.setdefault('enemies_sighted', {}).setdefault(current_turn, []).append(fleet_id)
                                rating = self.rate_fleet(fleet_id, self.fleet_sum_tups_to_estat_dicts([(1, self.empire_standard_fighter)]))
                                if rating.get('overall', 0) > 0.25 * my_milship_rating:
                                    self.misc.setdefault('dangerous_enemies_sighted', {}).setdefault(current_turn, []).append(fleet_id)
        e_f_dict = [cur_e_fighters, old_e_fighters][len(cur_e_fighters) == 1]
        std_fighter = sorted([(v, k) for k, v in e_f_dict.items()])[-1][1]
        self.empire_standard_enemy = std_fighter
        self.empire_standard_enemy_rating = std_fighter[0] * std_fighter[-1]  # using a very simple rating method here

        # assess fleet and planet threats & my local fleets
        for sys_id in system_id_list:
            sys_status = self.systemStatus.setdefault(sys_id, {})
            system = universe.getSystem(sys_id)
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
                oldstyle_rating = self.old_rate_fleet(fid)
                newstyle_rating = self.rate_fleet(fid, self.fleet_sum_tups_to_estat_dicts([(1, self.empire_standard_fighter)]))
                if fleet.speed == 0:
                    monster_ratings.append(newstyle_rating)
                else:
                    mobile_fleets.append(fid)
                    if fleet.unowned:
                        mob_ratings.append(newstyle_rating)
                    else:
                        enemy_ratings.append(newstyle_rating)
                if oldstyle_rating.get('overall', 0) != newstyle_rating.get('overall', 0):
                    loc = "at %s" % system
                    fleetname = fleet.name
                    print "AiState.update_system_status for fleet %s id (%d)%s got different newstyle rating (%s) and oldstyle rating (%s)" % (fleetname, fid, loc, newstyle_rating, oldstyle_rating)
            enemy_attack = sum([rating.get('attack', 0) for rating in enemy_ratings])
            enemy_health = sum([rating.get('health', 0) for rating in enemy_ratings])
            enemy_rating = enemy_attack * enemy_health
            monster_attack = sum([rating.get('attack', 0) for rating in monster_ratings])
            monster_health = sum([rating.get('health', 0) for rating in monster_ratings])
            monster_rating = monster_attack * monster_health
            mob_attack = sum([rating.get('attack', 0) for rating in mob_ratings])
            mob_health = sum([rating.get('health', 0) for rating in mob_ratings])
            mob_rating = mob_attack * mob_health
            if fleetsLostBySystem.get(sys_id, []):
                # print "     Assessing threats on turn %d ; noting that fleets were just lost in system %d , enemy fleets were %s seen as of turn %d, of which %s survived"%(
                # current_turn, sys_id, ["not", ""][saw_enemies_at_system.get(sys_id, False)], partial_vis_turn, local_enemy_fleet_ids)
                lost_fleet_attack = sum([rating.get('attack', 0) for rating in fleetsLostBySystem.get(sys_id, {})])
                lost_fleet_health = sum([rating.get('health', 0) for rating in fleetsLostBySystem.get(sys_id, {})])
                lost_fleet_rating = lost_fleet_attack * lost_fleet_health
            if not system or partial_vis_turn == -9999:  # under current visibility rules should not be possible to have any losses or other info here, but just in case...
                print "Have never had partial vis for system %d ( %s ) -- basing threat assessment on old info and lost ships" % (sys_id, sys_status.get('name', "name unknown"))
                sys_status.setdefault('local_fleet_threats', set())
                sys_status['planetThreat'] = 0
                sys_status['fleetThreat'] = int(max(FleetUtilsAI.combine_ratings(enemy_rating, mob_rating), 0.98 * sys_status.get('fleetThreat', 0), 1.1 * lost_fleet_rating - monster_rating))
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
                    pattack += prating['attack']
                    phealth += prating['health']
            sys_status['planetThreat'] = pattack*phealth
            sys_status['mydefenses'] = {'overall': mypattack*myphealth, 'attack': mypattack, 'health': myphealth}

            if max(sys_status.get('totalThreat', 0), pattack*phealth) >= 0.6 * lost_fleet_rating:  # previous threat assessment could account for losses, ignore the losses now
                lost_fleet_rating = 0

            # TODO: track visibility across turns in order to distinguish the blip of visibility in (losing) combat,
            # which FO currently treats as being for the previous turn, partially superseding the previous visibility for that turn
            if not partial_vis_turn == current_turn:  # (universe.getVisibility(sys_id, self.empire_id) >= fo.visibility.partial):
                print "System %s currently not visible" % system
                sys_status.setdefault('local_fleet_threats', set())
                # print "Stale visibility for system %d ( %s ) -- last seen %d, current Turn %d -- basing threat assessment on old info and lost ships"%(sys_id, sys_status.get('name', "name unknown"), partial_vis_turn, currentTurn)
                sys_status['fleetThreat'] = int(max(FleetUtilsAI.combine_ratings(enemy_rating, mob_rating), 0.98*sys_status.get('fleetThreat', 0), 1.1 * lost_fleet_rating - max(sys_status.get('monsterThreat', 0), monster_rating)))
                sys_status['enemy_threat'] = int(max(enemy_rating, 0.98*sys_status.get('enemy_threat', 0), 1.1 * lost_fleet_rating - max(sys_status.get('monsterThreat', 0), monster_rating)))
                sys_status['monsterThreat'] = int(max(monster_rating, 0.98*sys_status.get('monsterThreat', 0)))
                # sys_status['totalThreat'] = ((pattack + enemy_attack + monster_attack) ** 0.8) * ((phealth + enemy_health + monster_health)** 0.6)  # reevaluate this
                sys_status['totalThreat'] = max(FleetUtilsAI.combine_ratings_list([enemy_rating, mob_rating, monster_rating, pattack * phealth]), lost_fleet_rating)
            else:  # system considered visible #TODO: reevaluate as visibility rules change
                print "System %s currently visible" % system
                sys_status['local_fleet_threats'] = set(mobile_fleets)
                sys_status['fleetThreat'] = int(max(FleetUtilsAI.combine_ratings(enemy_rating, mob_rating), lost_fleet_rating - monster_rating))  # includes mobile monsters
                sys_status['enemy_threat'] = int(max(enemy_rating, lost_fleet_rating - monster_rating))  # does NOT include mobile monsters
                sys_status['monsterThreat'] = monster_rating
                sys_status['totalThreat'] = FleetUtilsAI.combine_ratings_list([enemy_rating, mob_rating, monster_rating, pattack * phealth])
            sys_status['regional_fleet_threats'] = sys_status['local_fleet_threats'].copy()

            if partial_vis_turn > 0 and sys_id not in supply_unobstructed_systems:  # has been seen with Partial Vis, but is currently supply-blocked
                sys_status['fleetThreat'] = max(sys_status['fleetThreat'], min_hidden_attack * min_hidden_health)
                sys_status['totalThreat'] = max(sys_status['totalThreat'], ((pattack + min_hidden_attack) ** 0.8) * ((phealth + min_hidden_health) ** 0.6))

        enemy_supply, enemy_near_supply = self.assess_enemy_supply()  # TODO: assess change in enemy supply over time
        # assess secondary threats (threats of surrounding systems) and update my fleet rating
        for sys_id in system_id_list:
            sys_status = self.systemStatus[sys_id]
            sys_status['enemies_supplied'] = enemy_supply.get(sys_id, [])
            sys_status['enemies_nearly_supplied'] = enemy_near_supply.get(sys_id, [])
            myattack, myhealth = 0, 0
            for fid in sys_status['myfleets']:
                this_rating = self.get_rating(fid, True, self.fleet_sum_tups_to_estat_dicts([(1, self.empire_standard_enemy)]))
                myattack += this_rating['attack']
                myhealth += this_rating['health']
            if sys_id != -1:
                sys_status['myFleetRating'] = myattack * myhealth
                sys_status['all_local_defenses'] = FleetUtilsAI.combine_ratings(sys_status['myFleetRating'], sys_status['mydefenses']['overall'])
            sys_status['neighbors'] = set(dict_from_map(universe.getSystemNeighborsMap(sys_id, self.empireID)))
            
        for sys_id in system_id_list:
            sys_status = self.systemStatus[sys_id]
            neighbors = sys_status.get('neighbors', set())
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
            threat_keys = ['fleetThreat', 'neighborThreat', 'jump2_threat']  # for local system includes both enemies and mobs
            sys_status['regional_threat'] = FleetUtilsAI.combine_ratings_list(map(lambda x: sys_status.get(x, 0), threat_keys))
            # TODO: investigate cases where regional_threat has been nonzero but no regional_threat_fleets
            # (probably due to attenuating history of past threats)
            sys_status.setdefault('regional_fleet_threats', set()).update(j1_threats, j2_threats)
            # threat, max_threat, myrating, j4_threats = self.area_ratings(jump4ring)
            # sys_status['jump4_threat'] = threat
            # sys_status['my_jump4_rating'] = myrating

    def area_ratings(self, system_ids):
        """Returns (fleet_threat, max_threat, myFleetRating) compiled over a group of systems."""
        max_threat = 0
        threat = 0
        myrating = 0
        threat_fleets = set()
        for sys_id in system_ids:
            sys_status = self.systemStatus.get(sys_id, {})
            # TODO: have distinct treatment for both enemy_threat and fleetThreat, respectively
            fthreat = sys_status.get('enemy_threat', 0)
            if fthreat > max_threat:
                max_threat = fthreat
            threat = FleetUtilsAI.combine_ratings(threat, fthreat)
            myrating = FleetUtilsAI.combine_ratings(myrating, sys_status.get('myFleetRating', 0))
            # myrating = FleetUtilsAI.combine_ratings(myrating, sys_status.get('all_local_defenses', 0))
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
            these_types = mission.get_mission_types()
            if any(wanted_mission_type in these_types for wanted_mission_type in mission_types):
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
            if mission.target_id not in fleet_ids:
                deleted_fleet_ids.append(mission.target_id)
        for deleted_fleet_id in deleted_fleet_ids:
            self.__remove_fleet_mission(deleted_fleet_id)

        for mission in self.get_all_fleet_missions():
            mission.clean_invalid_targets()

    def has_target(self, mission_type, target):
        for mission in self.get_fleet_missions_with_any_mission_types([mission_type]):
            if mission.has_target(mission_type, target):
                return True
        return False

    def rate_psuedo_fleet(self, ship_info):
        return self.rate_fleet(-1, enemy_stats=self.fleet_sum_tups_to_estat_dicts([(1, self.empire_standard_enemy)]), ship_info=ship_info)
        
    def rate_fleet(self, fleet_id, enemy_stats=None, ship_info=None):
        """ For enemy stats format see adjust_stats_vs_enemy()
            returns {'overall': adjusted attack*health, 'tally':sum_over_ships of ship_rating, 'attack':attack, 'health':health, 'nships':nships, 'summary':fleet_summary_tups}
            fleet_summary_tups is tuple of (nships, profile_tuple) tuples,
            where a profile tuple is ( total_attack, attacks, shields, max_structure)
            where attacks is a tuple of (n_shots, damage) tuples
            ship_info is an optional (used iff fleetID==-1) list of ( id, design_id, species_name) tuples where id can be -1 for a design-only analysis"""
        if enemy_stats is None:
            enemy_stats = [(1, {})]
        universe = fo.getUniverse()
        
        if fleet_id != -1:
            fleet = universe.getFleet(fleet_id)
            # if fleet and (not fleet.aggressive) and fleet.ownedBy(self.empireID):
            #     fleet.setAggressive(True)
            if not fleet:
                return {}
            ship_info = [(ship_id, ship.designID, ship.speciesName) for ship_id, ship in [(ship_id, universe.getShip(ship_id)) for ship_id in fleet.shipIDs] if ship]
        elif not ship_info:
            return {}
                
        rating = 0
        attack = 0
        health = 0
        nships = 0
        summary = {}
        for ship_id, design_id, species_name in ship_info:
            # could approximate by design, but checking meters has better current accuracy
            ship = universe.getShip(ship_id)
            stats = dict(self.get_weighted_design_stats(design_id, species_name))
            max_struct = stats['structure']
            if ship:
                structure = ship.currentMeterValue(fo.meterType.structure)
                shields = ship.currentMeterValue(fo.meterType.shield)
                stats['structure'] = structure
                stats['shields'] = shields
            self.adjust_stats_vs_enemy(stats, enemy_stats)
            rating += stats['attack'] * stats['structure']
            attack += stats['attack']
            health += stats['structure']
            ship_summary = (stats['attack'], tuple([tuple(item) for item in stats['attacks'].items()] or [(0, 0)]), stats['shields'], max_struct)
            summary.setdefault(ship_summary, [0])[0] += 1  # increment tally of ships with this summary profile
            nships += 1
        fleet_summary_tuples = [(v[0], k) for k, v in summary.items()]
        return {'overall': attack*health, 'tally': rating, 'attack': attack, 'health': health, 'nships': nships, 'summary': fleet_summary_tuples}

    def old_rate_fleet(self, fleet_id):
        universe = fo.getUniverse()
        fleet = universe.getFleet(fleet_id)
        # if fleet and not fleet.aggressive and fleet.ownedBy(self.empireID):
        #     fleet.setAggressive(True)
        if not fleet:
            return {}
        rating = 0
        attack = 0
        health = 0
        nships = 0
        for ship_id in fleet.shipIDs:
            # could approximate by design, but checking meters has better current accuracy
            ship = universe.getShip(ship_id)
            if not ship:
                continue
            stats = self.get_design_id_stats(ship.designID)
            rating += stats['attack'] * (stats['structure'] + stats['shields'])
            attack += stats['attack']
            health += (stats['structure'] + stats['shields'])
            nships += 1
        return {'overall': attack*health, 'tally': rating, 'attack': attack, 'health': health, 'nships': nships}

    def get_rating(self, fleet_id, force_new=False, enemy_stats=None):
        """Returns a dict with various rating info."""
        if fleet_id in self.fleetStatus and not force_new and enemy_stats is None:
            return self.fleetStatus[fleet_id].get('rating', {})
        else:
            fleet = fo.getUniverse().getFleet(fleet_id)
            if not fleet:
                return {}  # TODO: also ensure any info for that fleet is deleted
            status = {'rating': self.rate_fleet(fleet_id, enemy_stats), 'sysID': fleet.systemID, 'nships': len(fleet.shipIDs)}
            self.fleetStatus[fleet_id] = status
            return status['rating']

    def update_fleet_rating(self, fleet_id):
        return self.get_rating(fleet_id, force_new=True)

    def get_piloting_grades(self, species_name):
        if species_name not in piloting_grades:
            spec_tags = []
            if species_name:
                species = fo.getSpecies(species_name)
                if species:
                    spec_tags = species.tags
                else:
                    print "Error: get_piloting_grades couldn't retrieve species '%s'" % species_name
            piloting_grades[species_name] = (get_ai_tag_grade(spec_tags, 'WEAPONS'),
                                             get_ai_tag_grade(spec_tags, 'SHIELDS'))
        return piloting_grades[species_name]

    def weight_attacks(self, attacks, grade):
        """Re-weights attacks based on species piloting grade."""
        # TODO: make more accurate based off weapons lists
        weight = {'NO': -1, 'BAD': -0.25, '': 0.0, 'GOOD': 0.25, 'GREAT': 0.5, 'ULTIMATE': 1.0}.get(grade, 0.0)
        newattacks = {}
        for attack, count in attacks.items():
            newattacks[attack + round(attack * weight)] = count
        return newattacks

    def weight_shields(self, shields, grade):
        """Re-weights shields based on species defense bonus."""
        offset = {'NO': 0, 'BAD': 0, '': 0, 'GOOD': 1.0, 'GREAT': 0, 'ULTIMATE': 0}.get(grade, 0)
        return shields + offset

    def get_weighted_design_stats(self, design_id, species_name=""):
        """Rate a design, including species pilot effects
            returns dict of attacks {dmg1:count1}, attack, shields, structure."""
        weapons_grade, shields_grade = self.get_piloting_grades(species_name)
        design_stats = dict(self.get_design_id_stats(design_id))  # new dict so we don't modify our original data
        myattacks = self.weight_attacks(design_stats.get('attacks', {}), weapons_grade)
        design_stats['attacks'] = myattacks
        myshields = self.weight_shields(design_stats.get('shields', 0), shields_grade)
        design_stats['attack'] = sum([a * b for a, b in myattacks.items()])
        design_stats['shields'] = myshields
        return design_stats

    def adjust_stats_vs_enemy(self, ship_stats, enemy_stats=None):
        """Rate a ship w/r/t a particular enemy, adjusts ship_stats in place
            ship_stats: {'attacks':attacks, 'structure': str, 'shields': sh } 
            enemy stats: None or [ (num1, estats1), (num2, estats2), ]
            estats: {'attacks':attacks, 'shields': sh , structure:str} 
            attacks: {dmg1:count1, dmg2:count2}.
            """
        if enemy_stats is None:
            enemy_stats = [(1, {})]
        # orig_stats = copy.deepcopy(ship_stats)
        myattacks = ship_stats.get('attacks', {})
        mystructure = ship_stats.get('structure', 1)
        myshields = ship_stats.get('shields', 0)
        total_enemy_weights = 0
        attack_tally = 0
        structure_tally = 0
        for enemygroup in enemy_stats:
            count = enemygroup[0]
            estats = enemygroup[1]
            if estats == {}:
                attack_tally += count * sum([a * b for a, b in myattacks.items()])
                attack_total = sum([num * max(0, a_key) for a_key, num in myattacks.items()])
                attack_net = max(sum([num * max(0, a_key - myshields) for a_key, num in myattacks.items()]), 0.1 * attack_total)  # TODO: reconsider capping at 10-fold boost
                # attack_diff = attack_total - attack_net
                if attack_net > 0:  # will be unless no attacks at all, due to above calc
                    shield_boost = mystructure * ((attack_total / attack_net)-1)
                else:
                    shield_boost = 0
                structure_tally += count * (mystructure + shield_boost)  # uses num of my attacks for proxy calc of structure help from shield
                total_enemy_weights += count
                continue
            eshields = estats.get('shields', 0)
            tempattacktally = 0
            tempstruc = max(1e-4, estats.get('structure', 1))
            thisweight = count * tempstruc
            total_enemy_weights += thisweight
            e_attacks = estats.get('attacks', {})
            # structure_tally += thisweight * max(mystructure, min(e_attacks) - myshields) # TODO: improve shielded attack calc
            attack_total = sum([num * max(0, a_key) for a_key, num in e_attacks.items()])  # doesnt adjust for shields
            attack_net = max(sum([num * max(0, a_key - myshields) for a_key, num in e_attacks.items()]), 0.1 * attack_total)  # TODO: reconsider approach
            # attack_diff = attack_total - attack_net
            if attack_net > 0:  # will be unless no attacks at all, due to above calc
                shield_boost = mystructure * ((attack_total / attack_net)-1)
            else:
                shield_boost = 0
            structure_tally += thisweight * (mystructure + shield_boost) 
            for attack, nattacks in myattacks.items():
                adjustedattack = max(0, attack - eshields)
                thisattack = min(tempstruc, nattacks * adjustedattack)
                tempattacktally += thisattack
                tempstruc -= thisattack
                if tempstruc <= 0:
                    break
            attack_tally += thisweight * tempattacktally
        weighted_attack = attack_tally / total_enemy_weights
        weighted_structure = structure_tally / total_enemy_weights
        ship_stats['attack'] = weighted_attack
        ship_stats['structure'] = weighted_structure
        ship_stats['weighted'] = True
        return ship_stats

    def calc_tactical_rating_adjustment(self, partslist):
        adjust_dict = {"FU_IMPROVED_ENGINE_COUPLINGS": 0.1,
                       "FU_N_DIMENSIONAL_ENGINE_MATRIX": 0.21
                       }
        return sum([adjust_dict.get(partname, 0.0) for partname in partslist])

    def get_design_id_stats(self, design_id):
        if design_id is None:
            return {'attack': 0, 'structure': 0, 'shields': 0, 'attacks': {}, 'tact_adj': 0}
        elif design_id in self.designStats:
            return self.designStats[design_id]
        design = fo.getShipDesign(design_id)
        if design:
            attacks = {}
            for attack in list(design.attackStats):
                attacks[attack] = attacks.get(attack, 0) + 1
            parts = design.parts
            part_types = [fo.getPartType(part) for part in parts if part]
            shield_parts = [part for part in part_types if part.partClass == fo.shipPartClass.shields]
            shields = max([part.capacity for part in shield_parts]) if shield_parts else 0
            detector_parts = [part for part in part_types if part.partClass == fo.shipPartClass.detection]
            detect_bonus = max([part.capacity for part in detector_parts]) if detector_parts else 0
            # stats = {'attack':design.attack, 'structure':(design.structure + detect_bonus), 'shields':shields, 'attacks':attacks}
            stats = {'attack': design.attack, 'structure': design.structure, 'shields': shields, 
                     'attacks': attacks, 'tact_adj': self.calc_tactical_rating_adjustment(parts)}
        else:
            stats = {'attack': 0, 'structure': 0, 'shields': 0, 'attacks': {}, 'tact_adj': 0}
        self.designStats[design_id] = stats
        return stats

    def get_ship_role(self, ship_design_id):
        """Returns ship role for given designID, assesses and adds as needed."""

        if ship_design_id in self.__shipRoleByDesignID and self.__shipRoleByDesignID[ship_design_id] != EnumsAI.AIShipRoleType.SHIP_ROLE_INVALID:  # if thought was invalid, recheck to be sure
            return self.__shipRoleByDesignID[ship_design_id]
        else:
            self.get_design_id_stats(ship_design_id)  # just to update with info for this new design
            role = FleetUtilsAI.assess_ship_design_role(fo.getShipDesign(ship_design_id))
            self.__shipRoleByDesignID[ship_design_id] = role
            return role

    def add_ship_role(self, ship_design_id, ship_role):
        """Adds a ship designID/role pair."""
        if not (ship_role in EnumsAI.get_ship_roles_types()):
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

        if not force_new and fleet_id in self.__fleetRoleByID and self.__fleetRoleByID[fleet_id] != AIFleetMissionType.FLEET_MISSION_INVALID:
            return self.__fleetRoleByID[fleet_id]
        else:
            role = FleetUtilsAI.assess_fleet_role(fleet_id)
            self.__fleetRoleByID[fleet_id] = role
            make_aggressive = False
            if role in [AIFleetMissionType.FLEET_MISSION_COLONISATION,  
                        AIFleetMissionType.FLEET_MISSION_OUTPOST,
                        AIFleetMissionType.FLEET_MISSION_ORBITAL_INVASION,
                        AIFleetMissionType.FLEET_MISSION_ORBITAL_COLONISATION,
                        AIFleetMissionType.FLEET_MISSION_ORBITAL_OUTPOST
                        ]:
                pass
            elif role in [AIFleetMissionType.FLEET_MISSION_EXPLORATION,
                          AIFleetMissionType.FLEET_MISSION_INVASION
                          ]:
                this_rating = self.get_rating(fleet_id)
                if float(this_rating.get('overall', 0))/this_rating.get('nships', 1) >= 0.5 * MilitaryAI.cur_best_mil_ship_rating():
                    make_aggressive = True
            else:
                make_aggressive = True
            fo.issueAggressionOrder(fleet_id, make_aggressive)
            return role

    def add_fleet_role(self, fleet_id, mission_type):
        """Adds a fleet ID/role pair."""

        if not EnumsAI.check_validity(mission_type):
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
            self.get_rating(fleetID)
            self.ensure_have_fleet_missions([fleetID])
        self.__clean_fleet_roles(just_resumed=True)
        fleetsLostBySystem.clear()
        fleetsLostByID.clear()
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
        invasion_missions = self.get_fleet_missions_with_any_mission_types([EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION])
        for mission in invasion_missions:
            mission.clear_fleet_orders()
            mission.clear_targets(([-1] + mission.get_mission_types()[:1])[-1])

    def __clean_fleet_roles(self, just_resumed=False):
        """Removes fleetRoles if a fleet has been lost, and update fleet Ratings."""
        for sys_id in self.systemStatus:
            self.systemStatus[sys_id]['myFleetRating'] = 0

        # deleteRoles = []
        universe = fo.getUniverse()
        ok_fleets = FleetUtilsAI.get_empire_fleet_ids()
        fleet_list = sorted(list(self.__fleetRoleByID))
        unaccounted_fleets = set(ok_fleets) - set(fleet_list)
        ship_count = 0
        print "----------------------------------------------------------------------------------"
        print "in CleanFleetRoles"
        print "fleet_list : %s" % fleet_list
        print "-----------"
        print "FleetUtils empire-owned fleet_list : %s" % ok_fleets
        print "-----------"
        if unaccounted_fleets:
            print "Fleets unaccounted for in Empire Records:", unaccounted_fleets
            print "-----------"
        print "-----------"
        min_threat_rating = {'overall': MilitaryAI.MinThreat, 'attack': MilitaryAI.MinThreat ** 0.5, 'health': MilitaryAI.MinThreat ** 0.5}
        fighters = {(0, ((0, 0),), 0.0, 5.0): [0]}  # start with a dummy entry
        for fleet_id in fleet_list:
            status = self.fleetStatus.setdefault(fleet_id, {})
            rating = status.get('rating', {'overall': 0, 'attack': 0, 'health': 0})
            new_rating = self.rate_fleet(fleet_id, self.fleet_sum_tups_to_estat_dicts([(1, self.empire_standard_enemy)]))
            old_sys_id = status.get('sysID', -2)
            fleet = universe.getFleet(fleet_id)
            # if fleet and not fleet.aggressive:
            #     fleet.setAggressive(True)
            if fleet:
                sys_id = fleet.systemID
                if old_sys_id in [-2, -1]:
                    old_sys_id = sys_id
                status['nships'] = len(fleet.shipIDs)
                ship_count += status['nships']
            else:
                sys_id = old_sys_id  # can still retrieve a fleet object even if fleet was just destroyed, so shouldn't get here
            if fleet_id not in ok_fleets:  # or fleet.empty:
                if self.__fleetRoleByID.get(fleet_id, -1) != -1:
                    if not just_resumed:
                        if rating.get('overall', 0) > MilitaryAI.MinThreat:
                            fleetsLostBySystem.setdefault(old_sys_id, []).append(rating)
                        else:
                            fleetsLostBySystem.setdefault(old_sys_id, []).append(min_threat_rating)
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
                print "Fleet %d (%s) oldRating: %6d | new_rating %6d | at system %d (%s) | next system %d (%s)" % (fleet_id, fleet.name, rating.get('overall', 0), new_rating.get('overall', 0),
                                                                                                                   fleet.systemID, sys1_name, fleet.nextSystemID, sys2_name)
                print "Fleet %d (%s) summary: %s" % (fleet_id, fleet.name, rating.get('summary', None))
                status['rating'] = new_rating
                for count, sum_stats in new_rating['summary']:
                    if sum_stats[0] > 0:
                        fighters.setdefault(sum_stats, [0])[0] += count
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
                            if mMT0.target_type == TargetType.TARGET_SYSTEM:
                                status['sysID'] = mMT0.target_id  # hmm, but might still be a fair ways from here
        self.shipCount = ship_count
        std_fighter = sorted([(v, k) for k, v in fighters.items()])[-1][1]  # selects k with highest count (from fighters[k])
        self.empire_standard_fighter = std_fighter
        print "------------------------"
        # Next string used in charts. Don't modify it!
        print "Empire Ship Count: ", ship_count
        print "Empire standard fighter summary: ", std_fighter
        print "------------------------"

    def get_explorable_systems(self, explorable_systems_type):
        """Get all explorable systems determined by type."""
        # return copy.deepcopy(self.__explorableSystemByType[explorableSystemsType])
        if explorable_systems_type == AIExplorableSystemType.EXPLORABLE_SYSTEM_EXPLORED:
            return list(self.exploredSystemIDs)
        elif explorable_systems_type == AIExplorableSystemType.EXPLORABLE_SYSTEM_UNEXPLORED:
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
            print "    %-4d: %s" % item
        print "--------------------"
        # TODO: check length of fleets for losses or do in AIstat.__cleanRoles
        known_fleets = self.get_fleet_roles_map()
        self.newlySplitFleets.clear()

        fleets_to_split = [fleet_id for fleet_id in FleetUtilsAI.get_empire_fleet_ids() if fleet_id not in known_fleets]

        if fleets_to_split:
            universe = fo.getUniverse()
            print "splitting new fleets"
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
