"""
ResourcesAI.py provides generate_resources_orders which sets the focus for all of the planets in the empire

The method is to start with a raw list of all of the planets in the empire.
It considers in turn growth factors, production specials, defense requirements
and finally the targetted ratio of research/production. Each decision on a planet
transfers the planet from the raw list to the baked list, until all planets
have their future focus decided.
"""

import freeOrionAIInterface as fo  # pylint: disable=import-error
import FreeOrionAI as foAI
from EnumsAI import PriorityType, get_priority_resource_types, FocusType
import PlanetUtilsAI
import random
import ColonisationAI
import AIDependencies
import FleetUtilsAI
from freeorion_debug import Timer
from freeorion_tools import tech_is_complete

resource_timer = Timer('timer_bucket')

#Local Constants
IFocus = FocusType.FOCUS_INDUSTRY
RFocus = FocusType.FOCUS_RESEARCH
MFocus = FocusType.FOCUS_MINING  # not currently used in content
GFocus = FocusType.FOCUS_GROWTH
PFocus = FocusType.FOCUS_PROTECTION
fociMap = {IFocus: "Industry", RFocus: "Research", MFocus: "Mining", GFocus: "Growth", PFocus: "Defense"}

#TODO use the priorityRatio to weight
RESEARCH_WEIGHTING = 2.0

useGrowth = True
limitAssessments = False

lastFociCheck = [0]


class PlanetFocusInfo(object):
    """ The current, possible and future foci and output of one planet."""
    def __init__(self, planet):
        self.planet = planet
        self.current_focus = planet.focus
        self.current_output = (planet.currentMeterValue(fo.meterType.industry)
                               , planet.currentMeterValue(fo.meterType.research))
        self.possible_output = {}
        itarget = planet.currentMeterValue(fo.meterType.targetIndustry)
        rtarget = planet.currentMeterValue(fo.meterType.targetResearch)
        self.possible_output[self.current_focus] = (itarget, rtarget)
        self.future_focus = self.current_focus


class PlanetFocusManager(object):
    """PlanetFocusManager tracks all of the empire's planets, what their current and future focus will be."""

    def __init__(self):
        universe = fo.getUniverse()

        resource_timer.start("getPlanets")
        planet_ids = list(PlanetUtilsAI.get_owned_planets_by_empire(universe.planetIDs))

        # resource_timer.start("Shuffle")
        # shuffle(generalPlanetIDs)

        resource_timer.start("Targets")
        planet_infos = [PlanetFocusInfo(universe.getPlanet(pid)) for pid in  planet_ids]
        self.all_planet_info = {}
        self.all_planet_info.update(zip(planet_ids, planet_infos))

        self.raw_planet_info = dict(self.all_planet_info)
        self.baked_planet_info = dict()

        for pid, pinfo in self.raw_planet_info.items():
            focusable = pinfo.planet.currentMeterValue(fo.meterType.targetPopulation) > 0
            if not focusable:
                self.baked_planet_info[pid] = self.raw_planet_info.pop(pid)


    def bake_future_focus(self, pid, focus, update=True):
        """Set the focus and moves it from the raw list to the baked list of planets
        Return success or failure
        """
        pinfo = self.raw_planet_info.get(pid)
        success = (pinfo is not None and
                   (pinfo.current_focus == focus
                    or (focus in pinfo.planet.availableFoci
                        and fo.issueChangeFocusOrder(pid, focus))))
        if success:
            pinfo.future_focus = focus
            self.baked_planet_info[pid] = self.raw_planet_info.pop(pid)

            if update and pinfo.current_focus != focus:
                # TODO determine if this should update all planets (for reporting) or just the remaining unplaced planets
                universe = fo.getUniverse()
                universe.updateMeterEstimates(self.raw_planet_info.keys())
                itarget = pinfo.planet.currentMeterValue(fo.meterType.targetIndustry)
                rtarget = pinfo.planet.currentMeterValue(fo.meterType.targetResearch)
                pinfo.possible_output[focus] = (itarget, rtarget)
        return success


    def calculate_planet_infos(self, pids):
        """ Calculates for each possible focus the target output of each planet and stores it in planet info
        It excludes baked planets from consideration.
        Note: The results will not be strictly correct if any planets have global effects
        """
        #TODO this function depends on the specific rule that off-focus meter value are always the minimum value
        universe = fo.getUniverse()
        unbaked_pids = [pid for pid in pids if not pid in self.baked_planet_info]
        planet_infos = [(pid, self.all_planet_info[pid], self.all_planet_info[pid].planet) for pid in unbaked_pids]
        for pid, pinfo, planet in planet_infos:
            if IFocus in planet.availableFoci and planet.focus != IFocus:
                fo.issueChangeFocusOrder(pid, IFocus)  # may not be able to take, but try

        universe.updateMeterEstimates(unbaked_pids)
        for pid, pinfo, planet in planet_infos:
            itarget = planet.currentMeterValue(fo.meterType.targetIndustry)
            rtarget = planet.currentMeterValue(fo.meterType.targetResearch)
            if planet.focus == IFocus:
                pinfo.possible_output[IFocus] = (itarget, rtarget)
                pinfo.possible_output[GFocus] = rtarget
            else:
                pinfo.possible_output[IFocus] = (0, 0)
                pinfo.possible_output[GFocus] = 0
            if RFocus in planet.availableFoci and planet.focus != RFocus:
                fo.issueChangeFocusOrder(pid, RFocus)  # may not be able to take, but try

        universe.updateMeterEstimates(unbaked_pids)
        for pid, pinfo, planet in planet_infos:
            can_focus = planet.currentMeterValue(fo.meterType.targetPopulation) > 0
            itarget = planet.currentMeterValue(fo.meterType.targetIndustry)
            rtarget = planet.currentMeterValue(fo.meterType.targetResearch)
            if planet.focus == RFocus:
                pinfo.possible_output[RFocus] = (itarget, rtarget)
                pinfo.possible_output[GFocus] = (itarget, pinfo.possible_output[GFocus])
            else:
                pinfo.possible_output[RFocus] = (0, 0)
                pinfo.possible_output[GFocus] = (0, pinfo.possible_output[GFocus])
            if can_focus and pinfo.current_focus != planet.focus:
                fo.issueChangeFocusOrder(pid, pinfo.current_focus)  # put it back to what it was

        universe.updateMeterEstimates(unbaked_pids)
        # Protection focus will give the same off-focus Industry and Research targets as Growth Focus
        for pid, pinfo, planet in planet_infos:
            pinfo.possible_output[PFocus] = pinfo.possible_output[GFocus]

class Reporter(object):
    """Reporter contains some file scope functions to report"""

    def __init__(self, focus_manager):
        self.focus_manager = focus_manager
        self.sections = []
        self.captured_ids = set()

    def capture_section_info(self, title):
        """Grab ids of all the newly baked planets."""
        new_captured_ids = set(self.focus_manager.baked_planet_info)
        new_ids = new_captured_ids - self.captured_ids
        if new_ids:
            self.captured_ids = new_captured_ids
            self.sections.append((title, list(new_ids)))

    table_format = "%34s | %17s | %17s  | %13s | %13s  | %17s |"

    @staticmethod
    def print_resource_ai_header():
        print "\n============================"
        print "Collecting info to assess Planet Focus Changes\n"

    @staticmethod
    def print_table_header():
        print "==================================="
        print Reporter.table_format % ("Planet", "current RP/PP", "old target RP/PP"
                                       , "current Focus", "newFocus", "new target RP/PP")

    def print_table_footer(self, priorityRatio):
        current_industry_target = 0
        current_research_target = 0
        all_industry_industry_target = 0
        all_industry_research_target = 0
        all_research_industry_target = 0
        all_research_research_target = 0
        total_changed = 0
        for pinfo in self.focus_manager.all_planet_info.values():
            if pinfo.current_focus != pinfo.future_focus:
                total_changed += 1

            planet = pinfo.planet
            fPP, fRP = pinfo.possible_output[pinfo.future_focus]
            current_industry_target += fPP
            current_research_target += fRP

            iPP, iRP = pinfo.possible_output[IFocus] if IFocus in planet.availableFoci else (fPP, fRP)
            all_industry_industry_target += iPP
            all_industry_research_target += iRP

            rPP, rRP = pinfo.possible_output[RFocus] if RFocus in planet.availableFoci else (fPP, fRP)
            all_research_industry_target += rPP
            all_research_research_target += rRP

        print "-----------------------------------"
        print "Planet Focus Assignments to achieve target RP/PP ratio of %.2f from current ratio of %.2f ( %.1f / %.1f )" \
            % (priorityRatio, current_research_target / (current_industry_target + 0.0001)
               , current_research_target, current_industry_target)
        print "Max Industry assignments would result in target RP/PP ratio of %.2f ( %.1f / %.1f )" \
            % (all_industry_research_target / (all_industry_industry_target + 0.0001)
               , all_industry_research_target, all_industry_industry_target)
        print "Max Research assignments would result in target RP/PP ratio of %.2f ( %.1f / %.1f )" \
            % (all_research_research_target / (all_research_industry_target + 0.0001)
               , all_research_research_target, all_research_industry_target)
        print "-----------------------------------"
        print "Final Ratio Target (turn %4d) RP/PP : %.2f ( %.1f / %.1f ) after %d Focus changes" \
            % (fo.currentTurn(), current_research_target / (current_industry_target + 0.0001)
               , current_research_target, current_industry_target, total_changed)

    def print_table(self, priorityRatio):
        """Prints a table of all of the captured sections of assignments"""
        self.print_table_header()

        for title, id_set in self.sections:
            print Reporter.table_format % (("---------- "+title+" ------------------------------")[:33], "", "", "", "", "")
            id_set.sort()  #pay sort cost only when printing
            for pid in id_set:
                pinfo = self.focus_manager.baked_planet_info[pid]
                oldFocus = pinfo.current_focus
                newFocus = pinfo.future_focus
                cPP, cRP = pinfo.current_output
                otPP, otRP = pinfo.possible_output.get(oldFocus, (0, 0))
                ntPP, ntRP = pinfo.possible_output[newFocus]
                print (Reporter.table_format %
                       ("pID (%3d) %22s" % (pid, pinfo.planet.name[-22:]),
                        "c: %5.1f / %5.1f" % (cRP, cPP),
                        "cT: %5.1f / %5.1f" % (otRP, otPP),
                        "cF: %8s" % fociMap.get(oldFocus, 'unknown'),
                        "nF: %8s" % fociMap.get(newFocus, 'unset'),
                        "cT: %5.1f / %5.1f" % (ntRP, ntPP)))
        self.print_table_footer(priorityRatio)


    @staticmethod
    def print_resource_ai_footer():
        empire = fo.getEmpire()
        aPP, aRP = empire.productionPoints, empire.resourceProduction(fo.resourceType.research)
        # Next string used in charts. Don't modify it!
        print "Current Output (turn %4d) RP/PP : %.2f ( %.1f / %.1f )" % (fo.currentTurn(), aRP / (aPP + 0.0001), aRP, aPP)
        print "------------------------"
        print "ResourcesAI Time Requirements:"


    @staticmethod
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
            print "    ResourcePriority |Score: %s | %s " % (evaluationPair[0], evaluationPair[1])

        # what is the focus of available resource centers?
        print
        warnings = {}
        #TODO combine this with previous table to reduce report duplication?
        print "Planet Resources Foci:"
        for planetID in empirePlanetIDs:
            planet = universe.getPlanet(planetID)
            planetPopulation = planet.currentMeterValue(fo.meterType.population)
            maxPop = planet.currentMeterValue(fo.meterType.targetPopulation)
            if maxPop < 1 and planetPopulation > 0:
                warnings[planet.name] = (planetPopulation, maxPop)
            print "  ID: %d Name: % 18s -- % 6s % 8s  Focus: % 8s Species: %s Pop: %2d/%2d" % (planetID, planet.name, planet.size, planet.type, "_".join(str(planet.focus).split("_")[1:])[:8], planet.speciesName, planetPopulation, maxPop)
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


def assess_protection_focus(pid, pinfo):
    """Return True if planet should use Protection Focus"""
    this_planet = pinfo.planet
    sys_status = foAI.foAIstate.systemStatus.get(this_planet.systemID, {})
    threat_from_supply = (0.25 * foAI.foAIstate.empire_standard_enemy_rating *
                          min(2, len(sys_status.get('enemies_nearly_supplied', []))))
    print "Planet %s has regional+supply threat of %.1f" % ('P_%d<%s>' % (pid, this_planet.name), threat_from_supply)
    regional_threat = sys_status.get('regional_threat', 0) + threat_from_supply
    if not regional_threat:  # no need for protection
        if pinfo.current_focus == PFocus:
            print "Advising dropping Protection Focus at %s due to no regional threat" % this_planet
        return False
    cur_prod_val = weighted_sum_output(pinfo.current_output)
    target_prod_val = max(map(weighted_sum_output, [pinfo.possible_output[IFocus], pinfo.possible_output[RFocus]]))
    prot_prod_val = weighted_sum_output(pinfo.possible_output[PFocus])
    local_production_diff = 0.8 * cur_prod_val + 0.2 * target_prod_val - prot_prod_val
    fleet_threat = sys_status.get('fleetThreat', 0)
    # TODO: relax the below rejection once the overall determination of PFocus is better tuned
    if not fleet_threat and local_production_diff > 8:
        if pinfo.current_focus == PFocus:
            print "Advising dropping Protection Focus at %s due to excessive productivity loss" % this_planet
        return False
    local_p_defenses = sys_status.get('mydefenses', {}).get('overall', 0)
    # TODO have adjusted_p_defenses take other in-system planets into account
    adjusted_p_defenses = local_p_defenses * (1.0 if pinfo.current_focus != PFocus else 0.5)
    local_fleet_rating = sys_status.get('myFleetRating', 0)
    combined_local_defenses = sys_status.get('all_local_defenses', 0)
    my_neighbor_rating = sys_status.get('my_neighbor_rating', 0)
    neighbor_threat = sys_status.get('neighborThreat', 0)
    safety_factor = 1.2 if pinfo.current_focus == PFocus else 0.5
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
            (pinfo.current_focus != PFocus or  # too late to start protection TODO: but maybe regen worth it
             # protection forcus only useful here if it maintains an elevated level
             all([AIDependencies.PROT_FOCUS_MULTIPLIER * a <= b for a, b in def_meter_pairs]))):
        use_protection = False
        reason = "A"
    elif ((pinfo.current_focus != PFocus and cur_shield < max_shield - 2 and
           not tech_is_complete(AIDependencies.PLANET_BARRIER_I_TECH)) and
          (cur_defense < max_defense - 2 and not tech_is_complete(AIDependencies.DEFENSE_REGEN_1_TECH)) and
          (cur_troops < max_troops - 2)):
        use_protection = False
        reason = "B1"
    elif ((pinfo.current_focus == PFocus and cur_shield*AIDependencies.PROT_FOCUS_MULTIPLIER < max_shield-2 and
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
          (pinfo.current_focus != PFocus or
           (0.5 * safety_factor * regional_threat <= local_fleet_rating and
            fleet_threat == 0 and neighbor_threat < combined_local_defenses and
            local_production_diff > 5))):
        use_protection = False
        reason = "F"
    elif (regional_threat <= FleetUtilsAI.combine_ratings(local_fleet_rating, adjusted_p_defenses) and
          safety_factor * regional_threat <=
          FleetUtilsAI.combine_ratings_list([my_neighbor_rating, local_fleet_rating, adjusted_p_defenses]) and
          local_production_diff > 5):
        use_protection = False
        reason = "G"
    if use_protection or pinfo.current_focus == PFocus:
        print ("Advising %sProtection Focus (reason %s) for planet %s, with local_prod_diff of %.1f, comb. local"
               " defenses %.1f, local fleet rating %.1f and regional threat %.1f, threat sources: %s") % (
                   ["dropping ", ""][use_protection], reason, this_planet, local_production_diff, combined_local_defenses,
                   local_fleet_rating, regional_threat, sys_status['regional_fleet_threats'])
    return use_protection


def use_planet_growth_specials(focus_manager):
    '''set resource foci of planets with potentially useful growth factors. Remove planets from list of candidates.'''
    if useGrowth:
        # TODO: also consider potential future benefit re currently unpopulated planets
        for metab, metabIncPop in ColonisationAI.empire_metabolisms.items():
            for special in [aspec for aspec in AIDependencies.metabolismBoostMap.get(metab, []) if aspec in ColonisationAI.available_growth_specials]:
                rankedPlanets = []
                for pid in ColonisationAI.available_growth_specials[special]:
                    pinfo = focus_manager.all_planet_info[pid]
                    planet = pinfo.planet
                    pop = planet.currentMeterValue(fo.meterType.population)
                    if (pop > metabIncPop - 2 * planet.size) or (GFocus not in planet.availableFoci):  # not enough benefit to lose local production, or can't put growth focus here
                        continue
                    for special2 in ["COMPUTRONIUM_SPECIAL"]:
                        if special2 in planet.specials:
                            break
                    else:  # didn't have any specials that would override interest in growth special
                        print "Considering Growth Focus for %s (%d) with special %s; planet has pop %.1f and %s metabolism incremental pop is %.1f" % (
                            planet.name, pid, special, pop, metab, metabIncPop)
                        if pinfo.current_focus == GFocus:
                            pop -= 4  # discourage changing current focus to minimize focus-changing penalties
                            rankedPlanets.append((pop, pid, pinfo.current_focus))
                if not rankedPlanets:
                    continue
                rankedPlanets.sort()
                print "Considering Growth Focus choice for special %s; possible planet pop, id pairs are %s" % (metab, rankedPlanets)
                for spSize, spPID, cur_focus in rankedPlanets:  # index 0 should be able to set focus, but just in case...
                    planet = focus_manager.all_planet_info[spPID].planet
                    if focus_manager.bake_future_focus(spPID, GFocus):
                        print "%s focus of planet %s (%d) at Growth Focus" % (["set", "left"][cur_focus == GFocus], planet.name, spPID)
                        break
                    else:
                        print "failed setting focus of planet %s (%d) at Growth Focus; focus left at %s" % (planet.name, spPID, planet.focus)


def use_planet_production_and_research_specials(focus_manager):
    '''Use production and research specials as appropriate.  Remove planets from list of candidates.'''
    #TODO remove reliance on rules knowledge.  Just scan for specials with production
    #and research bonuses and use what you find. Perhaps maintain a list
    # of know types of specials
    universe = fo.getUniverse()
    already_have_comp_moon = False
    for pid, pinfo in focus_manager.raw_planet_info.items():
        planet = pinfo.planet
        if "COMPUTRONIUM_SPECIAL" in planet.specials and RFocus in planet.availableFoci and not already_have_comp_moon:
            if focus_manager.bake_future_focus(pid, RFocus):
                already_have_comp_moon = True
                print "%s focus of planet %s (%d) (with Computronium Moon) at Research Focus" % (["set", "left"][pinfo.current_focus == RFocus], planet.name, pid)
                continue
        if "HONEYCOMB_SPECIAL" in planet.specials and IFocus in planet.availableFoci:
            if focus_manager.bake_future_focus(pid, IFocus):
                print "%s focus of planet %s (%d) (with Honeycomb) at Industry Focus" % (["set", "left"][pinfo.current_focus == IFocus], planet.name, pid)
                continue
        if ((([bld.buildingTypeName for bld in map(universe.getObject, planet.buildingIDs) if bld.buildingTypeName in
               ["BLD_CONC_CAMP", "BLD_CONC_CAMP_REMNANT"]] != [])
             or ([ccspec for ccspec in planet.specials if ccspec in
                  ["CONC_CAMP_MASTER_SPECIAL", "CONC_CAMP_SLAVE_SPECIAL"]] != []))
                and IFocus in planet.availableFoci):
            if focus_manager.bake_future_focus(pid, IFocus):
                if pinfo.current_focus != IFocus:
                    print ("Tried setting %s for Concentration Camp planet %s (%d) with species %s and current focus %s, got result %d and focus %s" %
                           (pinfo.future_focus, planet.name, pid, planet.speciesName, pinfo.current_focus, True, planet.focus))
                print "%s focus of planet %s (%d) (with Concentration Camps/Remnants) at Industry Focus" % (["set", "left"][pinfo.current_focus == IFocus], planet.name, pid)
                continue
            else:
                newplanet = universe.getPlanet(pid)
                print ("Error: Failed setting %s for Concentration Camp planet %s (%d) with species %s and current focus %s, but new planet copy shows %s" %
                       (pinfo.future_focus, planet.name, pid, planet.speciesName, planet.focus, newplanet.focus))


def set_planet_protection_foci(focus_manager):
    '''Assess and set protection foci'''
    universe = fo.getUniverse()
    for pid, pinfo in focus_manager.raw_planet_info.items():
        planet = pinfo.planet
        if PFocus in planet.availableFoci and assess_protection_focus(pid, pinfo):
            curFocus = planet.focus
            if focus_manager.bake_future_focus(pid, PFocus):
                if curFocus != PFocus:
                    print ("Tried setting %s for planet %s (%d) with species %s and current focus %s, got result %d and focus %s" %
                           (pinfo.future_focus, planet.name, pid, planet.speciesName, curFocus, True, planet.focus))
                print "%s focus of planet %s (%d) at Protection(Defense) Focus" % (["set", "left"][curFocus == PFocus], planet.name, pid)
                continue
            else:
                newplanet = universe.getPlanet(pid)
                print ("Error: Failed setting %s for planet %s (%d) with species %s and current focus %s, but new planet copy shows %s" %
                       (focus_manager.new_foci[pid], planet.name, pid, planet.speciesName, planet.focus, newplanet.focus))


def set_planet_happiness_foci(focus_manager):
    """Assess and set planet focus to preferred focus depending on happiness"""
    #TODO Assess need to set planet to preferred focus to improve happiness


def set_planet_industry_and_research_foci(focus_manager, priorityRatio):
    """Adjust planet's industry versus research focus while targetting the given ratio and avoiding penalties from changing focus"""
    print "\n-----------------------------------------"
    print "Making Planet Focus Change Determinations\n"

    ratios = []
    # for each planet, calculate RP:PP value ratio at which industry/Mining focus and research focus would have the same total value, & sort by that
    # include a bias to slightly discourage changing foci
    curTargetPP = 0.001
    curTargetRP = 0.001
    resource_timer.start("Loop")  # loop
    has_force = tech_is_complete("CON_FRC_ENRG_STRC")
    #cumulative all industry focus
    ctPP0, ctRP0 = 0, 0

    #Handle presets which only have possible output for preset focus
    for pid, pinfo in focus_manager.baked_planet_info.items():
        nPP, nRP = pinfo.possible_output[pinfo.future_focus]
        curTargetPP += nPP
        curTargetRP += nRP
        ctPP0 += nPP
        ctRP0 += nRP

    # tally max Industry
    for pid, pinfo in focus_manager.raw_planet_info.items():
        iPP, iRP = pinfo.possible_output[IFocus]
        ctPP0 += iPP
        ctRP0 += iRP
        if RFocus not in pinfo.planet.availableFoci:
            focus_manager.bake_future_focus(pid, pinfo.current_focus, False)

    #smallest possible ratio of research to industry with an all industry focus
    maxi_ratio = ctRP0 / max(0.01, ctPP0)

    for adj_round in [2, 3, 4]:
        for pid, pinfo in focus_manager.raw_planet_info.items():
            II, IR = pinfo.possible_output[IFocus]
            RI, RR = pinfo.possible_output[RFocus]
            CI, CR = pinfo.current_output
            research_penalty = (pinfo.current_focus != RFocus)
            # calculate factor F at which II + F * IR == RI + F * RR =====> F = ( II-RI ) / (RR-IR)
            thisFactor = (II - RI) / max(0.01, RR - IR)  # don't let denominator be zero for planets where focus doesn't change RP
            planet = pinfo.planet
            if adj_round == 2:  # take research at planets with very cheap research
                if (maxi_ratio < priorityRatio) and (curTargetRP < priorityRatio * ctPP0) and (thisFactor <= 1.0):
                    curTargetPP += RI
                    curTargetRP += RR
                    focus_manager.bake_future_focus(pid, RFocus, False)
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
                    focus_manager.bake_future_focus(pid, RFocus, False)
                continue
            if adj_round == 4: #assume default IFocus
                curTargetPP += II  # icurTargets initially calculated by Industry focus, which will be our default focus
                curTargetRP += IR
                ratios.append((thisFactor, pid, pinfo))

    ratios.sort()
    printedHeader = False
    gotAlgo = tech_is_complete("LRN_ALGO_ELEGANCE")
    for ratio, pid, pinfo in ratios:
        do_research = False  # (focus_manager.new_foci[pid]==RFocus)
        if (priorityRatio < (curTargetRP / (curTargetPP + 0.0001))) and not do_research:  # we have enough RP
            if ratio < 1.1 and foAI.foAIstate.aggression > fo.aggression.cautious:  # but wait, RP is still super cheap relative to PP, maybe will take more RP
                if priorityRatio < 1.5 * (curTargetRP / (curTargetPP + 0.0001)):  # yeah, really a glut of RP, stop taking RP
                    break
            else:  # RP not super cheap & we have enough, stop taking it
                break
        II, IR = pinfo.possible_output[IFocus]
        RI, RR = pinfo.possible_output[RFocus]
        # if focus_manager.current_focus[pid] == MFocus:
        # II = max( II, focus_manager.possible_output[MFocus][0] )
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
            oldFocus = pinfo.current_focus
            cPP, cRP = pinfo.current_output
            otPP, otRP = pinfo.possible_output[oldFocus]
            ntPP, ntRP = pinfo.possible_output[RFocus]
            print "pID (%3d) %22s | c: %5.1f / %5.1f | cT: %5.1f / %5.1f |  cF: %8s | nF: %8s | cT: %5.1f / %5.1f | %.2f" % (pid, pinfo.planet.name, cRP, cPP, otRP, otPP, fociMap.get(oldFocus, 'unknown'), fociMap[RFocus], ntRP, ntPP, ratio)
            continue  # RP is getting too expensive, but might be willing to still allocate from a planet with less PP to lose
        # if focus_manager.planet_map[pid].currentMeterValue(fo.meterType.targetPopulation) >0: #only set to research if pop won't die out
        focus_manager.bake_future_focus(pid, RFocus, False)
        curTargetRP += (RR - IR)
        curTargetPP -= (II - RI)

    #Any planet in the ratios list and still raw is set to industry
    for ratio, pid, pinfo in ratios:
        if pid in focus_manager.raw_planet_info:
            focus_manager.bake_future_focus(pid, IFocus, False)


def set_planet_resource_foci():
    """set resource focus of planets """

    Reporter.print_resource_ai_header()
    currentTurn = fo.currentTurn()
    # set the random seed (based on galaxy seed, empire ID and current turn)
    # for game-reload consistency
    freq = min(3, (max(5, currentTurn - 80)) / 4.0) ** (1.0 / 3)
    if not (limitAssessments and (abs(currentTurn - lastFociCheck[0]) < 1.5 * freq) and (random.random() < 1.0 / freq)):
        lastFociCheck[0] = currentTurn
        resource_timer.start("Filter")
        resource_timer.start("Priority")
        # TODO: take into acct splintering of resource groups
        # fleetSupplyableSystemIDs = empire.fleetSupplyableSystemIDs
        # fleetSupplyablePlanetIDs = PlanetUtilsAI.get_planets_in__systems_ids(fleetSupplyableSystemIDs)
        ppPrio = foAI.foAIstate.get_priority(PriorityType.RESOURCE_PRODUCTION)
        rpPrio = foAI.foAIstate.get_priority(PriorityType.RESOURCE_RESEARCH)
        priorityRatio = float(rpPrio) / (ppPrio + 0.0001)

        focus_manager = PlanetFocusManager()

        reporter = Reporter(focus_manager)
        reporter.capture_section_info("Unfocusable")

        use_planet_growth_specials(focus_manager)
        use_planet_production_and_research_specials(focus_manager)
        reporter.capture_section_info("Specials")

        focus_manager.calculate_planet_infos(focus_manager.raw_planet_info.keys())

        set_planet_protection_foci(focus_manager)
        reporter.capture_section_info("Protection")

        set_planet_happiness_foci(focus_manager)
        reporter.capture_section_info("Happiness")

        set_planet_industry_and_research_foci(focus_manager, priorityRatio)
        reporter.capture_section_info("Typical")

        reporter.print_table(priorityRatio)

        resource_timer.end()

    Reporter.print_resource_ai_footer()


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

    Reporter.print_resources_priority()
    # print "ResourcesAI Time Requirements:"
