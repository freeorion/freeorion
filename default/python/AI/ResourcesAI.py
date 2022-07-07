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
import freeOrionAIInterface as fo
from logging import debug, info, warning
from operator import itemgetter

import AIDependencies
import PlanetUtilsAI
from aistate_interface import get_aistate
from colonization.calculate_planet_colonization_rating import empire_metabolisms
from common.print_utils import Table, Text
from empire.growth_specials import get_growth_specials
from EnumsAI import FocusType, PriorityType, get_priority_resource_types
from freeorion_tools import (
    combine_ratings,
    get_named_real,
    policy_is_adopted,
    tech_is_complete,
)
from freeorion_tools.statistics import stats
from freeorion_tools.timers import AITimer
from PolicyAI import algo_research, bureaucracy

resource_timer = AITimer("timer_bucket")

# Local Constants
INDUSTRY = FocusType.FOCUS_INDUSTRY
RESEARCH = FocusType.FOCUS_RESEARCH
GROWTH = FocusType.FOCUS_GROWTH
PROTECTION = FocusType.FOCUS_PROTECTION
INFLUENCE = FocusType.FOCUS_INFLUENCE


def _focus_name(focus: str) -> str:
    _known_names = {
        INDUSTRY: "Industry",
        RESEARCH: "Research",
        GROWTH: "Growth",
        PROTECTION: "Defense",
        INFLUENCE: "Influence",
    }
    return _known_names.get(focus, focus)


class PlanetFocusInfo:
    """The current, possible and future foci and output of one planet."""

    def __init__(self, planet):
        self.planet = planet
        self.current_focus = planet.focus
        self.current_output = (
            planet.currentMeterValue(fo.meterType.industry),
            planet.currentMeterValue(fo.meterType.research),
            planet.currentMeterValue(fo.meterType.influence),
        )
        self.possible_output = {}
        industry_target = planet.currentMeterValue(fo.meterType.targetIndustry)
        research_target = planet.currentMeterValue(fo.meterType.targetResearch)
        influence_target = planet.currentMeterValue(fo.meterType.targetInfluence)
        self.possible_output[self.current_focus] = (industry_target, research_target, influence_target)
        self.future_focus = self.current_focus


class PlanetFocusManager:
    """PlanetFocusManager tracks all of the empire's planets, what their current and future focus will be."""

    def __init__(self):
        universe = fo.getUniverse()

        resource_timer.start("getPlanets")
        planet_ids = list(PlanetUtilsAI.get_owned_planets_by_empire())

        resource_timer.start("Targets")
        self.all_planet_info = {pid: PlanetFocusInfo(universe.getPlanet(pid)) for pid in planet_ids}

        self.raw_planet_info = dict(self.all_planet_info)
        self.baked_planet_info = {}
        for pid, pinfo in list(self.raw_planet_info.items()):
            if not pinfo.planet.availableFoci:
                self.baked_planet_info[pid] = self.raw_planet_info.pop(pid)
        aistate = get_aistate()
        self.priority = (
            aistate.get_priority(PriorityType.RESOURCE_PRODUCTION),
            aistate.get_priority(PriorityType.RESOURCE_RESEARCH),
            aistate.get_priority(PriorityType.RESOURCE_INFLUENCE),
        )

    def bake_future_focus(self, pid, focus, update=True, force=False):
        """Set the focus and moves it from the raw list to the baked list of planets.

        pid -- pid
        focus -- future focus to use
        update -- If update is True then the meters of the raw planets will be updated.
                  If the planet's change of focus will have a global effect (growth,
                  production or research special), then update should be True.
        Return success or failure
        """
        pinfo = self.raw_planet_info.get(pid)
        if not pinfo:
            return False
        if (focus == INDUSTRY or focus == RESEARCH) and not force:
            idx = 0 if focus == INDUSTRY else 1
            # check for influence instead
            focus_gain = pinfo.possible_output[focus][idx] - pinfo.possible_output[INFLUENCE][idx]
            influence_gain = pinfo.possible_output[INFLUENCE][2] - pinfo.possible_output[focus][2]
            debug(
                f"{pinfo.planet.name} current: {_focus_name(pinfo.current_focus)}, requested: {_focus_name(focus)}, "
                f"requested_gain: {focus_gain}, influence_gain: {influence_gain}"
            )
            if influence_gain * self.priority[2] > focus_gain * self.priority[idx]:
                debug(
                    f"Choosing influence over {_focus_name(focus)}."
                    f" {influence_gain:.2f} * {self.priority[2]:.1f}"
                    f" > {focus_gain:.2f} * {self.priority[idx]:.1f}"
                )
                focus = INFLUENCE
        last_turn = fo.currentTurn()
        if (  # define idx constants for accessing these tuples
            focus != PROTECTION
            and focus != pinfo.current_focus
            and policy_is_adopted(bureaucracy)
            and pinfo.current_output[2] + 0.2 < pinfo.possible_output[pinfo.current_focus][2]
            and pinfo.planet.LastTurnColonized != last_turn
            and pinfo.planet.LastTurnConquered != last_turn
        ):
            # avoid going down too much by repeated focus changes while having bureaucracy
            info(
                "Avoid focus change on %s due to bureaucracy. IPprod: current=%.2f, target=%.2f",
                pinfo.planet.name,
                pinfo.current_output[2],
                pinfo.possible_output[pinfo.current_focus][2],
            )
            return False

        success = bool(
            pinfo.current_focus == focus
            or (focus in pinfo.planet.availableFoci and fo.issueChangeFocusOrder(pid, focus))
        )
        if success:
            if update and pinfo.current_focus != focus:
                universe = fo.getUniverse()
                universe.updateMeterEstimates(self.raw_planet_info.keys())
                industry_target = pinfo.planet.currentMeterValue(fo.meterType.targetIndustry)
                research_target = pinfo.planet.currentMeterValue(fo.meterType.targetResearch)
                influence_target = pinfo.planet.currentMeterValue(fo.meterType.targetInfluence)
                pinfo.possible_output[focus] = (industry_target, research_target, influence_target)

            pinfo.future_focus = focus
            self.baked_planet_info[pid] = self.raw_planet_info.pop(pid)
        return success

    def calculate_planet_infos(self, pids):
        """Calculates for each possible focus the target output of each planet and stores it in planet info
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

        def merge(po1, po2):
            """merge two partially determined possible_outputs"""
            return (
                po1[0] or po2[0],
                po1[1] or po2[1],
                po1[2] or po2[2],
            )

        universe.updateMeterEstimates(unbaked_pids)
        for pid, pinfo, planet in planet_infos:
            po = pinfo.possible_output
            industry_target = planet.currentMeterValue(fo.meterType.targetIndustry)
            research_target = planet.currentMeterValue(fo.meterType.targetResearch)
            influence_target = planet.currentMeterValue(fo.meterType.targetInfluence)
            if planet.focus == INDUSTRY:
                # each focus only affects its own meter
                po[INDUSTRY] = (industry_target, research_target, influence_target)
                po[GROWTH] = (0, research_target, influence_target)
                po[INFLUENCE] = (0, research_target, 0)
            else:
                po[INDUSTRY] = (0, 0, 0)
                po[GROWTH] = (0, 0, 0)
                po[INFLUENCE] = (0, 0, 0)
            if RESEARCH in planet.availableFoci and planet.focus != RESEARCH:
                fo.issueChangeFocusOrder(pid, RESEARCH)  # may not be able to take, but try

        universe.updateMeterEstimates(unbaked_pids)
        for pid, pinfo, planet in planet_infos:
            po = pinfo.possible_output
            industry_target = planet.currentMeterValue(fo.meterType.targetIndustry)
            research_target = planet.currentMeterValue(fo.meterType.targetResearch)
            influence_target = planet.currentMeterValue(fo.meterType.targetInfluence)
            if planet.focus == RESEARCH:
                po[RESEARCH] = (industry_target, research_target, influence_target)
                po[GROWTH] = merge(po[GROWTH], (industry_target, 0, influence_target))
                po[INFLUENCE] = merge(po[INFLUENCE], (industry_target, 0, 0))
            else:
                po[RESEARCH] = (0, 0, 0)
            if INFLUENCE in planet.availableFoci and planet.focus != INFLUENCE:
                fo.issueChangeFocusOrder(pid, INFLUENCE)  # may not be able to take, but try

        universe.updateMeterEstimates(unbaked_pids)
        for pid, pinfo, planet in planet_infos:
            po = pinfo.possible_output
            industry_target = planet.currentMeterValue(fo.meterType.targetIndustry)
            research_target = planet.currentMeterValue(fo.meterType.targetResearch)
            influence_target = planet.currentMeterValue(fo.meterType.targetInfluence)
            if planet.focus == INFLUENCE:
                po[INFLUENCE] = (industry_target, research_target, influence_target)
            else:
                po[INFLUENCE] = (0, 0, 0)
            if pinfo.planet.availableFoci and pinfo.current_focus != planet.focus:
                fo.issueChangeFocusOrder(pid, pinfo.current_focus)  # put it back to what it was

        universe.updateMeterEstimates(unbaked_pids)
        # Protection focus will give the same off-focus Industry and Research targets as Growth Focus
        for pid, pinfo, planet in planet_infos:
            po = pinfo.possible_output
            po[PROTECTION] = po[GROWTH]
            for focus in [INDUSTRY, RESEARCH, INFLUENCE, PROTECTION]:
                debug(
                    "Resource output of planet %s for focus %s is %f/%f/%f",
                    pinfo.planet,
                    focus,
                    po[focus][0],
                    po[focus][1],
                    po[focus][2],
                )


class Reporter:
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
        debug("\n============================")
        debug("Collecting info to assess Planet Focus Changes\n")

    @staticmethod
    def print_table_header():
        debug("===================================")
        debug(
            Reporter.table_format,
            "Planet",
            "current RP/PP",
            "old target RP/PP",
            "current Focus",
            "newFocus",
            "new target RP/PP",
        )

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

            old_pp, old_rp, _ = pinfo.possible_output[pinfo.current_focus]
            current_industry_target += old_pp
            current_research_target += old_rp

            future_pp, future_rp, _ = pinfo.possible_output[pinfo.future_focus]
            new_industry_target += future_pp
            new_research_target += future_rp

            industry_pp, industry_rp, _ = (
                pinfo.possible_output[INDUSTRY] if INDUSTRY in pinfo.possible_output else (future_pp, future_rp, 0)
            )
            all_industry_industry_target += industry_pp
            all_industry_research_target += industry_rp

            research_pp, research_rp, _ = (
                pinfo.possible_output[RESEARCH] if RESEARCH in pinfo.possible_output else (future_pp, future_rp, 0)
            )
            all_research_industry_target += research_pp
            all_research_research_target += research_rp

        debug("-----------------------------------")
        debug(
            "Planet Focus Assignments to achieve target RP/PP ratio of %.2f"
            " from current target ratio of %.2f ( %.1f / %.1f )",
            priority_ratio,
            current_research_target / (current_industry_target + 0.0001),
            current_research_target,
            current_industry_target,
        )
        debug(
            "Max Industry assignments would result in target RP/PP ratio of %.2f ( %.1f / %.1f )",
            all_industry_research_target / (all_industry_industry_target + 0.0001),
            all_industry_research_target,
            all_industry_industry_target,
        )
        debug(
            "Max Research assignments would result in target RP/PP ratio of %.2f ( %.1f / %.1f )",
            all_research_research_target / (all_research_industry_target + 0.0001),
            all_research_research_target,
            all_research_industry_target,
        )
        debug("-----------------------------------")
        debug(
            "Final Ratio Target (turn %4d) RP/PP : %.2f ( %.1f / %.1f ) after %d Focus changes",
            fo.currentTurn(),
            new_research_target / (new_industry_target + 0.0001),
            new_research_target,
            new_industry_target,
            total_changed,
        )

    def print_table(self, priority_ratio):
        """Prints a table of all of the captured sections of assignments."""
        self.print_table_header()

        for title, id_set in self.sections:
            debug(
                Reporter.table_format,
                ("---------- " + title + " ------------------------------")[:33],
                "",
                "",
                "",
                "",
                "",
            )
            id_set.sort()  # pay sort cost only when printing
            for pid in id_set:
                pinfo = self.focus_manager.baked_planet_info[pid]
                old_focus = pinfo.current_focus
                new_focus = pinfo.future_focus
                current_pp, curren_rp, _ = pinfo.current_output
                ot_pp, ot_rp, _ = pinfo.possible_output.get(old_focus, (0, 0))
                nt_pp, nt_rp, _ = pinfo.possible_output[new_focus]
                debug(
                    Reporter.table_format,
                    "pID (%3d) %22s" % (pid, pinfo.planet.name[-22:]),
                    "c: %5.1f / %5.1f" % (curren_rp, current_pp),
                    "cT: %5.1f / %5.1f" % (ot_rp, ot_pp),
                    "cF: %8s" % _focus_name(old_focus),
                    "nF: %8s" % _focus_name(new_focus),
                    "cT: %5.1f / %5.1f" % (nt_rp, nt_pp),
                )
        self.print_table_footer(priority_ratio)

    @staticmethod
    def dump_output():
        empire = fo.getEmpire()
        pp, rp = empire.productionPoints, empire.resourceProduction(fo.resourceType.research)
        stats.output(fo.currentTurn(), rp, pp)

    @staticmethod
    def print_resources_priority():
        """Calculate top resource priority."""
        universe = fo.getUniverse()
        empire = fo.getEmpire()
        empire_planet_ids = PlanetUtilsAI.get_owned_planets_by_empire()
        debug("Resource Priorities:")
        resource_priorities = {}
        aistate = get_aistate()
        for priority_type in get_priority_resource_types():
            resource_priorities[priority_type] = aistate.get_priority(priority_type)

        sorted_priorities = sorted(resource_priorities.items(), key=itemgetter(1), reverse=True)
        top_priority = -1
        for evaluation_priority, evaluation_score in sorted_priorities:
            if top_priority < 0:
                top_priority = evaluation_priority
            debug("  %s: %.2f", evaluation_priority, evaluation_score)

        # what is the focus of available resource centers?
        debug("")
        warnings = {}
        foci_table = Table(
            Text("Planet"),
            Text("Size"),
            Text("Type"),
            Text("Focus"),
            Text("Species"),
            Text("Pop"),
            table_name="Planetary Foci Overview Turn %d" % fo.currentTurn(),
        )
        for pid in empire_planet_ids:
            planet = universe.getPlanet(pid)
            population = planet.currentMeterValue(fo.meterType.population)
            max_population = planet.currentMeterValue(fo.meterType.targetPopulation)
            if max_population < 1 and population > 0:
                warnings[planet.name] = (population, max_population)
            foci_table.add_row(
                planet,
                planet.size,
                planet.type,
                "_".join(str(planet.focus).split("_")[1:])[:8],
                planet.speciesName,
                "%.1f/%.1f" % (population, max_population),
            )
        foci_table.print_table(info)
        debug(
            "Empire Totals:\nPopulation: %5d \nProduction: %5d\nResearch: %5d\n",
            empire.population(),
            empire.productionPoints,
            empire.resourceProduction(fo.resourceType.research),
        )
        for name, (cp, mp) in warnings.items():
            warning("Population Warning! -- %s has unsustainable current pop %d -- target %d", name, cp, mp)


def weighted_sum_output(op):
    """Return a weighted sum of planetary output.
    :param tuple (outputs, priorities) for industry, research and influence),
    :return: weighted sum of industry and research
    """
    return op[0][0] * op[1][0] + op[0][1] * op[1][1] + op[0][2] * op[1][2]


def assess_protection_focus(pinfo, priority):
    """Return True if planet should use Protection Focus."""
    this_planet = pinfo.planet
    # this is unrelated to military threats
    stability_bonus = (pinfo.current_focus == PROTECTION) * get_named_real("PROTECION_FOCUS_STABILITY_BONUS")
    # industry and research produce nothing below
    threshold = -1 * (pinfo.current_focus not in (INDUSTRY, RESEARCH))
    # Negative IP lowers stability. Trying to counter this by setting planets to Protection just makes it worse!
    ip = fo.getEmpire().resourceAvailable(fo.resourceType.influence)
    if ip >= 0 and this_planet.currentMeterValue(fo.meterType.targetHappiness) < threshold + stability_bonus:
        debug("Advising Protection Focus at %s to avoid rebellion", this_planet)
        return True
    aistate = get_aistate()
    sys_status = aistate.systemStatus.get(this_planet.systemID, {})
    threat_from_supply = (
        0.25 * aistate.empire_standard_enemy_rating * min(2, len(sys_status.get("enemies_nearly_supplied", [])))
    )
    debug("%s has regional+supply threat of %.1f", this_planet, threat_from_supply)
    regional_threat = sys_status.get("regional_threat", 0) + threat_from_supply
    if not regional_threat:  # no need for protection
        if pinfo.current_focus == PROTECTION:
            debug("Advising dropping Protection Focus at %s due to no regional threat", this_planet)
        return False
    cur_prod_val = weighted_sum_output((pinfo.current_output, priority))
    target_prod_val = max(
        map(
            weighted_sum_output,
            [
                (pinfo.possible_output[INDUSTRY], priority),
                (pinfo.possible_output[RESEARCH], priority),
                (pinfo.possible_output[INFLUENCE], priority),
            ],
        )
    )
    prot_prod_val = weighted_sum_output((pinfo.possible_output[PROTECTION], priority))
    local_production_diff = 0.5 * cur_prod_val + 0.5 * target_prod_val - prot_prod_val
    fleet_threat = sys_status.get("fleetThreat", 0)
    # TODO: relax the below rejection once the overall determination of PFocus is better tuned
    # priorities have a magnitude of 50
    if not fleet_threat and local_production_diff > 200:
        if pinfo.current_focus == PROTECTION:
            debug("Advising dropping Protection Focus at %s due to excessive productivity loss", this_planet)
        return False
    local_p_defenses = sys_status.get("mydefenses", {}).get("overall", 0)
    # TODO have adjusted_p_defenses take other in-system planets into account
    adjusted_p_defenses = local_p_defenses * (1.0 if pinfo.current_focus != PROTECTION else 0.5)
    local_fleet_rating = sys_status.get("myFleetRating", 0)
    combined_local_defenses = sys_status.get("all_local_defenses", 0)
    my_neighbor_rating = sys_status.get("my_neighbor_rating", 0)
    neighbor_threat = sys_status.get("neighborThreat", 0)
    safety_factor = 1.2 if pinfo.current_focus == PROTECTION else 0.5
    cur_shield = this_planet.initialMeterValue(fo.meterType.shield)
    max_shield = this_planet.initialMeterValue(fo.meterType.maxShield)
    cur_troops = this_planet.initialMeterValue(fo.meterType.troops)
    max_troops = this_planet.initialMeterValue(fo.meterType.maxTroops)
    cur_defense = this_planet.initialMeterValue(fo.meterType.defense)
    max_defense = this_planet.initialMeterValue(fo.meterType.maxDefense)
    def_meter_pairs = [(cur_troops, max_troops), (cur_shield, max_shield), (cur_defense, max_defense)]
    use_protection = True
    reason = ""
    if fleet_threat and (  # i.e., an enemy is sitting on us
        pinfo.current_focus != PROTECTION
        or  # too late to start protection TODO: but maybe regen worth it
        # protection focus only useful here if it maintains an elevated level
        all([AIDependencies.PROT_FOCUS_MULTIPLIER * a <= b for a, b in def_meter_pairs])
    ):
        use_protection = False
        reason = "A"
    elif (
        (
            pinfo.current_focus != PROTECTION
            and cur_shield < max_shield - 2
            and not tech_is_complete(AIDependencies.PLANET_BARRIER_I_TECH)
        )
        and (cur_defense < max_defense - 2 and not tech_is_complete(AIDependencies.DEFENSE_REGEN_1_TECH))
        and (cur_troops < max_troops - 2)
    ):
        use_protection = False
        reason = "B1"
    elif (
        (
            pinfo.current_focus == PROTECTION
            and cur_shield * AIDependencies.PROT_FOCUS_MULTIPLIER < max_shield - 2
            and not tech_is_complete(AIDependencies.PLANET_BARRIER_I_TECH)
        )
        and (
            cur_defense * AIDependencies.PROT_FOCUS_MULTIPLIER < max_defense - 2
            and not tech_is_complete(AIDependencies.DEFENSE_REGEN_1_TECH)
        )
        and (cur_troops * AIDependencies.PROT_FOCUS_MULTIPLIER < max_troops - 2)
    ):
        use_protection = False
        reason = "B2"
    elif max(max_shield, max_troops, max_defense) < 3:
        # joke defenses, don't bother with protection focus
        use_protection = False
        reason = "C"
    elif regional_threat and local_production_diff <= 2.0:
        use_protection = True
        reason = "D"
    elif safety_factor * regional_threat <= local_fleet_rating:
        use_protection = False
        reason = "E"
    elif safety_factor * regional_threat <= combined_local_defenses and (
        pinfo.current_focus != PROTECTION
        or (
            0.5 * safety_factor * regional_threat <= local_fleet_rating
            and fleet_threat == 0
            and neighbor_threat < combined_local_defenses
            and local_production_diff > 5
        )
    ):
        use_protection = False
        reason = "F"
    elif (
        regional_threat <= combine_ratings(local_fleet_rating, adjusted_p_defenses)
        and safety_factor * regional_threat
        <= combine_ratings(my_neighbor_rating, local_fleet_rating, adjusted_p_defenses)
        and local_production_diff > 5
    ):
        use_protection = False
        reason = "G"
    if use_protection or pinfo.current_focus == PROTECTION:
        debug(
            "Advising %sProtection Focus (reason %s) for planet %s, with local_prod_diff of %.1f, comb. local"
            " defenses %.1f, local fleet rating %.1f and regional threat %.1f, threat sources: %s",
            ["dropping ", ""][use_protection],
            reason,
            this_planet,
            local_production_diff,
            combined_local_defenses,
            local_fleet_rating,
            regional_threat,
            sys_status["regional_fleet_threats"],
        )
    return use_protection


def set_planet_growth_specials(focus_manager):
    """set resource foci of planets with potentially useful growth factors. Remove planets from list of candidates."""
    if not get_aistate().character.may_use_growth_focus():
        return

    # TODO Consider actual resource output of the candidate locations rather than only population
    for special, locations in get_growth_specials().items():
        # Find which metabolism is boosted by this special
        metabolism = AIDependencies.metabolismBoosts.get(special)
        if not metabolism:
            warning("Entry in available growth special not mapped to a metabolism")
            continue

        # Find the total population bonus we could get by using growth focus
        potential_pop_increase = empire_metabolisms.get(metabolism, 0)
        if not potential_pop_increase:
            continue

        debug(
            "Considering setting growth focus for %s at locations %s for potential population bonus of %.1f"
            % (special, locations, potential_pop_increase)
        )

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
            pop_gain = potential_pop_increase - planet.habitableSize
            if pop > pop_gain:
                _print_evaluation("would lose more pop (%.1f) than gain everywhere else (%.1f)." % (pop, pop_gain))
                continue

            # If we have a computronium special here, then research focus will have higher priority.
            if AIDependencies.COMPUTRONIUM_SPECIAL in planet.specials and RESEARCH in planet.availableFoci:
                _print_evaluation("has a usable %s" % AIDependencies.COMPUTRONIUM_SPECIAL)
                continue

            _print_evaluation(
                "considered (pop %.1f, growth gain %.1f, current focus %s)" % (pop, pop_gain, pinfo.current_focus)
            )

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
            warning("  --> Failed to set growth focus at all candidate locations.")


def set_planet_production_and_research_specials(focus_manager):
    """Set production and research specials.
    Sets production/research specials for known (COMPUTRONIUM, HONEYCOMB and CONC_CAMP)
    production/research specials.
    Remove planets from list of candidates using bake_future_focus."""
    # TODO use "best" COMPUTRON planet instead of first found, where "best" means least industry loss,
    # least threatened, no foci change penalty etc.
    universe = fo.getUniverse()
    already_have_comp_moon = False
    for pid, pinfo in list(focus_manager.raw_planet_info.items()):
        planet = pinfo.planet
        if (
            AIDependencies.COMPUTRONIUM_SPECIAL in planet.specials
            and RESEARCH in planet.availableFoci
            and not already_have_comp_moon
        ):
            if focus_manager.bake_future_focus(pid, RESEARCH, force=True):
                already_have_comp_moon = True
                debug(
                    "%s focus of planet %s (%d) (with Computronium Moon) at Research Focus",
                    ["set", "left"][pinfo.current_focus == RESEARCH],
                    planet.name,
                    pid,
                )
                continue
        if "HONEYCOMB_SPECIAL" in planet.specials and INDUSTRY in planet.availableFoci:
            if focus_manager.bake_future_focus(pid, INDUSTRY, force=True):
                debug(
                    "%s focus of planet %s (%d) (with Honeycomb) at Industry Focus",
                    ["set", "left"][pinfo.current_focus == INDUSTRY],
                    planet.name,
                    pid,
                )
                continue
        if (
            (
                [
                    bld.buildingTypeName
                    for bld in map(universe.getBuilding, planet.buildingIDs)
                    if bld.buildingTypeName in ["BLD_CONC_CAMP", "BLD_CONC_CAMP_REMNANT"]
                ]
            )
            or (
                [
                    ccspec
                    for ccspec in planet.specials
                    if ccspec in ["CONC_CAMP_MASTER_SPECIAL", "CONC_CAMP_SLAVE_SPECIAL"]
                ]
            )
        ) and INDUSTRY in planet.availableFoci:
            if focus_manager.bake_future_focus(pid, INDUSTRY):
                debug(
                    "%s focus of planet %s (%d) (with Concentration Camps/Remnants) at Industry Focus",
                    ["set", "left"][pinfo.current_focus == INDUSTRY],
                    planet.name,
                    pid,
                )
                continue
            else:
                new_planet = universe.getPlanet(pid)
                warning(
                    "Failed setting %s for Concentration Camp planet %s (%d) "
                    "with species %s and current focus %s, but new planet copy shows %s",
                    pinfo.future_focus,
                    planet.name,
                    pid,
                    planet.speciesName,
                    planet.focus,
                    new_planet.focus,
                )


def set_planet_protection_foci(focus_manager):
    """Assess and set protection foci"""
    universe = fo.getUniverse()
    for pid, pinfo in list(focus_manager.raw_planet_info.items()):
        planet = pinfo.planet
        if PROTECTION in planet.availableFoci and assess_protection_focus(pinfo, focus_manager.priority):
            current_focus = planet.focus
            if focus_manager.bake_future_focus(pid, PROTECTION):
                if current_focus != PROTECTION:
                    debug(
                        "Tried setting %s for planet %s (%d) with species %s and current focus %s, "
                        "got result %d and focus %s",
                        pinfo.future_focus,
                        planet.name,
                        pid,
                        planet.speciesName,
                        current_focus,
                        True,
                        planet.focus,
                    )
                debug(
                    "%s focus of planet %s (%d) at Protection(Defense) Focus",
                    ["set", "left"][current_focus == PROTECTION],
                    planet.name,
                    pid,
                )
                continue
            else:
                new_planet = universe.getPlanet(pid)
                warning(
                    "Failed setting PROTECTION for planet %s (%d) with species %s and current focus %s, "
                    "but new planet copy shows %s",
                    planet.name,
                    pid,
                    planet.speciesName,
                    planet.focus,
                    new_planet.focus,
                )


def set_planet_industry_research_influence_foci(focus_manager, priority_ratio):
    """Adjust planet's industry versus research focus while targeting the given ratio and
    avoiding penalties from changing focus."""
    debug("\n-----------------------------------------")
    debug("Making Planet Focus Change Determinations\n")

    ratios = []
    # for each planet, calculate RP:PP value ratio at which industry focus and
    # research focus would have the same total value, & sort by that include a
    # bias to slightly discourage changing foci
    target_pp = 0.001
    target_rp = 0.001
    target_ip = 0.001
    resource_timer.start("Loop")  # loop
    # has_force = tech_is_complete("CON_FRC_ENRG_STRC")
    # cumulative all industry focus
    cumulative_pp, cumulative_rp, cumulative_ip = 0, 0, 0

    # Handle presets which only have possible output for preset focus
    for pid, pinfo in focus_manager.baked_planet_info.items():
        future_pp, future_rp, future_ip = pinfo.possible_output[pinfo.future_focus]
        target_pp += future_pp
        target_rp += future_rp
        target_ip += future_ip
        cumulative_pp += future_pp
        cumulative_rp += future_rp
        cumulative_ip += future_ip

    # tally max Industry
    for pid, pinfo in list(focus_manager.raw_planet_info.items()):
        i_pp, i_rp, i_ip = pinfo.possible_output[INDUSTRY]
        cumulative_pp += i_pp
        cumulative_rp += i_rp
        cumulative_ip += i_ip
        if RESEARCH not in pinfo.planet.availableFoci:
            if focus_manager.bake_future_focus(pid, INDUSTRY, False):
                target_pp += i_pp
                target_rp += i_rp

    # smallest possible ratio of research to industry with an all industry focus
    maxi_ratio = cumulative_rp / max(0.01, cumulative_pp)

    aistate = get_aistate()
    # target_focus = None
    for adj_round in [1, 2, 3]:
        for pid, pinfo in list(focus_manager.raw_planet_info.items()):
            # xy = y output when focus x, p for production(INDUSTRY), c for current
            pp, pr, pi = pinfo.possible_output[INDUSTRY]
            rp, rr, ri = pinfo.possible_output[RESEARCH]
            ip, ir, ii = pinfo.possible_output[INFLUENCE]
            # calculate factor F at which pp + F * pr == rp + F * rr =====> F = ( pp-rp ) / (rr-pr)
            factor = (pp - rp) / max(0.01, rr - pr)
            # Races much better at producing shouldn't switch too early, better produce the history analyzer quickly
            if factor >= 1.5 and pid == fo.getEmpire().capitalID and pinfo.current_focus == INDUSTRY:
                # factor at start is 2.66 for Egassem and 2 for good industry / bad research.
                # Egassem switch just before applying Bureaucracy in turn 8
                switch_turn = 7 if factor > 2 else 6
                if fo.currentTurn() < switch_turn:
                    continue
            if adj_round == 1:  # take research at planets that can not use industry focus
                if INDUSTRY not in pinfo.planet.availableFoci:
                    target_pp += rp
                    target_rp += rr
                    focus_manager.bake_future_focus(pid, RESEARCH, False)
                continue
            if adj_round == 2:  # take research at planets with very cheap research
                if (maxi_ratio < priority_ratio) and (target_rp < priority_ratio * cumulative_pp) and (factor <= 1.0):
                    target_pp += rp
                    target_rp += rr
                    focus_manager.bake_future_focus(pid, RESEARCH, False)
                continue
            if adj_round == 3:  # assume default IFocus
                target_pp += pp  # icurTargets initially calculated by Industry focus, which will be our default focus
                target_rp += pr
                ratios.append((factor, pid, pinfo))

    ratios.sort()
    printed_header = False
    got_algo = policy_is_adopted(algo_research)
    for ratio, pid, pinfo in ratios:
        if priority_ratio < (target_rp / (target_pp + 0.0001)):  # we have enough RP
            if ratio < 1.1 and aistate.character.may_research_heavily():
                # but wait, RP is still super cheap relative to PP, maybe will take more RP
                if priority_ratio < 1.5 * (target_rp / (target_pp + 0.0001)):
                    # yeah, really a glut of RP, stop taking RP
                    break
            else:  # RP not super cheap & we have enough, stop taking it
                break
        pp, pr, pi = pinfo.possible_output[INDUSTRY]
        rp, rr, ri = pinfo.possible_output[RESEARCH]
        if (
            (ratio > 2.0 and target_pp < 15 and got_algo)
            or (ratio > 2.5 and target_pp < 25 and pp > 5 and got_algo)
            or (ratio > 3.0 and target_pp < 40 and pp > 5 and got_algo)
            or (ratio > 4.0 and target_pp < 100 and pp > 10)
            or ((target_rp + rr - pr) / max(0.001, target_pp - pp + rp) > 2 * priority_ratio)
        ):
            # we already have algo elegance and more RP would be too expensive, or overkill
            if not printed_header:
                printed_header = True
                debug("Rejecting further Research Focus choices as too expensive:")
                debug(
                    "%34s|%20s|%15s |%15s|%15s |%15s |%15s",
                    "                      Planet ",
                    " current RP/PP ",
                    " current target RP/PP ",
                    "current Focus ",
                    "  rejectedFocus ",
                    " rejected target RP/PP ",
                    "rejected RP-PP EQF",
                )
            old_focus = pinfo.current_focus
            c_pp, c_rp, _ = pinfo.current_output
            ot_pp, ot_rp, _ = pinfo.possible_output[old_focus]
            nt_pp, nt_rp, _ = pinfo.possible_output[RESEARCH]
            debug(
                "pID (%3d) %22s | c: %5.1f / %5.1f | cT: %5.1f / %5.1f"
                " |  cF: %8s | nF: %8s | cT: %5.1f / %5.1f | %.2f",
                pid,
                pinfo.planet.name,
                c_rp,
                c_pp,
                ot_rp,
                ot_pp,
                _focus_name(old_focus),
                _focus_name(RESEARCH),
                nt_rp,
                nt_pp,
                ratio,
            )
            # RP is getting too expensive, but might be willing to still allocate from a planet with less PP to lose
            continue
        focus_manager.bake_future_focus(pid, RESEARCH, False)
        target_rp += rr - pr
        target_pp -= pp - rp

    # Any planet still raw is set to industry
    for pid in list(focus_manager.raw_planet_info.keys()):
        focus_manager.bake_future_focus(pid, INDUSTRY, False)


def set_planet_resource_foci():
    """set resource focus of planets"""

    Reporter.print_resource_ai_header()
    resource_timer.start("Priority")
    # TODO: take into acct splintering of resource groups
    aistate = get_aistate()
    production_priority = aistate.get_priority(PriorityType.RESOURCE_PRODUCTION)
    research_priority = aistate.get_priority(PriorityType.RESOURCE_RESEARCH)
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

    set_planet_industry_research_influence_foci(focus_manager, priority_ratio)
    reporter.capture_section_info("Typical")

    reporter.print_table(priority_ratio)

    resource_timer.stop_print_and_clear()

    Reporter.dump_output()


def generate_resources_orders():
    """generate resources focus orders"""

    set_planet_resource_foci()

    Reporter.print_resources_priority()
