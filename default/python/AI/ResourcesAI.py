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
from itertools import chain
from logging import debug, info, warning
from operator import itemgetter
from typing import NamedTuple, Optional

import AIDependencies
import PlanetUtilsAI
import ProductionAI
from aistate_interface import get_aistate
from buildings import BuildingType
from colonization.calculate_planet_colonization_rating import empire_metabolisms
from common.fo_typing import PlanetId
from common.print_utils import Table, Text
from empire.growth_specials import get_growth_specials
from EnumsAI import FocusType, PriorityType, get_priority_resource_types
from freeorion_tools import get_named_real, tech_is_complete, get_species_industry, get_species_research
from freeorion_tools.bonus_calculation import adjust_direction
from freeorion_tools.statistics import stats
from freeorion_tools.timers import AITimer
from PolicyAI import PolicyManager, bureaucracy, liberty
from turn_state import (
    computronium_candidates,
    honeycomb_candidates,
    population_with_industry_focus,
)

resource_timer = AITimer("timer_bucket")

# Local Constants
INDUSTRY = FocusType.FOCUS_INDUSTRY
RESEARCH = FocusType.FOCUS_RESEARCH
GROWTH = FocusType.FOCUS_GROWTH
PROTECTION = FocusType.FOCUS_PROTECTION
INFLUENCE = FocusType.FOCUS_INFLUENCE
supported_foci = {INDUSTRY, RESEARCH, INFLUENCE, GROWTH, PROTECTION}


def _focus_name(focus: str) -> str:
    _known_names = {
        INDUSTRY: "Industry",
        RESEARCH: "Research",
        GROWTH: "Growth",
        PROTECTION: "Defense",
        INFLUENCE: "Influence",
    }
    return _known_names.get(focus, focus)


class Output(NamedTuple):
    industry: float
    research: float
    influence: float
    stability: float
    value: float


class PlanetFocusInfo:
    """The current, possible and future foci and output of one planet."""

    def __init__(self, planet: fo.planet):
        self.planet = planet
        self.current_focus = planet.focus
        self.current_output = Output(
            industry=planet.currentMeterValue(fo.meterType.industry),
            research=planet.currentMeterValue(fo.meterType.research),
            influence=planet.currentMeterValue(fo.meterType.influence),
            stability=planet.currentMeterValue(fo.meterType.happiness),
            value=0.0,  # cannot calculate the value here, do we even need it?
        )
        self.possible_output = {}
        self.future_focus = self.current_focus
        self.best_output_focus = None
        self.second_best_focus = None
        self.options = {focus for focus in planet.availableFoci if focus in supported_foci}

    def __str__(self):
        return str(self.planet)

    def __repr__(self):
        return self.planet.__repr__()

    def __lt__(self, other):
        """
        Sort according to best_over_second() in reverse order.
        This is used as a secondary criteria after value_below_best.
        Since value_below_best needs a focus argument, we cannot use it here.
        """
        return self.best_over_second() >= other.best_over_second()

    def value_below_best(self, focus: str) -> float:
        return self.possible_output[focus].value - self.possible_output[self.best_output_focus].value

    def best_over_second(self) -> float:
        return self.possible_output[self.best_output_focus].value - self.possible_output[self.second_best_focus].value

    def evaluate(self, focus: str) -> float:
        """
        Compare focus with best alternative.
        If focus produces best, return a positive value, giving how much its output is better than second best.
        If another focus produces better, returns a negative value, giving how much it is worse than best.
        """
        sqrt_population = min(1.0, self.planet.currentMeterValue(fo.meterType.population)) ** 0.5
        if focus == self.best_output_focus:
            return self.best_over_second() / sqrt_population
        else:
            return self.value_below_best(focus) / sqrt_population


class PlanetFocusManager:
    """PlanetFocusManager tracks all of the empire's planets, what their current and future focus will be."""

    def __init__(self):
        universe = fo.getUniverse()

        resource_timer.start("getPlanets")
        self.planet_ids = [planet.id for planet in PlanetUtilsAI.get_empire_populated_planets()]

        resource_timer.start("Targets")
        self.planet_info = {pid: PlanetFocusInfo(universe.getPlanet(pid)) for pid in self.planet_ids}
        self.baked_planet_info = {}
        aistate = get_aistate()
        self.priority_industry = aistate.get_priority(PriorityType.RESOURCE_PRODUCTION)
        self.priority_research = aistate.get_priority(PriorityType.RESOURCE_RESEARCH)
        self.priority_influence = aistate.get_priority(PriorityType.RESOURCE_INFLUENCE)
        self.have_bureaucracy = bureaucracy in fo.getEmpire().adoptedPolicies
        self.have_liberty = liberty in fo.getEmpire().adoptedPolicies
        self.have_automation = tech_is_complete(AIDependencies.PRO_AUTO_1)
        self.have_ai = tech_is_complete(AIDependencies.LRN_ARTIF_MINDS_1)
        self.calculate_planet_infos()

    def set_planetary_foci(self, reporter):
        self.set_planet_growth_specials()
        self.set_planet_production_and_research_specials()
        reporter.capture_section_info("Specials")
        # TODO redo. Note that setting protection for stability is handled production_value and set_influence
        # self.set_planet_protection_foci()
        # reporter.capture_section_info("Protection")
        self.set_influence_focus()
        reporter.capture_section_info("Influence")
        self.start_capital()
        self.set_other_foci()
        reporter.capture_section_info("Typical")
        reporter.print_table(self.priority_research / self.priority_industry)

    def bake_future_focus(self, pid, focus, force=False):
        """Set the focus and moves it from the raw list to the baked list of planets.
        pid -- pid
        focus -- future focus to use
        Return success or failure
        """
        pinfo = self.planet_info.get(pid)
        debug(f"baking {focus} on {pinfo.planet}")
        if not pinfo:
            return False
        if (focus == INDUSTRY or focus == RESEARCH) and not force:
            if self.check_avoid_change_due_to_bureaucracy(pinfo, focus):
                return False

        success = bool(
            pinfo.current_focus == focus
            or (focus in pinfo.planet.availableFoci and fo.issueChangeFocusOrder(pid, focus))
        )
        if success:
            if pinfo.current_focus != focus:
                PolicyManager.report_focus_change()
            pinfo.future_focus = focus
            self.baked_planet_info[pid] = self.planet_info.pop(pid)
        debug(f"success={success}")
        return success

    def check_avoid_change_due_to_bureaucracy(self, pinfo: PlanetFocusInfo, focus: str) -> bool:
        """Check whether we better not change a focus to avoid the influence penalty from bureaucracy."""
        if not self.have_bureaucracy or focus == pinfo.current_focus:
            return False  # No bureaucracy or no change
        last_turn = fo.currentTurn() - 1
        if pinfo.planet.LastTurnColonized == last_turn or pinfo.planet.LastTurnConquered == last_turn:
            return False  # no penalty for newly settled/conquered planets
        if pinfo.current_output.influence + 0.5 < pinfo.possible_output[pinfo.current_focus].influence:
            # avoid repeated focus changes: When influence production already is below target, do not
            # get it further down by switching again (I've seen one planet at -30...)
            debug(f"Avoid focus change on {pinfo.planet.name} due bureaucracy and influence not maxed.")
            return True
        return False

    def production_value(self, pp: float, rp: float, ip: float, stability: float, focus: str) -> float:
        """Calculate value of the given production output."""
        # For some reason even rebelling planets can produce influence. Also, we must avoid a death cycle: when
        # stability goes down due to influence debt, trying to counter instability by setting influence producing
        # planets to protection mode just makes it worse.
        if stability <= 0.0 and focus != INFLUENCE:
            return 0.0
        return (
            pp * self.priority_industry
            + rp * self.priority_research
            + ip * self.priority_influence
            # add a small amount for stability to make sure output of protection is better than growth
            + stability / 10
        )

    def possible_output_with_focus(self, planet: fo.planet, focus: str, stability: Optional[float] = None) -> Output:
        """
        Estimate Output of planet, which should have been set to focus and meters should be current.
        Special case: if stability is giving, calculate output from current focus, but with adapted stability.
        """
        if planet.focus != focus and stability is None:  # basic output of growth is determined using protection
            return Output(0.0, 0.0, 0.0, 0.0, 0.0)

        industry_target = planet.currentMeterValue(fo.meterType.targetIndustry)
        research_target = planet.currentMeterValue(fo.meterType.targetResearch)
        influence_target = planet.currentMeterValue(fo.meterType.targetInfluence)
        current_stability = planet.currentMeterValue(fo.meterType.happiness)
        # note that stability 0.0 is a valid value
        target_stability = planet.currentMeterValue(fo.meterType.targetHappiness) if stability is None else stability
        if self.have_liberty and focus == RESEARCH:
            research_target += PlanetUtilsAI.adjust_liberty(planet, planet.currentMeterValue(fo.meterType.population))
        if self.have_automation:
            min_stability = get_named_real("PRO_ADAPTIVE_AUTO_MIN_STABILITY")
            value = get_named_real("PRO_ADAPTIVE_AUTO_TARGET_INDUSTRY_FLAT")
            industry_target += value * adjust_direction(min_stability, current_stability, target_stability)
        if self.have_ai:
            min_stability = get_named_real("LRN_NASCENT_AI_MIN_STABILITY")
            value = get_named_real("LRN_NASCENT_AI_TARGET_RESEARCH_FLAT")
            research_target += value * adjust_direction(min_stability, current_stability, target_stability)
        if target_stability < 0:
            industry_target = research_target = 0.0
        # There are a lot more adjustments, some also depend on supply connection. Hopefully one day we get
        # meter_update with adjusted stability...

        return Output(
            industry=industry_target,
            research=research_target,
            influence=influence_target,
            stability=target_stability,
            value=self.production_value(industry_target, research_target, influence_target, target_stability, focus),
        )

    def calculate_planet_infos(self):  # noqa complexity
        """
        Calculates for each possible focus an estimated target output of each planet and stores it in planet info.
        """

        def set_focus(focus_):
            if focus_ in pinfo.planet.availableFoci and pinfo.planet.focus != focus_:
                fo.issueChangeFocusOrder(pid, focus_)

        universe = fo.getUniverse()
        for pid, pinfo in self.planet_info.items():
            set_focus(INDUSTRY)
        universe.updateMeterEstimates(self.planet_ids)
        for pid, pinfo in self.planet_info.items():
            pinfo.possible_output[INDUSTRY] = self.possible_output_with_focus(pinfo.planet, INDUSTRY)
            set_focus(RESEARCH)
        universe.updateMeterEstimates(self.planet_ids)
        for pid, pinfo in self.planet_info.items():
            pinfo.possible_output[RESEARCH] = self.possible_output_with_focus(pinfo.planet, RESEARCH)
            set_focus(INFLUENCE)
        universe.updateMeterEstimates(self.planet_ids)
        for pid, pinfo in self.planet_info.items():
            pinfo.possible_output[INFLUENCE] = self.possible_output_with_focus(pinfo.planet, INFLUENCE)
            set_focus(PROTECTION)
        universe.updateMeterEstimates(self.planet_ids)
        for pid, pinfo in self.planet_info.items():
            pinfo.possible_output[PROTECTION] = self.possible_output_with_focus(pinfo.planet, PROTECTION)
            # PROTECTION and GROWTH only produce through focus-less bonuses, so we can do without another meterEstimate
            stability = PlanetUtilsAI.stability_with_focus(pinfo.planet, GROWTH)
            pinfo.possible_output[GROWTH] = self.possible_output_with_focus(pinfo.planet, PROTECTION, stability)
            # revert to current focus.
            set_focus(pinfo.current_focus)
        universe.updateMeterEstimates(self.planet_ids)
        for pinfo in self.planet_info.values():
            rated = sorted([(pinfo.possible_output[focus].value, focus) for focus in supported_foci], reverse=True)
            pinfo.best_output_focus = rated[0][1]
            pinfo.second_best_focus = rated[1][1]
        for pid, pinfo in self.planet_info.items():
            po = pinfo.possible_output
            for focus in supported_foci:
                o = po[focus]
                debug(
                    f"Possible output of planet {pinfo.planet} for {focus} is {o.value} ({o.industry}/"
                    f"{o.research}/{o.influence} stability={o.stability})"
                )
            debug(f"best output: {pinfo.best_output_focus}")

    def set_planet_growth_specials(self):  # noqa complexity
        """Consider growth focus for planets with useful growth specials. Remove planets from list of candidates."""
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
                pinfo = self.planet_info.get(pid)
                planet = fo.getUniverse().getPlanet(pid)
                if not pinfo or GROWTH not in pinfo.planet.availableFoci:
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
                if self.bake_future_focus(pid, GROWTH):
                    debug("  --> Using growth focus at %s" % planet)
                    break
            else:
                warning("  --> Failed to set growth focus at all candidate locations.")

    def set_planet_production_and_research_specials(self):
        """Set production and research specials.
        Sets production/research specials for known (COMPUTRONIUM, HONEYCOMB and CONC_CAMP)
        production/research specials.
        Remove planets from list of candidates using bake_future_focus.
        """
        # loss should be >= 0 with 0 meaning research is the best focus anyway
        for loss, pid in sorted(
            [(self.planet_info[pid].value_below_best(RESEARCH), pid) for pid in computronium_candidates()]
        ):
            # Computronium moon has a rather high stability requirement, in many cases it may not be worth anything
            if self.evaluate_computronium(pid, loss):
                self.bake_future_focus(pid, RESEARCH, True)
                break

        for loss, pid in sorted(
            [(self.planet_info[pid].value_below_best(INDUSTRY), pid) for pid in honeycomb_candidates()]
        ):
            # honeycomb has no stability requirement, it should almost always be useful
            # TODO check for supply
            pp_gain = population_with_industry_focus() * get_named_real("HONEYCOMB_TARGET_INDUSTRY_PERPOP")
            if loss < pp_gain * self.priority_industry:
                self.bake_future_focus(pid, INDUSTRY, True)
                break
        # TODO concentration camps, currently the AI does not build them...

    def evaluate_computronium(self, pid: PlanetId, loss: float) -> bool:
        """
        Determine whether switching the given planet with a computronium to research focus is worse the loss.
        """
        researchers = 0.0  # amount of people that would profit from the special
        min_stability = get_named_real("COMPUTRONIUM_MIN_STABILITY")
        per_pop = get_named_real("COMPUTRONIUM_TARGET_RESEARCH_PERPOP")
        # TODO: check for supply as well
        for _, pinfo in self.planet_info:
            if pinfo.current_focus == RESEARCH and pinfo.possible_output[RESEARCH].stability >= min_stability:
                researchers += pinfo.planet.currentMeterValue(fo.meterType.population)
        return researchers * per_pop * self.priority_research > loss

    def set_influence_focus(self):
        """Assign planets to influence production."""
        current_ip = fo.getEmpire().resourceAvailable(fo.resourceType.influence)
        focused_planets = sorted(
            [
                (pinfo.evaluate(INFLUENCE), pinfo)
                for pinfo in self.planet_info.values()
                if pinfo.current_focus == INFLUENCE
            ]
        )
        candidate_planets = sorted(
            [
                (pinfo.evaluate(INFLUENCE), pinfo)
                for pinfo in self.planet_info.values()
                if pinfo.current_focus != INFLUENCE and INFLUENCE in pinfo.options
            ]
        )
        target_ip_production = sum(
            pi.possible_output[pi.current_focus].influence
            for pi in chain(self.planet_info.values(), self.baked_planet_info.values())
        )
        ratio = (current_ip + 10 * target_ip_production) / max(1.0, self.priority_influence)  # in turn 1 prio is 0
        debug(
            f"Evaluating influence: IP: {current_ip}, target production: {target_ip_production}, prio:"
            f"{self.priority_influence} => ratio: {ratio}\n"
            f"focused_planets: {focused_planets}\n"
            f"candidates: {candidate_planets}"
        )
        if candidate_planets and (ratio < 1.0 or ratio < 25.0 and candidate_planets[-1][0] > ratio):
            debug("setting influence focus to last candidate")
            self.bake_future_focus(candidate_planets[-1][1].planet.id, INFLUENCE)
        if focused_planets:
            pinfo = focused_planets[0][1]
            below_max = pinfo.possible_output[pinfo.current_focus].influence - pinfo.current_output.influence
            if below_max < 0.5 and (ratio > 30.0 or ratio > 2.0 and focused_planets[0][0] < -100.0 / ratio):
                debug("removing influence focus of first focus planet")
                pinfo = focused_planets.pop(0)
            # all (others) remain influence focussed.
            for _, pinfo in focused_planets:
                self.bake_future_focus(pinfo.planet.id, INFLUENCE)

        ProductionAI.candidate_for_translator = None
        translator_locations = BuildingType.TRANSLATOR.built_or_queued_at()
        for _, pinfo in focused_planets:
            if pinfo.planet.id not in translator_locations:
                ProductionAI.candidate_for_translator = pinfo.planet.id
                break

    def start_capital(self):
        """A small, handcrafted start optimisation for bad researchers."""
        if fo.currentTurn() < 7:
            capital = fo.getUniverse().getPlanet(fo.getEmpire().capitalID)
            if capital:
                factor = get_species_industry(capital.speciesName) / get_species_research(capital.speciesName)
                pinfo = self.planet_info[capital.id]
                if factor >= 1.5 and pinfo and pinfo.current_focus == INDUSTRY:
                    # factor at start is 2.66 for Egassem and 2 for good industry / bad research.
                    switch_turn = 7 if factor > 2 else 6
                    if fo.currentTurn() < switch_turn:
                        debug("Special handling: keeping capital at industry.")
                        self.bake_future_focus(capital.id, INDUSTRY)
                    # else: don't do anything special, standard handling will likely switch to RESEARCH

    def set_other_foci(self):
        """
        Set remaining planet's foci.
        If best focus is protection, it is set.
        Others are set to industry or research, aiming for a production ratio like the ratio of the priorities.
        """
        debug("set_other_foci...")
        industry_or_research = []
        for pid, pinfo in dict(self.planet_info).items():
            # Note that if focus hasn't been baked as INFLUENCE yet, it shouldn't be.
            if pinfo.best_output_focus in (INDUSTRY, RESEARCH, INFLUENCE) and {INDUSTRY, RESEARCH} <= pinfo.options:
                industry_value = pinfo.possible_output[INDUSTRY].value
                research_value = pinfo.possible_output[RESEARCH].value
                industry_or_research.append((industry_value - research_value, pinfo))
            else:
                # Either protection due to stability, or planet does not support both foci
                self.bake_future_focus(pid, pinfo.best_output_focus)
        industry_or_research.sort()
        debug(f"Assigning ind/res, candidates: {industry_or_research}")
        current_pp_target = sum(pi.possible_output[pi.current_focus].industry for pi in self.baked_planet_info.values())
        current_rp_target = sum(pi.possible_output[pi.current_focus].research for pi in self.baked_planet_info.values())
        while industry_or_research:
            # Mini offset to avoid comparing 0 vs 0, so higher priority wins if we have not baked anything yet.
            pp_per_priority = (current_pp_target + 0.1) / self.priority_industry
            ip_per_priority = (current_rp_target + 0.1) / self.priority_research
            debug(
                f"pp: {current_pp_target}/{self.priority_industry}={pp_per_priority}, "
                f"rp: {current_rp_target}/{self.priority_research}={ip_per_priority}"
            )
            if pp_per_priority > ip_per_priority:
                # set one to research, best research (or worst industry) planets are at the beginning
                _, pinfo = industry_or_research.pop(0)
                self.bake_future_focus(pinfo.planet.id, RESEARCH)
            else:
                _, pinfo = industry_or_research.pop(-1)
                self.bake_future_focus(pinfo.planet.id, INDUSTRY)
            current_pp_target += pinfo.possible_output[pinfo.future_focus].industry
            current_rp_target += pinfo.possible_output[pinfo.future_focus].research


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
        for pinfo in self.focus_manager.baked_planet_info.values():
            if pinfo.current_focus != pinfo.future_focus:
                total_changed += 1

            old_pp, old_rp, _, _, _ = pinfo.possible_output[pinfo.current_focus]
            current_industry_target += old_pp
            current_research_target += old_rp

            future_pp, future_rp, _, _, _ = pinfo.possible_output[pinfo.future_focus]
            new_industry_target += future_pp
            new_research_target += future_rp

            industry_pp, industry_rp, _, _, _ = (
                pinfo.possible_output[INDUSTRY] if INDUSTRY in pinfo.possible_output else (future_pp, future_rp, 0, 0)
            )
            all_industry_industry_target += industry_pp
            all_industry_research_target += industry_rp

            research_pp, research_rp, _, _, _ = (
                pinfo.possible_output[RESEARCH] if RESEARCH in pinfo.possible_output else (future_pp, future_rp, 0, 0)
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
                current_pp, curren_rp, _, _, _ = pinfo.current_output
                ot_pp, ot_rp, _, _, _ = pinfo.possible_output.get(old_focus, (0, 0, 0, 0))
                nt_pp, nt_rp, _, _, _ = pinfo.possible_output[new_focus]
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


def generate_resources_orders():
    """generate resources focus orders"""

    Reporter.print_resource_ai_header()
    resource_timer.start("Focus Infos")
    focus_manager = PlanetFocusManager()
    reporter = Reporter(focus_manager)
    focus_manager.set_planetary_foci(reporter)
    resource_timer.stop_print_and_clear()

    Reporter.dump_output()
    Reporter.print_resources_priority()
