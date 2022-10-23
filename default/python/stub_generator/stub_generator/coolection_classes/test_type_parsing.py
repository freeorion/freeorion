import pytest

from stub_generator.stub_generator.coolection_classes import make_type


@pytest.mark.parametrize(
    ("string, hint"),
    (
        # Base types
        ("Int", "int"),
        ("Float", "float"),
        ("Double", "float"),
        ("String", "str"),
        # Simple collections
        ("IntPair", "Tuple[int, int]"),
        ("IntSet", "Set[int]"),
        ("IntVec", "Vec[int]"),
        # Embedded collections
        ("IntSetSet", "Set[Set[int]]"),
        # mapping
        ("StringIntMap", "Map[str, int]"),
        ("IntSetFloatMap", "Map[Set[int], float]"),
        ("IntMeterTypeAccountingInfoVecMapMap", "Map[int, Map[meterType, Vec[AccountingInfo]]]"),
        # Not matchable
        ("Hello", "Hello"),
        ("HelloVec", "HelloVec"),
        # Empty
        ("", ""),
        (None, ""),
        # Underscores are ignored
        ("_I_n_t_", "int"),
        ("Int_Set", "Set[int]"),
        ("Int_MeterType_AccountingInfoVecMapMap", "Map[int, Map[meterType, Vec[AccountingInfo]]]"),
    ),
)
def test_make_type(string, hint):
    assert make_type(string) == hint
