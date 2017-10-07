"""
ResourcesAI.py provides generate_resources_orders which sets the focus for all of the planets in the empire

The method is to start with a raw list of all of the planets in the empire.
It considers in turn growth factors, production specials, defense requirements
and finally the targeted ratio of research/production. Each decision on a planet
transfers the planet from the raw list to the baked list, until all planets
have their future focus decided.
"""
# Note: The algorithm is not stable with respect to pid order.  i.e. Two empire with
#       exactly the same colonies, but different pids may make different choices.
import freeOrionAIInterface as fo  # pylint: disable=import-error
import FreeOrionAI as foAI
from EnumsAI import PriorityType, get_priority_resource_types, FocusType
import PlanetUtilsAI
import random
import ColonisationAI
import AIDependencies
import CombatRatingsAI
from common.print_utils import Table, Text
from freeorion_tools import tech_is_complete, AITimer
from common.configure_logging import convenience_function_references_for_logger
(debug, info, warn, error, fatal) = convenience_function_references_for_logger(__name__)

resource_timer = AITimer('timer_bucket')

# Local Constants
INDUSTRY = FocusType.FOCUS_INDUSTRY
RESEARCH = FocusType.FOCUS_RESEARCH
GROWTH = FocusType.FOCUS_GROWTH
PROTECTION = FocusType.FOCUS_PROTECTION
_focus_names = {INDUSTRY: "Industry", RESEARCH: "Research", GROWTH: "Growth", PROTECTION: "Defense"}

# TODO use the priorityRatio to weight
RESEARCH_WEIGHTING = 2.3

limitAssessments = False

lastFociCheck = [0]


class PlanetFocusInfo(object):
    """ The current, possible and future foci and output of one planet."""
    def __init__(self, planet):
        self.planet = planet
        self.current_focus = planet.focus
        self.current_output = (planet.currentMeterValue(fo.meterType.industry),
                               planet.currentMeterValue(fo.meterType.research))
        self.possible_output = {}
        industry_target = planet.currentMeterValue(fo.meterType.targetIndustry)
        research_target = planet.currentMeterValue(fo.meterType.targetResearch)
        self.possible_output[self.current_focus] = (industry_target, research_target)
        self.future_focus = self.current_focus


class PlanetFocusManager(object):
    """PlanetFocusManager tracks all of the empire's planets, what their current and future focus will be."""

    def __init__(self):
        universe = fo.getUniverse()

        resource_timer.start("getPlanets")
        planet_ids = list(PlanetUtilsAI.get_owned_planets_by_empire(universe.planetIDs))

        resource_timer.start("Targets")
        self.all_planet_info = {pid: PlanetFocusInfo(universe.getPlanet(pid)) for pid in planet_ids}

        self.raw_planet_info = dict(self.all_planet_info)
        self.baked_planet_info = {}

        for pid, pinfo in self.raw_planet_info.items():
            if not pinfo.planet.availableFoci:
                self.baked_planet_info[pid] = self.raw_planet_info.pop(pid)

    def bake_future_focus(self, pid, focus, update=True):
        """Set the focus and moves it from the raw list to the baked list of planets.

        pid -- pid
        focus -- future focus to use
        update -- If update is True then the meters of the raw planets will be updated.
                  If the planet's change of focus will have a global effect (growth,
                  production or research special), then update should be True.
        Return success or failure
        """
        pinfo = self.raw_planet_info.get(pid)
        success = bool(pinfo is not None and
                       (pinfo.current_focus == focus
                        or (focus in pinfo.planet.availableFoci
                            and fo.issueChangeFocusOrder(pid, focus))))
        if success:
            if update and pinfo.current_focus != focus:
                universe = fo.getUniverse()
                universe.updateMeterEstimates(self.raw_planet_info.keys())
                industry_target = pinfo.planet.currentMeterValue(fo.meterType.targetIndustry)
                research_target = pinfo.planet.currentMeterValue(fo.meterType.targetResearch)
                pinfo.possible_output[focus] = (industry_target, research_target)

            pinfo.future_focus = focus
            self.baked_planet_info[pid] = self.raw_planet_info.pop(pid)
        return success

    def calculate_planet_infos(self, pids):
        """ Calculates for each possible focus the target output of each planet and stores it in planet info
        It excludes baked planets from consideration.
        Note: The results will not be strictly correct if any planets have global effects
        """
        # TODO this function depends on the specific rule that off-focus meter value are always the minimum value
        universe = fo.getUniverse()
        unbaked_pids = [pid for pid in pids if pid not in self.baked_planet_info]
        planet_infos = [(pid, self.all_planet_info[pid], self.all_planet_info[pid].planet) for pid in unbaked_pids]
        for pid, pinfo, planet in planet_infos:
            if INDUSTRY in planet.availableFoci and planet.focus != INDUSTRY:
                fo.issueChangeFocusOrder(pid, INDUSTRY)  # may not be able to take, but try

        universe.updateMeterEstimates(unbaked_pids)
        for pid, pinfo, planet in planet_infos:
            industry_target = planet.currentMeterValue(fo.meterType.targetIndustry)
            research_target = planet.currentMeterValue(fo.meterType.targetResearch)
            if planet.focus == INDUSTRY:
                pinfo.possible_output[INDUSTRY] = (industry_target, research_target)
                pinfo.possible_output[GROWTH] = research_target
            else:
                pinfo.possible_output[INDUSTRY] = (0, 0)
                pinfo.possible_output[GROWTH] = 0
            if RESEARCH in planet.availableFoci and planet.focus != RESEARCH:
                fo.issueChangeFocusOrder(pid, RESEARCH)  # may not be able to take, but try

        universe.updateMeterEstimates(unbaked_pids)
        for pid, pinfo, planet in planet_infos:
            industry_target = planet.currentMeterValue(fo.meterType.targetIndustry)
            research_target = planet.currentMeterValue(fo.meterType.targetResearch)
            if planet.focus == RESEARCH:
                pinfo.possible_output[RESEARCH] = (industry_target, research_target)
                pinfo.possible_output[GROWTH] = (industry_target, pinfo.possible_output[GROWTH])
            else:
                pinfo.possible_output[RESEARCH] = (0, 0)
                pinfo.possible_output[GROWTH] = (0, pinfo.possible_output[GROWTH])
            if pinfo.planet.availableFoci and pinfo.current_focus != planet.focus:
                fo.issueChangeFocusOrder(pid, pinfo.current_focus)  # put it back to what it was

        universe.updateMeterEstimates(unbaked_pids)
        # Protection focus will give the same off-focus Industry and Research targets as Growth Focus
        for pid, pinfo, planet in planet_infos:
            pinfo.possible_output[PROTECTION] = pinfo.possible_output[GROWTH]


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
        print Reporter.table_format % ("Planet", "current RP/PP", "old target RP/PP",
                                       "current Focus", "newFocus", "new target RP/PP")

    def print_table_footer(self, priority_ratio):
        current_industry_target = 0
        current_research_target = 0
        new_industry_target = 0
        new_research_target = 0
        all_industry_industry_target = 0
        all_industry_research_target = 0
        all_research_industry_target = 0
        all_research_research_target = 0
        total_changed = 0
        for pinfo in self.focus_manager.all_planet_info.values():
            if pinfo.current_focus != pinfo.future_focus:
                total_changed += 1

            old_pp, old_rp = pinfo.possible_output[pinfo.current_focus]
            current_industry_target += old_pp
            current_research_target += old_rp

            future_pp, future_rp = pinfo.possible_output[pinfo.future_focus]
            new_industry_target += future_pp
            new_research_target += future_rp

            industry_pp, industry_rp = pinfo.possible_output[INDUSTRY] if INDUSTRY in pinfo.possible_output else (future_pp, future_rp)
            all_industry_industry_target += industry_pp
            all_industry_research_target += industry_rp

            research_pp, research_rp = pinfo.possible_output[RESEARCH] if RESEARCH in pinfo.possible_output else (future_pp, future_rp)
            all_research_industry_target += research_pp
            all_research_research_target += research_rp

        print "-----------------------------------"
        print "Planet Focus Assignments to achieve target RP/PP ratio of %.2f from current target ratio of %.2f ( %.1f / %.1f )" \
            % (priority_ratio, current_research_target / (current_industry_target + 0.0001),
               current_research_target, current_industry_target)
        print "Max Industry assignments would result in target RP/PP ratio of %.2f ( %.1f / %.1f )" \
            % (all_industry_research_target / (all_industry_industry_target + 0.0001),
               all_industry_research_target, all_industry_industry_target)
        print "Max Research assignments would result in target RP/PP ratio of %.2f ( %.1f / %.1f )" \
            % (all_research_research_target / (all_research_industry_target + 0.0001),
               all_research_research_target, all_research_industry_target)
        print "-----------------------------------"
        print "Final Ratio Target (turn %4d) RP/PP : %.2f ( %.1f / %.1f ) after %d Focus changes" \
            % (fo.currentTurn(), new_research_target / (new_industry_target + 0.0001),
               new_research_target, new_industry_target, total_changed)

    def print_table(self, priority_ratio):
        """Prints a table of all of the captured sections of assignments."""
        self.print_table_header()

        for title, id_set in self.sections:
            print Reporter.table_format % (("---------- " + title + " ------------------------------")[:33], "", "", "", "", "")
            id_set.sort()  # pay sort cost only when printing
            for pid in id_set:
                pinfo = self.focus_manager.baked_planet_info[pid]
                old_focus = pinfo.current_focus
                new_focus = pinfo.future_focus
                current_pp, curren_rp = pinfo.current_output
                ot_pp, ot_rp = pinfo.possible_output.get(old_focus, (0, 0))
                nt_pp, nt_rp = pinfo.possible_output[new_focus]
                print (Reporter.table_format %
                       ("pID (%3d) %22s" % (pid, pinfo.planet.name[-22:]),
                        "c: %5.1f / %5.1f" % (curren_rp, current_pp),
                        "cT: %5.1f / %5.1f" % (ot_rp, ot_pp),
                        "cF: %8s" % _focus_names.get(old_focus, 'unknown'),
                        "nF: %8s" % _focus_names.get(new_focus, 'unset'),
                        "cT: %5.1f / %5.1f" % (nt_rp, nt_pp)))
        self.print_table_footer(priority_ratio)

    @staticmethod
    def print_resource_ai_footer():
        empire = fo.getEmpire()
        pp, rp = empire.productionPoints, empire.resourceProduction(fo.resourceType.research)
        # Next string used in charts. Don't modify it!
        print "Current Output (turn %4d) RP/PP : %.2f ( %.1f / %.1f )" % (fo.currentTurn(), rp / (pp + 0.0001), rp, pp)
        print "------------------------"
        print "ResourcesAI Time Requirements:"

    @staticmethod
    def print_resources_priority():
        """Calculate top resource priority."""
        universe = fo.getUniverse()
        empire = fo.getEmpire()
        empire_planet_ids = PlanetUtilsAI.get_owned_planets_by_empire(universe.planetIDs)
        print "Resource Priorities:"
        resource_priorities = {}
        for priority_type in get_priority_resource_types():
            resource_priorities[priority_type] = foAI.foAIstate.get_priority(priority_type)

        sorted_priorities = resource_priorities.items()
        sorted_priorities.sort(lambda x, y: cmp(x[1], y[1]), reverse=True)
        top_priority = -1
        for evaluation_priority, evaluation_score in sorted_priorities:
            if top_priority < 0:
                top_priority = evaluation_priority
            print "  %s: %.2f" % (evaluation_priority, evaluation_score)

        # what is the focus of available resource centers?
        print
        warnings = {}
        foci_table = Table([
                Text('Planet'),
                Text('Size'),
                Text('Type'),
                Text('Focus'),
                Text('Species'),
                Text('Pop')
            ], table_name="Planetary Foci Overview Turn %d" % fo.currentTurn())
        for pid in empire_planet_ids:
            planet = universe.getPlanet(pid)
            population = planet.currentMeterValue(fo.meterType.population)
            max_population = planet.currentMeterValue(fo.meterType.targetPopulation)
            if max_population < 1 and population > 0:
                warnings[planet.name] = (population, max_population)
            foci_table.add_row([
                planet,
                planet.size,
                planet.type,
                "_".join(str(planet.focus).split("_")[1:])[:8],
                planet.speciesName,
                "%.1f/%.1f" % (population, max_population)
            ])
        foci_table.print_table()
        print "Empire Totals:\nPopulation: %5d \nProduction: %5d\nResearch: %5d\n" % (
            empire.population(), empire.productionPoints, empire.resourceProduction(fo.resourceType.research))
        for name, (cp, mp) in warnings.iteritems():
            print "Population Warning! -- %s has unsustainable current pop %d -- target %d" % (name, cp, mp)


def weighted_sum_output(outputs):
    """Return a weighted sum of planetary output.
    :param outputs: (industry, research)
    :return: weighted sum of industry and research
    """
    return outputs[0] + RESEARCH_WEIGHTING * outputs[1]


def assess_protection_focus(pid, pinfo):
    """Return True if planet should use Protection Focus."""
    this_planet = pinfo.planet
    sys_status = foAI.foAIstate.systemStatus.get(this_planet.systemID, {})
    threat_from_supply = (0.25 * foAI.foAIstate.empire_standard_enemy_rating *
                          min(2, len(sys_status.get('enemies_nearly_supplied', []))))
    print "Planet %s has regional+supply threat of %.1f" % ('P_%d<%s>' % (pid, this_planet.name), threat_from_supply)
    regional_threat = sys_status.get('regional_threat', 0) + threat_from_supply
    if not regional_threat:  # no need for protection
        if pinfo.current_focus == PROTECTION:
            print "Advising dropping Protection Focus at %s due to no regional threat" % this_planet
        return False
    cur_prod_val = weighted_sum_output(pinfo.current_output)
    target_prod_val = max(map(weighted_sum_output, [pinfo.possible_output[INDUSTRY], pinfo.possible_output[RESEARCH]]))
    prot_prod_val = weighted_sum_output(pinfo.possible_output[PROTECTION])
    local_production_diff = 0.8 * cur_prod_val + 0.2 * target_prod_val - prot_prod_val
    fleet_threat = sys_status.get('fleetThreat', 0)
    # TODO: relax the below rejection once the overall determination of PFocus is better tuned
    if not fleet_threat and local_production_diff > 8:
        if pinfo.current_focus == PROTECTION:
            print "Advising dropping Protection Focus at %s due to excessive productivity loss" % this_planet
        return False
    local_p_defenses = sys_status.get('mydefenses', {}).get('overall', 0)
    # TODO have adjusted_p_defenses take other in-system planets into account
    adjusted_p_defenses = local_p_defenses * (1.0 if pinfo.current_focus != PROTECTION else 0.5)
    local_fleet_rating = sys_status.get('myFleetRating', 0)
    combined_local_defenses = sys_status.get('all_local_defenses', 0)
    my_neighbor_rating = sys_status.get('my_neighbor_rating', 0)
    neighbor_threat = sys_status.get('neighborThreat', 0)
    safety_factor = 1.2 if pinfo.current_focus == PROTECTION else 0.5
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
            (pinfo.current_focus != PROTECTION or  # too late to start protection TODO: but maybe regen worth it
             # protection focus only useful here if it maintains an elevated level
             all([AIDependencies.PROT_FOCUS_MULTIPLIER * a <= b for a, b in def_meter_pairs]))):
        use_protection = False
        reason = "A"
    elif ((pinfo.current_focus != PROTECTION and cur_shield < max_shield - 2 and
           not tech_is_complete(AIDependencies.PLANET_BARRIER_I_TECH)) and
          (cur_defense < max_defense - 2 and not tech_is_complete(AIDependencies.DEFENSE_REGEN_1_TECH)) and
          (cur_troops < max_troops - 2)):
        use_protection = False
        reason = "B1"
    elif ((pinfo.current_focus == PROTECTION and cur_shield * AIDependencies.PROT_FOCUS_MULTIPLIER < max_shield - 2 and
           not tech_is_complete(AIDependencies.PLANET_BARRIER_I_TECH)) and
          (cur_defense * AIDependencies.PROT_FOCUS_MULTIPLIER < max_defense - 2 and
           not tech_is_complete(AIDependencies.DEFENSE_REGEN_1_TECH)) and
          (cur_troops * AIDependencies.PROT_FOCUS_MULTIPLIER < max_troops - 2)):
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
          (pinfo.current_focus != PROTECTION or
           (0.5 * safety_factor * regional_threat <= local_fleet_rating and
            fleet_threat == 0 and neighbor_threat < combined_local_defenses and
            local_production_diff > 5))):
        use_protection = False
        reason = "F"
    elif (regional_threat <= CombatRatingsAI.combine_ratings(local_fleet_rating, adjusted_p_defenses) and
          safety_factor * regional_threat <=
          CombatRatingsAI.combine_ratings_list([my_neighbor_rating, local_fleet_rating, adjusted_p_defenses]) and
          local_production_diff > 5):
        use_protection = False
        reason = "G"
    if use_protection or pinfo.current_focus == PROTECTION:
        print ("Advising %sProtection Focus (reason %s) for planet %s, with local_prod_diff of %.1f, comb. local"
               " defenses %.1f, local fleet rating %.1f and regional threat %.1f, threat sources: %s") % (
                   ["dropping ", ""][use_protection], reason, this_planet, local_production_diff, combined_local_defenses,
                   local_fleet_rating, regional_threat, sys_status['regional_fleet_threats'])
    return use_protection


def set_planet_growth_specials(focus_manager):
    """set resource foci of planets with potentially useful growth factors. Remove planets from list of candidates."""
    if not foAI.foAIstate.character.may_use_growth_focus():
        return

    # TODO Consider actual resource output of the candidate locations rather than only population
    for special, locations in ColonisationAI.available_growth_specials.iteritems():
        # Find which metabolism is boosted by this special
        metabolism = AIDependencies.metabolismBoosts.get(special)
        if not metabolism:
            warn("Entry in available growth special not mapped to a metabolism")
            continue

        # Find the total population bonus we could get by using growth focus
        potential_pop_increase = ColonisationAI.empire_metabolisms.get(metabolism, 0)
        if not potential_pop_increase:
            continue

        debug("Considering setting growth focus for %s at locations %s for potential population bonus of %.1f" % (
            special, locations, potential_pop_increase))

        # Find the best suited planet to use growth special on, i.e. the planet where
        # we will lose the least amount of resource generation when using growth focus.
        def _print_evaluation(evaluation):
            """Local helper function printing a formatted evaluation."""
            debug("  - %s %s" % (planet, evaluation))
        ranked_planets = []
        for pid in locations:
            pinfo = focus_manager.all_planet_info[pid]
            planet = pinfo.planet
            if GROWTH not in planet.availableFoci:
                _print_evaluation("has no growth focus available.")
                continue

            # the increased population on the planet using this growth focus
            # is mostly wasted, so ignore it for now.
            pop = planet.currentMeterValue(fo.meterType.population)
            pop_gain = potential_pop_increase - planet.size
            if pop > pop_gain:
                _print_evaluation("would lose more pop (%.1f) than gain everywhere else (%.1f)." % (pop, pop_gain))
                continue

            # If we have a computronium special here, then research focus will have higher priority.
            if AIDependencies.COMPUTRONIUM_SPECIAL in planet.specials and RESEARCH in planet.availableFoci:
                _print_evaluation("has a usable %s" % AIDependencies.COMPUTRONIUM_SPECIAL)
                continue

            _print_evaluation("considered (pop %.1f, growth gain %.1f, current focus %s)" % (
                pop, pop_gain, pinfo.current_focus))

            # add a bias to discourage switching out growth focus to avoid focus change penalties
            if pinfo.current_focus == GROWTH:
                pop -= 4

            ranked_planets.append((pop, pid, planet))

        if not ranked_planets:
            debug("  --> No suitable location found.")
            continue

        # sort possible locations by population in ascending order and set population
        # bonus at the planet with lowest possible population loss.
        ranked_planets.sort()
        for pop, pid, planet in ranked_planets:
            if focus_manager.bake_future_focus(pid, GROWTH):
                debug("  --> Using growth focus at %s" % planet)
                break
        else:
            warn("  --> Failed to set growth focus at all candidate locations.")


def set_planet_production_and_research_specials(focus_manager):
    """Set production and research specials.
    Sets production/research specials for known (COMPUTRONIUM, HONEYCOMB and CONC_CAMP)
    production/research specials.
    Remove planets from list of candidates using bake_future_focus."""
    # TODO remove reliance on rules knowledge.  Just scan for specials with production
    # and research bonuses and use what you find. Perhaps maintain a list
    # of know types of specials
    # TODO use "best" COMPUTRON planet instead of first found, where "best" means least industry loss,
    # least threatened, no foci change penalty etc.
    universe = fo.getUniverse()
    already_have_comp_moon = False
    for pid, pinfo in focus_manager.raw_planet_info.items():
        planet = pinfo.planet
        if AIDependencies.COMPUTRONIUM_SPECIAL in planet.specials and RESEARCH in planet.availableFoci and not already_have_comp_moon:
            if focus_manager.bake_future_focus(pid, RESEARCH):
                already_have_comp_moon = True
                print "%s focus of planet %s (%d) (with Computronium Moon) at Research Focus" % (["set", "left"][pinfo.current_focus == RESEARCH], planet.name, pid)
                continue
        if "HONEYCOMB_SPECIAL" in planet.specials and INDUSTRY in planet.availableFoci:
            if focus_manager.bake_future_focus(pid, INDUSTRY):
                print "%s focus of planet %s (%d) (with Honeycomb) at Industry Focus" % (["set", "left"][pinfo.current_focus == INDUSTRY], planet.name, pid)
                continue
        if ((([bld.buildingTypeName for bld in map(universe.getBuilding, planet.buildingIDs) if bld.buildingTypeName in
               ["BLD_CONC_CAMP", "BLD_CONC_CAMP_REMNANT"]])
             or ([ccspec for ccspec in planet.specials if ccspec in
                  ["CONC_CAMP_MASTER_SPECIAL", "CONC_CAMP_SLAVE_SPECIAL"]]))
                and INDUSTRY in planet.availableFoci):
            if focus_manager.bake_future_focus(pid, INDUSTRY):
                print "%s focus of planet %s (%d) (with Concentration Camps/Remnants) at Industry Focus" % (["set", "left"][pinfo.current_focus == INDUSTRY], planet.name, pid)
                continue
            else:
                new_planet = universe.getPlanet(pid)
                warn("Failed setting %s for Concentration Camp planet %s (%d)"
                     " with species %s and current focus %s, but new planet copy shows %s" % (
                        pinfo.future_focus, planet.name, pid, planet.speciesName, planet.focus, new_planet.focus))


def set_planet_protection_foci(focus_manager):
    """Assess and set protection foci"""
    universe = fo.getUniverse()
    for pid, pinfo in focus_manager.raw_planet_info.items():
        planet = pinfo.planet
        if PROTECTION in planet.availableFoci and assess_protection_focus(pid, pinfo):
            current_focus = planet.focus
            if focus_manager.bake_future_focus(pid, PROTECTION):
                if current_focus != PROTECTION:
                    print ("Tried setting %s for planet %s (%d) with species %s and current focus %s, got result %d and focus %s" %
                           (pinfo.future_focus, planet.name, pid, planet.speciesName, current_focus, True, planet.focus))
                print "%s focus of planet %s (%d) at Protection(Defense) Focus" % (["set", "left"][current_focus == PROTECTION], planet.name, pid)
                continue
            else:
                newplanet = universe.getPlanet(pid)
                warn("Failed setting %s for planet %s (%d) with species %s and current focus %s,"
                     " but new planet copy shows %s" % (focus_manager.new_foci[pid], planet.name, pid,
                                                        planet.speciesName, planet.focus, newplanet.focus))


def set_planet_happiness_foci(focus_manager):
    """Assess and set planet focus to preferred focus depending on happiness."""
    # TODO Assess need to set planet to preferred focus to improve happiness
    pass


def set_planet_industry_and_research_foci(focus_manager, priority_ratio):
    """Adjust planet's industry versus research focus while targeting the given ratio and
     avoiding penalties from changing focus."""
    print "\n-----------------------------------------"
    print "Making Planet Focus Change Determinations\n"

    ratios = []
    # for each planet, calculate RP:PP value ratio at which industry focus and
    # research focus would have the same total value, & sort by that include a
    # bias to slightly discourage changing foci
    target_pp = 0.001
    target_rp = 0.001
    resource_timer.start("Loop")  # loop
    has_force = tech_is_complete("CON_FRC_ENRG_STRC")
    # cumulative all industry focus
    cumulative_pp, cumulative_rp = 0, 0

    # Handle presets which only have possible output for preset focus
    for pid, pinfo in focus_manager.baked_planet_info.items():
        future_pp, future_rp = pinfo.possible_output[pinfo.future_focus]
        target_pp += future_pp
        target_rp += future_rp
        cumulative_pp += future_pp
        cumulative_rp += future_rp

    # tally max Industry
    for pid, pinfo in focus_manager.raw_planet_info.items():
        i_pp, i_rp = pinfo.possible_output[INDUSTRY]
        cumulative_pp += i_pp
        cumulative_rp += i_rp
        if RESEARCH not in pinfo.planet.availableFoci:
            if focus_manager.bake_future_focus(pid, INDUSTRY, False):
                target_pp += i_pp
                target_rp += i_rp

    # smallest possible ratio of research to industry with an all industry focus
    maxi_ratio = cumulative_rp / max(0.01, cumulative_pp)

    for adj_round in [2, 3, 4]:
        for pid, pinfo in focus_manager.raw_planet_info.items():
            ii, tr = pinfo.possible_output[INDUSTRY]
            ri, rr = pinfo.possible_output[RESEARCH]
            ci, cr = pinfo.current_output
            research_penalty = (pinfo.current_focus != RESEARCH)
            # calculate factor F at which ii + F * tr == ri + F * rr =====> F = ( ii-ri ) / (rr-tr)
            factor = (ii - ri) / max(0.01, rr - tr)  # don't let denominator be zero for planets where focus doesn't change RP
            planet = pinfo.planet
            if adj_round == 2:  # take research at planets with very cheap research
                if (maxi_ratio < priority_ratio) and (target_rp < priority_ratio * cumulative_pp) and (factor <= 1.0):
                    target_pp += ri
                    target_rp += rr
                    focus_manager.bake_future_focus(pid, RESEARCH, False)
                continue
            if adj_round == 3:  # take research at planets where can do reasonable balance
                if has_force or foAI.foAIstate.character.may_dither_focus_to_gain_research() or (target_rp >= priority_ratio * cumulative_pp):
                    continue
                pop = planet.currentMeterValue(fo.meterType.population)
                t_pop = planet.currentMeterValue(fo.meterType.targetPopulation)
                # if AI is aggressive+, and this planet in range where temporary Research focus can get an additional RP at cost of 1 PP, and still need some RP, then do it
                if pop < t_pop - 5:
                    continue
                if (ci > ii + 8) or (((rr > ii) or ((rr - cr) >= 1 + 2 * research_penalty)) and ((rr - tr) >= 3) and ((cr - tr) >= 0.7 * ((ii - ci) * (1 + 0.1 * research_penalty)))):
                    target_pp += ci - 1 - research_penalty
                    target_rp += cr + 1
                    focus_manager.bake_future_focus(pid, RESEARCH, False)
                continue
            if adj_round == 4:  # assume default IFocus
                target_pp += ii  # icurTargets initially calculated by Industry focus, which will be our default focus
                target_rp += tr
                ratios.append((factor, pid, pinfo))

    ratios.sort()
    printed_header = False
    got_algo = tech_is_complete("LRN_ALGO_ELEGANCE")
    for ratio, pid, pinfo in ratios:
        if priority_ratio < (target_rp / (target_pp + 0.0001)):  # we have enough RP
            if ratio < 1.1 and foAI.foAIstate.character.may_research_heavily():  # but wait, RP is still super cheap relative to PP, maybe will take more RP
                if priority_ratio < 1.5 * (target_rp / (target_pp + 0.0001)):  # yeah, really a glut of RP, stop taking RP
                    break
            else:  # RP not super cheap & we have enough, stop taking it
                break
        ii, tr = pinfo.possible_output[INDUSTRY]
        ri, rr = pinfo.possible_output[RESEARCH]
        # if focus_manager.current_focus[pid] == MFocus:
        # ii = max( ii, focus_manager.possible_output[MFocus][0] )
        if ((ratio > 2.0 and target_pp < 15 and got_algo) or
            (ratio > 2.5 and target_pp < 25 and ii > 5 and got_algo) or
            (ratio > 3.0 and target_pp < 40 and ii > 5 and got_algo) or
            (ratio > 4.0 and target_pp < 100 and ii > 10) or
            ((target_rp + rr - tr) / max(0.001, target_pp - ii + ri) > 2 * priority_ratio)):  # we already have algo elegance and more RP would be too expensive, or overkill
            if not printed_header:
                printed_header = True
                print "Rejecting further Research Focus choices as too expensive:"
                print "%34s|%20s|%15s |%15s|%15s |%15s |%15s" % ("                      Planet ", " current RP/PP ", " current target RP/PP ", "current Focus ", "  rejectedFocus ", " rejected target RP/PP ", "rejected RP-PP EQF")
            old_focus = pinfo.current_focus
            c_pp, c_rp = pinfo.current_output
            ot_pp, ot_rp = pinfo.possible_output[old_focus]
            nt_pp, nt_rp = pinfo.possible_output[RESEARCH]
            print "pID (%3d) %22s | c: %5.1f / %5.1f | cT: %5.1f / %5.1f |  cF: %8s | nF: %8s | cT: %5.1f / %5.1f | %.2f" % (pid, pinfo.planet.name, c_rp, c_pp, ot_rp, ot_pp, _focus_names.get(old_focus, 'unknown'), _focus_names[RESEARCH], nt_rp, nt_pp, ratio)
            continue  # RP is getting too expensive, but might be willing to still allocate from a planet with less PP to lose
        # if focus_manager.planet_map[pid].currentMeterValue(fo.meterType.targetPopulation) >0: #only set to research if pop won't die out
        focus_manager.bake_future_focus(pid, RESEARCH, False)
        target_rp += (rr - tr)
        target_pp -= (ii - ri)

    # Any planet still raw is set to industry
    for pid in focus_manager.raw_planet_info.keys():
        focus_manager.bake_future_focus(pid, INDUSTRY, False)


def set_planet_resource_foci():
    """set resource focus of planets """

    Reporter.print_resource_ai_header()
    turn = fo.currentTurn()
    # set the random seed (based on galaxy seed, empire ID and current turn)
    # for game-reload consistency
    freq = min(3, (max(5, turn - 80)) / 4.0) ** (1.0 / 3)
    if not (limitAssessments and (abs(turn - lastFociCheck[0]) < 1.5 * freq) and (random.random() < 1.0 / freq)):
        lastFociCheck[0] = turn
        resource_timer.start("Filter")
        resource_timer.start("Priority")
        # TODO: take into acct splintering of resource groups
        # fleetSupplyableSystemIDs = empire.fleetSupplyableSystemIDs
        # fleetSupplyablePlanetIDs = PlanetUtilsAI.get_planets_in__systems_ids(fleetSupplyableSystemIDs)
        production_priority = foAI.foAIstate.get_priority(PriorityType.RESOURCE_PRODUCTION)
        research_priority = foAI.foAIstate.get_priority(PriorityType.RESOURCE_RESEARCH)
        priority_ratio = float(research_priority) / (production_priority + 0.0001)

        focus_manager = PlanetFocusManager()

        reporter = Reporter(focus_manager)
        reporter.capture_section_info("Unfocusable")

        set_planet_growth_specials(focus_manager)
        set_planet_production_and_research_specials(focus_manager)
        reporter.capture_section_info("Specials")

        focus_manager.calculate_planet_infos(focus_manager.raw_planet_info.keys())

        set_planet_protection_foci(focus_manager)
        reporter.capture_section_info("Protection")

        set_planet_happiness_foci(focus_manager)
        reporter.capture_section_info("Happiness")

        set_planet_industry_and_research_foci(focus_manager, priority_ratio)
        reporter.capture_section_info("Typical")

        reporter.print_table(priority_ratio)

        resource_timer.stop_print_and_clear()

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
