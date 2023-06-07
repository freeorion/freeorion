from dataclasses import dataclass, field

from CombatRatingsAI import ShipCombatStats
from common.fo_typing import AttackCount, AttackDamage


@dataclass
class DesignStats:
    attacks: dict[AttackDamage, AttackCount] = field(default_factory=dict)
    structure: float = 0
    shields: float = 0
    fuel: float = 0
    speed: float = 0
    stealth: float = 0
    detection: float = 0
    troops: float = 0
    colonisation: int = -1  # -1 since 0 indicates an outpost (capacity = 0)
    fuel_per_turn: float = 0
    organic_growth: float = 0
    maximum_organic_growth: float = 0
    repair_per_turn: float = 0
    asteroid_stealth: float = 0
    solar_stealth: float = 0
    fighter_capacity: float = 0
    fighter_launch_rate: float = 0
    fighter_damage: float = 0
    flak_shots: float = 0
    has_interceptors: bool = False
    damage_vs_planets: float = 0
    has_bomber: bool = False
    shield_type: str = None

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
