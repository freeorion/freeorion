from collections.abc import Mapping, Sequence, Set

import pytest

from common.fo_typing import PlanetId
from stub_generator.stub_generator.rtype.utils import get_name_for_mapping


@pytest.mark.parametrize(
    ("type_", "string"),
    [
        (int, "int"),
        (Sequence[int], "Sequence[int]"),
        (Sequence[PlanetId], "Sequence[PlanetId]"),
        (Set[int], "Set[int]"),
        (Set[PlanetId], "Set[PlanetId]"),
        (Mapping[int, PlanetId], "Mapping[int, PlanetId]"),
        (Sequence[Sequence[int]], "Sequence[Sequence[int]]"),
        (Sequence[Sequence[PlanetId]], "Sequence[Sequence[PlanetId]]"),
    ],
)
def test_get_name_for_mapping(type_, string):
    print()
    assert get_name_for_mapping(type_) == string
