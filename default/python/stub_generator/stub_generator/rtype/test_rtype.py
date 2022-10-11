import pytest

from stub_generator.stub_generator.rtype._base_rtype_mapping import make_type


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
        ("Dbl", "float"),
        ("String", "str"),
        # Simple collections
        ("IntPair", "Tuple[int, int]"),
        ("IntSet", "Set[int]"),
        ("IntVec", "Vec[int]"),
        # Embedded collections
        ("IntSetSet", "Set[Set[int]]"),
        # mapping
        ("StringIntMap", "Map[str, int]"),
        ("IntSetFltMap", "Map[Set[int], float]"),
        ("IntMeterTypeAccountingInfoVecMapMap", "Map[int, Map[meterType, Sequence[AccountingInfo]]]"),
        # Not matchable
        ("Hello", "Hello"),
        ("HelloVec", "HelloVec"),
        # Empty
        ("", ""),
        (None, ""),
        # Underscores are ignored
        ("_I_n_t_", "int"),
        ("Int_Set", "Set[int]"),
        ("Int_MeterType_AccountingInfoVecMapMap", "Map[int, Map[meterType, Sequence[AccountingInfo]]]"),
    ),
)
def test_make_type(string, hint):
    assert make_type(string) == hint
