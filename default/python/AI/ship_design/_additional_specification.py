import freeOrionAIInterface as fo

from aistate_interface import get_aistate
from CombatRatingsAI import ShipCombatStats


class AdditionalSpecifications:
    """This class is a container for all kind of additional information
    and requirements we may want to use when assessing ship designs.

    methods for external use:
    convert_to_tuple(): Builds and returns a tuple of the class attributes
    update_enemy(enemy): updates enemy stats
    """

    def __init__(self):
        # TODO: Extend this framework according to needs of future implementations
        self.minimum_fuel = 0
        self.minimum_speed = 0
        self.orbital = False
        self.minimum_structure = 1
        self.minimum_fighter_launch_rate = 0
        self.enemy_shields = 1  # to avoid spamming flak cannons
        self.max_enemy_weapon_strength = 0
        self.avg_enemy_weapon_strength = 0
        self.expected_turns_till_fight = 2
        current_turn = fo.currentTurn()
        if current_turn < 80:
            self.enemy_mine_dmg = 0  # TODO: Implement the detection of actual enemy mine damage
        elif current_turn < 150:
            self.enemy_mine_dmg = 2
        elif current_turn < 230:
            self.enemy_mine_dmg = 6
        else:
            self.enemy_mine_dmg = 14
        self.enemy = None
        self.update_enemy(get_aistate().get_standard_enemy())

    def update_enemy(self, enemy: ShipCombatStats):
        """Read out the enemies stats and save them."""
        self.enemy = enemy
        self.enemy_shields = enemy.shields
        self.enemy_shields += 1  # add bias against weak weapons to account to allow weapons to stay longer relevant.

        enemy_attack_stats = enemy.attacks
        if enemy_attack_stats:
            self.max_enemy_weapon_strength = max(enemy_attack_stats.keys())
            n = 0
            d = 0
            for dmg, count in enemy_attack_stats.items():
                d += dmg * count
                n += count
            self.avg_enemy_weapon_strength = d // n  # TODO check if we need floor division here

    def convert_to_tuple(self):
        """Create a tuple of this class' attributes (e.g. to use as key in dict).

        :returns: tuple (minFuel,minSpeed,enemyDmg,enemyShield,enemyMineDmg)
        """
        return (
            "minFuel: %s" % self.minimum_fuel,
            "minSpeed: %s" % self.minimum_speed,
            "enemyDmg: %s" % self.max_enemy_weapon_strength,
            "enemyShields: %s" % self.enemy_shields,
            "enemyMineDmg: %s" % self.enemy_mine_dmg,
        )
