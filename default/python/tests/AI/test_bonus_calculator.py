import pytest
from freeorion_tools.bonus_calculation import Bonus, adjust_direction


@pytest.mark.parametrize(
    ("required_stability", "actual_stability", "expected"),
    ((0.0, 1.0, 5.5), (1.0, 1.0, 5.5), (2.0, 1.0, 0)),
)
def test_get_bonus_with_available_depends_on_stability(required_stability, actual_stability, expected):
    bonus = Bonus(True, required_stability, 5.5)
    assert bonus.get_bonus(actual_stability) == expected


@pytest.mark.parametrize(
    ("required_stability", "actual_stability"),
    ((0.0, 1.0), (1.0, 1.0), (2.0, 1.0)),
)
def test_get_bonus_with_not_available_is_zero(required_stability, actual_stability):
    bonus = Bonus(False, required_stability, 5.5)
    assert bonus.get_bonus(actual_stability) == 0.0


@pytest.mark.parametrize(
    ("threshold", "current", "target", "expected"),
    ((10.0, 9.0, 6.0, 0), (10.0, 8.0, 9.0, 0), (9.0, 6.5, 9.0, 1), (10.0, 12.0, 9.9, -1)),
)
def test_adjust_direction(threshold, current, target, expected):
    assert adjust_direction(threshold, current, target) == expected
