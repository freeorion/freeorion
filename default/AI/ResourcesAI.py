import freeOrionAIInterface as fo  # pylint: disable=import-error
import FreeOrionAI as foAI
from EnumsAI import AIPriorityType, get_priority_resource_types, AIFocusType
import PlanetUtilsAI
import random
import ColonisationAI
import AIDependencies
import FleetUtilsAI
from freeorion_debug import Timer
from freeorion_tools import tech_is_complete

resource_timer = Timer('timer_bucket')

AIPriorityTypeNames = AIPriorityType()
newTargets = {}
currentFocus = {}
currentOutput = {}
planetMap = {}
IFocus = AIFocusType.FOCUS_INDUSTRY
RFocus = AIFocusType.FOCUS_RESEARCH
MFocus = AIFocusType.FOCUS_MINING  # not currently used in content
GFocus = AIFocusType.FOCUS_GROWTH
PFocus = AIFocusType.FOCUS_PROTECTION

RESEARCH_WEIGHTING = 2.0

useGrowth = True
limitAssessments = False

lastFociCheck = [0]


def _fill_data_dicts(planet_ids):
    universe = fo.getUniverse()
    newTargets.clear()
    currentFocus.clear()
    currentOutput.clear()
    planets = [(pid, planetMap[pid]) for pid in planet_ids]
    for pid, planet in planets:
        currentFocus[pid] = planet.focus
        currentOutput.setdefault(pid, {})[IFocus] = planet.currentMeterValue(fo.meterType.industry)
        currentOutput[pid][RFocus] = planet.currentMeterValue(fo.meterType.research)
        if IFocus in planet.availableFoci and planet.focus != IFocus:
            fo.issueChangeFocusOrder(pid, IFocus)  # may not be able to take, but try
    universe.updateMeterEstimates(planet_ids)
    for pid, planet in planets:
        itarget = planet.currentMeterValue(fo.meterType.targetIndustry)
        rtarget = planet.currentMeterValue(fo.meterType.targetResearch)
        if planet.focus == IFocus:
            newTargets.setdefault(pid, {})[IFocus] = (itarget, rtarget)
            newTargets.setdefault(pid, {})[GFocus] = [0, rtarget]
        else:
            newTargets.setdefault(pid, {})[IFocus] = (0, 0)
            newTargets.setdefault(pid, {})[GFocus] = [0, 0]
        # if currentFocus[pid] == MFocus:
        # newTargets[pid][MFocus] = (mtarget, rtarget)
        if RFocus in planet.availableFoci and planet.focus != RFocus:
            fo.issueChangeFocusOrder(pid, RFocus)  # may not be able to take, but try
    universe.updateMeterEstimates(planet_ids)
    for pid, planet in planets:
        can_focus = planet.currentMeterValue(fo.meterType.targetPopulation) > 0
        itarget = planet.currentMeterValue(fo.meterType.targetIndustry)
        rtarget = planet.currentMeterValue(fo.meterType.targetResearch)
        if planet.focus == RFocus:
            newTargets.setdefault(pid, {})[RFocus] = (itarget, rtarget)
            newTargets[pid][GFocus][0] = itarget
        else:
            newTargets.setdefault(pid, {})[RFocus] = (0, 0)
            newTargets[pid][GFocus][0] = 0
        if can_focus and currentFocus[pid] != planet.focus:
            fo.issueChangeFocusOrder(pid, currentFocus[pid])  # put it back to what it was
    universe.updateMeterEstimates(planet_ids)
    # Protection focus will give the same off-focus Industry and Research targets as Growth Focus
    for pid, planet in planets:
        newTargets[pid][PFocus] = newTargets[pid][GFocus]


def get_resource_target_totals(empirePlanetIDs):
    pp = sum(x.currentMeterValue(fo.meterType.targetIndustry) for x in planetMap.values())
    rp = sum(x.currentMeterValue(fo.meterType.targetResearch) for x in planetMap.values())
    _fill_data_dicts(empirePlanetIDs)
    return pp, rp


def print_resources_priority():
    """calculate top resource priority"""
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empirePlanetIDs = PlanetUtilsAI.get_owned_planets_by_empire(universe.planetIDs)
    print "Resource Management:"
    print
    print "Resource Priorities:"
    resourcePriorities = {}
    for priorityType in get_priority_resource_types():
        resourcePriorities[priorityType] = foAI.foAIstate.get_priority(priorityType)

    sortedPriorities = resourcePriorities.items()
    sortedPriorities.sort(lambda x, y: cmp(x[1], y[1]), reverse=True)
    topPriority = -1
    for evaluationPair in sortedPriorities:
        if topPriority < 0:
            topPriority = evaluationPair[0]
        print "    ResourcePriority |Score: %s | %s " % (AIPriorityTypeNames.name(evaluationPair[0]), evaluationPair[1])

    # what is the focus of available resource centers?
    print
    warnings = {}
    print "Planet Resources Foci:"
    for planetID in empirePlanetIDs:
        planet = universe.getPlanet(planetID)
        planetPopulation = planet.currentMeterValue(fo.meterType.population)
        maxPop = planet.currentMeterValue(fo.meterType.targetPopulation)
        if maxPop < 1 and planetPopulation > 0:
            warnings[planet.name] = (planetPopulation, maxPop)
        statusStr = "  ID: " + str(planetID) + " Name: % 18s -- % 6s % 8s " % (str(planet.name), str(planet.size), str(planet.type))
        statusStr += " Focus: % 8s" % ("_".join(str(planet.focus).split("_")[1:])[:8]) + " Species: " + str(planet.speciesName) + " Pop: %2d/%2d" % (planetPopulation, maxPop)
        print statusStr
    print "\n\nEmpire Totals:\nPopulation: %5d \nProduction: %5d\nResearch: %5d\n" % (empire.population(), empire.productionPoints, empire.resourceProduction(fo.resourceType.research))
    if warnings != {}:
        for pname in warnings:
            mp, cp = warnings[pname]
            print "Population Warning! -- %s has unsustainable current pop %d -- target %d" % (pname, cp, mp)
        print
    warnings.clear()


def weighted_sum_output(outputs):
    """Return a weighted sum of planetary output
    :param outputs: (industry, research)
    :return: weighted sum of industry and research
    """
    return outputs[0] + RESEARCH_WEIGHTING * outputs[1]


def assess_protection_focus(pid):
    """Return True if planet should use Protection Focus"""
    this_planet = planetMap[pid]
    sys_status = foAI.foAIstate.systemStatus.get(this_planet.systemID, {})
    threat_from_supply = (0.25 * foAI.foAIstate.empire_standard_enemy_rating *
                          min(2, len(sys_status.get('enemies_nearly_supplied', []))))
    print "Planet %s has regional+supply threat of %.1f" % ('P_%d<%s>'%(pid, this_planet.name), threat_from_supply)
    regional_threat = sys_status.get('regional_threat', 0) + threat_from_supply
    if not regional_threat:  # no need for protection
        if currentFocus[pid] == PFocus:
            print "Advising dropping Protection Focus at %s due to no regional threat" % this_planet
        return False
    cur_prod_val = weighted_sum_output([currentOutput[pid][IFocus], currentOutput[pid][RFocus]])
    target_prod_val = max(map(weighted_sum_output, [newTargets[pid][IFocus], newTargets[pid][RFocus]]))
    prot_prod_val = weighted_sum_output(newTargets[pid][PFocus])
    local_production_diff = 0.8 * cur_prod_val + 0.2 * target_prod_val - prot_prod_val
    fleet_threat = sys_status.get('fleetThreat', 0)
    # TODO: relax the below rejection once the overall determination of PFocus is better tuned
    if not fleet_threat and local_production_diff > 8:
        if currentFocus[pid] == PFocus:
            print "Advising dropping Protection Focus at %s due to excessive productivity loss" % this_planet
        return False
    local_p_defenses = sys_status.get('mydefenses', {}).get('overall',0)
    # TODO have adjusted_p_defenses take other in-system planets into account
    adjusted_p_defenses = local_p_defenses * (1.0 if currentFocus[pid] != PFocus else 0.5)
    local_fleet_rating = sys_status.get('myFleetRating', 0)
    combined_local_defenses = sys_status.get('all_local_defenses', 0)
    my_neighbor_rating = sys_status.get('my_neighbor_rating', 0)
    neighbor_threat = sys_status.get('neighborThreat', 0)
    safety_factor = 1.2 if currentFocus[pid] == PFocus else 0.5
    cur_shield = this_planet.currentMeterValue(fo.meterType.shield)
    max_shield = this_planet.currentMeterValue(fo.meterType.maxShield)
    cur_troops = this_planet.currentMeterValue(fo.meterType.troops)
    max_troops = this_planet.currentMeterValue(fo.meterType.maxTroops)
    cur_defense = this_planet.currentMeterValue(fo.meterType.defense)
    max_defense = this_planet.currentMeterValue(fo.meterType.maxDefense)
    def_meter_pairs = [(cur_troops, max_troops), (cur_shield, max_shield), (cur_defense, max_defense)]
    use_protection = True
    reason = ""
    if (fleet_threat and  # i.e., an enemy is sitting on us
              (currentFocus[pid] != PFocus or  # too late to start protection TODO: but maybe regen worth it
              # protection forcus only useful here if it maintains an elevated level
              all([AIDependencies.PROT_FOCUS_MULTIPLIER * a <= b for a,b in def_meter_pairs]))):
        use_protection = False
        reason = "A"
    elif ((currentFocus[pid] != PFocus and cur_shield < max_shield - 2 and
               not tech_is_complete(AIDependencies.PLANET_BARRIER_I_TECH)) and
              (cur_defense < max_defense - 2 and not tech_is_complete(AIDependencies.DEFENSE_REGEN_1_TECH)) and
              (cur_troops < max_troops - 2)):
        use_protection = False
        reason = "B1"
    elif ((currentFocus[pid] == PFocus and cur_shield*AIDependencies.PROT_FOCUS_MULTIPLIER < max_shield-2 and
               not tech_is_complete(AIDependencies.PLANET_BARRIER_I_TECH)) and
              (cur_defense*AIDependencies.PROT_FOCUS_MULTIPLIER < max_defense-2 and
                   not tech_is_complete(AIDependencies.DEFENSE_REGEN_1_TECH)) and
              (cur_troops*AIDependencies.PROT_FOCUS_MULTIPLIER < max_troops-2)):
        use_protection = False
        reason = "B2"
    elif max(max_shield, max_troops, max_defense) < 3:  # joke defenses, don't bother with protection focus
        use_protection = False
        reason = "C"
    elif regional_threat and local_production_diff <= 2.0:
        reason = "D"
        pass  # i.e., use_protection = True
    elif safety_factor * regional_threat <= local_fleet_rating:
        use_protection = False
        reason = "E"
    elif (safety_factor * regional_threat <= combined_local_defenses and
              (currentFocus[pid] != PFocus or
              (0.5 * safety_factor * regional_threat <= local_fleet_rating and
                   fleet_threat == 0 and neighbor_threat < combined_local_defenses and
                   local_production_diff > 5))):
        use_protection = False
        reason = "F"
    elif (regional_threat <= FleetUtilsAI.combine_ratings(local_fleet_rating, adjusted_p_defenses) and
              safety_factor * regional_threat <=
              FleetUtilsAI.combine_ratings_list([my_neighbor_rating, local_fleet_rating, adjusted_p_defenses])  and
              local_production_diff > 5):
        use_protection = False
        reason = "G"
    if use_protection or currentFocus[pid] == PFocus:
        print ("Advising %sProtection Focus (reason %s) for planet %s, with local_prod_diff of %.1f, comb. local"
               " defenses %.1f, local fleet rating %.1f and regional threat %.1f, threat sources: %s") % (
            ["dropping ", ""][use_protection], reason, this_planet, local_production_diff, combined_local_defenses,
            local_fleet_rating, regional_threat, sys_status['regional_fleet_threats'])
    return use_protection

def set_planet_resource_foci():
    """set resource focus of planets """
    newFoci = {}

    print "\n============================"
    print "Collecting info to assess Planet Focus Changes\n"
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    currentTurn = fo.currentTurn()
    # set the random seed (based on galaxy seed, empire ID and current turn)
    # for game-reload consistency 
    freq = min(3, (max(5, currentTurn - 80)) / 4.0) ** (1.0 / 3)
    if not (limitAssessments and (abs(currentTurn - lastFociCheck[0]) < 1.5 * freq) and (random.random() < 1.0 / freq)):
        lastFociCheck[0] = currentTurn
        resource_timer.start("getPlanets")
        empirePlanetIDs = list(PlanetUtilsAI.get_owned_planets_by_empire(universe.planetIDs))
        resource_timer.start("Filter")
        resource_timer.start("Priority")
        # TODO: take into acct splintering of resource groups
        # fleetSupplyableSystemIDs = empire.fleetSupplyableSystemIDs
        # fleetSupplyablePlanetIDs = PlanetUtilsAI.get_planets_in__systems_ids(fleetSupplyableSystemIDs)
        ppPrio = foAI.foAIstate.get_priority(AIPriorityType.PRIORITY_RESOURCE_PRODUCTION)
        rpPrio = foAI.foAIstate.get_priority(AIPriorityType.PRIORITY_RESOURCE_RESEARCH)
        priorityRatio = float(rpPrio) / (ppPrio + 0.0001)
        resource_timer.start("Shuffle")
        # not supporting Growth for general planets until also adding code to make sure would actually benefit
        # shuffle(generalPlanetIDs)
        resource_timer.start("Targets")
        planets = map(universe.getPlanet, empirePlanetIDs)
        planetMap.clear()
        planetMap.update(zip(empirePlanetIDs, planets))
        if useGrowth:
            for metab, metabIncPop in ColonisationAI.empire_metabolisms.items():
                for special in [aspec for aspec in AIDependencies.metabolismBoostMap.get(metab, []) if aspec in ColonisationAI.available_growth_specials]:
                    rankedPlanets = []
                    for pid in ColonisationAI.available_growth_specials[special]:
                        planet = planetMap[pid]
                        cur_focus = planet.focus
                        pop = planet.currentMeterValue(fo.meterType.population)
                        if (pop > metabIncPop - 2 * planet.size) or (GFocus not in planet.availableFoci):  # not enough benefit to lose local production, or can't put growth focus here
                            continue
                        for special2 in ["COMPUTRONIUM_SPECIAL"]:
                            if special2 in planet.specials:
                                break
                        else:  # didn't have any specials that would override interest in growth special
                            print "Considering Growth Focus for %s (%d) with special %s; planet has pop %.1f and %s metabolism incremental pop is %.1f" % (
                                planet.name, pid, special, pop, metab, metabIncPop)
                            if cur_focus == GFocus:
                                pop -= 4  # discourage changing current focus to minimize focus-changing penalties
                            rankedPlanets.append((pop, pid, cur_focus))
                    if not rankedPlanets:
                        continue
                    rankedPlanets.sort()
                    print "Considering Growth Focus choice for special %s; possible planet pop, id pairs are %s" % (metab, rankedPlanets)
                    for spSize, spPID, cur_focus in rankedPlanets:  # index 0 should be able to set focus, but just in case...
                        result = 1
                        if cur_focus != GFocus:
                            result = fo.issueChangeFocusOrder(spPID, GFocus)
                        if result == 1:
                            newFoci[spPID] = GFocus
                            if spPID in empirePlanetIDs:
                                del empirePlanetIDs[empirePlanetIDs.index(spPID)]
                            print "%s focus of planet %s (%d) at Growth Focus" % (["set", "left"][cur_focus == GFocus], planetMap[spPID].name, spPID)
                            break
                        else:
                            print "failed setting focus of planet %s (%d) at Growth Focus; focus left at %s" % (planetMap[spPID].name, spPID, planetMap[spPID].focus)
        already_have_comp_moon = False
        for pid in empirePlanetIDs:
            planet = planetMap[pid]
            if "COMPUTRONIUM_SPECIAL" in planet.specials and RFocus in planet.availableFoci and not already_have_comp_moon:
                curFocus = planet.focus
                newFoci[pid] = RFocus
                result = 0
                if curFocus != RFocus:
                    result = fo.issueChangeFocusOrder(pid, RFocus)
                    if result == 1:
                        universe.updateMeterEstimates(empirePlanetIDs)
                if curFocus == RFocus or result == 1:
                    already_have_comp_moon = True
                    print "%s focus of planet %s (%d) (with Computronium Moon) at Research Focus" % (["set", "left"][curFocus == RFocus], planetMap[pid].name, pid)
                    if pid in empirePlanetIDs:
                        del empirePlanetIDs[empirePlanetIDs.index(pid)]
                    continue
            if "HONEYCOMB_SPECIAL" in planet.specials and IFocus in planet.availableFoci:
                curFocus = planet.focus
                newFoci[pid] = IFocus
                result = 0
                if curFocus != IFocus:
                    result = fo.issueChangeFocusOrder(pid, IFocus)
                    if result == 1:
                        universe.updateMeterEstimates(empirePlanetIDs)
                if curFocus == IFocus or result == 1:
                    print "%s focus of planet %s (%d) (with Honeycomb) at Industry Focus" % (["set", "left"][curFocus == IFocus], planetMap[pid].name, pid)
                    if pid in empirePlanetIDs:
                        del empirePlanetIDs[empirePlanetIDs.index(pid)]
                    continue
            if ((([bld.buildingTypeName for bld in map(universe.getObject, planet.buildingIDs) if bld.buildingTypeName in
                    ["BLD_CONC_CAMP", "BLD_CONC_CAMP_REMNANT"]] != []) or
                             ([ccspec for ccspec in planet.specials if ccspec in ["CONC_CAMP_MASTER_SPECIAL", "CONC_CAMP_SLAVE_SPECIAL"]] != []))
                    and IFocus in planet.availableFoci):
                curFocus = planet.focus
                newFoci[pid] = IFocus
                result = 0
                if curFocus != IFocus:
                    result = fo.issueChangeFocusOrder(pid, IFocus)
                    if result == 1:
                        print ("Tried setting %s for Concentration Camp planet %s (%d) with species %s and current focus %s, got result %d and focus %s" %
                               (newFoci[pid], planet.name, pid, planet.speciesName, curFocus, result, planetMap[pid].focus))
                        universe.updateMeterEstimates(empirePlanetIDs)
                    if (result != 1) or planetMap[pid].focus != IFocus:
                        newplanet = universe.getPlanet(pid)
                        print ("Error: Failed setting %s for Concentration Camp planet %s (%d) with species %s and current focus %s, but new planet copy shows %s" %
                               (newFoci[pid], planetMap[pid].name, pid, planetMap[pid].speciesName, planetMap[pid].focus, newplanet.focus))
                if curFocus == IFocus or result == 1:
                    print "%s focus of planet %s (%d) (with Concentration Camps/Remnants) at Industry Focus" % (["set", "left"][curFocus == IFocus], planetMap[pid].name, pid)
                    if pid in empirePlanetIDs:
                        del empirePlanetIDs[empirePlanetIDs.index(pid)]
                    continue
        # pp, rp = get_resource_target_totals(empirePlanetIDs, planetMap)
        pp, rp = get_resource_target_totals(planetMap.keys())

        for pid in empirePlanetIDs:
            planet = planetMap[pid]
            if PFocus in planet.availableFoci and assess_protection_focus(pid):
                curFocus = planet.focus
                newFoci[pid] = PFocus
                result = 0
                if curFocus != PFocus:
                    result = fo.issueChangeFocusOrder(pid, PFocus)
                    if result == 1:
                        print ("Tried setting %s for planet %s (%d) with species %s and current focus %s, got result %d and focus %s" %
                               (newFoci[pid], planet.name, pid, planet.speciesName, curFocus, result, planet.focus))
                        universe.updateMeterEstimates(empirePlanetIDs)
                    if (result != 1) or planet.focus != PFocus:
                        newplanet = universe.getPlanet(pid)
                        print ("Error: Failed setting %s for planet %s (%d) with species %s and current focus %s, but new planet copy shows %s" %
                               (newFoci[pid], planet.name, pid, planet.speciesName, planet.focus, newplanet.focus))
                if curFocus == PFocus or result == 1:
                    print "%s focus of planet %s (%d) at Protection(Defense) Focus" % (["set", "left"][curFocus == PFocus], planet.name, pid)
                    if pid in empirePlanetIDs:
                        del empirePlanetIDs[empirePlanetIDs.index(pid)]
                    continue

        print "\n-----------------------------------------"
        print "Making Planet Focus Change Determinations\n"

        ratios = []
        # for each planet, calculate RP:PP value ratio at which industry/Mining focus and research focus would have the same total value, & sort by that
        # include a bias to slightly discourage changing foci
        curTargetPP = 0.001
        curTargetRP = 0.001
        resource_timer.start("Loop")  # loop
        has_force = tech_is_complete("CON_FRC_ENRG_STRC")
        preset_ids = set(planetMap.keys()) - set(empirePlanetIDs)
        ctPP0, ctRP0 = 0, 0
        for pid in preset_ids:
            nPP, nRP = newTargets.get(pid, {}).get(planetMap[pid].focus, [0, 0])
            curTargetPP += nPP
            curTargetRP += nRP
            iPP, iRP = newTargets.get(pid, {}).get(IFocus, [0, 0])
            ctPP0 += iPP
            ctRP0 += iRP

        id_set = set(empirePlanetIDs)
        for adj_round in [1, 2, 3, 4]:
            maxi_ratio = ctRP0 / max(0.01, ctPP0)  # should only change between rounds 1 and 2
            for pid in list(id_set):
                if adj_round == 1:  # tally max Industry
                    iPP, iRP = newTargets.get(pid, {}).get(IFocus, [0, 0])
                    ctPP0 += iPP
                    ctRP0 += iRP
                    continue
                II, IR = newTargets[pid][IFocus]
                RI, RR = newTargets[pid][RFocus]
                CI, CR = currentOutput[pid][IFocus], currentOutput[pid][RFocus]
                research_penalty = (currentFocus[pid] != RFocus)
                # calculate factor F at which II + F * IR == RI + F * RR =====> F = ( II-RI ) / (RR-IR)
                thisFactor = (II - RI) / max(0.01, RR - IR)  # don't let denominator be zero for planets where focus doesn't change RP
                planet = planetMap[pid]
                if adj_round == 2:  # take research at planets with very cheap research
                    if (maxi_ratio < priorityRatio) and (curTargetRP < priorityRatio * ctPP0) and (thisFactor <= 1.0):
                        curTargetPP += RI
                        curTargetRP += RR
                        newFoci[pid] = RFocus
                        id_set.discard(pid)
                    continue
                if adj_round == 3:  # take research at planets where can do reasonable balance
                    if has_force or (foAI.foAIstate.aggression < fo.aggression.aggressive) or (curTargetRP >= priorityRatio * ctPP0):
                        continue
                    pop = planet.currentMeterValue(fo.meterType.population)
                    t_pop = planet.currentMeterValue(fo.meterType.targetPopulation)
                    # if AI is aggressive+, and this planet in range where temporary Research focus can get an additional RP at cost of 1 PP, and still need some RP, then do it
                    if pop < t_pop - 5:
                        continue
                    if (CI > II + 8) or (((RR > II) or ((RR - CR) >= 1 + 2 * research_penalty)) and ((RR - IR) >= 3) and ((CR - IR) >= 0.7 * ((II - CI) * (1 + 0.1 * research_penalty)))):
                        curTargetPP += CI - 1 - research_penalty
                        curTargetRP += CR + 1
                        newFoci[pid] = RFocus
                        id_set.discard(pid)
                    continue
                # adj_round == 4 assume default IFocus
                curTargetPP += II  # icurTargets initially calculated by Industry focus, which will be our default focus
                curTargetRP += IR
                newFoci[pid] = IFocus
                ratios.append((thisFactor, pid))

        ratios.sort()
        printedHeader = False
        fociMap = {IFocus: "Industry", RFocus: "Research", MFocus: "Mining", GFocus: "Growth", PFocus: "Defense"}
        gotAlgo = tech_is_complete("LRN_ALGO_ELEGANCE")
        for ratio, pid in ratios:
            do_research = False  # (newFoci[pid]==RFocus)
            if (priorityRatio < (curTargetRP / (curTargetPP + 0.0001))) and not do_research:  # we have enough RP
                if ratio < 1.1 and foAI.foAIstate.aggression > fo.aggression.cautious:  # but wait, RP is still super cheap relative to PP, maybe will take more RP
                    if priorityRatio < 1.5 * (curTargetRP / (curTargetPP + 0.0001)):  # yeah, really a glut of RP, stop taking RP
                        break
                else:  # RP not super cheap & we have enough, stop taking it
                    break
            II, IR = newTargets[pid][IFocus]
            RI, RR = newTargets[pid][RFocus]
            # if currentFocus[pid] == MFocus:
            # II = max( II, newTargets[pid][MFocus][0] )
            if (not do_research and (
                   (ratio > 2.0 and curTargetPP < 15 and gotAlgo) or
                   (ratio > 2.5 and curTargetPP < 25 and II > 5 and gotAlgo) or
                   (ratio > 3.0 and curTargetPP < 40 and II > 5 and gotAlgo) or
                   (ratio > 4.0 and curTargetPP < 100 and II > 10) or
                   ((curTargetRP + RR - IR) / max(0.001, curTargetPP - II + RI) > 2 * priorityRatio))):  # we already have algo elegance and more RP would be too expensive, or overkill
                if not printedHeader:
                    printedHeader = True
                    print "Rejecting further Research Focus choices as too expensive:"
                    print "%34s|%20s|%15s |%15s|%15s |%15s |%15s" % ("                      Planet ", " current RP/PP ", " current target RP/PP ", "current Focus ", "  rejectedFocus ", " rejected target RP/PP ", "rejected RP-PP EQF")
                    oldFocus = currentFocus[pid]
                    cPP, cRP = currentOutput[pid][IFocus], currentOutput[pid][RFocus]
                    otPP, otRP = newTargets[pid].get(oldFocus, (0, 0))
                    ntPP, ntRP = newTargets[pid].get(RFocus, (0, 0))
                    print "pID (%3d) %22s | c: %5.1f / %5.1f | cT: %5.1f / %5.1f |  cF: %8s | nF: %8s | cT: %5.1f / %5.1f | %.2f" % (pid, planetMap[pid].name, cRP, cPP, otRP, otPP, fociMap.get(oldFocus, 'unknown'), fociMap[RFocus], ntRP, ntPP, ratio)
                    continue  # RP is getting too expensive, but might be willing to still allocate from a planet with less PP to lose
            # if planetMap[pid].currentMeterValue(fo.meterType.targetPopulation) >0: #only set to research if pop won't die out
            newFoci[pid] = RFocus
            curTargetRP += (RR - IR)
            curTargetPP -= (II - RI)
        print "============================"
        print "Planet Focus Assignments to achieve target RP/PP ratio of %.2f from current ratio of %.2f ( %.1f / %.1f )" % (priorityRatio, rp / (pp + 0.0001), rp, pp)
        print "Max Industry assignments would result in target RP/PP ratio of %.2f ( %.1f / %.1f )" % (ctRP0 / (ctPP0 + 0.0001), ctRP0, ctPP0)
        print "-------------------------------------"
        print "%34s|%20s|%15s |%15s|%15s |%15s " % ("                      Planet ", " current RP/PP ", " current target RP/PP ", "current Focus ", "  newFocus ", " new target RP/PP ")
        totalChanged = 0
        for id_set in empirePlanetIDs, preset_ids:
            for pid in id_set:
                canFocus = planetMap[pid].currentMeterValue(fo.meterType.targetPopulation) > 0
                oldFocus = currentFocus[pid]
                newFocus = newFoci.get(pid, IFocus)
                cPP, cRP = currentOutput[pid][IFocus], currentOutput[pid][RFocus]
                otPP, otRP = newTargets[pid].get(oldFocus, (0, 0))
                ntPP, ntRP = otPP, otRP
                if newFocus != oldFocus and newFocus in planetMap[pid].availableFoci:  # planetMap[pid].focus
                    totalChanged += 1
                    if newFocus != planetMap[pid].focus:
                        result = fo.issueChangeFocusOrder(pid, newFocus)
                        if result == 1:
                            ntPP, ntRP = newTargets[pid].get(newFocus, (0, 0))
                        else:
                            print "Trouble changing focus of planet %s (%d) to %s" % (planetMap[pid].name, pid, newFocus)
                print "pID (%3d) %22s | c: %5.1f / %5.1f | cT: %5.1f / %5.1f |  cF: %8s | nF: %8s | cT: %5.1f / %5.1f " % (pid, planetMap[pid].name, cRP, cPP, otRP, otPP, fociMap.get(oldFocus, 'unknown'), fociMap[newFocus], ntRP, ntPP)
            print "-------------------------------------"
        print "-------------------------------------"
        print "Final Ratio Target (turn %4d) RP/PP : %.2f ( %.1f / %.1f ) after %d Focus changes" % (fo.currentTurn(), curTargetRP / (curTargetPP + 0.0001), curTargetRP, curTargetPP, totalChanged)
        resource_timer.end()
    aPP, aRP = empire.productionPoints, empire.resourceProduction(fo.resourceType.research)
    # Next string used in charts. Don't modify it!
    print "Current Output (turn %4d) RP/PP : %.2f ( %.1f / %.1f )" % (fo.currentTurn(), aRP / (aPP + 0.0001), aRP, aPP)
    print "------------------------"
    print "ResourcesAI Time Requirements:"


def generate_resources_orders():
    """generate resources focus orders"""

    # calculate top resource priority
    # topResourcePriority()

    # set resource foci of planets
    # setCapitalIDResourceFocus()

    # -----------------------------
    # setGeneralPlanetResourceFocus()
    set_planet_resource_foci()

    # ------------------------------
    # setAsteroidsResourceFocus()

    # setGasGiantsResourceFocus()

    print_resources_priority()
    # print "ResourcesAI Time Requirements:"
