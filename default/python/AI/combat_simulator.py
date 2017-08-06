"""

This module

"""

import random
import copy
import os

import freeOrionAIInterface as fo
import PlanetUtilsAI
import ShipDesignAI
from freeorion_tools import chat_human

WIN_A = -1
WIN_B = 1
DRAW = 0


ships_A = []
ships_B = []


# TODO: Tech boni etc for parts
boni_A = {}
boni_B = {}


class CombatManager(object):

    def __init__(self):
        self.faction_A = Faction(boni_A)
        self.faction_B = Faction(boni_B)

    def parse_shiplist(self):
        del ships_A[:]
        del ships_B[:]
        active_faction = 0
        print "parsing Textfile..."
        with open(os.path.dirname(os.path.abspath(__file__)) + "\\SimulationShiplist.txt", "r") as infile:
            while True:
                this_line = infile.readline()
                if not this_line:
                    print "End of file reached."
                    break  # end of file
                this_line = this_line.replace('\n', '')
                if "FACTION_A" == this_line:
                    chat_human("Adding Ships to Faction A now.")
                    active_faction = 1
                elif "FACTION_B" == this_line:
                    chat_human("Adding Ships to Faction B now.")
                    active_faction = 2
                elif this_line:
                    # may be empty now that \n is cleared
                    if active_faction == 1:
                        self.faction_A.add_ship(design_name=this_line)
                    elif active_faction == 2:
                        self.faction_B.add_ship(design_name=this_line)

    def run(self, iterations):
        import cProfile, pstats, StringIO
        pr = cProfile.Profile()
        pr.enable()
        self.__init__()
        self.parse_shiplist()

        results = []
        lost_pp_a = []
        lost_pp_b = []

        for i in xrange(iterations):
            self.faction_A.reset()
            self.faction_B.reset()

            original_number_of_ships_a = self.faction_A.remaining_ships()
            original_pp_a = self.faction_A.remaining_pp()

            original_number_of_ships_b = self.faction_B.remaining_ships()
            original_pp_b = self.faction_B.remaining_pp()
            if i == 0:
                chat_human("Number of ships A: %d (cost: %.1f PP)" % (original_number_of_ships_a, original_pp_a))
                chat_human("Number of ships B: %d (cost: %.1f PP)" % (original_number_of_ships_b, original_pp_b))

            for combat_round in xrange(3):
                self.faction_A.shoot(self.faction_B.current_objects)
                self.faction_B.shoot(self.faction_A.current_objects)
                self.faction_A.launch_fighters()
                self.faction_B.launch_fighters()
                self.faction_A.update()
                self.faction_B.update()
                if not self.faction_A.is_alive() or not self.faction_B.is_alive():
                    break

            remaining_pp_a = self.faction_A.remaining_pp()
            lost_pp_a.append(original_pp_a - remaining_pp_a)

            remaining_pp_b = self.faction_B.remaining_pp()
            lost_pp_b.append(original_pp_b - remaining_pp_b)

            if original_number_of_ships_a:
                win_a = self.faction_A.remaining_ships() > 0
            else:
                win_a = self.faction_A.remaining_planets() > 0
            if original_number_of_ships_b:
                win_b = self.faction_B.remaining_ships() > 0
            else:
                win_b = self.faction_B.remaining_planets() > 0

            if win_a and win_b:
                results.append(DRAW)
            elif win_a:
                results.append(WIN_A)
            elif win_b:
                results.append(WIN_B)
            else:
                results.append(DRAW)

        mean_losses_a = mean(lost_pp_a)
        std_losses_a = std(lost_pp_a)
        mean_losses_b = mean(lost_pp_b)
        std_losses_b = std(lost_pp_b)

        draws = len([r for r in results if r == DRAW])
        wins_a = len([r for r in results if r == WIN_A])
        wins_b = len([r for r in results if r == WIN_B])

        chat_human("+++ SIMULATION RESULTS+++")
        chat_human("Simulated a total number of %d iterations." % iterations)
        chat_human("Wins A: %d (%.1f%%)" % (wins_a, 100*float(wins_a)/iterations))
        chat_human("Wins B: %d (%.1f%%)" % (wins_b, 100*float(wins_b)/iterations))
        chat_human("Draws: %d (%.1f%%)" % (draws, 100*float(draws)/iterations))
        chat_human("Losses Player A: (%.1f +- %.1f) PP" % (mean_losses_a or 0, std_losses_a or 0))
        chat_human("Losses Player B: (%.1f +- %.1f) PP" % (mean_losses_b or 0, std_losses_b or 0))
        pr.disable()
        s = StringIO.StringIO()
        sortby = 'cumulative'
        ps = pstats.Stats(pr, stream=s).sort_stats(sortby)
        ps.print_stats()
        print s.getvalue()

def mean(values):
    if not values:
        return None
    return float(sum(values))/len(values)


def variance(values):
    if not values or len(values) == 1:
        # we calculate sample variance, so need sample size > 1 to get any meaningful result
        return None
    m = mean(values)
    diff_squared = [(v - m)**2 for v in values]
    return float(sum(diff_squared)) / (len(values)-1)


def std(values):
    v = variance(values)
    return v and v**.5 or None


class Faction(object):

    def __init__(self, bonuses=None):
        self.current_objects = []
        self.destroyed_objects = []
        self.bonuses = bonuses or {}

    def shoot(self, enemy_objects):

        for cls in ALL_OBJECTS:
            cls.current_possible_targets[:] = [obj for obj in enemy_objects if cls.is_possible_target(obj)]

        for o in self.current_objects:
            o.shoot()

    def launch_fighters(self):
        for o in self.current_objects:
            launched_fighters = o.launch_fighters()
            if launched_fighters:
                for i in xrange(int(launched_fighters[0])):
                    self.current_objects.append(Fighter(launched_fighters[1]))

    def update(self):
        for o in self.current_objects:
            o.update()
            if not o.is_alive():
                self.destroyed_objects.append(o)
        self.current_objects = [o for o in self.current_objects if o not in self.destroyed_objects]

    def add_ship(self, design_id=None, design_name=None):
        design = None
        if design_id is not None:
            chat_human('Adding ship with id %d' % design_id)
            design = fo.getShipDesign(design_id)
        elif design_name is not None:
            chat_human('Adding ship with name %s' % design_name)
            design = ShipDesignAI._get_design_by_name(design_name)
        if not design:
            chat_human('ERROR: Could not find design.')
            return
        design_stats = get_stats_from_design(design, self.bonuses)
        print design_stats
        if design_stats:
            self.current_objects.append(Ship(*design_stats))
        else:
            chat_human('ERROR: Could not get design stats...')

    def add_planet(self, defense, shields):
        self.current_objects.append(Planet(defense, shields))

    def reset(self):
        self.current_objects = self.current_objects + self.destroyed_objects
        self.current_objects = [obj for obj in self.current_objects if not isinstance(obj, Fighter)]
        self.destroyed_objects = []
        for obj in self.current_objects:
            obj.reset()

    def is_alive(self):
        return bool(self.current_objects)

    def remaining_ships(self):
        return len([o for o in self.current_objects if isinstance(o, Ship)])

    def remaining_planets(self):
        return len([o for o in self.current_objects if isinstance(o, Planet)])

    def remaining_pp(self):
        return sum(o.cost for o in self.current_objects)


class CombatObject(object):

    ignore_shields = False
    current_possible_targets = []

    def __init__(self):
        self.cost = 0

    def reset(self):
        pass

    def _shots(self):
        """Yield all shots of the object in the current combat round.

        :return: shot damage
        :rtype: __generator[float]
        """
        raise NotImplementedError

    @staticmethod
    def is_possible_target(obj):
        """Return true if passed object is a possible target.

        :param obj: object to be checked
        :type obj: CombatObject
        :return: True if possible target
        :rtype: bool
        """
        raise NotImplementedError

    def _health(self):
        """Return the remaining "health", e.g. structure for a ship.

        :return: health of the object
        :rtype: float
        """
        raise NotImplementedError

    def shoot(self):
        """Fire all available shots at random but possible enemy targets, damaging them.

        :param enemy_objects: all enemy objects alive at the beginning of the turn
        :type enemy_objects: list[CombatObject]
        """
        if not self.current_possible_targets:
            return
        
        for dmg in self._shots():
            target = random.choice(self.current_possible_targets)
            target.do_damage(dmg, self.ignore_shields)

    def do_damage(self, dmg, ignore_shields=False):
        """Apply the damage dealt to this object.

        :param dmg: Raw damage of the attack
        :type dmg: float
        :param ignore_shields: If true, the attack ignores the shields of this object.
        :type ignore_shields: bool
        """
        raise NotImplementedError

    def launch_fighters(self):
        """Launch fighters in this turn.

        :return: (number_of_launched_fighters, fighter_damage)
        :rtype: tuple[int, float]
        """
        raise NotImplementedError

    def update(self):
        """Post-combat turn update this object."""
        raise NotImplementedError

    def is_alive(self):
        """Check if this object is still alive (i.e. it was not destroyed yet)

        :return: True if object is still alive
        :rtype: bool
        """
        return self._health() > 0


class Ship(CombatObject):

    ignore_shields = False

    def __init__(self, attacks, structure, shields, fighter_capacity, fighter_rate, fighter_damage, cost):
        super(Ship, self).__init__()
        self._orig_structure = structure
        self._orig_fighter_capacity = fighter_capacity
        self.attacks = attacks
        self.shots = []
        for dmg, n in self.attacks.iteritems():
            self.shots.extend(int(n) * [dmg])
        self.structure = structure
        self.shields = shields
        self.fighter_capacity = fighter_capacity
        self.fighter_rate = fighter_rate
        self.fighter_damage = fighter_damage
        self.cost = cost

    def reset(self):
        self.structure = self._orig_structure
        self.fighter_capacity = self._orig_fighter_capacity

    def _shots(self):
        return self.shots

    @staticmethod
    def is_possible_target(obj):
        return True

    def _health(self):
        return self.structure

    def do_damage(self, dmg, ignore_shields=False):
        if not ignore_shields:
            dmg -= self.shields
        dmg = max(dmg, 0)
        self.structure -= dmg
        # print "Remaining structure:", self.structure

    def launch_fighters(self):
        if not self.fighter_capacity or self.fighter_rate:
            return tuple()
        num_launched = min(self.fighter_capacity, self.fighter_rate)
        damage = self.fighter_damage
        self.fighter_capacity -= num_launched
        return num_launched, damage

    def update(self):
        pass


class Fighter(CombatObject):

    ignore_shields = True

    def __init__(self, damage):
        super(Fighter, self).__init__()
        self.structure = 1
        self.damage = damage

    def reset(self):
        self.structure = 1

    def _shots(self):
        yield self.damage

    @staticmethod
    def is_possible_target(obj):
        if isinstance(obj, Planet):
            return False
        return True

    def launch_fighters(self):
        return tuple()

    def do_damage(self, dmg, ignore_shields=False):
        if dmg > 0:
            self.structure = 0

    def _health(self):
        return self.structure

    def update(self):
        pass


class Planet(CombatObject):

    ignore_shields = False

    def __init__(self, defense, shields):
        super(Planet, self).__init__()
        self._orig_defense = defense
        self._orig_shields = shields
        self.defense = defense
        self.shields = shields
        self.defense_after_turn = defense  # only update defense after turn

    def reset(self):
        self.defense = self._orig_defense
        self.shields = self._orig_shields

    def _shots(self):
        yield self.defense

    @staticmethod
    def is_possible_target(obj):
        if isinstance(obj, Ship):
            return True
        return False

    def launch_fighters(self):
        return tuple()

    def do_damage(self, dmg, ignore_shields=False):
        # first, subtract damage from shields
        dmg = max(dmg, 0)
        shield_dmg = min(dmg, self.shields)
        self.shields -= shield_dmg
        defense_dmg = min(dmg - shield_dmg, self.defense_after_turn)
        self.defense_after_turn -= defense_dmg

    def update(self):
        self.defense = max(self.defense_after_turn, 0)

    def _health(self):
        return self.defense + self.shields


def get_stats_from_design(design, bonuses):
    """

    :param design:
    :param bonuses:
    :rtype: tuple([dict, float, float, float, float, float, float])
    :return:
    """
    if not design:
        return

    attacks = {}
    fighter_launchrate = 0
    fighter_damage = 0
    fighter_capacity = 0
    structure = design.structure
    shields = design.shields + bonuses.get('shield', 0)

    loc = PlanetUtilsAI.get_capital()
    cost = design.productionCost(fo.empireID(), loc)

    for partname in design.parts:
        part = ShipDesignAI.get_part_type(partname)
        if not part:
            continue
        pc = part.partClass
        if pc == fo.shipPartClass.shortRange:
            modifier_dmg, modifier_shots = bonuses.get(partname, (0, 0))
            damage = part.capacity + modifier_dmg
            shots = part.secondaryStat + modifier_shots
            attacks[damage] = attacks.get(damage, 0) + shots
        elif pc == fo.shipPartClass.fighterBay:
            modifier = bonuses.get(partname, 0)
            fighter_launchrate += (part.capacity + modifier)
        elif pc == fo.shipPartClass.fighterHangar:
            modifier_dmg, modifier_capacity = bonuses.get(partname, (0, 0))
            fighter_capacity += (part.capacity + modifier_capacity)
            part_damage = (part.secondaryStat + modifier_dmg)
            if part_damage != fighter_damage and fighter_damage > 0:
                # the C++ code fails also in this regard, so FOCS content *should* not allow this.
                # TODO: Depending on future implementation, might actually need to handle this case.
                print "WARNING: Multiple hangar types present on one ship, estimates expected to be wrong."
            fighter_damage = max(fighter_damage, part_damage)

    print attacks

    return attacks, structure, shields, fighter_capacity, fighter_launchrate, fighter_damage, cost


ALL_OBJECTS = (Planet, Ship, Fighter)
