import pytest
from colonization.calculate_planet_colonization_rating import _colony_upkeep


@pytest.mark.parametrize(
    (
        "num_exobots",
        "num_normal",
        "num_outpost",
        "max_colony_upkeep",
        "owned_colonies_at_max_upkeep",
        "outpost_factor",
        "expected",
    ),
    (
        (0, 1, 0, 9.0, 1000, 0.25, 0.0),  # Game start: one capital and nothing else, no upkeep
        (0, 2, 0, 9.0, 1000, 0.25, 0.568925303),  # First colony (non-exobot)
        (0, 1, 5, 9.0, 1000, 0.25, 0.0),  # Capital and many outposts.
        (0, 2, 1, 9.0, 1000, 0.25, 0.603398656),  # One colony and one outpost.
        (0, 11, 2, 9.0, 1000, 0.25, 13.609877847),  # Ten colonies, two outposts.
        (10, 1, 0, 9.0, 1000, 0.25, 7.523348656),  # Capital and 10 exobots.
        (0, 999, 0, 9.0, 1000, 0.25, 8981.9955),  # One less than max colonies
        (0, 1000, 0, 9.0, 1000, 0.25, 8991.0),  # Max colonies
        (0, 1001, 0, 9.0, 1000, 0.25, 9000.0),  # One more than max colonies
    ),
)
def test_colony_upkeep(
    num_exobots: int,
    num_normal: int,
    num_outpost: int,
    max_colony_upkeep: float,
    owned_colonies_at_max_upkeep: int,
    outpost_factor: float,
    expected: float,
):
    assert pytest.approx(expected) == _colony_upkeep(
        num_exobots, num_normal, num_outpost, max_colony_upkeep, owned_colonies_at_max_upkeep, outpost_factor
    )
