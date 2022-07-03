import pytest
from freeorion_tools.bonus_calculation import Bonus


@pytest.mark.parametrize(
    ("bonus", "expected"),
    (
        (Bonus(True, 1, 5.5), 0),
        (Bonus(True, 2, 5.5), 0),
        (Bonus(True, 0, 5.5), 5.5),
        (Bonus(False, 0, 5.5), 0),
        (Bonus(False, 1, 5.5), 0),
        (Bonus(False, 2, 5.5), 0),
    ),
)
def test_get_bonus_for_stability_1(bonus, expected):
    assert bonus.get_bonus(1) == expected
