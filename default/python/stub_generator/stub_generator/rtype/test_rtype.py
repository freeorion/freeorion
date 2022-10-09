import pytest

from stub_generator.stub_generator.rtype.base_rtype_mapping import make_type


# IntIntDblMapMap
# IntPairVec
# IntSetSet
# IntSetFltMap


@pytest.mark.parametrize(
    ("string, hint"),
    (
        # Base types
        ("Int", "int"),
        ("Flt", "float"),
        ("String", "str"),
        # # Simple collections
        ("IntPair", "Tuple[int, int]"),
        ("IntSet", "Set[int]"),
        ("IntVec", "Sequence[int]"),
        # embeded collections
        ("IntSetSet", "Set[Set[int]]"),
        # mapping
        ("StringIntMap", "Map[str, int]"),
        ("IntSetFltMap", "Map[Set[int], float]"),
        # Not matchable
        ("Hello", "Hello"),
        ("HelloVec", "HelloVec"),
        # Empty
        ("", ""),
        (None, ""),
    ),
)
def test_make_type(string, hint):
    assert make_type(string) == hint
