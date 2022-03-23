import freeOrionAIInterface as fo
from logging import debug, error
from typing import Optional

import PlanetUtilsAI
from aistate_interface import get_aistate
from EnumsAI import PriorityType

propaganda = "PLC_PROPAGANDA"
algo_research = "PLC_ALGORITHMIC_RESEARCH"
moderation = "PLC_MODERATION"
industrialism = "PLC_INDUSTRIALISM"
technocracy = "PLC_TECHNOCRACY"
centralization = "PLC_CENTRALIZATION"
bureaucracy = "PLC_BUREAUCRACY"
infra1 = "PLC_PLANETARY_INFRA"
infra2 = "PLC_SYSTEM_INFRA"
infra3 = "PLC_INTERSTELLAR_INFRA"
basics = [
    propaganda,
    algo_research,
    infra1,
    # no use for the extra slot yet, and they are expensive
    # infra2,
    # infra3,
]


class PolicyManager:
    """Policy Manager for one round"""

    def __init__(self):
        # resourceAvailable includes this turns production, but that is wrong for influence
        self._empire = fo.getEmpire()
        self._universe = fo.getUniverse()
        self._aistate = get_aistate()
        self._ip = self._empire.resourceAvailable(fo.resourceType.influence) - self._get_infl_prod()
        self._adopted = set(self._empire.adoptedPolicies)
        self._originally_adopted = self._adopted
        self._adoptable = self._get_adoptable()
        empire_owned_planet_ids = PlanetUtilsAI.get_owned_planets_by_empire(self._universe.planetIDs)
        self._populated_planet_ids = PlanetUtilsAI.get_populated_planet_ids(empire_owned_planet_ids)
        self._num_populated = len(self._populated_planet_ids)
        self._num_outposts = len(empire_owned_planet_ids) - self._num_populated
        self._centralization_cost = fo.getPolicy(centralization).adoptionCost()
        self._bureaucracy_cost = fo.getPolicy(bureaucracy).adoptionCost()

    def generate_orders(self) -> None:
        """The main task of the class, called once every turn by FreeOrionAI."""
        debug(
            "Start turn %d IP: %.2f + %.2f, adoptable: %s, adopted: %s",
            fo.currentTurn(),
            self._ip,
            self._get_infl_prod(),
            self._adoptable,
            self._adopted,
        )
        self._process_basics()
        # we need the extra slots first
        if infra1 in self._adopted:
            self._process_bureaucracy()
            self._techno_or_industry()
        new_production = self._get_infl_prod(True)
        debug("End of turn IP: %.2f + %.2f", self._ip, new_production)
        self._determine_influence_priority(new_production)

    def _process_basics(self) -> None:
        for policy in basics:
            if policy in self._adoptable:
                self._adopt(policy)

    def _process_bureaucracy(self) -> None:
        if bureaucracy in self._adopted:
            max_turn = 19  # AI doesn't build Regional Admins yet (TBD count, palace may be gone, too)
            if fo.currentTurn() >= self._empire.turnsPoliciesAdopted[bureaucracy] + max_turn:
                self._deadopt(bureaucracy)
                self._try_adopt_centralization()
        else:
            self._try_adopt_bureaucracy()
        # We try not to adopt it when we cannot replace it, but enemy action may
        # make the best plans fails. Keeping it will usually cost too much influence.
        self._check_deadopt_centralization()

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

    def _check_deadopt_centralization(self) -> None:
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
        # How much IP would we have if we keep the current production for 3 turns?
        forecast = self._ip + 3 * new_production
        # If we are unable to keep bureaucracy, lower the forecast by roughly the amount
        # we will have to spend to activate it again.
        if infra1 in self._adopted and bureaucracy not in self._adopted and centralization not in self._adopted:
            forecast -= self._num_populated + self._centralization_cost + self._bureaucracy_cost
        if forecast < 40:
            # Basic value: the lower the forecast, the more urgent we need influence
            priority = 60 - forecast
        else:
            # For higher values, slowly decrease priority
            priority = 800 / forecast
        # The more planets we have, the more influence we need in general.
        # This formula likely will need more fine tuning. Note that we should not start too low, since
        # small planets can often be set to influence focus early on without losing a lot of production.
        # If we start too high, Abadoni will switch their capital to influence production, though.
        priority *= (3 + self._num_populated + self._num_outposts / 10) / 10
        debug("Setting influence priority to %.1f", priority)
        self._aistate.set_priority(PriorityType.RESOURCE_INFLUENCE, priority)

    def _get_infl_prod(self, update=False) -> float:
        """Get / Update IP production."""
        if update:
            self._universe.updateMeterEstimates(self._populated_planet_ids)
            fo.updateResourcePools()
        return self._empire.resourceProduction(fo.resourceType.influence)

    def _can_adopt(self, name: str, replace_other: Optional[str] = None) -> bool:
        """Note that when replace_other is set, this function currently assumes that with
        replace_other removed, there are no other exclusions for policy(name).
        If this won't do, the function needs to temporarily deadopt replace_other or
        policyPrereqsAndExclusionsOK must be extended so that replace_other can be passed to it.
        """
        if name in self._adopted:
            return False
        policy = fo.getPolicy(name)
        ret = name in self._empire.availablePolicies and (
            policy.adoptionCost() <= self._ip or name in self._originally_adopted
        )
        if replace_other and replace_other in self._adopted:
            return ret
        return (
            ret and self._empire.emptyPolicySlots[policy.category] and self._empire.policyPrereqsAndExclusionsOK(name)
        )

    def _get_adoptable(self) -> set[str]:
        """List of adoptable policies with the still available IP."""
        return {p for p in self._empire.availablePolicies if self._can_adopt(p)}

    def _adopt(self, name: str) -> None:
        """Find an emtpy slot and adopt named policy, if possible."""
        policy = fo.getPolicy(name)
        category = policy.category
        slot = self._find_empty_slot(category)
        if fo.issueAdoptPolicyOrder(name, category, slot):
            if name not in self._originally_adopted:
                self._ip -= policy.adoptionCost()
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
            self._adopted.remove(name)
            self._adoptable = self._get_adoptable()
            if name not in self._originally_adopted:
                # this is canceling an adoption order of the current turn
                self._ip += fo.getPolicy(name).adoptionCost()
            debug(f"Issued deadoption order for {name} turn {fo.currentTurn()}, remaining IP: {self._ip}")

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
        print("Influence:", self._ip)
        print("Infl. Prod.:", self._get_infl_prod())
        print("Num Planet (pop./outp.): %d/%d" % (self._num_populated, self._num_outposts))


def generate_policy_orders() -> None:
    PolicyManager().generate_orders()


def print_status() -> None:
    PolicyManager().print_status()
