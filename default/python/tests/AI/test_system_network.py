import freeOrionAIInterface as fo
from unittest.mock import patch

import pytest
from AIDependencies import INVALID_ID

from universe import system_network


class TestUniverse:
    connected_systems = {
        1: {2, 7},
        2: {1, 3},
        3: {2, 4, 5, 6},
        4: {3, 5},
        5: {3, 4, 6},
        6: {3, 5, 7},
        7: {1, 6, 8},
        8: {7, 9},
        9: {8, 10},
        10: {9},
        11: {12},
        12: {11},
    }

    # fo interface asks for an empire_id, so we need two arguments as well
    def getImmediateNeighbors(self, sys_id, _):
        return TestUniverse.connected_systems.get(sys_id, {})

    @staticmethod
    def systemsConnected(sx, sy, _):
        return sx in system_network.systems_connected_to_system(sy)


@pytest.fixture(autouse=True)
def connected_systems(monkeypatch):
    monkeypatch.setattr(fo, "getUniverse", TestUniverse)


def test_connections():
    """
    Test that the graph is bi-directional.
    This simplifies the implementation of TestUniverse and makes it easier to determine the expected results.
    """
    for i1, lanes in TestUniverse.connected_systems.items():
        for i2 in lanes:
            assert i1 in TestUniverse.connected_systems[i2]


@pytest.mark.parametrize(
    ("sys_id", "expected"),
    (
        (1, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}),
        (4, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}),
        (12, {11, 12}),
        (INVALID_ID, set()),
    ),
)
def test_systems_connected_to_system(sys_id, expected):
    assert system_network.systems_connected_to_system(sys_id) == expected


@pytest.mark.parametrize(
    ("sys_id", "n", "expected"),
    (
        (1, 0, {1}),
        (1, 1, {1, 2, 7}),
        (1, 2, {1, 2, 3, 6, 7, 8}),
        (1, 3, {1, 2, 3, 4, 5, 6, 7, 8, 9}),
        (1, 4, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}),
        (1, 5, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}),
        (10, 1, {9, 10}),
        (10, 2, {8, 9, 10}),
        (10, 3, {7, 8, 9, 10}),
        (10, 4, {1, 6, 7, 8, 9, 10}),
        (10, 5, {1, 2, 3, 5, 6, 7, 8, 9, 10}),
        (11, 5, {11, 12}),
    ),
)
def test_within_n_jumps(sys_id, n, expected):
    assert system_network.within_n_jumps(sys_id, n) == expected


@pytest.mark.parametrize(
    ("s1", "s2", "expected"), ((1, 2, True), (1, 12, False), (11, 1, False), (11, 12, True), (11, 13, False))
)
def test_systems_connected(s1, s2, expected, monkeypatch):
    with patch("universe.system_network.get_capital_sys_id") as get_capital_sys_id:
        get_capital_sys_id.return_value = 1
        assert system_network.systems_connected(s1, s2) == expected
        assert system_network.systems_connected(s2, s1) == expected
