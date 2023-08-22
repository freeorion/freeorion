from __future__ import annotations

import freeOrionAIInterface as fo
from collections.abc import Iterable
from copy import copy
from logging import debug, error
from typing import Callable, NamedTuple

import PlanetUtilsAI
from AIDependencies import Tags
from aistate_interface import get_aistate
from common.fo_typing import PlanetId, SpeciesName
from DiplomaticCorp import get_diplomatic_status
from EnumsAI import FocusType, PriorityType
from freeorion_tools import (
    assertion_fails,
    get_named_int,
    get_named_real,
    get_species_influence,
)
from freeorion_tools.caching import cache_for_current_turn
from freeorion_tools.statistics import stats
from ResearchAI import research_now
from turn_state import get_empire_planets_by_species, luxury_resources

economic_category = "ECONOMIC_CATEGORY"
industrialism = "PLC_INDUSTRIALISM"
technocracy = "PLC_TECHNOCRACY"
centralization = "PLC_CENTRALIZATION"  # currently unused
bureaucracy = "PLC_BUREAUCRACY"
capital_markets = "PLC_CAPITAL_MARKETS"
black_markets = "PLC_BLACK_MARKET"
traffic_control = "PLC_TRAFFIC_CONTROL"
infra1 = "PLC_PLANETARY_INFRA"
infra2 = "PLC_SYSTEM_INFRA"
infra3 = "PLC_INTERSTELLAR_INFRA"

military_category = "MILITARY_CATEGORY"
allied_repair = "PLC_ALLIED_REPAIR"
charge = "PLC_CHARGE"
scanning = "PLC_CONTINUOUS_SCANNING"
simplicity = "PLC_DESIGN_SIMPLICITY"
engineering = "PLC_ENGINEERING"
exploration = "PLC_EXPLORATION"
flanking = "PLC_FLANKING"
recruitment = "PLC_MARINE_RECRUITMENT"
# martial law and terror suppression currently ignored

social_category = "SOCIAL_CATEGORY"
propaganda = "PLC_PROPAGANDA"
algo_research = "PLC_ALGORITHMIC_RESEARCH"
liberty = "PLC_LIBERTY"
diversity = "PLC_DIVERSITY"
artisans = "PLC_ARTISAN_WORKSHOPS"
population = "PLC_POPULATION"
racial_purity = "PLC_RACIAL_PURITY"
conformance = "PLC_CONFORMANCE"
conformance_exclusions = {liberty, diversity, artisans}

initial_influence_priority = 18.0


class _EmpireOutput:
    def __init__(self, has_liberty: bool):
        self.industry = 0.0
        self.research = 0.0
        self.influence = 0.0
        self.population_stability = 0.0
        self.stability_scaling = 0.5
        self.has_liberty = has_liberty

    def __str__(self):
        return "pp=%.1f, rp=%.1f, ip=%.1f, population_stabililty=%d" % (
            self.industry,
            self.research,
            self.influence,
            self.population_stability,
        )

    def difference(self, other: _EmpireOutput) -> float:
        """Return how much this is better (>0) or worse (<0) than other."""
        aistate = get_aistate()
        delta_pp = (self.industry - other.industry) * aistate.get_priority(PriorityType.RESOURCE_PRODUCTION)
        delta_rp = (self.research - other.research) * aistate.get_priority(PriorityType.RESOURCE_RESEARCH)
        delta_ip = (self.influence - other.influence) * aistate.get_priority(PriorityType.RESOURCE_INFLUENCE)
        delta_stability = self.population_stability - other.population_stability
        result = delta_pp + delta_rp + delta_ip + delta_stability
        debug(f"difference: {delta_pp} + {delta_rp} + {delta_ip} + {delta_stability} => {result}")
        return result

    def is_better_than(self, other: _EmpireOutput, cost_for_other: float) -> bool:
        """
        Return True if this is better than changing to other at given IP cost.
        I.e. if is_better_than returns False, changing to other is considered worth the cost.
        """
        # delta is per round, adoption cost is a one-time cost
        cost = cost_for_other * get_aistate().get_priority(PriorityType.RESOURCE_INFLUENCE) / 10
        return self.difference(other) + cost > 0

    def add_planet(self, planet: fo.planet) -> None:
        """Add output of the given planet to this object."""
        self.industry += planet.currentMeterValue(fo.meterType.targetIndustry)
        self.research += planet.currentMeterValue(fo.meterType.targetResearch)
        self.influence += planet.currentMeterValue(fo.meterType.targetInfluence)
        current_population = planet.currentMeterValue(fo.meterType.population)
        target_population = planet.currentMeterValue(fo.meterType.targetPopulation)
        # current is what counts now, but look a little ahead
        population = (3 * current_population + target_population) / 4
        # no bonuses above 20
        stability = min(planet.currentMeterValue(fo.meterType.targetHappiness), 20)
        if stability < -1:
            # increase both for the risk of losing the colony
            population += 1
            stability -= 1
        elif stability >= 10:
            # 10 give a lot of bonuses, increases above 10 are less important
            stability = 11 + (stability - 10) / 2
        if self.has_liberty and planet.focus == FocusType.FOCUS_RESEARCH:
            self.research += PlanetUtilsAI.adjust_liberty(planet, population)
        self.population_stability += population * stability * self.stability_scaling


class _Alternative(NamedTuple):
    output: _EmpireOutput
    costs: float
    adopted: set


class PolicyManager:
    """Policy Manager for one round"""

    def __init__(self, status_only: bool = False):
        self._empire = fo.getEmpire()
        self._universe = fo.getUniverse()
        self._aistate = get_aistate()
        # resourceAvailable includes this turns production, but that is wrong for influence
        self._ip = self._empire.resourceAvailable(fo.resourceType.influence) - self._get_infl_prod()
        self._adopted = set(self._empire.adoptedPolicies)
        # When we continue a game in which we just adopted a policy, game state shows the policy as adopted,
        # but IP still unspent. Correct it here, then calculate anew whether we want to adopt it.
        if not status_only:
            for entry in self._empire.turnsPoliciesAdopted:
                if entry.data() == fo.currentTurn():
                    debug(f"reverting saved adopt {entry.key()}")
                    fo.issueDeadoptPolicyOrder(entry.key())
                    self._adopted.remove(entry.key())
        self._originally_adopted = copy(self._adopted)
        empire_owned_planet_ids = PlanetUtilsAI.get_owned_planets_by_empire()
        self._populated_planet_ids = PlanetUtilsAI.get_populated_planet_ids(empire_owned_planet_ids)
        self._num_populated = len(self._populated_planet_ids)
        self._num_outposts = len(empire_owned_planet_ids) - self._num_populated
        self._wanted_ip = 0.0
        self._adoptable = self._get_adoptable()
        self._available = set(self._empire.availablePolicies)
        self._rating_functions = {
            propaganda: self._rate_propaganda,
            algo_research: self._rate_algo_research,
            diversity: self._rate_diversity,
            artisans: self._rate_artisans,
            population: self._rate_population,
            liberty: lambda: 10,  # hack, only used by _try_without_conformance
            conformance: lambda: 999,  # not an actual rating, but _deadopt_one_of fails without it
            # military policies are mostly chosen by opinion, plus some rule-of-thumb values
            allied_repair: lambda: self._rate_opinion(charge),  # no effect, unless we have allies...
            charge: lambda: self._rate_opinion(charge),  # increases attack, reduces shields
            scanning: lambda: 10 + self._rate_opinion(scanning),  # May help us detect stealthy attackers
            simplicity: lambda: 20 + self._rate_opinion(simplicity),  # Makes simple ships cheaper
            engineering: self._rate_engineering_corps,
            exploration: lambda: 10 + self._rate_opinion(exploration),  # may give a little research, speeds up scouts
            flanking: lambda: 5 + self._rate_opinion(flanking),  # May give small a bonus in battle
            recruitment: lambda: 15 + self._rate_opinion(recruitment),  # cheaper troop ships
            # economic policies
            bureaucracy: self._rate_bureaucracy,
            capital_markets: self._rate_capital_markets,
            black_markets: self._rate_black_markets,
            traffic_control: lambda: 10 + self._rate_opinion(traffic_control),  # slightly faster ships
        }

    # Policies that may use IP "reserved" by wanted_ip.
    # The first two get IP specially reserved for them in the constructor.
    # If we are low on IP, adopting something that helps as gain IP should be worth it.
    # Technocracy and industrialism are important overall.
    _prioritised_policies = (propaganda, artisans, technocracy, industrialism)

    # This value is increased by ResourceAI via report_focus_change(). Generate orders copies the value into
    # a member variable and reset this value so that we can count anew next turn.
    _num_focus_changes = 0

    @staticmethod
    def report_focus_change() -> None:
        """
        Function for ResourceAI to report number of focus changes done this turn.
        Information affects whether we adapt or deadopt bureaucracy.
        """
        PolicyManager._num_focus_changes += 1

    def generate_orders(self) -> None:
        """The main task of the class, called once every turn by FreeOrionAI."""
        debug(
            "Start turn %d IP: %.2f + %.2f, adopted: %s, adoptable: %s",
            fo.currentTurn(),
            self._ip,
            self._get_infl_prod(),
            self._adopted,
            self._adoptable,
        )
        debug(f"Empty slots: {[str(s) for s in self._empire.emptyPolicySlots]}")
        self._process_social()
        self._process_military()
        self._process_economic()
        new_production = self._get_infl_prod(True)
        debug("End of turn IP: %.2f (wanted %.2f) + %.2f", self._ip, self._wanted_ip, new_production)
        self._determine_influence_priority(new_production)
        PolicyManager._num_focus_changes = 0

    def _process_social(self) -> None:
        """
        Process social policies.
        Propaganda is activated in turn 1 and usually replaced by liberty in 7.
        Since liberty and conformance affect so many other things, we check them first, then simply rate all others and
        try to adopt the best possible ones for the slots we have.
        """
        self._conformance_or_liberty()
        options = {propaganda, algo_research, diversity, artisans, population}
        if conformance in self._adopted:
            options -= conformance_exclusions
        self._process_policy_options(social_category, options)

    def _process_military(self) -> None:
        """Process military policies."""
        options = (allied_repair, charge, scanning, simplicity, engineering, exploration, flanking, recruitment)
        self._process_policy_options(military_category, options)

    def _process_economic(self) -> None:
        """Process economic policies."""
        # We need the extra slots, before adopting others economic policies.
        # Starting with centralization may make sense, but it's too complicated to program for the little gain.
        if infra1 in self._adopted:
            self._techno_or_industry()
            self._process_infrastructure23()
            if technocracy not in self._empire.availablePolicies and self._can_adopt(bureaucracy):
                # we have to adopt bureaucracy at least once at the beginning to unlock technocracy and IRAs
                self._adopt(bureaucracy)
            else:
                options = (bureaucracy, capital_markets, black_markets, traffic_control)
                self._process_policy_options(economic_category, options)
        elif infra1 in self._adoptable:
            self._adopt(infra1)

    def _process_policy_options(self, category: str, options: Iterable[str]) -> None:  # noqa: C901
        """
        Rate all given policies and deadopt policies with a negative rating.
        Then evaluate if any of the other is worth being adopted, possible even to replace another one.
        If we find a policy should be adopted, but lack the IP, increase _wanted_ip.
        Given policies must all have the same category.
        """
        adopted = []
        may_adopt = []
        for policy in options:
            if not self._potentially_available(policy):
                continue
            rating = self._rate_policy(policy)
            cost = fo.getPolicy(policy).adoptionCost()
            debug(f"_process_policy_options {policy} adopted={policy in self._adopted} rating={rating} cost={cost}")
            if policy in self._adopted:
                if rating < 0:
                    self._deadopt(policy)
                else:
                    adopted.append((rating, policy))
            elif rating > 6:
                may_adopt.append((rating, policy, cost))
        if may_adopt:
            adopted.sort()
            may_adopt.sort()
            # Policies do not change too often, one per turn should do
            new_rating, new_policy, new_cost = may_adopt.pop()
            rated_cost = self._rate_costs(new_cost)
            debug(
                f"_process_policy_options({category}): best new={new_policy}, rating={new_rating}, cost={new_cost}"
                f", rated_cost={rated_cost}, slots={self._empire.emptyPolicySlots[category]}"
            )
            if self._empire.emptyPolicySlots[category]:
                if new_rating > rated_cost:
                    self._we_want(new_policy, new_cost)
            elif adopted:
                old_rating, old_policy = adopted.pop(0)
                debug(f"_process_policy_options({category}): lowest adopted={old_policy}, rating={old_rating}")
                if new_rating - old_rating > rated_cost:
                    self._we_want(new_policy, new_cost, old_policy)

    def _we_want(self, name: str, cost: float, replace: str | None = None) -> None:
        if name not in self._empire.availablePolicies:
            self._try_unlock(name)
            return
        if self._can_adopt(name, replace):
            if replace:
                self._deadopt(replace)
            self._adopt(name)
        else:
            # We seem to have insufficient IP. Add cost to wanted_ip, to raise the influence priority
            self._wanted_ip += cost
            debug(f"Adopting {name} failed due to insufficient IP, adding {cost} to wanted_ip")

    def _unlocked_by(self, name: str) -> str | Callable | None:
        """
        Returns another policy that unlocks the given policy, a function to unlock it, or None.
        Returned functions can be called with False to check whether it could unlock the given policy.
        """
        if name == artisans:
            # Some day we should get that information from the server, but then, if it changes,
            # we need to adapt the AI anyway.
            return diversity
        if name == population:
            return self._unlock_population
        return None

    def _try_unlock(self, name: str) -> None:
        """Unlock the given policy, if possible. Note that unlocking always takes at least one turn."""
        unlocker = self._unlocked_by(name)
        if isinstance(unlocker, str):
            debug(f"Trying to adopt {unlocker} to unlock {name}")
            self._we_want(unlocker, fo.getPolicy(unlocker).adoptionCost())
        else:
            unlocker(True)

    def _unlock_population(self, do_unlock: bool) -> bool:
        """
        Unlock population by putting GRO_NANOTECH_MED and its prerequisites at the top of the research queue.
        Until Adaptive Automation has been researched, we do not want to use research points for unlocking
        population. Also, AA requires Nanotech Production, which is the most expensive prerequisite
        of Nanotech Medicine. So if AA has not been researched, this function only returns false.
        If do_unlock is false, the function only returns whether we could unlock it, i.e. AA is available.
        """
        # TBD: create Enum for technologies
        could_do = self._empire.techResearched("PRO_ADAPTIVE_AUTOMATION")
        if could_do and do_unlock:
            unlocker = "GRO_NANOTECH_MED"
            debug(f"Prioritising research for {unlocker} to unlock population")
            research_now(unlocker, True)
        return could_do

    @cache_for_current_turn
    def _rate_policy(self, name: str) -> float:
        """Gives a rough value of a policy."""
        rating_function = self._rating_functions.get(name)
        if assertion_fails(rating_function, f"_rate_policy({name}) not yet supported"):
            return 0.0
        return rating_function()

    def _rate_costs(self, costs: float) -> float:
        """
        How do we rate the costs in comparison to a policies rating?
        Generally costs / 3, but less if we have plenty of IP.
        """
        # avoid division by zero, with unreserved_ip < 1 the policy won't be adopted anyway
        unreserved_ip = max(1, self._ip - self._wanted_ip)
        # For big empires, costs of some policies go over 100 and their rating doesn't grow that much, but they may
        # also have lots of IP and the policy may still be useful. Note that costs should be less than unreserved_ip,
        # so if cost is e.g. 1/3 of unreserved_ip, this limits the rating to 0.5 * priority.
        return min(costs / 3, 1.5 * self._get_influence_priority() * costs / unreserved_ip)

    @staticmethod
    def _rate_opinion(name: str) -> float:
        """Return a rating value for the empire's planet opinion on a policy"""
        opinion = PlanetUtilsAI.get_planet_opinion(name)
        return 1.5 * (len(opinion.likes) - len(opinion.dislikes) * PlanetUtilsAI.dislike_factor())

    def _rate_propaganda(self) -> float:
        """Rate propaganda broadcasts"""
        ip_gain = get_named_real("PROPAGANDA_INFLUENCE_FLAT")
        ip_enemy = get_named_real("PROPAGANDA_INFLUENCE_ENEMY_FLAT")
        # Is there an easy way to determine how many enemies we see? For simplicity assume we see half of them.
        # Note that the enemy effect may be our gain (if enemy also uses propaganda), or the other player's loss.
        num_enemies = sum(
            0.5 for e in fo.allEmpireIDs() if e != fo.empireID() and get_diplomatic_status(e) == fo.diplomaticStatus.war
        )
        rating = self._get_influence_priority() * (ip_gain - num_enemies * ip_enemy) + self._rate_opinion(propaganda)
        debug(f"_rate_propaganda: rating={rating}")
        return rating

    def _rate_algo_research(self) -> float:
        """Rate algorithmic research"""
        min_stability = get_named_real("LRN_ALGO_RESEARCH_MIN_STABILITY")
        scaling = get_named_real("LRN_ALGO_RESEARCH_TARGET_RESEARCH_PERCONSTRUCTION")
        research_priority = self._aistate.get_priority(PriorityType.RESOURCE_RESEARCH)
        gain = sum(
            scaling
            * research_priority
            * min(
                planet.currentMeterValue(fo.meterType.population),
                planet.currentMeterValue(fo.meterType.construction),
            )
            for planet in PlanetUtilsAI.get_empire_populated_planets()
            if planet.focus == FocusType.FOCUS_RESEARCH
            and planet.currentMeterValue(fo.meterType.targetHappiness) >= min_stability
        )
        rating = gain + self._rate_opinion(algo_research)
        debug(f"_rate_algo_research: rating={rating}")
        return rating

    def _conformance_or_liberty(self) -> _EmpireOutput:
        """
        Try policies conformance and liberty.
        Liberty is available very early, but to help setting up basic policies we keep propaganda at least
        until we have infra1. Getting a second slot before that should be nearly impossible.
        """
        if infra1 in self._adopted:
            debug("Evaluating conformance/liberty, first without change:")
            current_output = self._calculate_empire_output()
            if conformance in self._adopted:
                # this may also try liberty
                self._try_without_conformance(current_output)
            else:
                current_output, conformance_cost = self._try_conformance(current_output)
                self._process_liberty(current_output, conformance_cost)

    def _try_without_conformance(self, current_output: _EmpireOutput) -> None:
        """
        Consider de-adopting conformance.
        Conformance always improves stability (even when disliked). Theoretically this could be negative when
        using black markets and/or necessity. So far we ignore this and only try de-adopting it when we can
        adopt one or more of its exclusions instead.
        """
        if not any(self._can_adopt(p, conformance) for p in conformance_exclusions):
            return
        self._deadopt(conformance)
        # For this we use a fixed, low rating for liberty since later in the game (it must be later, since we have
        # conformance) it is usually not so great. Even more so when the empire was using conformance.
        adopt_options = sorted([(self._rate_policy(p), p) for p in conformance_exclusions], reverse=True)
        best = _Alternative(current_output, 0.0, set())
        costs = 0.0
        adopted = set()
        # Note that this function does not unlock artisans. Perhaps it should?
        # _process_policy_options cannot deadopt conformance to do it.
        for rating, policy in adopt_options:
            if rating > 0.0 and self._can_adopt(policy):
                self._adopt(policy)
                costs += fo.getPolicy(policy).adoptionCost()
                adopted.add(policy)
                new_output = self._calculate_empire_output()
                if new_output.is_better_than(best.output, costs - best.costs):
                    best = _Alternative(new_output, costs, adopted)
        for policy in adopted - best.adopted:
            # best wasn't the last one, so undo all further adoptions
            self._deadopt(policy)
        if not best.adopted:
            # No alternative was better than current_output with conformance.
            self._adopt(conformance)

    def _try_conformance(self, current_output: _EmpireOutput) -> tuple[_EmpireOutput, float]:
        """
        Try adopting conformance, may replace another social policies.
        Conformance is rather good, but has several exclusions. So this may have to de-adopt all exclusions just
        to try adoption conformance.
        """
        exclusions = conformance_exclusions & self._adopted
        other_could_replace = {propaganda, algo_research, population} & self._adopted
        adoption_cost = fo.getPolicy(conformance).adoptionCost()
        if self._can_adopt(conformance):
            self._adopt(conformance)
            with_conformance = self._calculate_empire_output()
            if current_output.is_better_than(with_conformance, adoption_cost):
                self._deadopt(conformance)
                return current_output, 0
            return with_conformance, adoption_cost
        elif self._can_adopt(conformance, exclusions | other_could_replace):
            if exclusions:
                for policy in exclusions:
                    self._deadopt(policy)
            else:
                self._deadopt_one_of(other_could_replace)
            self._adopt(conformance)
            with_conformance = self._calculate_empire_output()
            if current_output.is_better_than(with_conformance, adoption_cost):
                self._deadopt(conformance)
                if exclusions:
                    for policy in exclusions:
                        self._adopt(policy)
                else:
                    self._readopt_selected_one()
                return current_output, 0
            return with_conformance, adoption_cost
        # cannot adopt it at all
        return current_output, 0

    def _process_liberty(self, current_output: _EmpireOutput, conformance_cost: float) -> None:
        """
        Adopt or deadopt liberty, may replace another social policy.
        """
        could_replace = {propaganda, algo_research, diversity, artisans} & self._adopted
        if conformance in self._adopted:
            could_replace = {conformance}  # cannot adopt them together
        if liberty in self._adopted or self._can_adopt(liberty, could_replace):
            adoption_cost = 0.0 if liberty in self._originally_adopted else fo.getPolicy(liberty).adoptionCost()
            # liberty will generally make species less happy, but generates research
            if liberty in self._adopted:
                self._deadopt(liberty)
                without_liberty = self._calculate_empire_output()
                if current_output.is_better_than(without_liberty, 0.0):
                    self._adopt(liberty)
            elif self._can_adopt(liberty):
                self._adopt(liberty)
                with_liberty = self._calculate_empire_output()
                if current_output.is_better_than(with_liberty, adoption_cost):
                    self._deadopt(liberty)
            else:
                # Can only adopt it by replacing something. Note that we ignore the other replaced policies rating
                # here, since its effect is also evaluated by _calculate_empire_output.
                self._deadopt_one_of(could_replace)
                self._adopt(liberty)
                with_liberty = self._calculate_empire_output()
                # Current cost is only relevant when we adopted conformance in _try_conformance. Then we must compare
                # current_output with conformance to with_liberty at the cost difference of cost.
                if current_output.is_better_than(with_liberty, adoption_cost - conformance_cost):
                    self._deadopt(liberty)
                    self._readopt_selected_one()

    def _calculate_empire_output(self) -> _EmpireOutput:
        """Update empire meters, then calculate an estimation of the expected empire output."""
        # TBD: here we could use the stability-adapted update, that would be much better than trying
        # to rate the stability effects again production effects.
        self._universe.updateMeterEstimates(self._populated_planet_ids)
        result = _EmpireOutput(liberty in self._adopted)
        for pid in self._populated_planet_ids:
            result.add_planet(self._universe.getPlanet(pid))
        debug(f"Empire output: {result}")
        return result

    def _rate_diversity(self) -> float:
        """Rate diversity."""
        if diversity not in self._empire.availablePolicies:
            return 0.0
        # diversity affects stability, but also gives a bonus to research-focused planets and a little influence,
        diversity_value = len(get_empire_planets_by_species()) - get_named_int("PLC_DIVERSITY_THRESHOLD")
        diversity_scaling = get_named_real("PLC_DIVERSITY_SCALING")
        # Research bonus goes to research-focused planets only. With priority there are usually none.
        research_priority = self._aistate.get_priority(PriorityType.RESOURCE_RESEARCH)
        research_bonus = max(0, research_priority - 20)
        # + 4 for global influence
        global_influence_bonus = 4
        rating = self._rate_opinion(diversity) + diversity_scaling * diversity_value * (
            self._num_populated + global_influence_bonus + research_bonus
        )
        debug(
            f"_rate_diversity: rating={rating}. diversity_value={diversity_value}, "
            f"research priority={research_priority}"
        )
        return rating

    def _rate_artisans(self) -> float:
        """Rate artisan workshops."""
        # Diversity unlocks artisans, so if we have diversity, we could unlock artisans
        if diversity not in self._empire.availablePolicies:
            return 0.0
        # TBD: could use _calculate_empire_output, but we'd still have to check the species, since artistic
        # species may not be switched to influence.
        rating = 0.0
        artists = []
        for species_name, planets in get_empire_planets_by_species().items():
            species = fo.getSpecies(species_name)
            if Tags.ARTISTIC in species.tags:
                artists.append(species_name)
                for pid in planets:
                    rating += self._rate_artisan_planet(pid, species_name)
        rating += self._rate_opinion(artisans)
        debug(f"_rate_artisans: {rating}, artists: {artists}")
        return rating

    def _rate_artisan_planet(self, pid: PlanetId, species_name: SpeciesName) -> float:
        focus_bonus = get_named_real("ARTISANS_INFLUENCE_FLAT_FOCUS")
        focus_minimum = get_named_real("ARTISANS_MIN_STABILITY_FOCUS")
        species_focus_bonus = focus_bonus * get_species_influence(species_name)
        planet = self._universe.getPlanet(pid)
        stability = planet.currentMeterValue(fo.meterType.targetHappiness)
        # First check whether the planet would currently get the focus bonus.
        if planet.focus == FocusType.FOCUS_INFLUENCE:
            return 3 * species_focus_bonus if stability >= focus_minimum else 0.0

        # Planet does not have influence focus...
        # Check for the non-focus bonus. Since we would get this "for free", rate it higher
        non_focus_bonus = get_named_real("ARTISANS_INFLUENCE_FLAT_NO_FOCUS")
        non_focus_minimum = get_named_real("ARTISANS_MIN_STABILITY_NO_FOCUS")
        rating = 0.0
        if stability >= non_focus_minimum:
            rating += 4 * non_focus_bonus
        # Check whether this planet would get the focus bonus, if we'd switch it to influence.
        if PlanetUtilsAI.stability_with_focus(planet, FocusType.FOCUS_INFLUENCE) >= focus_minimum:
            rating += species_focus_bonus
        return rating

    def _rate_engineering_corps(self) -> float:
        """
        Rate Engineering Corps.
        This policy reduces ship upkeep at the cost of influence, it won't do us any good if we are short on
        influence, but otherwise may help us maintain big fleets.
        """
        ships_owned = self._empire.totalShipsOwned
        debug(f"totalShipsOwned={ships_owned}")
        influence_priority = self._get_influence_priority()
        # since influence_priority tends to vary strongly, do not return negative unless
        influence_priority_threshold = 40
        if influence_priority > 2 * influence_priority_threshold:
            return -1  # any negative value means, deadopt
        if influence_priority > influence_priority_threshold:
            return 0  # do not deadopt yet
        opinion = self._rate_opinion(engineering)
        return opinion + ships_owned / 25 * (influence_priority_threshold - influence_priority)

    def _rate_population(self) -> float:
        rating = self._rate_opinion(population)
        for pid in self._populated_planet_ids:
            planet = self._universe.getPlanet(pid)
            current_population = planet.currentMeterValue(fo.meterType.population)
            target_population = planet.currentMeterValue(fo.meterType.targetPopulation)
            ratio = min(1.0, current_population / max(target_population, 0.01))
            empty_weight = 5
            # Almost empty_weight for a newly found colony on a big planet, half weight for half full, etc.
            rating += empty_weight * (1 - ratio)
        return rating

    def _rate_bureaucracy(self) -> float:
        if bureaucracy not in self._adopted and not self._can_adopt(bureaucracy):
            return 0.0
        debug(f"Evaluating bureaucracy, first without change. Focus changes: {self._num_focus_changes}")
        current_output = self._calculate_empire_output()
        focus_change_cost = self._bureaucracy_focus_change_penalty() * self._num_focus_changes
        if bureaucracy in self._adopted:
            self._deadopt(bureaucracy)
            without_bureaucracy = self._calculate_empire_output()
            rating = current_output.difference(without_bureaucracy)
            self._adopt(bureaucracy)
        else:
            self._adopt(bureaucracy)
            with_bureaucracy = self._calculate_empire_output()
            rating = with_bureaucracy.difference(current_output)
            self._deadopt(bureaucracy)
        return rating - focus_change_cost

    @staticmethod
    def _could_be_set_to_influence(planet: fo.planet) -> bool:
        """Does the given planet has a population that could be set to influence focus?"""
        return planet.speciesName and FocusType.FOCUS_INFLUENCE in fo.getSpecies(planet.speciesName).foci

    def _rate_capital_markets(self) -> float:
        """This has a lot of effects, add a fixed value for others, check the luxury export and influence dept."""
        rating = self._rate_opinion(capital_markets) + 10
        if self._ip < 0.0:
            # when we have an influence dept, capital_markets increases the penalty
            rating += self._ip * get_named_real("CAPITAL_MARKETS_DEBT_INSTABILITY") * 10
        for special, planets in luxury_resources().items():
            likes = PlanetUtilsAI.get_planet_opinion(special).likes
            # TBD: check for supply chains? Well, there are many more places we could/should do that.
            # Also, when we do, we'd have to be careful not to change everything just because an enemy
            # disrupts our supply chain temporarily.
            if any(planet.focus == FocusType.FOCUS_INFLUENCE for planet in planets):
                rating += 1.5 * len(likes)
            elif any(self._could_be_set_to_influence(planet) for planet in planets):
                rating += 0.5 * len(likes)
        return rating

    def _rate_black_markets(self) -> float:
        """This is a tricky one since it requires planets with low stability. But it may be useful."""
        max_stability = get_named_real("PLC_BLACK_MARKET_MAX_STABILITY")
        bonus = get_named_real("SPECIAL_BLACK_MARKET_INFLUENCE_FOCUS_BONUS")
        gain = 0.0
        for special, planets in luxury_resources().items():
            likes = PlanetUtilsAI.get_planet_opinion(special).likes
            for planet in PlanetUtilsAI.get_empire_populated_planets():
                if planet.id in likes:
                    skill = get_species_influence(planet.speciesName)
                    if planet.focus == FocusType.FOCUS_INFLUENCE:
                        if planet.currentMeterValue(fo.meterType.targetHappiness) <= max_stability:
                            gain += bonus * skill
                    else:
                        if (
                            self._could_be_set_to_influence(planet)
                            and PlanetUtilsAI.stability_with_focus(planet, FocusType.FOCUS_INFLUENCE) <= max_stability
                        ):
                            gain += bonus * skill / 3.0
        return self._rate_opinion(black_markets) + gain * 4.0

    def _techno_or_industry(self) -> None:
        """Adopt technocracy or industrialism, depending on AI priorities."""
        research_prio = self._aistate.get_priority(PriorityType.RESOURCE_RESEARCH)
        production_prio = self._aistate.get_priority(PriorityType.RESOURCE_PRODUCTION)
        may_switch_to_techno = self._can_adopt(technocracy, industrialism)
        may_switch_to_industry = self._can_adopt(industrialism, technocracy)
        # do not switch too often
        if technocracy in self._adopted:
            research_prio *= 1.25
        elif industrialism in self._adopted:
            production_prio *= 1.25
        if may_switch_to_industry and research_prio < production_prio:
            self._deadopt(technocracy)
            self._adopt(industrialism)
        elif may_switch_to_techno and research_prio > production_prio:
            self._deadopt(industrialism)
            self._adopt(technocracy)
        debug(
            "Prio res./ind. = %.1f/%.1f, maySwitch = %s/%s",
            research_prio,
            production_prio,
            may_switch_to_techno,
            may_switch_to_industry,
        )

    @cache_for_current_turn
    def _bureaucracy_focus_change_penalty(self):
        """
        How much influence do we lose due to a focus change while bureaucracy is adopted?
        With bureaucracy a focus change set influence to -3, the overall effect depends on how quickly influence
        productions raises.
        """
        have_capital_markets = capital_markets in self._adopted
        have_transorganic_sentience = self._empire.techResearched("GRO_TRANSORG_SENT")
        cost = 3.0 if have_capital_markets else 4.0 if have_transorganic_sentience else 6.0
        # this is a one-time effect, so do not rate it too high
        return cost * 0.5

    def _process_infrastructure23(self) -> None:
        """Try to adopt infrastructure2 and 3."""
        if infra3 in self._adopted:
            return
        # Note that bureaucracy is not in the list, because de-adopting it will likely trigger a some focus changes,
        # which will then get penalised when we re-adopt it the next turn.
        could_replace = {industrialism, technocracy, capital_markets, black_markets, traffic_control} & self._adopted
        if infra2 not in self._adopted:
            if self._can_adopt(infra2, could_replace) and self._get_influence_priority() < 18:
                self._maybe_adopt_infrastructure(infra2, could_replace)
        else:
            if self._can_adopt(infra3, could_replace) and self._get_influence_priority() < 12:
                self._maybe_adopt_infrastructure(infra3, could_replace)

    def _maybe_adopt_infrastructure(self, policy: str, could_replace: Iterable[str]) -> None:
        """
        Adopts the specified policy, if we have a free slot.
        Without a slot, it deadopts the cheapest of could_replace, but only if we have enough IP to readopt it
        next turn with some more to spare. Note that adopting infra3 gives us an extra slot, so we may want
        to adopt two policies afterwards.
        """
        if self._can_adopt(policy):
            self._adopt(policy)
        elif could_replace:
            # deadopt the cheapest when we have enough IP to re-adopt it next turn without getting too low.
            choices = sorted([(fo.getPolicy(name).adoptionCost(), name) for name in could_replace])
            policy_cost = fo.getPolicy(policy).adoptionCost()
            if self._ip - self._wanted_ip >= policy_cost + 2 * choices[0][0]:
                self._deadopt(choices[0][1])
                self._adopt(policy)

    def _determine_influence_priority(self, new_production: float) -> None:
        """Determine and set influence priority."""
        # avoid wildly varying priorities while we adopt the basics: simply a fixed value for some turns
        if fo.currentTurn() <= 15:
            if fo.currentTurn() == 1:
                # I'd prefer 20, but that makes Abadoni switch to Influence in turn 3.
                # TBD work on ResourceAI again...
                self._set_priority(initial_influence_priority, True)
            return
        # How much IP would we have available if we keep the current production for 3 turns?
        forecast = self._ip - self._wanted_ip + 3 * new_production
        threshold = 20
        if forecast < threshold:
            # Basic value: the lower the forecast, the more urgent we need influence
            priority = 20 + threshold - forecast
        else:
            # For higher values, slowly decrease priority
            priority = 20 * (threshold / forecast) ** 0.6
        # The more planets we have, the more influence we need in general, outpost have only a minor effect
        priority *= (3 + self._num_populated + self._num_outposts / 10) / 10
        if infra3 in self._adopted:
            priority /= 1.5  # we've done the big step, no need to amass so much anymore
        self._set_priority(priority, False)

    def _set_priority(self, calculated_priority: float, ignore_old: bool) -> None:
        """Set and log influence priority."""
        if ignore_old:
            new_priority = calculated_priority
        else:
            old_priority = self._get_influence_priority()
            # to further smoothen the values, only use 1/3 of the calculated value
            new_priority = (2 * old_priority + calculated_priority) / 3
        debug("Setting influence priority to %.1f, turn %d", new_priority, fo.currentTurn())
        self._aistate.set_priority(PriorityType.RESOURCE_INFLUENCE, new_priority)

    def _get_infl_prod(self, update=False) -> float:
        """Get / Update IP production."""
        if update:
            self._universe.updateMeterEstimates(self._populated_planet_ids)
            fo.updateResourcePools()
        return self._empire.resourceProduction(fo.resourceType.influence)

    def _potentially_available(self, name: str) -> bool:
        """Determine whether we have a way to adopt given policy."""
        if name in self._available:
            return True
        unlocker = self._unlocked_by(name)
        if unlocker is None:
            return False
        if isinstance(unlocker, str):
            return unlocker in self._available
        else:
            return unlocker(False)

    def _can_adopt(self, name: str, replace: str | set[str] | None = None) -> bool:
        """
        Can we adopt named policy, possibly by replacing (one of) replace?
        Note that when replace_other is set, this function currently assumes that with
        replace_other removed, there are no other exclusions for policy(name).
        If this won't do, the function needs to temporarily deadopt replace_other or
        policyPrereqsAndExclusionsOK must be extended so that replace_other can be passed to it.
        """
        if name in self._adopted:
            return False
        policy = fo.getPolicy(name)
        wanted = 0.0 if name in self._prioritised_policies else self._wanted_ip
        ret = name in self._empire.availablePolicies and (
            policy.adoptionCost() <= (self._ip - wanted) or name in self._originally_adopted
        )
        if isinstance(replace, str) and replace in self._adopted:
            return ret
        if isinstance(replace, set) and replace & self._adopted:
            return ret
        return (
            ret
            and self._empire.emptyPolicySlots[policy.category] > 0
            and self._empire.policyPrereqsAndExclusionsOK(name)
        )

    def _get_adoptable(self) -> set[str]:
        """List of adoptable policies with the still available IP."""
        return {p for p in self._empire.availablePolicies if self._can_adopt(p)}

    def _adopt(self, name: str) -> None:
        """Find an emtpy slot and adopt named policy, if possible."""
        policy = fo.getPolicy(name)
        category = policy.category
        slot = self._find_empty_slot(category)
        # getting True does not guarantee that is has worked, so doublecheck the policy has been adopted
        if fo.issueAdoptPolicyOrder(name, category, slot) and name in self._empire.adoptedPolicies:
            if name not in self._originally_adopted:
                self._ip -= policy.adoptionCost()
            stats.adopt_policy(name)
            debug(f"Issued adoption order for {name} in slot {slot} turn {fo.currentTurn()}, remaining IP: {self._ip}")
            self._adoptable = self._get_adoptable()
            self._adopted.add(name)
        else:
            error("Failed to adopt %s in slot %d", name, slot)

    def _find_empty_slot(self, category: str) -> int:
        """This assumes there is an empty slot, otherwise adopt will fail."""
        slot = 0
        # there should be a better way...
        cat_slots = self._empire.categoriesSlotPolicies
        if category in cat_slots:
            slots = cat_slots[category]
            for slot in range(0, 99):
                if slot not in slots:
                    break
        return slot

    def _deadopt(self, name: str) -> None:
        """Deadopt name, if it is adopted."""
        if name in self._adopted:
            fo.issueDeadoptPolicyOrder(name)
            stats.deadopt_policy(name)
            self._adopted.remove(name)
            self._adoptable = self._get_adoptable()
            if name not in self._originally_adopted:
                # this is canceling an adoption order of the current turn
                self._ip += fo.getPolicy(name).adoptionCost()
            debug(f"Issued deadoption order for {name} turn {fo.currentTurn()}, remaining IP: {self._ip}")

    def _deadopt_one_of(self, names: set[str]) -> None:
        """
        Deadopt one of the given policies and store its name.
        This can be used after a decision is made, or to temporarily deadopt one for
        decision-making. In the later case, use _readopt_selected_one() to revert it.
        """
        selection = sorted((self._rate_policy(name), name) for name in names)
        debug(f"selection = {selection}")
        for _, name in selection:
            if name in self._adopted:
                self._selected_deadopt = name
                self._deadopt(name)
                break

    def _readopt_selected_one(self) -> None:
        """Re-adopt the policy deadopted by the last call to _deadopt_one_of()."""
        self._adopt(self._selected_deadopt)

    def _get_influence_priority(self) -> float:
        """Return current priority for resource type influence."""
        if fo.currentTurn() == 1:
            # AIState starts with 0, and we only set the value at the end of turn 1
            return initial_influence_priority
        else:
            return self._aistate.get_priority(PriorityType.RESOURCE_INFLUENCE)

    def print_status(self) -> None:
        # only for interactive debugging
        print("\nAdoptable Policies:")
        for p in self._get_adoptable():
            print("  ", p)
        print("Empty Slots:")
        for s in self._empire.emptyPolicySlots:
            print("  ", s)
        print("Adopted:")
        for category in self._empire.categoriesSlotPolicies:
            print(" ", category.key() + ":")
            for slot in category.data():
                print("   ", slot.key(), "->", slot.data(), "turn", self._empire.turnPolicyAdopted(slot.data()))
        print(f"Influence: {self._ip} (wanted: {self._wanted_ip})")
        print("Infl. Prod.:", self._get_infl_prod())
        print("Num Planet (pop./outp.): %d/%d" % (self._num_populated, self._num_outposts))


def generate_policy_orders() -> None:
    PolicyManager().generate_orders()


def print_status() -> None:
    PolicyManager(True).print_status()
