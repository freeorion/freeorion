import pytest
from freeorion_tools.bonus_calculation import Bonus


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
