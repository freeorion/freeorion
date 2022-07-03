from typing import NamedTuple


class Bonus(NamedTuple):
    available: bool
    required_stability: float
    value: float

    def get_bonus(self, stability: float) -> float:
        if not self.available:
            return 0.0
        return self.value if stability >= self.required_stability else 0.0
