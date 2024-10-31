"""
ResourcesAI.py provides generate_resources_orders which sets the focus for all populated planets in the empire.

The method is to start with a raw list of all the populated planets in the empire.
It considers in turn growth factors, production specials, defense requirements
and finally the targeted ratio of research/production. Each decision on a planet
transfers the planet from the raw list to the baked list, until all planets
have their future focus decided.
"""
from __future__ import annotations

import freeOrionAIInterface as fo
from collections.abc import Sequence
from functools import total_ordering
from itertools import chain
from logging import debug, info, warning
from operator import itemgetter
from typing import NamedTuple

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
from freeorion_tools import get_named_real, get_species_industry, get_species_research, tech_is_complete
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
USELESS_RATING = -99999  # focus is either not available or stability is too bad


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
    rating: float  # sum of all production values weighed by priorities for overall comparisons


class FocusRating(NamedTuple):
    rating: float
    focus: str


class PlanetFocusRating:
    """
    Best and second after the best focuses for the planet.
    """

    def __init__(self, focuses: Sequence[tuple[float, str]]):
        """
        Initialize object wih best and second best focuses.

        There is a number of focuses equal length of the `supported_foci`
        """
        assert len(focuses) == len(supported_foci)
        focuses = sorted((FocusRating(rating, focus) for rating, focus in focuses), reverse=True)
        self.best = focuses[0]
        self.second = focuses[1]


@total_ordering
class PlanetFocusInfo:
    """The current, possible and future foci and output of one planet."""

    def __init__(self, planet: fo.planet):
        self.planet = planet
        # Make sure current_focus is set, without it some calculation may go astray.
        self.current_focus = planet.focus if planet.focus else fo.getSpecies(planet.speciesName).preferredFocus
        self.current_output = Output(
            industry=planet.currentMeterValue(fo.meterType.industry),
            research=planet.currentMeterValue(fo.meterType.research),
            influence=planet.currentMeterValue(fo.meterType.influence),
            stability=planet.currentMeterValue(fo.meterType.happiness),
            rating=0.0,  # cannot calculate the rating here, do we even need it?
        )
        self.possible_output = {}
        self.future_focus = self.current_focus
        self.rated_foci: PlanetFocusRating = None  # initialized after the object creation
        self.options = {focus for focus in planet.availableFoci if focus in supported_foci}

    def __repr__(self):
        return f"PlanetFocusInfo({repr(self.planet)})"

    def __eq__(self, other: PlanetFocusInfo):
        return self.planet.id == other.planet.id

    def __lt__(self, other: PlanetFocusInfo):
        """
        Dummy sorting.
        In this module, we sort Tuples[float, PlanetFocusInfo]. To do so, PlanetFocusInfo must be sortable, but if the
        float is the same, we don't really care which comes first.
        """
        return self.planet.id < other.planet.id

    def set_rated_foci(self):
        self.rated_foci = PlanetFocusRating([(self.possible_output[focus].rating, focus) for focus in supported_foci])

    def rating_below_best(self, focus: str) -> float:
        return self.possible_output[focus].rating - self.rated_foci.best.rating

    def best_over_second(self) -> float:
        return self.rated_foci.best.rating - self.rated_foci.second.rating

    def difference(self, focus1: str, focus2: str) -> float:
        """How much is the output of focus1 better or worse than output of focus2."""
        return self.possible_output[focus1].rating - self.possible_output[focus2].rating

    def evaluate(self, focus: str) -> float:
        """
        Compare focus with best alternative.
        If focus produces best, return a positive rating, giving how much its output is better than second best.
        If another focus produces better, returns a negative rating, giving how much it is worse than best.
        Result is divided by sqrt(population) to prefer switching smaller planets.
        """
        sqrt_population = max(1.0, self.planet.currentMeterValue(fo.meterType.population)) ** 0.5
        if sqrt_population == 0:
            sqrt_population = 0.0001  # prevent Zero division

        if focus == self.rated_foci.best.focus:
            # TBD replace division by using immediate_lost...
            return self.best_over_second() / sqrt_population
        else:
            return self.rating_below_best(focus) / sqrt_population


class PlanetFocusManager:
    """PlanetFocusManager tracks all of the empire's planets, what their current and future focus will be."""

    def __init__(self):
        universe = fo.getUniverse()

        resource_timer.start("getPlanets")
        self.planet_ids = [planet.id for planet in PlanetUtilsAI.get_empire_populated_planets()]

        resource_timer.start("Targets")
        self.planet_info: dict[PlanetId, PlanetFocusInfo] = {
            pid: PlanetFocusInfo(universe.getPlanet(pid)) for pid in self.planet_ids
        }
        self.baked_planet_info = {}
        aistate = get_aistate()
        self.priority_industry = aistate.get_priority(PriorityType.RESOURCE_PRODUCTION)
        self.priority_research = aistate.get_priority(PriorityType.RESOURCE_RESEARCH)
        self.priority_influence = aistate.get_priority(PriorityType.RESOURCE_INFLUENCE)
        self.calculate_planet_infos()

    def set_planetary_foci(self, reporter: Reporter):
        self.set_planet_growth_specials()
        self.set_planet_production_and_research_specials()
        reporter.capture_section_info("Specials")
        # TODO redo. Note that setting protection for stability is handled production_value and set_influence
        # self.set_planet_protection_foci()
        # reporter.capture_section_info("Protection")
        self.set_influence_focus()
        reporter.capture_section_info("Influence")
        self.early_capital_handling()
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
        if bureaucracy not in fo.getEmpire().adoptedPolicies or focus == pinfo.current_focus:
            return False  # No bureaucracy or no change
        last_turn = fo.currentTurn() - 1
        if pinfo.planet.lastTurnColonized == last_turn or pinfo.planet.lastTurnConquered == last_turn:
            return False  # no penalty for newly settled/conquered planets
        if pinfo.current_output.influence + 0.5 < pinfo.possible_output[pinfo.current_focus].influence:
            # avoid repeated focus changes: When influence production already is below target, do not
            # get it further down by switching again (I've seen one planet at -30...)
            debug(f"Avoid focus change on {pinfo.planet.name} due bureaucracy and influence not maxed.")
            return True
        return False

    def rate_output(self, pp: float, rp: float, ip: float, stability: float, focus: str) -> float:
        """Calculate rating of the given production output."""
        # For some reason even rebelling planets can produce influence. Also, we must avoid a death cycle: when
        # stability goes down due to influence debt, trying to counter instability by setting influence producing
        # planets to protection focus just makes it worse.
        if stability <= 0.0 and focus != INFLUENCE:
            return USELESS_RATING
        return (
            pp * self.priority_industry
            + rp * self.priority_research
            + ip * self.priority_influence
            # add a small amount for stability to make sure output of protection is better than growth
            + stability / 10
        )

    def possible_output_with_focus(self, planet: fo.planet, focus: str, stability: float | None = None) -> Output:
        """
        Estimate Output of planet, which should have been set to focus and meters should be current.
        Special case: if stability is giving, calculate output from current focus, but with adapted stability.
        """
        if planet.focus != focus and stability is None:  # basic output of growth is determined using protection
            return Output(0.0, 0.0, 0.0, 0.0, USELESS_RATING)

        industry_target = planet.currentMeterValue(fo.meterType.targetIndustry)
        research_target = planet.currentMeterValue(fo.meterType.targetResearch)
        influence_target = planet.currentMeterValue(fo.meterType.targetInfluence)
        current_stability = planet.currentMeterValue(fo.meterType.happiness)
        # note that stability 0.0 is a valid value
        target_stability = planet.currentMeterValue(fo.meterType.targetHappiness) if stability is None else stability
        if liberty in fo.getEmpire().adoptedPolicies and focus == RESEARCH:
            research_target += PlanetUtilsAI.adjust_liberty(planet, planet.currentMeterValue(fo.meterType.population))
        if tech_is_complete(AIDependencies.PRO_AUTO_1):
            min_stability = get_named_real("PRO_ADAPTIVE_AUTO_MIN_STABILITY")
            flat = get_named_real("PRO_ADAPTIVE_AUTO_TARGET_INDUSTRY_FLAT")
            industry_target += flat * adjust_direction(min_stability, current_stability, target_stability)
        if tech_is_complete(AIDependencies.LRN_ARTIF_MINDS_1):
            min_stability = get_named_real("LRN_NASCENT_AI_MIN_STABILITY")
            flat = get_named_real("LRN_NASCENT_AI_TARGET_RESEARCH_FLAT")
            research_target += flat * adjust_direction(min_stability, current_stability, target_stability)
        if target_stability < 0:
            industry_target = research_target = 0.0
        # There are a lot more adjustments, some also depend on supply connection. Hopefully one day we get
        # meter_update with adjusted stability...

        return Output(
            industry=industry_target,
            research=research_target,
            influence=influence_target,
            stability=target_stability,
            rating=self.rate_output(industry_target, research_target, influence_target, target_stability, focus),
        )

    def calculate_planet_infos(self):  # noqa complexity
        """
        Calculates for each possible focus an estimated target output of each planet and stores it in planet info.
        """

        def set_focus(pinfo, focus):
            if focus in pinfo.planet.availableFoci and pinfo.planet.focus != focus:
                fo.issueChangeFocusOrder(pinfo.planet.id, focus)

        def collect_estimates(planet_info, focus, callback, second_focus=None):
            for pinfo in planet_info.values():
                set_focus(pinfo, focus)
            universe.updateMeterEstimates(list(planet_info))
            for pinfo in planet_info.values():
                pinfo.possible_output[focus] = callback(pinfo.planet, focus)
            if second_focus:
                for pinfo in planet_info.values():
                    stability = PlanetUtilsAI.stability_with_focus(pinfo.planet, GROWTH)
                    pinfo.possible_output[GROWTH] = callback(pinfo.planet, PROTECTION, stability)

        universe = fo.getUniverse()
        collect_estimates(self.planet_info, INDUSTRY, self.possible_output_with_focus)
        collect_estimates(self.planet_info, RESEARCH, self.possible_output_with_focus)
        collect_estimates(self.planet_info, INFLUENCE, self.possible_output_with_focus)
        # PROTECTION and GROWTH only produce through focus-less bonuses, so they can do with one meter estimation
        collect_estimates(self.planet_info, PROTECTION, self.possible_output_with_focus, GROWTH)
        # revert to current focus and update meters again
        for pinfo in self.planet_info.values():
            set_focus(pinfo, pinfo.current_focus)
        universe.updateMeterEstimates(self.planet_ids)

        for pinfo in self.planet_info.values():
            pinfo.set_rated_foci()
        self.print_pinfo_table()

    def print_pinfo_table(self):
        def output_table_format(o: Output) -> str:
            if o.rating == USELESS_RATING:
                return "---"
            else:
                return f"{o.rating:.1f} {o.industry:.1f} {o.research:.1f} {o.influence:.1f} {o.stability:.1f}"

        debug("Values per focus: rating pp rp ip stability")
        pinfo_table = Table(
            Text("Planet"),
            Text("Best Focus"),
            Text("Industry"),
            Text("Research"),
            Text("Influence"),
            Text("Protection"),
            Text("Growth"),
            table_name="Potential Planetary Output Overview Turn %d" % fo.currentTurn(),
        )
        for pinfo in self.planet_info.values():
            pinfo_table.add_row(
                pinfo.planet,
                _focus_name(pinfo.rated_foci.best.focus),
                output_table_format(pinfo.possible_output[INDUSTRY]),
                output_table_format(pinfo.possible_output[RESEARCH]),
                output_table_format(pinfo.possible_output[INFLUENCE]),
                output_table_format(pinfo.possible_output[PROTECTION]),
                output_table_format(pinfo.possible_output[GROWTH]),
            )
        info(pinfo_table)

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
                f"Considering setting growth focus for {special} at locations {locations} for potential population bonus of {potential_pop_increase:.1f}"
            )

            # Find the best suited planet to use growth special on, i.e. the planet where
            # we will lose the least amount of resource generation when using growth focus.
            def _print_evaluation(evaluation):
                """Local helper function printing a formatted evaluation."""
                debug(f"  - {planet} {evaluation}")

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
                    _print_evaluation(f"would lose more pop ({pop:.1f}) than gain everywhere else ({pop_gain:.1f}).")
                    continue

                # If we have a computronium special here, then research focus will have higher priority.
                if AIDependencies.COMPUTRONIUM_SPECIAL in planet.specials and RESEARCH in planet.availableFoci:
                    _print_evaluation("has a usable %s" % AIDependencies.COMPUTRONIUM_SPECIAL)
                    continue

                _print_evaluation(
                    f"considered (pop {pop:.1f}, growth gain {pop_gain:.1f}, current focus {pinfo.current_focus})"
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
            [(self.planet_info[pid].rating_below_best(RESEARCH), pid) for pid in computronium_candidates()]
        ):
            # Computronium moon has a rather high stability requirement, in many cases it may not be worth anything
            if self.evaluate_computronium(pid, loss):
                self.bake_future_focus(pid, RESEARCH, True)
                break

        for loss, pid in sorted(
            [(self.planet_info[pid].rating_below_best(INDUSTRY), pid) for pid in honeycomb_candidates()]
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
        Determine whether switching the given planet with a computronium to research focus is worth the loss.
        """
        researchers = 0.0  # amount of people that would profit from the special
        min_stability = get_named_real("COMPUTRONIUM_MIN_STABILITY")
        per_pop = get_named_real("COMPUTRONIUM_TARGET_RESEARCH_PERPOP")
        # TODO: check for supply as well
        for _, pinfo in self.planet_info.items():
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
            if pi.current_focus in pi.possible_output
        )
        ratio = (current_ip + 10 * target_ip_production) / max(1.5, self.priority_influence)  # in turn 1 prio is 0
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

    def early_capital_handling(self):
        """
        A small, handcrafted start optimisation for bad researchers.
        Even if research is needed most, they do better by quickly finishing the Automatic History Analyzer.
        """
        if fo.currentTurn() < 7:
            capital = fo.getUniverse().getPlanet(fo.getEmpire().capitalID)
            if capital:
                factor = get_species_industry(capital.speciesName) / get_species_research(capital.speciesName)
                pinfo = self.planet_info[capital.id]
                if factor >= 2.0 and pinfo and pinfo.current_focus == INDUSTRY:
                    # factor at start is 2.66 for Egassem and 2 for good industry / bad research.
                    # 1.5 currently does not exist and for normal industry / bad research (1.33) the hack is no good.
                    switch_turn = 7 if factor > 2 else 6
                    if fo.currentTurn() < switch_turn:
                        debug("Special handling: keeping capital at industry.")
                        self.bake_future_focus(capital.id, INDUSTRY)
                    # else: don't do anything special, standard handling will likely switch to RESEARCH

    def immediate_loss_on_switch_to(self, pinfo: PlanetFocusInfo, focus: str) -> float:
        """
        When switching to focus, how much would the current production go down?
        E.g. when switching from research to production, how much does research go down?
        Since it may take several turns until we reach the new targets, this can be used as an indicator how
        'expensive' the switch would be.
        """
        # This is not totally correct, since it won't go down to the minimum immediately. but it should be a good
        # enough estimate to compare how expensive a different switches would be.
        low_rating = self.rate_output(
            pp=min(pinfo.current_output.industry, pinfo.possible_output[focus].industry),
            rp=min(pinfo.current_output.research, pinfo.possible_output[focus].research),
            ip=min(pinfo.current_output.influence, pinfo.possible_output[focus].influence),
            stability=min(pinfo.current_output.stability, pinfo.possible_output[focus].stability),
            focus=focus,
        )
        current_rating = self.rate_output(
            pp=pinfo.current_output.industry,
            rp=pinfo.current_output.research,
            ip=pinfo.current_output.influence,
            stability=pinfo.current_output.stability,
            focus=pinfo.current_focus,
        )

        # note that this may be zero, e.g. for a newly conquered planet or one that currently has negative stability
        return current_rating - low_rating

    def change_candidate(
        self, best_so_far: PlanetFocusInfo | None, candidate: PlanetFocusInfo, from_focus: str, to_focus: str
    ) -> PlanetFocusInfo:
        """
        Returns either candidate or best_so_far (if not None), depending on which one should rather do the
        given focus switch.
        """
        if best_so_far is None:
            return candidate
        best_val = best_so_far.possible_output[to_focus].rating - best_so_far.possible_output[from_focus].rating
        cand_val = candidate.possible_output[to_focus].rating - candidate.possible_output[from_focus].rating
        if best_val > 0:
            # to_focus would be an improvement for best_so_far, chose candidate if it gains even more
            return candidate if cand_val > best_val else best_so_far
        elif cand_val > 0:
            # to_focus would be an improvement for candidate
            return candidate
        else:
            # Neither would be better, prefer planets with a smaller immediate and long term loss
            # Small offsets, since both values may be zero
            best_val = (best_val - 0.1) * (self.immediate_loss_on_switch_to(best_so_far, to_focus) ** 0.5 + 0.1)
            cand_val = (cand_val - 0.1) * (self.immediate_loss_on_switch_to(candidate, to_focus) ** 0.5 + 0.1)
            return candidate if cand_val > best_val else best_so_far

    def get_planned_pp_rp_per_prio(self) -> (float, float, float, float):
        """
        Calculate ratios of planned PP and RP and both values divided by their priority.
        It uses future_focus of all planets, setting this value without baking it can be used to check alternatives.
        """
        planned_pp_target = sum(
            pi.possible_output[pi.future_focus].industry
            for pi in chain(self.baked_planet_info.values(), self.planet_info.values())
            if pi.future_focus in pi.possible_output
        )
        planned_rp_target = sum(
            pi.possible_output[pi.future_focus].research
            for pi in chain(self.baked_planet_info.values(), self.planet_info.values())
            if pi.future_focus in pi.possible_output
        )
        pp_per_priority = planned_pp_target / self.priority_industry
        # When the AI finished all research, research prio becomes 0
        rp_per_priority = planned_rp_target / self.priority_research if self.priority_research else 0.0
        debug(
            f"pp: {planned_pp_target}/{self.priority_industry}={pp_per_priority}, "
            f"rp: {planned_rp_target}/{self.priority_research}={rp_per_priority}"
        )
        return planned_pp_target, planned_rp_target, pp_per_priority, rp_per_priority

    def set_other_foci(self):
        """
        Set remaining planet's foci.
        If best focus is protection, set it, else if only one of industry and research is possible, set it.
        All others are set to industry or research, aiming for a production ratio like the ratio of the priorities.
        If we have one or more candidates currently set to neither industry nor research, we assign these, otherwise
        we may switch one planet from industry to research or vice versa, or let two planets swap their foci.
        """
        debug("set_other_foci...")
        may_switch_to_research = None
        may_switch_to_industry = None
        set_to_neither = []
        num_industry = num_research = 0
        for pid, pinfo in dict(self.planet_info).items():
            # Note that if focus hasn't been baked as INFLUENCE yet, it shouldn't be.
            if pinfo.rated_foci.best.focus in (INDUSTRY, RESEARCH, INFLUENCE) and {INDUSTRY, RESEARCH} <= pinfo.options:
                if pinfo.current_focus == INDUSTRY:
                    num_industry = num_industry + 1
                    may_switch_to_research = self.change_candidate(may_switch_to_research, pinfo, INDUSTRY, RESEARCH)
                elif pinfo.current_focus == RESEARCH:
                    num_research = num_research + 1
                    may_switch_to_industry = self.change_candidate(may_switch_to_industry, pinfo, RESEARCH, INDUSTRY)
                else:
                    set_to_neither.append(pinfo)
            else:
                # Either protection due to stability, or planet does not support both foci
                self.bake_future_focus(pid, pinfo.rated_foci.best.focus)
        if set_to_neither:
            self.chose_industry_or_research(set_to_neither)
        else:
            # Never switch all planets to the lower priority focus, especially not the capital while it is the only one
            if self.priority_industry > self.priority_research and num_industry < 2:
                may_switch_to_research = None
            if self.priority_research > self.priority_industry and num_research < 2:
                may_switch_to_industry = None
            self.chose_switches(may_switch_to_research, may_switch_to_industry)

    def chose_industry_or_research(self, set_to_neither: list[PlanetFocusInfo]) -> None:
        """Assign planets in set_to_neither to industry or research focus."""
        set_to_neither.sort()
        debug(f"chose_industry_or_research set_to_neither={set_to_neither}")
        while set_to_neither:
            # TBD: can we also use balanced_output_value here? Most likely its only one planet anyway...
            _, _, pp_per_priority, rp_per_priority = self.get_planned_pp_rp_per_prio()
            # sorting is best research to best industry
            if pp_per_priority > rp_per_priority:
                self.bake_future_focus(set_to_neither[0].planet.id, RESEARCH)
                set_to_neither.pop(0)
            else:
                self.bake_future_focus(set_to_neither[-1].planet.id, INDUSTRY)
                set_to_neither.pop()

    def chose_switches(self, to_research: PlanetFocusInfo | None, to_industry: PlanetFocusInfo | None) -> None:
        """
        Decide which of the two candidates should switch between industry and research, if any.
        """
        debug(f"chose_switches, candidates: research={to_research or '-'}, industry={to_industry or '-'}")
        tr_gain = to_research.difference(RESEARCH, INDUSTRY) if to_research else USELESS_RATING
        ti_gain = to_industry.difference(INDUSTRY, RESEARCH) if to_industry else USELESS_RATING
        if tr_gain > 0 and ti_gain > 0:
            debug("both are better with the new focus")
            self.bake_future_focus(to_research.planet.id, RESEARCH)
            self.bake_future_focus(to_industry.planet.id, INDUSTRY)
        else:
            pp, rp, pp_per_priority, rp_per_priority = self.get_planned_pp_rp_per_prio()
            current_best = self.balanced_output_value(self.priority_industry, pp, self.priority_research, rp)
            debug(f"current: {current_best:.2f}")
            if to_research:
                current_best = self.check_switch_for_ratio(to_research, RESEARCH, current_best)
            # TBD: can we skip the second call, if the first was an improvement?
            if to_industry:
                self.check_switch_for_ratio(to_industry, INDUSTRY, current_best)

    def check_switch_for_ratio(self, candidate: PlanetFocusInfo, new_focus: str, old_best: float) -> float:
        """
        Check whether a switch would improve the balanced output.
        If so, do the switch and return the new best value, else return old_best.
        """
        candidate.future_focus = new_focus
        pp, rp, _, _ = self.get_planned_pp_rp_per_prio()
        new_value = self.balanced_output_value(self.priority_industry, pp, self.priority_research, rp)
        debug(f"With switch to {new_focus}: {new_value:.2f}")
        if new_value < old_best:
            candidate.future_focus = candidate.current_focus
        self.bake_future_focus(candidate.planet.id, candidate.future_focus)
        return max(old_best, new_value)

    @staticmethod
    def balanced_output_value(prio1: float, production1: float, prio2: float, production2: float) -> float:
        """Evaluate possible production values with a preference for a production ratio equal to the priority ratio."""
        return prio1 * production1 + prio2 * production2 + min(prio1 * production2, prio2 * production1)


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

            if pinfo.current_focus in pinfo.possible_output:
                old_pp, old_rp, *_ = pinfo.possible_output[pinfo.current_focus]
                current_industry_target += old_pp
                current_research_target += old_rp

            if pinfo.future_focus in pinfo.possible_output:
                future_pp, future_rp, *_ = pinfo.possible_output[pinfo.future_focus]
                new_industry_target += future_pp
                new_research_target += future_rp

            industry_pp, industry_rp, *_ = (
                pinfo.possible_output[INDUSTRY] if INDUSTRY in pinfo.possible_output else (future_pp, future_rp, 0, 0)
            )
            all_industry_industry_target += industry_pp
            all_industry_research_target += industry_rp

            research_pp, research_rp, *_ = (
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
                current_pp, curren_rp, *_ = pinfo.current_output
                ot_pp, ot_rp, *_ = pinfo.possible_output.get(old_focus, (0, 0, 0, 0))
                nt_pp, nt_rp, *_ = pinfo.possible_output[new_focus]
                debug(
                    Reporter.table_format,
                    "pID (%3d) %22s" % (pid, pinfo.planet.name[-22:]),
                    f"c: {curren_rp:5.1f} / {current_pp:5.1f}",
                    f"cT: {ot_rp:5.1f} / {ot_pp:5.1f}",
                    "cF: %8s" % _focus_name(old_focus),
                    "nF: %8s" % _focus_name(new_focus),
                    f"cT: {nt_rp:5.1f} / {nt_pp:5.1f}",
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
                f"{population:.1f}/{max_population:.1f}",
            )
        info(foci_table)
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
