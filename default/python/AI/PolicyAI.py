import freeOrionAIInterface as fo
from logging import debug, error

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
    # no use for the extra slot yet and they are expensive
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
        self._adoptable = self._get_adoptable()
        empire_owned_planet_ids = PlanetUtilsAI.get_owned_planets_by_empire(self._universe.planetIDs)
        self._populated_planet_ids = PlanetUtilsAI.get_populated_planet_ids(empire_owned_planet_ids)
        self._num_populated = len(self._populated_planet_ids)
        self._num_outposts = len(empire_owned_planet_ids) - self._num_populated
        self._centralization_cost = fo.getPolicy(centralization).adoptionCost()
        self._bureaucracy_cost = fo.getPolicy(bureaucracy).adoptionCost()

    def generate_orders(self):
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

    def _process_basics(self):
        for policy in basics:
            if policy in self._adoptable:
                self._adopt(policy)

    def _process_bureaucracy(self):
        if bureaucracy in self._adopted:
            max_turn = 19  # AI doesn't build Regional Admins yet (TBD count, palace may be gone, too)
            if fo.currentTurn() >= self._empire.turnsPoliciesAdopted[bureaucracy] + max_turn:
                self._deadopt(bureaucracy)
                self._try_adopt_centralization()
        else:
            self._try_adopt_bureaucracy()
        self._check_deadopt_centralization()

    def _try_adopt_centralization(self):
        """Try to adopt centralization as a prerequisit for bureaucracy.
        We only want actually adopt it if we can adopt bureaucracy next turn."""
        if centralization not in self._adoptable:
            return
        if self._centralization_cost + self._bureaucracy_cost > self._ip + self._get_infl_prod():
            return
        self._adopt(centralization)
        # adopting centralization usually lowers IP production, so use
        # updated production to calculate if it will be enough for b. next turn.
        if self._bureaucracy_cost + self._num_populated > self._ip + self._get_infl_prod(True):
            self._deadopt(centralization)
            self._ip += self._centralization_cost

    def _try_adopt_bureaucracy(self):
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
                self._adopt(infra1, True)

    def _check_deadopt_centralization(self):
        """Deadopt centralization after one turn to get rid of the IP cost and get
        the slot for technocracy or industrialism."""
        turns_adopted = self._empire.turnsPoliciesAdopted
        if centralization in self._adopted and turns_adopted[centralization] != fo.currentTurn():
            # simplified: always deadopt to free the slot for technocracy
            self._deadopt(centralization)

    def _techno_or_industry(self):
        research_prio = self._aistate.get_priority(PriorityType.RESOURCE_RESEARCH)
        production_prio = self._aistate.get_priority(PriorityType.RESOURCE_PRODUCTION)
        may_switch_to_techno = self._can_adopt(technocracy, industrialism)
        may_switch_to_industry = self._can_adopt(industrialism, technocracy)
        # do not switch too often
        if technocracy in self._adopted:
            research_prio *= 1.25
            may_switch_to_techno = False
        elif industrialism in self._adopted:
            production_prio *= 1.25
            may_switch_to_industry = False
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

    def _determine_influence_priority(self, new_production):
        # priority shouldn't only be based on current value
        forecast = self._ip + 3 * new_production
        if infra1 in self._adopted and bureaucracy not in self._adopted and centralization not in self._adopted:
            forecast -= self._num_populated + self._centralization_cost + self._bureaucracy_cost
        if forecast < 40:
            prio = 60 - forecast
        else:
            prio = 800 / forecast
        prio *= (3 + self._num_populated + self._num_outposts / 10) / 10
        # if not infra3 in self._adopted: prio += 10
        debug("Setting influcence priority to %.1f", prio)
        self._aistate.set_priority(PriorityType.RESOURCE_INFLUENCE, prio)

    def _get_infl_prod(self, update=False):
        """Get / Update IP production."""
        if update:
            self._universe.updateMeterEstimates(self._populated_planet_ids)
            fo.updateResourcePools()
        return self._empire.resourceProduction(fo.resourceType.influence)

    def _can_adopt(self, name, replace_other=None):
        policy = fo.getPolicy(name)
        ret = name in self._empire.availablePolicies and policy.adoptionCost() <= self._ip
        if replace_other and replace_other in self._adopted:
            return ret
        return (
            ret and self._empire.emptyPolicySlots[policy.category] and self._empire.policyPrereqsAndExclusionsOK(name)
        )

    def _get_adoptable(self):
        """List of adoptable policies with the still available IP."""
        ret = set()
        for p in self._empire.availablePolicies:
            if p in self._adopted:
                continue
            if self._can_adopt(p):
                ret.add(p)
        return ret

    def _adopt(self, name, readopt=False):
        """Find an emtpy slot and adopt named policy, if possible."""
        policy = fo.getPolicy(name)
        cat = policy.category
        slot = self._find_empty_slot(cat)
        chk = fo.issueAdoptPolicyOrder(name, cat, slot)
        if chk:
            debug("Issued adoption order for %s in slot %d turn %d", name, slot, fo.currentTurn())
            if not readopt:
                self._ip -= policy.adoptionCost()
            self._adoptable = self._get_adoptable()
        else:
            error("Failed to adopt %s in slot %d", name, slot)

    def _find_empty_slot(self, cat):
        """This assumes there is an empty slot, otherwise adopt will fail."""
        slot = 0
        # there should be a better way...
        cat_slots = self._empire.categoriesSlotPolicies
        if cat in cat_slots:
            slots = cat_slots[cat]
            for slot in range(0, 99):
                if slot not in slots:
                    break
        return slot

    def _deadopt(self, name):
        """Deadopt name, if it is adopted."""
        if name in self._adopted:
            debug("Issued deadoption order for %s turn %d", name, fo.currentTurn())
            fo.issueDeadoptPolicyOrder(name)
            self._adopted.remove(name)
            self._adoptable = self._get_adoptable()

    def status(self):
        # only for interactive debugging
        print("\nAdoptable Policies:")
        for p in self._get_adoptable():
            print("  ", p)
        print("Empty Slots:")
        for s in self._empire.emptyPolicySlots:
            print("  ", s)
        print("Adopted:")
        for cat in self._empire.categoriesSlotPolicies:
            if cat:
                print(" ", cat.key() + ":")
                for slot in cat.data():
                    print("   ", slot.key(), "->", slot.data(), "turn", self._empire.turnPolicyAdopted(slot.data()))
        print("Influence:", self._ip)
        print("Infl. Prod.:", self._get_infl_prod())
        print("Num Planet (pop./outp.): %d/%d" % (self._num_populated, self._num_outposts))


def generate_policy_orders():
    PolicyManager().generate_orders()


def status():
    PolicyManager().status()
