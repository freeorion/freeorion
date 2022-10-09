from CombatRatingsAI import ShipCombatStats


class DesignStats:
    def __init__(self):
        self.attacks = {}  # {damage: shots_per_round}
        self.reset()  # this call initiates remaining instance variables!

    # noinspection PyAttributeOutsideInit
    def reset(self):
        self.attacks.clear()
        self.structure = 0
        self.shields = 0
        self.fuel = 0
        self.speed = 0
        self.stealth = 0
        self.detection = 0
        self.troops = 0
        self.colonisation = -1  # -1 since 0 indicates an outpost (capacity = 0)
        self.fuel_per_turn = 0
        self.organic_growth = 0
        self.maximum_organic_growth = 0
        self.repair_per_turn = 0
        self.asteroid_stealth = 0
        self.solar_stealth = 0
        self.fighter_capacity = 0
        self.fighter_launch_rate = 0
        self.fighter_damage = 0
        self.flak_shots = 0
        self.has_interceptors = False
        self.damage_vs_planets = 0
        self.has_bomber = False
        self.shield_type = None

    def convert_to_combat_stats(self):
        """Return a tuple as expected by CombatRatingsAI"""
        return ShipCombatStats(
            attacks=self.attacks,
            structure=self.structure,
            shields=self.shields,
            fighter_capacity=self.fighter_capacity,
            fighter_launch_rate=self.fighter_launch_rate,
            fighter_damage=self.fighter_damage,
            flak_shots=self.flak_shots,
            has_interceptors=self.has_interceptors,
            damage_vs_planets=self.damage_vs_planets,
            has_bomber=self.has_bomber,
        )
