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
        self.empire = fo.getEmpire()
        self.universe = fo.getUniverse()
        self.ip = self.empire.resourceAvailable(fo.resourceType.influence) - self.get_infl_prod()
        self.adopted = set(self.empire.adoptedPolicies)
        self.adoptable = self.get_adoptable()
        empire_owned_planet_ids = PlanetUtilsAI.get_owned_planets_by_empire(self.universe.planetIDs)
        self.planet_ids = PlanetUtilsAI.get_populated_planet_ids(empire_owned_planet_ids)
        self.num_populated = len(self.planet_ids)
        self.num_outposts = len(empire_owned_planet_ids) - self.num_populated
        self.aistate = get_aistate()

    def generate_orders(self):
        """generate policy orders"""
        debug(
            "Start turn %d IP: %.2f + %.2f, adoptable: %s, adopted: %s",
            fo.currentTurn(),
            self.ip,
            self.get_infl_prod(),
            str(self.adoptable),
            str(self.adopted),
        )
        for policy in basics:
            if policy in self.adoptable:
                self.adopt(policy)
        # we need the extra slots first
        if infra1 in self.adopted:
            if bureaucracy in self.adopted:
                max_turn = 19  # AI doesn't build Regional Admins yet (TBD count, palace may be gone, too)
                if fo.currentTurn() >= self.empire.turnsPoliciesAdopted[bureaucracy] + max_turn:
                    self.deadopt(bureaucracy)
                    self.try_adopt_centralization()
            else:
                self.try_adopt_bureaucracy()
            self.check_deadopt_centralization()
            self.techno_or_industry()
        new_production = self.get_infl_prod(True)
        debug("End of turn IP: %.2f + %.2f", self.ip, new_production)
        # priority shouldn't only be based on current value
        forecast = self.ip + 3 * new_production
        if infra1 in self.adopted and bureaucracy not in self.adopted and centralization not in self.adopted:
            ccost = fo.getPolicy(centralization).adoptionCost()
            bcost = fo.getPolicy(bureaucracy).adoptionCost()
            forecast -= self.num_populated + ccost + bcost
        if forecast < 40:
            prio = 60 - forecast
        else:
            prio = 800 / forecast
        prio *= (3 + self.num_populated + self.num_outposts / 10) / 10
        # if not infra3 in self.adopted: prio += 10
        debug("Setting influcence priority to %.1f", prio)
        self.aistate.set_priority(PriorityType.RESOURCE_INFLUENCE, prio)

    def try_adopt_centralization(self):
        """Try to adopt centralization as a prerequisit for bureaucracy.
        We only want actually adopt it if we can adopt bureaucracy next turn."""
        if centralization not in self.adoptable:
            return
        ccost = fo.getPolicy(centralization).adoptionCost()
        bcost = fo.getPolicy(bureaucracy).adoptionCost()
        if ccost + bcost > self.ip + self.get_infl_prod():
            return
        self.adopt(centralization)
        # adopting centralization usually lowers IP production, so use
        # updated production to calculate if it will be enough for b. next turn.
        if bcost + self.num_populated > self.ip + self.get_infl_prod(True):
            self.deadopt(centralization)
            self.ip += ccost

    def try_adopt_bureaucracy(self):
        """Try to adopt bureaucracy, starting with centralization if necessary."""
        if bureaucracy in self.adoptable:
            self.adopt(bureaucracy)
        elif centralization not in self.adopted:
            self.try_adopt_centralization()
        elif self.empire.policyPrereqsAndExclusionsOK(bureaucracy):
            bcost = fo.getPolicy(bureaucracy).adoptionCost()
            if bcost <= self.ip:
                # when we are here, no free slot should be the only reason, so we
                # replace centralization by bureaucracy, using a temporary slot,
                # while the game doesn't allow it directly.
                self.deadopt(infra1)
                self.adopt(bureaucracy)
                self.deadopt(centralization)
                self.adopt(infra1, True)

    def check_deadopt_centralization(self):
        """deadopt centralization after one turn to get rid of the IP cost and get
        the slot for technocracy or industrialism"""
        turns_adopted = self.empire.turnsPoliciesAdopted
        if centralization in self.adopted and turns_adopted[centralization] != fo.currentTurn():
            # simplified: always deadopt to free the slot for technocracy
            self.deadopt(centralization)

    def techno_or_industry(self):
        research_prio = self.aistate.get_priority(PriorityType.RESOURCE_RESEARCH)
        production_prio = self.aistate.get_priority(PriorityType.RESOURCE_PRODUCTION)
        may_switch_to_techno = self.can_adopt(technocracy, industrialism)
        may_switch_to_industry = self.can_adopt(industrialism, technocracy)
        # do not switch too often
        if technocracy in self.adopted:
            research_prio *= 1.25
            may_switch_to_techno = False
        elif industrialism in self.adopted:
            production_prio *= 1.25
            may_switch_to_industry = False
        if may_switch_to_industry and research_prio < production_prio:
            self.deadopt(technocracy)
            self.adopt(industrialism)
        elif may_switch_to_techno and research_prio > production_prio:
            self.deadopt(industrialism)
            self.adopt(technocracy)
        debug(
            "Prio res./ind. = %.1f/%.1f, maySwitch = %s/%s",
            research_prio,
            production_prio,
            str(may_switch_to_techno),
            str(may_switch_to_industry),
        )

    def get_infl_prod(self, update=False):
        """get IP production"""
        if update:
            self.universe.updateMeterEstimates(self.planet_ids)
            fo.updateResourcePools()
        return self.empire.resourceProduction(fo.resourceType.influence)

    def can_adopt(self, name, replace_other=None):
        policy = fo.getPolicy(name)
        ret = name in self.empire.availablePolicies and policy.adoptionCost() <= self.ip
        if replace_other and replace_other in self.adopted:
            return ret
        return ret and self.empire.emptyPolicySlots[policy.category] and self.empire.policyPrereqsAndExclusionsOK(name)

    def get_adoptable(self):
        """List of adoptable policies with the still available IP"""
        ret = set()
        for p in self.empire.availablePolicies:
            if p in self.adopted:
                continue
            if self.can_adopt(p):
                ret.add(p)
        return ret

    def adopt(self, name, readopt=False):
        """Find an emtpy slot and adopt named policy, if possible"""
        policy = fo.getPolicy(name)
        cat = policy.category
        slot = self.find_empty_slot(cat)
        chk = fo.issueAdoptPolicyOrder(name, cat, slot)
        if chk:
            debug("Issued adoption order for " + name + " in slot " + str(slot) + " turn " + str(fo.currentTurn()))
            self.adopted.add(name)
            if not readopt:
                self.ip -= policy.adoptionCost()
            self.adoptable = self.get_adoptable()
        else:
            error("Failed to adopt " + name + " in slot " + str(slot))

    def find_empty_slot(self, cat):
        """This assumes there is an empty slot, otherwise adopt will fail"""
        slot = 0
        # there should be a better way...
        cat_slots = self.empire.categoriesSlotPolicies
        if cat in cat_slots:
            slots = cat_slots[cat]
            for slot in range(0, 99):
                if slot not in slots:
                    break
        return slot

    def deadopt(self, name):
        """deadopt name, if it is adopted"""
        if name in self.adopted:
            debug("Issued deadoption order for " + name + " turn " + str(fo.currentTurn()))
            fo.issueDeadoptPolicyOrder(name)
            self.adopted.remove(name)
            self.adoptable = self.get_adoptable()

    def status(self):
        # only for interactive debugging
        print("\nAdoptable Policies:")
        for p in self.get_adoptable():
            print("  ", p)
        print("Empty Slots:")
        for s in self.empire.emptyPolicySlots:
            print("  ", s)
        print("Adopted:")
        for cat in self.empire.categoriesSlotPolicies:
            if cat:
                print(" ", cat.key() + ":")
                for slot in cat.data():
                    print("   ", slot.key(), "->", slot.data(), "turn", self.empire.turnPolicyAdopted(slot.data()))
        print("Influence:", self.ip)
        print("Infl. Prod.:", self.get_infl_prod())
        print("Num Planet (pop./outp.): %d/%d" % (self.num_populated, self.num_outposts))


def generate_policy_orders():
    PolicyManager().generate_orders()


def status():
    PolicyManager().status()
