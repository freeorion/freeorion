import pytest

from stub_generator.stub_generator.collection_classes import is_collection_type, make_type


@pytest.mark.parametrize(
    ("string, hint"),
    (
        # Base types
        ("Int", "int"),
        ("Float", "float"),
        ("Double", "float"),
        ("String", "str"),
        ("Bool", "bool"),
        # Simple collections
        ("IntPair", "tuple[int, int]"),
        ("IntSet", "Set[int]"),
        ("IntVec", "Sequence[int]"),
        # Embedded collections
        ("IntSetSet", "Set[Set[int]]"),
        # mapping
        ("StringIntMap", "Mapping[str, int]"),
        ("IntSetFloatMap", "Mapping[Set[int], float]"),
        ("IntMeterTypeAccountingInfoVecMapMap", "Mapping[int, Mapping[meterType, Sequence[AccountingInfo]]]"),
        # Not matchable
        ("Hello", "Hello"),
        ("HelloVec", "HelloVec"),
        # Empty
        ("", ""),
        (None, ""),
        # Underscores are ignored
        ("_I_n_t_", "int"),
        ("Int_Set", "Set[int]"),
        ("Int_MeterType_AccountingInfoVecMapMap", "Mapping[int, Mapping[meterType, Sequence[AccountingInfo]]]"),
    ),
)
def test_make_type(string, hint):
    assert make_type(string) == hint


@pytest.mark.parametrize(
    "type_string",
    ["IntSet", "IntVec", "IntIntMap", "AccountingInfoVec", "IntDoubleMap", "MeterTypeAccountingInfoVecPair"],
)
def test_collection_type_is_true(type_string):
    assert is_collection_type(type_string) is True


@pytest.mark.parametrize("type_string", ["Int", "Hello", "universe"])
def test_collection_type_is_false(type_string):
    assert is_collection_type(type_string) is False
