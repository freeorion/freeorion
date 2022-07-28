from __future__ import annotations

import freeOrionAIInterface as fo
from copy import copy
from logging import debug, error
from typing import Callable, Iterable, Optional, Set, Union

import PlanetUtilsAI
from AIDependencies import Tags
from aistate_interface import get_aistate
from common.fo_typing import PlanetId, SpeciesName
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
from turn_state import get_empire_planets_by_species

economic_category = "ECONOMIC_CATEGORY"
industrialism = "PLC_INDUSTRIALISM"
technocracy = "PLC_TECHNOCRACY"
centralization = "PLC_CENTRALIZATION"
bureaucracy = "PLC_BUREAUCRACY"
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
flanking = "PLC_FLANKING_DESC"
recruitment = "PLC_MARINE_RECRUITMENT"
# martial law and terror suppression currently ignored

social_category = "SOCIAL_CATEGORY"
propaganda = "PLC_PROPAGANDA"
algo_research = "PLC_ALGORITHMIC_RESEARCH"
liberty = "PLC_LIBERTY"
diversity = "PLC_DIVERSITY"
artisans = "PLC_ARTISAN_WORKSHOPS"
population = "PLC_POPULATION"


class _EmpireOutput:
    def __init__(self):
        self.industry = 0.0
        self.research = 0.0
        self.influence = 0.0
        self.population_stability = 0.0
        self.stability_scaling = 0.75

    def __str__(self):
        return "pp=%.1f, rp=%.1f, ip=%.1f, population_stabililty=%d" % (
            self.industry,
            self.research,
            self.influence,
            self.population_stability,
        )

    def is_better_than(self, other: _EmpireOutput, cost_for_other: float) -> bool:
        """Return true if this is better than changing to other at given IP cost."""
        aistate = get_aistate()
        delta_pp = (self.industry - other.industry) * aistate.get_priority(PriorityType.RESOURCE_PRODUCTION)
        delta_rp = (self.research - other.research) * aistate.get_priority(PriorityType.RESOURCE_RESEARCH)
        delta_ip = (self.influence - other.influence) * aistate.get_priority(PriorityType.RESOURCE_INFLUENCE)
        delta_stability = self.population_stability - other.population_stability
        # delta is per round, adoption cost is a one-time cost
        cost = cost_for_other * aistate.get_priority(PriorityType.RESOURCE_INFLUENCE) / 10
        result = delta_pp + delta_rp + delta_ip + delta_stability + cost > 0
        debug(f"is_better_than: {delta_pp} + {delta_rp} + {delta_ip} + {delta_stability} + {cost} > 0 => {result}")
        return result

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
        self.population_stability += population * stability * self.stability_scaling


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
        self._max_turn_bureaucracy = self._calculate_max_turn_bureaucracy()
        self._centralization_cost = fo.getPolicy(centralization).adoptionCost()
        self._bureaucracy_cost = fo.getPolicy(bureaucracy).adoptionCost()
        self._wanted_ip = self._wanted_for_bureaucracy()
        self._adoptable = self._get_adoptable()
        self._available = set(self._empire.availablePolicies)
        self._rating_functions = {
            propaganda: lambda: 20 + self._rate_opinion(propaganda),
            algo_research: self._rate_algo_research,
            diversity: self._rate_diversity,
            artisans: self._rate_artisans,
            population: self._rate_population,
            # military policies are mostly chosen by opinion, plus some rule-of-thumb values
            allied_repair: lambda: self._rate_opinion(charge),  # no effect, unless we have allies...
            charge: lambda: 5 + self._rate_opinion(charge),  # A small bonus in battle
            scanning: lambda: 20 + self._rate_opinion(scanning),  # May help us detect ships and planets
            simplicity: lambda: 20 + self._rate_opinion(simplicity),  # Makes simple ships cheaper
            engineering: self._rate_engineering_corps,
            exploration: lambda: 10 + self._rate_opinion(exploration),  # may give a little research, speeds up scouts
            flanking: lambda: 5 + self._rate_opinion(flanking),  # A small bonus in battle
            recruitment: lambda: 15 + self._rate_opinion(recruitment),  # cheaper troop ships
        }

    # Policies that may use IP "reserved" by wanted_ip.
    # The first two get IP specially reserved for them in the constructor.
    # If we are low on IP, adopting something that helps as gain IP should be worth it.
    # Technocracy and industrialism are important overall.
    _prioritised_policies = (bureaucracy, centralization, propaganda, artisans, technocracy, industrialism)

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
        self._process_infrastructure()
        # We need the extra slots, before adopting others economic policies.
        # TBD?: Possibly start with Centralization.
        if infra1 in self._adopted:
            self._process_bureaucracy()
            self._techno_or_industry()
        new_production = self._get_infl_prod(True)
        debug("End of turn IP: %.2f (wanted %.2f) + %.2f", self._ip, self._wanted_ip, new_production)
        self._determine_influence_priority(new_production)

    def _process_social(self) -> None:
        """
        Process social policies.
        So far we use propaganda, liberty, diversity, artisan workshops and algo. research.
        Propaganda is activated in turn 1 and usually replaced by liberty in 7.
        Since liberty affects so many other things, we check it first, then simply rate all others and
        try to adopt the best possible ones for the slots we have.
        """
        self._process_liberty()
        self._process_policy_options(social_category, (propaganda, algo_research, diversity, artisans, population))

    def _process_military(self) -> None:
        """Process military policies."""
        options = (allied_repair, charge, scanning, simplicity, engineering, exploration, flanking, recruitment)
        self._process_policy_options(military_category, options)

    def _process_policy_options(self, category: str, options: Iterable[str]) -> None:
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

    def _we_want(self, name: str, cost: float, replace: Optional[str] = None) -> None:
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

    def _unlocked_by(self, name: str) -> Union[str, Callable, None]:
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
        priority = self._aistate.get_priority(PriorityType.RESOURCE_INFLUENCE)
        # For big empires, costs of some policies go over 100 and their rating doesn't grow that much, but they may
        # also have lots of IP and the policy may still be useful. Note that costs should be less than unreserved_ip,
        # so if cost is e.g. 1/3 of unreserved_ip, this limits the rating to 0.5 * priority.
        return min(costs / 3, 1.5 * priority * costs / unreserved_ip)

    @staticmethod
    def _rate_opinion(name: str) -> float:
        """Return a rating value for the empire's planet opinion on a policy"""
        opinion = PlanetUtilsAI.get_planet_opinion(name)
        return 1.5 * (len(opinion.likes) - len(opinion.dislikes) * PlanetUtilsAI.dislike_factor())

    def _rate_algo_research(self) -> float:
        """Rate algorithmic research"""
        # First should do, but things may change. Liberty is more important!
        if fo.currentTurn() <= 10 or algo_research not in self._empire.availablePolicies:
            return 0.0
        # At 20 there are typically no planets with research focus
        # TBD: check how many planet have research focus or use meter_update?
        base_value = max(0, self._aistate.get_priority(PriorityType.RESOURCE_RESEARCH) - 20)
        rating = base_value + self._rate_opinion(algo_research)
        debug(f"_rate_algo_research: rating={rating}")
        return rating

    def _process_liberty(self) -> None:
        """
        Adopt or deadopt liberty, may replace another social policy.
        Liberty is available very early, but to help setting up basic policies we keep propaganda at least
        until we have infra1. Getting a second slot before that should be nearly impossible.
        Centralization is always kept only for one turn and if population has a strong opinion on it,
        stability calculation may be extremely different from normal turns, leading to a change that would
        possibly be reverted next turn, so we skip processing liberty while centralization is adopted.
        """
        could_replace = {propaganda, algo_research, diversity, artisans} & self._adopted
        if (
            infra1 in self._adopted
            and centralization not in self._adopted
            and (liberty in self._adopted or self._can_adopt(liberty, could_replace))
        ):
            adoption_cost = fo.getPolicy(liberty).adoptionCost()
            # liberty will generally make species less happy, but generates research
            debug("Evaluating liberty, first without change:")
            current_output = self._calculate_empire_output()
            if liberty in self._adopted:
                self._deadopt(liberty)
                without_liberty = self._calculate_empire_output()
                if current_output.is_better_than(without_liberty, 0):
                    self._universe.updateMeterEstimates(self._populated_planet_ids)
                    self._adopt(liberty)
            elif self._can_adopt(liberty):
                self._adopt(liberty)
                with_liberty = self._calculate_empire_output()
                if current_output.is_better_than(with_liberty, adoption_cost):
                    self._universe.updateMeterEstimates(self._populated_planet_ids)
                    self._deadopt(liberty)
            else:
                # Can only adopt it by replacing something. Note that we ignore the other replaced policies rating
                # here, since its effect is also evaluated by _calculate_empire_output.
                self._deadopt_one_of(could_replace)
                self._adopt(liberty)
                with_liberty = self._calculate_empire_output()
                if current_output.is_better_than(with_liberty, adoption_cost):
                    self._universe.updateMeterEstimates(self._populated_planet_ids)
                    self._deadopt(liberty)
                    self._readopt_selected_one()

    def _calculate_empire_output(self) -> _EmpireOutput:
        """Update empire meters, then calculate an estimation of the expected empire output."""
        # TBD: here we could use the stability-adapted update, that would be much better than trying
        # to rate the stability effects again production effects.
        self._universe.updateMeterEstimates(self._populated_planet_ids)
        result = _EmpireOutput()
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
        influence_priority = self._aistate.get_priority(PriorityType.RESOURCE_INFLUENCE)
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

    def _process_infrastructure(self) -> None:
        """Handle infrastructure policies."""
        if infra1 in self._adoptable:
            self._adopt(infra1)
        # TBD infra2 / 3, then use the additional slot

    def _process_bureaucracy(self) -> None:
        """Handle adoption and regular re-adoption of bureaucracy and its prerequisite centralization."""
        if bureaucracy in self._adopted:
            if fo.currentTurn() >= self._empire.turnsPoliciesAdopted[bureaucracy] + self._max_turn_bureaucracy:
                self._deadopt(bureaucracy)
                self._try_adopt_centralization()
        else:
            self._try_adopt_bureaucracy()
        # We try not to adopt it when we cannot replace it, but enemy action may
        # make the best plans fails. Keeping it will usually cost too much influence.
        self._maybe_deadopt_centralization()

    def _try_adopt_centralization(self) -> None:
        """Try to adopt centralization as a prerequisite for bureaucracy.
        So far we only want actually adopt it if we can adopt bureaucracy next turn."""
        if centralization not in self._adoptable:
            return
        if self._centralization_cost + self._bureaucracy_cost > self._ip + self._get_infl_prod():
            return
        self._adopt(centralization)
        # Adopting centralization usually lowers IP production, so use updated production to calculate
        # if it will be enough for bureaucracy next turn.
        # Add a safety margin for enemy action, loss of supply chain connection could further reduce ip production.
        safety_margin = self._num_populated / 4
        if self._bureaucracy_cost + safety_margin > self._ip + self._get_infl_prod(True):
            debug(f"{self._bureaucracy_cost} + {safety_margin} <= {self._ip} + {self._get_infl_prod(True)}")

            self._deadopt(centralization)
            self._ip += self._centralization_cost

    def _try_adopt_bureaucracy(self) -> None:
        """Try to adopt bureaucracy, starting with centralization if necessary."""
        if bureaucracy in self._adoptable:
            self._adopt(bureaucracy)
        elif centralization not in self._adopted:
            self._try_adopt_centralization()
        elif self._empire.policyPrereqsAndExclusionsOK(bureaucracy):
            if self._bureaucracy_cost <= self._ip:
                # when we are here, no free slot should be the only reason, so we
                # replace centralization by bureaucracy, using a temporary slot,
                # while the game doesn't allow it directly.
                self._deadopt(infra1)
                self._adopt(bureaucracy)
                self._deadopt(centralization)
                self._adopt(infra1)

    def _maybe_deadopt_centralization(self) -> None:
        """Deadopt centralization after one turn to get rid of the IP cost and get
        the slot for technocracy or industrialism. So far, we never keep it for more than one turn."""
        turns_adopted = self._empire.turnsPoliciesAdopted
        if centralization in self._adopted and turns_adopted[centralization] != fo.currentTurn():
            # simplified: always deadopt to free the slot for technocracy
            self._deadopt(centralization)

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

    def _determine_influence_priority(self, new_production: float) -> None:
        """Determine and set influence priority."""
        # avoid wildly varying priorities while we adopt the basics: simply a fixed value for some turns
        if fo.currentTurn() <= 15:
            if fo.currentTurn() == 1:
                # I'd prefer 20, but that makes Abadoni switch to Influence in turn 3.
                # TBD work on ResourceAI again...
                self._set_priority(18.0, True)
            return
        # How much IP would we have available if we keep the current production for 3 turns?
        forecast = self._ip - self._wanted_ip + 3 * new_production
        # adopting _centralization_cost costs a lot and also drops the incoming, so add 1 per populated planet
        repeated_expenses = self._centralization_cost + self._bureaucracy_cost + self._num_populated
        threshold = 20 + repeated_expenses
        if forecast < threshold:
            # Basic value: the lower the forecast, the more urgent we need influence
            priority = 20 + threshold - forecast
        else:
            # For higher values, slowly decrease priority
            priority = 20 * (threshold / forecast) ** 0.6
        # The more planets we have, the more influence we need in general, outpost have only a minor effect
        priority *= (3 + self._num_populated + self._num_outposts / 10) / 10
        self._set_priority(priority, False)

    def _wanted_for_bureaucracy(self) -> float:
        """Determine how many IP we want to reserve for the bureaucracy cycle."""
        repeated_expenses = self._centralization_cost + self._bureaucracy_cost + self._num_populated
        if fo.currentTurn() <= 10:
            # allow setup up basics (propaganda, infra1, liberty) first
            return 0.0
        if bureaucracy in self._adopted:
            # To avoid a saw tooth effect when adopting centralisation, account part of the future cost while
            # bureaucracy is adopted. Ideally IP will produce a saw tooth graph that way, while priority
            # remains relatively stable.
            turns_adopted = fo.currentTurn() - self._empire.turnsPoliciesAdopted[bureaucracy]
            return repeated_expenses * turns_adopted / self._max_turn_bureaucracy
        if centralization in self._adopted:
            # we need _bureaucracy_cost next turn, so if production is negative, we must reserve more
            return self._bureaucracy_cost - min(0, self._get_infl_prod())
        # Neither bureaucracy nor centralization adopted
        return repeated_expenses

    @staticmethod
    def _calculate_max_turn_bureaucracy() -> int:
        """Determine how many turns we can currently keep bureaucracy."""
        # quick fix, bureaucracy no longer needs to be readopted regularly
        return 9999

    def _set_priority(self, calculated_priority: float, ignore_old: bool) -> None:
        """Set and log influence priority."""
        if ignore_old:
            new_priority = calculated_priority
        else:
            old_priority = self._aistate.get_priority(PriorityType.RESOURCE_INFLUENCE)
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

    def _can_adopt(self, name: str, replace: Union[str, Set[str], None] = None) -> bool:
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

    def _get_adoptable(self) -> Set[str]:
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

    def _deadopt_one_of(self, names: Set[str]) -> None:
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
