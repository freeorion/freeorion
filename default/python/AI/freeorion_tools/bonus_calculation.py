from typing import NamedTuple


class Bonus(NamedTuple):
    available: bool
    required_stability: float
    value: float

    def get_bonus(self, stability: float) -> float:
        if not self.available:
            return 0.0
        return self.value if stability >= self.required_stability else 0.0


def adjust_direction(threshold: float, current: float, target) -> int:
    """
    Returns -1, 0 or 1, depending on whether compared to current, target drops below, is on the same side or
    goes above the threshold.
    """
    return int(target >= threshold) - int(current >= threshold)
