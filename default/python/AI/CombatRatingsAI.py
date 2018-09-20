from collections import Counter
from logging import warn

import freeOrionAIInterface as fo
import FleetUtilsAI
from EnumsAI import MissionType
from freeorion_tools import get_ai_tag_grade, dict_to_tuple, tuple_to_dict, cache_by_session
from ShipDesignAI import get_part_type
from AIDependencies import INVALID_ID


def get_empire_standard_fighter():
    """Get the current empire standard fighter stats, i.e. the most common shiptype within the empire.

    :return: Stats of most common fighter in the empire
    :rtype: ShipCombatStats
    """
    stats_dict = Counter()
    for fleet_id in FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.MILITARY):
        ship_stats = FleetCombatStats(fleet_id, consider_refuel=True).get_ship_combat_stats()
        stats_dict.update(ship_stats)

    most_commons = stats_dict.most_common(1)
    if most_commons:
        return most_commons[0][0]
    else:
        return default_ship_stats()


def default_ship_stats():
    """ Return some ship stats to assume if no other intel is available.

    :return: Some weak standard ship
    :rtype: ShipCombatStats
    """
    attacks = (4.0, 1)
    structure = 15
    shields = 0
    fighters = 0
    launch_rate = 0
    fighter_damage = 0
    return ShipCombatStats(stats=(attacks, structure, shields, fighters, launch_rate, fighter_damage))


class ShipCombatStats(object):
    """Stores all relevant stats of a ship for combat strength evaluation."""
    class BasicStats(object):
        """Stores non-fighter-related stats."""
        def __init__(self, attacks, structure, shields):
            """

            :param attacks:
            :type attacks: dict|None
            :param structure:
            :type structure: int|None
            :param shields:
            :type shields: int|None
            :return:
            """
            self.structure = 1 if structure is None else structure
            self.shields = 0 if shields is None else shields
            self.attacks = {} if attacks is None else tuple_to_dict(attacks)

        def get_stats(self, hashable=False):
            """

            :param hashable: if hashable, return tuple instead of attacks-dict
            :return: attacks, structure, shields
            """
            if not hashable:
                return self.attacks, self.structure, self.shields
            else:
                return dict_to_tuple(self.attacks), self.structure, self.shields

        def __str__(self):
            return str(self.get_stats())

    class FighterStats(object):
        """ Stores fighter-related stats """
        def __init__(self, stat_tuple=None, capacity=0, launch_rate=0, damage=0):
            if stat_tuple and isinstance(stat_tuple, tuple):
                self.capacity, self.launch_rate, self.damage = stat_tuple
            else:
                self.capacity = capacity
                self.launch_rate = launch_rate
                self.damage = damage

        def __str__(self):
            return str(self.get_stats())

        def get_stats(self):
            """
            :return: capacity, launch_rate, damage
            """
            return self.capacity, self.launch_rate, self.damage

    def __init__(self, ship_id=INVALID_ID, consider_refuel=False, stats=None):
        self.__ship_id = ship_id
        self._consider_refuel = consider_refuel
        if stats:
            self._basic_stats = self.BasicStats(*stats[0:3])  # TODO: Should probably determine size dynamically
            self._fighter_stats = self.FighterStats(*stats[3:])
        else:
            self._basic_stats = self.BasicStats(None, None, None)
            self._fighter_stats = self.FighterStats(None, None, None)
            self.__get_stats_from_ship()

    def __hash__(self):
        return hash(self.get_basic_stats(hashable=True))

    def __str__(self):
        return str(self.get_stats())

    def __get_stats_from_ship(self):
        """Read and store combat related stats from ship"""
        universe = fo.getUniverse()
        ship = universe.getShip(self.__ship_id)
        if not ship:
            return  # TODO: Add some estimate for stealthed ships

        if self._consider_refuel:
            structure = ship.initialMeterValue(fo.meterType.maxStructure)
            shields = ship.initialMeterValue(fo.meterType.maxShield)
        else:
            structure = ship.initialMeterValue(fo.meterType.structure)
            shields = ship.initialMeterValue(fo.meterType.shield)
        attacks = {}
        fighter_launch_rate = 0
        fighter_capacity = 0
        fighter_damage = 0
        design = ship.design
        if design and (ship.isArmed or ship.hasFighters):
            meter_choice = fo.meterType.maxCapacity if self._consider_refuel else fo.meterType.capacity
            for partname in design.parts:
                if not partname:
                    continue
                pc = get_part_type(partname).partClass
                if pc == fo.shipPartClass.shortRange:
                    damage = ship.currentPartMeterValue(meter_choice, partname)
                    attacks[damage] = attacks.get(damage, 0) + 1
                elif pc == fo.shipPartClass.fighterBay:
                    fighter_launch_rate += ship.currentPartMeterValue(fo.meterType.capacity, partname)
                elif pc == fo.shipPartClass.fighterHangar:
                    fighter_capacity += ship.currentPartMeterValue(meter_choice, partname)
                    part_damage = ship.currentPartMeterValue(fo.meterType.secondaryStat, partname)
                    if part_damage != fighter_damage and fighter_damage > 0:
                        # the C++ code fails also in this regard, so FOCS content *should* not allow this.
                        # TODO: Depending on future implementation, might actually need to handle this case.
                        warn("Multiple hangar types present on one ship, estimates expected to be wrong.")
                    fighter_damage = max(fighter_damage, part_damage)
        self._basic_stats = self.BasicStats(attacks, structure, shields)
        self._fighter_stats = self.FighterStats(fighter_capacity, fighter_launch_rate, fighter_damage)

    def get_basic_stats(self, hashable=False):
        """Get non-fighter-related combat stats of the ship.

        :param hashable: if true, returns tuple instead of attacks-dict
        :return: attacks, structure, shields
        :rtype: (dict|tuple, int, int)
        """
        return self._basic_stats.get_stats(hashable=hashable)

    def get_fighter_stats(self):
        """ Get fighter related combat stats
        :return: capacity, launch_rate, damage
        """
        return self._fighter_stats.get_stats()

    def get_rating(self, enemy_stats=None, ignore_fighters=False):
        """Calculate a rating against specified enemy.

        :param enemy_stats: Enemy stats to be rated against
        :type enemy_stats: ShipCombatStats
        :param ignore_fighters: If True, acts as if fighters are not launched
        :type ignore_fighters: bool
        :return: rating against specified enemy
        :rtype: float
        """
        # adjust base stats according to enemy stats
        def _rating():
            return my_total_attack * my_structure

        my_attacks, my_structure, my_shields = self.get_basic_stats()
        e_avg_attack = 1
        if enemy_stats:
            e_attacks, e_structure, e_shields = enemy_stats.get_basic_stats()
            if e_attacks:
                e_num_attacks = sum(n for n in e_attacks.values())
                e_total_attack = sum(n*dmg for dmg, n in e_attacks.iteritems())
                e_avg_attack = e_total_attack / e_num_attacks
                e_net_attack = sum(n*max(dmg - my_shields, .001) for dmg, n in e_attacks.iteritems())
                e_net_attack = max(e_net_attack, .1*e_total_attack)
                shield_factor = e_total_attack / e_net_attack
                my_structure *= max(1, shield_factor)
            my_total_attack = sum(n*max(dmg - e_shields, .001) for dmg, n in my_attacks.iteritems())
        else:
            my_total_attack = sum(n*dmg for dmg, n in my_attacks.iteritems())
            my_structure += my_shields

        if ignore_fighters:
            return _rating()

        # consider fighter attacks
        capacity, launch_rate, fighter_damage = self.get_fighter_stats()
        launched_1st_bout = min(capacity, launch_rate)
        launched_2nd_bout = min(max(capacity - launch_rate, 0), launch_rate)
        survival_rate = .2  # chance of a fighter launched in bout 1 to live in turn 3 TODO Actual estimation
        total_fighter_damage = fighter_damage * (launched_1st_bout * (1+survival_rate) + launched_2nd_bout)
        fighter_damage_per_bout = total_fighter_damage / 3
        my_total_attack += fighter_damage_per_bout

        # consider fighter protection factor
        fighters_shot_down = (1-survival_rate**2) * launched_1st_bout + (1-survival_rate) * launched_2nd_bout
        damage_prevented = fighters_shot_down * e_avg_attack
        my_structure += damage_prevented
        return _rating()

    def get_rating_vs_planets(self):
        return self.get_rating(ignore_fighters=True)

    def get_stats(self, hashable=False):
        """ Get all combat related stats of the ship.

        :param hashable: if true, return tuple instead of dict for attacks
        :return: attacks, structure, shields, fighter-capacity, fighter-launch_rate, fighter-damage
        """
        return self.get_basic_stats(hashable=hashable) + self.get_fighter_stats()


class FleetCombatStats(object):
    """Stores combat related stats of the fleet."""
    def __init__(self, fleet_id=INVALID_ID, consider_refuel=False):
        self.__fleet_id = fleet_id
        self._consider_refuel = consider_refuel
        self.__ship_stats = []
        self.__get_stats_from_fleet()

    def get_ship_stats(self, hashable=False):
        """Get combat stats of all ships in fleet.

        :param hashable: if true, returns tuple instead of dict for attacks
        :type hashable: bool
        :return: list of ship stats
        :rtype: list
        """
        return map(lambda x: x.get_stats(hashable=hashable), self.__ship_stats)

    def get_ship_combat_stats(self):
        """Returns list of ShipCombatStats of fleet."""
        return list(self.__ship_stats)

    def get_rating(self, enemy_stats=None, ignore_fighters=False):
        """Calculates the rating of the fleet by combining all its ships ratings.

        :param enemy_stats: enemy to be rated against
        :type enemy_stats: ShipCombatStats
        :param ignore_fighters: If True, acts as if fighters are not launched
        :type ignore_fighters: bool
        :return: Rating of the fleet
        :rtype: float
        """
        return combine_ratings_list(map(lambda x: x.get_rating(enemy_stats, ignore_fighters), self.__ship_stats))

    def get_rating_vs_planets(self):
        return self.get_rating(ignore_fighters=True)

    def __get_stats_from_fleet(self):
        """Calculate fleet combat stats (i.e. the stats of all its ships)."""
        universe = fo.getUniverse()
        fleet = universe.getFleet(self.__fleet_id)
        if not fleet:
            return
        for ship_id in fleet.shipIDs:
            self.__ship_stats.append(ShipCombatStats(ship_id, self._consider_refuel))


def get_fleet_rating(fleet_id, enemy_stats=None):
    """Get rating for the fleet against specified enemy.

    :param fleet_id: fleet to be rated
    :type fleet_id: int
    :param enemy_stats: enemy to be rated against
    :type enemy_stats: ShipCombatStats
    :return: Rating
    :rtype: float
    """
    return FleetCombatStats(fleet_id, consider_refuel=False).get_rating(enemy_stats)


def get_fleet_rating_against_planets(fleet_id):
    return FleetCombatStats(fleet_id, consider_refuel=False).get_rating_vs_planets()


@cache_by_session
def _get_species_grades(species_name, grade_type):
    spec_tags = []
    if species_name:
        species = fo.getSpecies(species_name)
        if species:
            spec_tags = species.tags
        else:
            warn("get_species_grades() couldn't retrieve species '%s'\n" % species_name)
    return get_ai_tag_grade(spec_tags, grade_type)


def get_pilot_weapons_grade(species_name):
    """
    Return pilot grade string.

    :rtype str
    """
    return _get_species_grades(species_name, 'WEAPONS')


def get_species_troops_grade(species_name):
    """
    Return troop grade string.

    :rtype str
    """
    return _get_species_grades(species_name, 'ATTACKTROOPS')


def get_species_shield_grade(species_name):
    """
    Return shield grade string.

    :rtype str
    """
    return _get_species_grades(species_name, 'SHIELDS')


def weight_attack_troops(troops, grade):
    """Re-weights troops on a ship based on species piloting grade.

    :type troops: float
    :type grade: str
    :return: piloting grade weighted troops
    :rtype: float
    """
    weight = {'NO': 0.0, 'BAD': 0.5, '': 1.0, 'GOOD': 1.5, 'GREAT': 2.0, 'ULTIMATE': 3.0}.get(grade, 1.0)
    return troops * weight


def weight_shields(shields, grade):
    """Re-weights shields based on species defense bonus."""
    offset = {'NO': 0, 'BAD': 0, '': 0, 'GOOD': 1.0, 'GREAT': 0, 'ULTIMATE': 0}.get(grade, 0)
    return shields + offset


def combine_ratings(rating1, rating2):
    """ Combines two combat ratings to a total rating.

    The formula takes into account the fact that the combined strength of two ships is more than the
    sum of its individual ratings. Basic idea as follows:

    We use the following definitions

    r: rating
    a: attack
    s: structure

    where we take into account effective values after accounting for e.g. shields effects.

    We generally define the rating of a ship as
    r_i = a_i*s_i                                                                   (1)

    A natural extension for the combined rating of two ships is
    r_tot = (a_1+a_2)*(s_1+s_2)                                                     (2)

    Assuming         a_i \approx s_i                                                (3)
    It follows that  a_i \approx \sqrt(r_i) \approx s_i                             (4)
    And thus         r_tot = (sqrt(r_1)+sqrt(r_2))^2 = r1 + r2 + 2*sqrt(r1*r2)      (5)

    Note that this function has commutative and associative properties.

    :param rating1:
    :type rating1: float
    :param rating2:
    :type rating2: float
    :return: combined rating
    :rtype: float
    """
    return rating1 + rating2 + 2 * (rating1 * rating2)**0.5


def combine_ratings_list(ratings_list):
    """ Combine ratings in the list.

    Repetitively calls combine_ratings() until finished.

    :param ratings_list: list of ratings to be combined
    :type ratings_list: list
    :return: combined rating
    :rtype: float
    """
    return reduce(combine_ratings, ratings_list) if ratings_list else 0


def rating_needed(target, current=0):
    """Estimate the needed rating to achieve target rating.

    :param target: Target rating to be reached
    :type target: float
    :param current: Already existing rating
    :type current: float
    :return: Estimated missing rating to reach target
    :rtype: float
    """
    if current >= target or target <= 0:
        return 0
    else:
        return target + current - 2 * (target * current)**0.5


def rating_difference(first_rating, second_rating):

    """Return the absolute nonlinear difference between ratings.

    :param first_rating: rating of a first force
    :type first_rating: float
    :param second_rating: rating of a second force
    :type second_rating: float
    :return: Estimated rating by which the greater force (nonlinearly) exceeds the lesser
    :rtype: float
    """

    return rating_needed(max(first_rating, second_rating), min(first_rating, second_rating))
