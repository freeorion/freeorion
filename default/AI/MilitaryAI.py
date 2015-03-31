import freeOrionAIInterface as fo  # pylint: disable=import-error
import FreeOrionAI as foAI
import AIstate
import AITarget
from EnumsAI import AIFleetMissionType
import FleetUtilsAI
import PlanetUtilsAI
import PriorityAI
import ColonisationAI
from freeorion_tools import ppstring

MinThreat = 10  # the minimum threat level that will be ascribed to an unknown threat capable of killing scouts
military_allocations = []
minMilAllocations = {}
totMilRating = 0
num_milships = 0
verbose_mil_reporting = False


def try_again(mil_fleet_ids, try_reset=False, thisround=""):
    for fid in mil_fleet_ids:
        mission = foAI.foAIstate.get_fleet_mission(fid)
        mission.clear_fleet_orders()
        mission.clear_targets(-1)
    get_military_fleets(tryReset=try_reset, thisround=thisround)
    return


def rating_needed(target, current=0):
    if current >= target:
        return 0
    else:
        return target + current - (4*target*current)**0.5


def get_safety_factor():
    return [4.0, 3.0, 2.0, 1.5, 1.2, 1.0][foAI.foAIstate.aggression]


def avail_mil_needing_repair(mil_fleet_ids, split_ships=False, on_mission=False):
    """returns tuple of lists-- ( ids_needing_repair, ids_not )"""
    fleet_buckets = [[], []]
    universe = fo.getUniverse()
    cutoff = [0.70, 0.25][on_mission]
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
        this_sys_id = (fleet.nextSystemID != -1 and fleet.nextSystemID) or fleet.systemID
        fleet_ok = (sum(ships_cur_health) >= cutoff * sum(ships_max_health))
        local_status = foAI.foAIstate.systemStatus.get(this_sys_id, {})
        my_local_rating = local_status.get('mydefenses', {}).get('overall', 0) + local_status.get('myFleetRating', 0)
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


def get_military_fleets(milFleetIDs=None, tryReset=True, thisround="Main"):
    """get armed military fleets"""
    global military_allocations, totMilRating, num_milships

    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empire_id = empire.empireID
    capital_id = PlanetUtilsAI.get_capital()
    if capital_id is None:
        homeworld = None
    else:
        homeworld = universe.getPlanet(capital_id)
    if homeworld:
        home_system_id = homeworld.systemID
    else:
        home_system_id = -1

    if milFleetIDs is not None:
        all_military_fleet_ids = milFleetIDs
    else:
        all_military_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(AIFleetMissionType.FLEET_MISSION_MILITARY)
    if tryReset and (fo.currentTurn() + empire_id) % 30 == 0 and thisround == "Main":
        try_again(all_military_fleet_ids, try_reset=False, thisround=thisround + " Reset")

    num_milships = 0
    for fid in all_military_fleet_ids:
        num_milships += foAI.foAIstate.fleetStatus.get(fid, {}).get('nships', 0)

    this_tot_mil_rating = sum(map(lambda x: foAI.foAIstate.get_rating(x).get('overall', 0), all_military_fleet_ids))
    if "Main" in thisround:
        totMilRating = this_tot_mil_rating
        print "=================================================="
        print "%s Round Total Military Rating: %d" % (thisround, totMilRating)
        print "---------------------------------"
        foAI.foAIstate.militaryRating = totMilRating

    milFleetIDs = list(FleetUtilsAI.extract_fleet_ids_without_mission_types(all_military_fleet_ids))
    mil_needing_repair_ids, milFleetIDs = avail_mil_needing_repair(milFleetIDs, split_ships=True)
    avail_mil_rating = sum(map(lambda x: foAI.foAIstate.get_rating(x).get('overall', 0), milFleetIDs))
    if "Main" in thisround:
        print "=================================================="
        print "%s Round Available Military Rating: %d" % (thisround, avail_mil_rating)
        print "---------------------------------"
    remaining_mil_rating = avail_mil_rating
    allocations = []
    allocation_groups = {}

    if not milFleetIDs:
        if "Main" in thisround:
            military_allocations = []
        return []

    # for each system, get total rating of fleets assigned to it
    already_assigned_rating = {}
    assigned_attack = {}
    assigned_hp = {}
    for sys_id in universe.systemIDs:
        assigned_attack[sys_id] = 0
        assigned_hp[sys_id] = 0
    for fleet_id in [fid for fid in all_military_fleet_ids if fid not in milFleetIDs]:
        ai_fleet_mission = foAI.foAIstate.get_fleet_mission(fleet_id)
        sys_targets = []
        for ai_fleet_mission_type in ai_fleet_mission.get_mission_types():
            ai_targets = ai_fleet_mission.get_targets(ai_fleet_mission_type)
            for aiTarget in ai_targets:
                sys_targets.extend(aiTarget.get_required_system_ai_targets())
        if not sys_targets:  # shouldn't really be possible
            continue
        last_sys = sys_targets[-1].target_id  # will count this fleet as assigned to last system in target list
        assigned_attack[last_sys] += foAI.foAIstate.get_rating(fleet_id).get('attack', 0)
        assigned_hp[last_sys] += foAI.foAIstate.get_rating(fleet_id).get('health', 0)
    for sys_id in universe.systemIDs:
        mydefenses = foAI.foAIstate.systemStatus.get(sys_id, {}).get('mydefenses', {})
        mypattack = mydefenses.get('attack', 0)
        myphealth = mydefenses.get('health', 0)
        already_assigned_rating[sys_id] = (assigned_attack[sys_id] + mypattack) * (assigned_hp[sys_id] + myphealth)

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
                system_dict[sys_id] = system_dict.get(sys_id, 0) + status.get('rating', {}).get('overall', 0)
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

    safety_factor = get_safety_factor()

    top_target_planets = [pid for pid, pscore, trp in AIstate.invasionTargets[:PriorityAI.allottedInvasionTargets] if pscore > 20] + [pid for pid, (pscore, spec) in foAI.foAIstate.colonisablePlanetIDs.items()[:10] if pscore > 20]
    top_target_planets.extend(foAI.foAIstate.qualifyingTroopBaseTargets.keys())
    top_target_systems = []
    for sys_id in AIstate.invasionTargetedSystemIDs + PlanetUtilsAI.get_systems(top_target_planets):
        if sys_id not in top_target_systems:
            top_target_systems.append(sys_id)  # doing this rather than set, to preserve order

    # allocation format: ( sysID, newAllocation, takeAny, maxMultiplier )
    # ================================
    # --------Capital Threat ----------
    capital_threat = safety_factor*(2 * threat_bias + sum([foAI.foAIstate.systemStatus[capital_sys_id][thrt_key] for thrt_key in ['totalThreat', 'neighborThreat']]))
    needed_rating = rating_needed(1.4*capital_threat, already_assigned_rating[capital_sys_id])
    new_alloc = 0
    if tryReset:
        if needed_rating > 0.5*avail_mil_rating:
            try_again(all_military_fleet_ids)
            return
    if needed_rating > 0:
        new_alloc = min(remaining_mil_rating, needed_rating)
        allocations.append((capital_sys_id, new_alloc, True, 1.6*capital_threat))
        allocation_groups.setdefault('capitol', []).append((capital_sys_id, new_alloc, True, 2*capital_threat))
        remaining_mil_rating -= new_alloc
    if "Main" in thisround or new_alloc > 0:
        if verbose_mil_reporting:
            print "Empire Capital System: (%d) %s -- threat : %d, military allocation: existing: %d ; new: %d" % (capital_sys_id, universe.getSystem(capital_sys_id).name, capital_threat, already_assigned_rating[capital_sys_id], new_alloc)
            print "-----------------"

    # ================================
    # --------Empire Occupied Systems ----------
    empire_planet_ids = PlanetUtilsAI.get_owned_planets_by_empire(universe.planetIDs)
    empire_occupied_system_ids = list(set(PlanetUtilsAI.get_systems(empire_planet_ids)) - {capital_sys_id})
    if "Main" in thisround:
        if verbose_mil_reporting:
            print "Empire-Occupied Systems: %s" % (["| %d %s |" % (eo_sys_id, universe.getSystem(eo_sys_id).name) for eo_sys_id in empire_occupied_system_ids])
            print "-----------------"
    new_alloc = 0
    if empire_occupied_system_ids:
        oc_sys_tot_threat = [(o_s_id, threat_bias + safety_factor*sum([foAI.foAIstate.systemStatus.get(o_s_id, {}).get(thrt_key, 0) for thrt_key in ['totalThreat', 'neighborThreat']]))
                                                                                                                            for o_s_id in empire_occupied_system_ids]
        totoc_sys_threat = sum([thrt for sid, thrt in oc_sys_tot_threat])
        tot_cur_alloc = sum([already_assigned_rating[sid] for sid, thrt in oc_sys_tot_threat])
        oc_sys_alloc = 0
        for sid, thrt in oc_sys_tot_threat:
            cur_alloc = already_assigned_rating[sid]
            needed_rating = rating_needed(1.3*thrt, cur_alloc)
            if (needed_rating > 0.8 * remaining_mil_rating) and tryReset:
                try_again(all_military_fleet_ids)
                return
            this_alloc = 0
            if needed_rating > 0 and remaining_mil_rating > 0:
                this_alloc = max(0, min(needed_rating, 0.5*avail_mil_rating, remaining_mil_rating))
                new_alloc += this_alloc
                allocations.append((sid, this_alloc, True, 1.6*thrt))
                allocation_groups.setdefault('occupied', []).append((sid, this_alloc, True, 2*thrt))
                remaining_mil_rating -= this_alloc
                oc_sys_alloc += this_alloc
            if "Main" in thisround or this_alloc > 0:
                if verbose_mil_reporting:
                    print "Provincial Occupied system %4d ( %10s ) has local threat %8d ; existing military allocation %d and new allocation %8d" % (sid, universe.getSystem(sid).name, thrt, cur_alloc, this_alloc)
        if "Main" in thisround or new_alloc > 0:
            if verbose_mil_reporting:
                print "Provincial Empire-Occupied Sytems under total threat: %d -- total mil allocation: existing %d ; new: %d" % (totoc_sys_threat, tot_cur_alloc, oc_sys_alloc)
                print "-----------------"

    # ================================
    # --------Top Targeted Systems ----------
    # TODO: do native invasions highest priority
    other_targeted_system_ids = top_target_systems
    if "Main" in thisround:
        if verbose_mil_reporting:
            print "Top Colony and Invasion Targeted Systems : %s" % (["| %d %s |" % (sys_id, universe.getSystem(sys_id).name) for sys_id in other_targeted_system_ids])
            print "-----------------"
    # for these, calc local threat only, no neighbor threat, but use a multiplier for fleet safety
    new_alloc = 0
    if other_targeted_system_ids:
        ot_sys_alloc = 0
        ot_sys_threat = [(o_s_id, threat_bias + safety_factor*(foAI.foAIstate.systemStatus.get(o_s_id, {}).get('totalThreat', 0)+0.5*foAI.foAIstate.systemStatus.get(o_s_id, {}).get('neightborThreat', 0))) for o_s_id in other_targeted_system_ids]
        totot_sys_threat = sum([thrt for sid, thrt in ot_sys_threat])
        tot_cur_alloc = sum([already_assigned_rating[sid] for sid, thrt in ot_sys_threat])
        for sid, thrt in ot_sys_threat:
            cur_alloc = already_assigned_rating[sid]
            needed_rating = rating_needed(1.4*thrt, cur_alloc)
            this_alloc = 0
            # only record more allocation for this invasion if we already started or have enough rating available
            take_any = already_assigned_rating[sid] > 0
            if needed_rating > 0 and (remaining_mil_rating > needed_rating or take_any):
                this_alloc = max(0, min(needed_rating, remaining_mil_rating))
                max_alloc = safety_factor*3*max(foAI.foAIstate.systemStatus.get(sid, {}).get('totalThreat', 0), foAI.foAIstate.systemStatus.get(sid, {}).get('neightborThreat', 0))
                new_alloc += this_alloc
                allocations.append((sid, this_alloc, take_any, max_alloc))
                allocation_groups.setdefault('topTargets', []).append((sid, this_alloc, take_any, max_alloc))
                remaining_mil_rating -= this_alloc
                ot_sys_alloc += this_alloc
            if "Main" in thisround or this_alloc > 0:
                if verbose_mil_reporting:
                    print "Targeted system %4d ( %10s ) has local threat %8d ; existing military allocation %d and new allocation %8d" % (sid, universe.getSystem(sid).name, thrt, cur_alloc, this_alloc)
        if "Main" in thisround or new_alloc > 0:
            if verbose_mil_reporting:
                print "-----------------"
                print "Top Colony and Invasion Targeted Systems under total threat: %d -- total mil allocation-- existing: %d ; new: %d" % (totot_sys_threat, tot_cur_alloc, ot_sys_alloc)
                print "-----------------"

    # ================================
    # --------Targeted Systems ----------
    # TODO: do native invasions highest priority
    other_targeted_system_ids = [sys_id for sys_id in set(PlanetUtilsAI.get_systems(AIstate.opponentPlanetIDs)) if sys_id not in top_target_systems]
    if "Main" in thisround:
        if verbose_mil_reporting:
            print "Other Invasion Targeted Systems : %s" % (["| %d %s |" % (sys_id, universe.getSystem(sys_id).name) for sys_id in other_targeted_system_ids])
            print "-----------------"
    # for these, calc local threat only, no neighbor threat, but use a multiplier for fleet safety
    new_alloc = 0
    if other_targeted_system_ids:
        ot_sys_alloc = 0
        ot_sys_threat = [(o_s_id, threat_bias + safety_factor*(foAI.foAIstate.systemStatus.get(o_s_id, {}).get('totalThreat', 0)+0.5*foAI.foAIstate.systemStatus.get(o_s_id, {}).get('neighborThreat', 0))) for o_s_id in other_targeted_system_ids]
        totot_sys_threat = sum([thrt for sid, thrt in ot_sys_threat])
        tot_cur_alloc = sum([already_assigned_rating[sid] for sid, thrt in ot_sys_threat])
        for sid, thrt in ot_sys_threat:
            cur_alloc = already_assigned_rating[sid]
            needed_rating = rating_needed(1.4*thrt, cur_alloc)
            this_alloc = 0
            # only record more allocation for this invasion if we already started or have enough rating available
            take_any = already_assigned_rating[sid] > 0
            if needed_rating > 0 and (remaining_mil_rating > needed_rating or take_any):
                this_alloc = max(0, min(needed_rating, remaining_mil_rating))
                new_alloc += this_alloc
                max_alloc = safety_factor*3*max(foAI.foAIstate.systemStatus.get(sid, {}).get('totalThreat', 0), foAI.foAIstate.systemStatus.get(sid, {}).get('neightborThreat', 0))
                allocations.append((sid, this_alloc, take_any, max_alloc))
                allocation_groups.setdefault('otherTargets', []).append((sid, this_alloc, take_any, max_alloc))
                remaining_mil_rating -= this_alloc
                ot_sys_alloc += this_alloc
            if "Main" in thisround or this_alloc > 0:
                if verbose_mil_reporting:
                    print "Targeted system %4d ( %10s ) has local threat %8d ; existing military allocation %d and new allocation %8d" % (sid, universe.getSystem(sid).name, thrt, cur_alloc, this_alloc)
        if "Main" in thisround or new_alloc > 0:
            if verbose_mil_reporting:
                print "-----------------"
                print "Invasion Targeted Systems under total threat: %d -- total mil allocation-- existing: %d ; new: %d" % (totot_sys_threat, tot_cur_alloc, ot_sys_alloc)
                print "-----------------"

    other_targeted_system_ids = [sys_id for sys_id in list(set(AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs)) if sys_id not in top_target_systems]
    if "Main" in thisround:
        if verbose_mil_reporting:
            print "Other Targeted Systems : %s" % (["| %d %s |" % (sys_id, universe.getSystem(sys_id).name) for sys_id in other_targeted_system_ids])
            print "-----------------"
    if other_targeted_system_ids:
        ot_sys_alloc = 0
        ot_sys_threat = [(o_s_id, safety_factor*(threat_bias +foAI.foAIstate.systemStatus.get(o_s_id, {}).get('totalThreat', 0)+0.5*foAI.foAIstate.systemStatus.get(o_s_id, {}).get('neighborThreat', 0))) for o_s_id in other_targeted_system_ids]
        totot_sys_threat = sum([thrt for sid, thrt in ot_sys_threat])
        tot_cur_alloc = sum([already_assigned_rating[sid] for sid, thrt in ot_sys_threat])
        new_alloc = 0
        for sid, thrt in ot_sys_threat:
            cur_alloc = already_assigned_rating[sid]
            needed_rating = rating_needed(1.2*thrt, cur_alloc)
            this_alloc = 0
            # only record more allocation for this invasion if we already started or have enough rating available
            take_any = already_assigned_rating[sid] > 0
            if needed_rating > 0 and (remaining_mil_rating > needed_rating or take_any):
                this_alloc = max(0, min(needed_rating, remaining_mil_rating))
                new_alloc += this_alloc
                max_alloc = safety_factor*3*max(foAI.foAIstate.systemStatus.get(sid, {}).get('totalThreat', 0), foAI.foAIstate.systemStatus.get(sid, {}).get('neightborThreat', 0))
                allocations.append((sid, this_alloc, take_any, max_alloc))
                allocation_groups.setdefault('otherTargets', []).append((sid, this_alloc, take_any, max_alloc))
                remaining_mil_rating -= this_alloc
                ot_sys_alloc += this_alloc
            if "Main" in thisround or this_alloc > 0:
                if verbose_mil_reporting:
                    print "Targeted system %4d ( %10s ) has local threat %8d ; existing military allocation %d and new allocation %8d" % (sid, universe.getSystem(sid).name, thrt, cur_alloc, this_alloc)
        if "Main" in thisround or new_alloc > 0:
            if verbose_mil_reporting:
                print "-----------------"
                print "Other Targeted Systems under total threat: %d -- total mil allocation-- existing: %d ; new: %d" % (totot_sys_threat, tot_cur_alloc, ot_sys_alloc)
                print "-----------------"

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
        if verbose_mil_reporting:
            print "Blockade Targeted Systems : %s" % (["| %d %s |" % (sys_id, universe.getSystem(sys_id).name) for sys_id in other_targeted_system_ids])
            print "-----------------"
    if other_targeted_system_ids:
        ot_sys_alloc = 0
        ot_sys_threat = [(o_s_id, threat_bias + safety_factor*(foAI.foAIstate.systemStatus.get(o_s_id, {}).get('totalThreat', 0) + 0.5*foAI.foAIstate.systemStatus.get(o_s_id, {}).get('neighborThreat', 0))) for o_s_id in other_targeted_system_ids]
        totot_sys_threat = sum([thrt for sid, thrt in ot_sys_threat])
        tot_cur_alloc = sum([already_assigned_rating[sid] for sid, thrt in ot_sys_threat])
        new_alloc = 0
        for sid, thrt in ot_sys_threat:
            cur_alloc = already_assigned_rating[sid]
            needed_rating = rating_needed(1.2*thrt, cur_alloc)
            this_alloc = 0
            # only record more allocation for this invasion if we already started or have enough rating available
            take_any = already_assigned_rating[sid] > 0
            if needed_rating > 0 and (remaining_mil_rating > needed_rating or take_any):
                this_alloc = max(0, min(needed_rating, remaining_mil_rating))
                new_alloc += this_alloc
                max_alloc = safety_factor*2*max(foAI.foAIstate.systemStatus.get(sid, {}).get('totalThreat', 0), foAI.foAIstate.systemStatus.get(sid, {}).get('neightborThreat', 0))
                allocations.append((sid, this_alloc, take_any, max_alloc))
                allocation_groups.setdefault('otherTargets', []).append((sid, this_alloc, take_any, max_alloc))
                remaining_mil_rating -= this_alloc
                ot_sys_alloc += this_alloc
            if "Main" in thisround or this_alloc > 0:
                if verbose_mil_reporting:
                    print "Blockade Targeted system %4d ( %10s ) has local threat %8d ; existing military allocation %d and new allocation %8d" % (sid, universe.getSystem(sid).name, thrt, cur_alloc, this_alloc)
        if "Main" in thisround or new_alloc > 0:
            if verbose_mil_reporting:
                print "-----------------"
                print "Blockade Targeted Systems under total threat: %d -- total mil allocation-- existing: %d ; new: %d" % (totot_sys_threat, tot_cur_alloc, ot_sys_alloc)
                print "-----------------"

    current_mil_systems = [sid for sid, alloc, take_any, mm in allocations]
    interior_ids = list(foAI.foAIstate.expInteriorSystemIDs)
    interior_targets1 = (targetable_ids.union(interior_ids)).difference(current_mil_systems)
    interior_targets = [sid for sid in interior_targets1 if (
        threat_bias + foAI.foAIstate.systemStatus.get(sid, {}).get('totalThreat', 0) > 0.8*already_assigned_rating[sid])]
    if "Main" in thisround:
        if verbose_mil_reporting:
            print
            print "Other Empire-Proximal Systems : %s" % (["| %d %s |" % (sys_id, universe.getSystem(sys_id).name) for sys_id in interior_targets1])
            print "-----------------"
    # for these, calc fleet threat only, no neighbor threat, but use a multiplier for fleet safety
    new_alloc = 0
    if interior_targets:
        ot_sys_alloc = 0
        ot_sys_threat = [(o_s_id, threat_bias +safety_factor*(foAI.foAIstate.systemStatus.get(o_s_id, {}).get('totalThreat', 0))) for o_s_id in interior_targets]
        totot_sys_threat = sum([thrt for sid, thrt in ot_sys_threat])
        tot_cur_alloc = sum([already_assigned_rating[sid] for sid, thrt in ot_sys_threat])
        for sid, thrt in ot_sys_threat:
            cur_alloc = already_assigned_rating[sid]
            needed_rating = rating_needed(1.2*thrt, cur_alloc)
            this_alloc = 0
            # only record more allocation for this invasion if we already started or have enough rating available
            take_any = already_assigned_rating[sid] > 0
            if needed_rating > 0 and (remaining_mil_rating > needed_rating or take_any):
                this_alloc = max(0, min(needed_rating, remaining_mil_rating))
                new_alloc += this_alloc
                max_alloc = safety_factor*2*max(foAI.foAIstate.systemStatus.get(sid, {}).get('totalThreat', 0), foAI.foAIstate.systemStatus.get(sid, {}).get('neightborThreat', 0))
                allocations.append((sid, this_alloc, take_any, max_alloc))
                allocation_groups.setdefault('otherTargets', []).append((sid, this_alloc, take_any, max_alloc))
                remaining_mil_rating -= this_alloc
                ot_sys_alloc += this_alloc
            if "Main" in thisround or this_alloc > 0:
                if verbose_mil_reporting:
                    print "Other interior system %4d ( %10s ) has local threat %8d ; existing military allocation %d and new allocation %8d" % (sid, universe.getSystem(sid).name, thrt, cur_alloc, this_alloc)
        if "Main" in thisround or new_alloc > 0:
            if verbose_mil_reporting:
                print "-----------------"
                print "Other Interior Systems under total threat: %d -- total mil allocation-- existing: %d ; new: %d" % (totot_sys_threat, tot_cur_alloc, ot_sys_alloc)
                print "-----------------"
    elif "Main" in thisround:
        if verbose_mil_reporting:
            print "-----------------"
            print "No Other Interior Systems with fleet threat "
            print "-----------------"

    monster_dens = []

    # explo_target_ids, _ = ExplorationAI.get_current_exploration_info(verbose=False)
    explo_target_ids = []
    if "Main" in thisround:
        if verbose_mil_reporting:
            print
            print "Exploration-targeted Systems: %s" % (["| %d %s |" % (sys_id, universe.getSystem(sys_id).name) for sys_id in explo_target_ids])
            print "-----------------"
    # for these, calc fleet threat only, no neighbor threat, but use a multiplier for fleet safety
    new_alloc = 0
    if explo_target_ids:
        ot_sys_alloc = 0
        ot_sys_threat = [(o_s_id, safety_factor*(foAI.foAIstate.systemStatus.get(o_s_id, {}).get('fleetThreat', 0) + foAI.foAIstate.systemStatus.get(o_s_id, {}).get('monsterThreat', 0) + foAI.foAIstate.systemStatus.get(o_s_id, {}).get('planetThreat', 0))) for o_s_id in explo_target_ids]
        totot_sys_threat = sum([thrt for sid, thrt in ot_sys_threat])
        tot_cur_alloc = sum([0.8*already_assigned_rating[sid] for sid, thrt in ot_sys_threat])
        for sid, thrt in ot_sys_threat:
            cur_alloc = already_assigned_rating[sid]
            needed_rating = rating_needed(1.2*thrt, cur_alloc)
            this_alloc = 0
            # only record more allocation for this invasion if we already started or have enough rating available
            take_any = False
            if needed_rating > 0 and (remaining_mil_rating > needed_rating or take_any):
                this_alloc = max(0, min(needed_rating, remaining_mil_rating))
                new_alloc += this_alloc
                max_alloc = safety_factor*2*max(foAI.foAIstate.systemStatus.get(sid, {}).get('totalThreat', 0), foAI.foAIstate.systemStatus.get(sid, {}).get('neightborThreat', 0))
                allocations.append((sid, this_alloc, take_any, max_alloc))
                allocation_groups.setdefault('exploreTargets', []).append((sid, this_alloc, take_any, max_alloc))
                remaining_mil_rating -= this_alloc
                ot_sys_alloc += this_alloc
            if "Main" in thisround or this_alloc > 0:
                if verbose_mil_reporting:
                    print "Exploration-targeted %4d ( %10s ) has local threat %8d ; existing military allocation %d and new allocation %8d" % (sid, universe.getSystem(sid).name, thrt, cur_alloc, this_alloc)
        if "Main" in thisround or new_alloc > 0:
            if verbose_mil_reporting:
                print "-----------------"
                print "Exploration-targeted s under total threat: %d -- total mil allocation-- existing: %d ; new: %d" % (totot_sys_threat, tot_cur_alloc, ot_sys_alloc)
                print "-----------------"

    visible_system_ids = foAI.foAIstate.visInteriorSystemIDs.keys() + foAI.foAIstate. visBorderSystemIDs.keys()
    accessible_system_ids = [sys_id for sys_id in visible_system_ids if universe.systemsConnected(sys_id, home_system_id, empire_id)]
    current_mil_systems = [sid for sid, alloc, take_any, multiplier in allocations if alloc > 0]
    border_targets1 = [sid for sid in accessible_system_ids if sid not in current_mil_systems]
    border_targets = [sid for sid in border_targets1 if (threat_bias + foAI.foAIstate.systemStatus.get(sid, {}).get('fleetThreat', 0) + foAI.foAIstate.systemStatus.get(sid, {}).get('planetThreat', 0) > 0.8*already_assigned_rating[sid])]
    if "Main" in thisround:
        if verbose_mil_reporting:
            print
            print "Empire-Accessible Systems not yet allocated military: %s" % (["| %d %s |" % (sys_id, universe.getSystem(sys_id) and universe.getSystem(sys_id).name) for sys_id in border_targets1])
            print "-----------------"
    # for these, calc fleet threat only, no neighbor threat, but use a multiplier for fleet safety
    new_alloc = 0
    if border_targets:
        ot_sys_alloc = 0
        ot_sys_threat = [(o_s_id, threat_bias + safety_factor*(foAI.foAIstate.systemStatus.get(o_s_id, {}).get('fleetThreat', 0) + foAI.foAIstate.systemStatus.get(o_s_id, {}).get('monsterThreat', 0) + foAI.foAIstate.systemStatus.get(o_s_id, {}).get('planetThreat', 0))) for o_s_id in border_targets]
        totot_sys_threat = sum([thrt for sid, thrt in ot_sys_threat])
        tot_cur_alloc = sum([0.8*already_assigned_rating[sid] for sid, thrt in ot_sys_threat])
        for sid, thrt in ot_sys_threat:
            cur_alloc = already_assigned_rating[sid]
            needed_rating = rating_needed(1.2*thrt, cur_alloc)
            this_alloc = 0
            # only record more allocation for this invasion if we already started or have enough rating available
            take_any = False
            if needed_rating > 0 and (remaining_mil_rating > needed_rating or take_any):
                this_alloc = max(0, min(needed_rating, remaining_mil_rating))
                new_alloc += this_alloc
                max_alloc = safety_factor*2*max(foAI.foAIstate.systemStatus.get(sid, {}).get('totalThreat', 0), foAI.foAIstate.systemStatus.get(sid, {}).get('neightborThreat', 0))
                allocations.append((sid, this_alloc, take_any, max_alloc))
                allocation_groups.setdefault('accessibleTargets', []).append((sid, this_alloc, take_any, max_alloc))
                remaining_mil_rating -= this_alloc
                ot_sys_alloc += this_alloc
            if "Main" in thisround or this_alloc > 0:
                if verbose_mil_reporting:
                    print "Other Empire-Accessible system %4d ( %10s ) has local biased threat %8d ; existing military allocation %d and new allocation %8d" % (sid, universe.getSystem(sid).name, thrt, cur_alloc, this_alloc)
        if "Main" in thisround or new_alloc > 0:
            if verbose_mil_reporting:
                print "-----------------"
                print "Other Empire-Accessible Systems under total biased threat: %d -- total mil allocation-- existing: %d ; new: %d" % (totot_sys_threat, tot_cur_alloc, ot_sys_alloc)
                print "-----------------"
    elif "Main" in thisround:
        if verbose_mil_reporting:
            print "-----------------"
            print "No Other Empire-Accessible Systems with biased local threat "
            print "-----------------"

    # monster den treatment probably unnecessary now
    if "Main" in thisround:
        if verbose_mil_reporting:
            print
            print "Big-Monster Dens: %s" % (["| %d %s |" % (sys_id, universe.getSystem(sys_id).name) for sys_id in monster_dens])
            print "-----------------"
    # for these, calc fleet threat only, no neighbor threat, but use a multiplier for fleet safety
    new_alloc = 0
    if monster_dens:
        ot_sys_alloc = 0
        ot_sys_threat = [(o_s_id, safety_factor*(foAI.foAIstate.systemStatus.get(o_s_id, {}).get('fleetThreat', 0)+foAI.foAIstate.systemStatus.get(o_s_id, {}).get('monsterThreat', 0) + foAI.foAIstate.systemStatus.get(o_s_id, {}).get('planetThreat', 0))) for o_s_id in monster_dens]
        for sid, thrt in ot_sys_threat:
            cur_alloc = 0.8*already_assigned_rating[sid]
            this_alloc = 0
            if (thrt > cur_alloc) and remaining_mil_rating > 2 * thrt:
                this_alloc = int(0.99999 + (thrt-cur_alloc)*1.5)
                new_alloc += this_alloc
                allocations.append((sid, this_alloc, False, 5))
                remaining_mil_rating -= this_alloc
                ot_sys_alloc += this_alloc
            if "Main" in thisround or this_alloc > 0:
                if verbose_mil_reporting:
                    print "Monster Den %4d ( %10s ) has local threat %8d ; existing military allocation %d and new allocation %8d" % (sid, universe.getSystem(sid).name, thrt, cur_alloc, this_alloc)
        if "Main" in thisround or new_alloc > 0:
            if verbose_mil_reporting:
                print "-----------------"

    if remaining_mil_rating <= 6:
        new_allocations = [(sid, alc, alc, ta) for (sid, alc, ta, mm) in allocations]
    else:
        print "%s Round Remaining military strength allocation %d will be allocated as surplus allocation to top current priorities" % (thisround, remaining_mil_rating)
        new_allocations = []
        for cat in ['capitol', 'topTargets', 'otherTargets', 'accessibleTargets', 'occupied', 'exploreTargets']:
            for sid, alloc, take_any, max_alloc in allocation_groups.get(cat, []):
                if remaining_mil_rating <= 0:
                    new_allocations.append((sid, alloc, alloc, take_any))
                else:
                    new_rating = min(remaining_mil_rating+alloc, max(alloc, rating_needed(max_alloc, already_assigned_rating[sid])))
                    new_allocations.append((sid, new_rating, alloc, take_any))
                    remaining_mil_rating -= (new_rating - alloc)

    if "Main" in thisround:
        military_allocations = new_allocations
    minMilAllocations.clear()
    minMilAllocations.update([(sid, alloc) for sid, alloc, take_any, mm in allocations])
    if verbose_mil_reporting or "Main" in thisround:
        print "------------------------------\nFinal %s Round Military Allocations: %s \n-----------------------" % (thisround, dict([(sid, alloc) for sid, alloc, minalloc, take_any in new_allocations]))

    # export military systems for other AI modules
    if "Main" in thisround:
        AIstate.militarySystemIDs = list(set([sid for sid, alloc, minalloc, take_any in new_allocations]).union([sid for sid in already_assigned_rating if already_assigned_rating[sid] > 0]))
    else:
        AIstate.militarySystemIDs = list(set([sid for sid, alloc, minalloc, take_any in new_allocations]).union(AIstate.militarySystemIDs))
    return new_allocations


def assign_military_fleets_to_systems(useFleetIDList=None, allocations=None):
    # assign military fleets to military theater systems
    global military_allocations
    universe = fo.getUniverse()
    if allocations is None:
        allocations = []

    doing_main = (useFleetIDList is None)
    if doing_main:
        base_defense_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(AIFleetMissionType.FLEET_MISSION_ORBITAL_DEFENSE)
        unassigned_base_defense_ids = FleetUtilsAI.extract_fleet_ids_without_mission_types(base_defense_ids)
        for fleet_id in unassigned_base_defense_ids:
            fleet = universe.getFleet(fleet_id)
            if not fleet:
                continue
            sys_id = fleet.systemID
            target = AITarget.TargetSystem(sys_id)
            fleet_mission = foAI.foAIstate.get_fleet_mission(fleet_id)
            fleet_mission.clear_fleet_orders()
            fleet_mission.clear_targets((fleet_mission.get_mission_types() + [-1])[0])
            mission_type = AIFleetMissionType.FLEET_MISSION_ORBITAL_DEFENSE
            fleet_mission.add_target(mission_type, target)

        all_military_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(AIFleetMissionType.FLEET_MISSION_MILITARY)
        AIstate.militaryFleetIDs = all_military_fleet_ids
        if not all_military_fleet_ids:
            military_allocations = []
            return
        avail_mil_fleet_ids = list(FleetUtilsAI.extract_fleet_ids_without_mission_types(all_military_fleet_ids))
        mil_needing_repair_ids, avail_mil_fleet_ids = avail_mil_needing_repair(avail_mil_fleet_ids)
        these_allocations = military_allocations
        print "=================================================="
        print "assigning military fleets"
        print "---------------------------------"
    else:
        avail_mil_fleet_ids = list(useFleetIDList)
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
        try:
            these_fleets = FleetUtilsAI.get_fleets_for_mission(1, {'rating': alloc}, {'rating': minalloc}, found_stats, "", systems_to_check=[sys_id], systems_checked=[],
                                                               fleet_pool_set=avail_mil_fleet_ids, fleet_list=found_fleets, verbose=False)
        except:
            continue
        if not these_fleets:
            if not found_fleets or not (FleetUtilsAI.stats_meet_reqs(found_stats, {'rating': minalloc}) or takeAny):
                if doing_main:
                    if verbose_mil_reporting:
                        print "NO available/suitable military allocation for system %d ( %s ) -- requested allocation %8d, found available rating %8d in fleets %s" % (sys_id, universe.getSystem(sys_id).name, minalloc, found_stats.get('rating', 0), found_fleets)
                avail_mil_fleet_ids.update(found_fleets)
                continue
            else:
                these_fleets = found_fleets
                # rating = sum( map(lambda x: foAI.foAIstate.rate_fleet(x), foundFleets ) )
                ratings = map(foAI.foAIstate.get_rating, found_fleets)
                rating = sum([fr.get('attack', 0) for fr in ratings]) * sum([fr.get('health', 0) for fr in ratings])
                if doing_main and verbose_mil_reporting:
                    if rating < minMilAllocations.get(sys_id, 0):
                        print "PARTIAL military allocation for system %d ( %s ) -- requested allocation %8d -- got %8d with fleets %s" % (sys_id, universe.getSystem(sys_id).name, minalloc, rating, these_fleets)
                    else:
                        print "FULL MIN military allocation for system %d ( %s ) -- requested allocation %8d -- got %8d with fleets %s" % (sys_id, universe.getSystem(sys_id).name, minMilAllocations.get(sys_id, 0), rating, these_fleets)
        elif doing_main and verbose_mil_reporting:
            print "FULL+ military allocation for system %d ( %s ) -- requested allocation %8d, got %8d with fleets %s" % (sys_id, universe.getSystem(sys_id).name, alloc, found_stats.get('rating', 0), these_fleets)
        target = AITarget.TargetSystem(sys_id)
        for fleet_id in these_fleets:
            fo.issueAggressionOrder(fleet_id, True)
            fleet_mission = foAI.foAIstate.get_fleet_mission(fleet_id)
            fleet_mission.clear_fleet_orders()
            fleet_mission.clear_targets((fleet_mission.get_mission_types() + [-1])[0])
            if sys_id in list(set(AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs + AIstate.invasionTargetedSystemIDs + AIstate.blockadeTargetedSystemIDs)):
                mission_type = AIFleetMissionType.FLEET_MISSION_SECURE
            else:
                mission_type = AIFleetMissionType.FLEET_MISSION_MILITARY
            fleet_mission.add_target(mission_type, target)
            fleet_mission.generate_fleet_orders()
            if not doing_main:
                foAI.foAIstate.misc.setdefault('ReassignedFleetMissions', []).append(fleet_mission)

    if doing_main:
        print "---------------------------------"
