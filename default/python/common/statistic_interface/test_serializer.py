import pytest
from common.statistic_interface._serizlizer import (
    DictSerializer,
    to_float,
    to_int,
    to_str,
)


@pytest.mark.parametrize(
    ("converter", "serialized", "deserialized"),
    (
        (to_int, "11", 11),
        (to_float, "11.11", 11.11),
        (to_str, "hello", "hello"),
        (
            DictSerializer(
                {
                    "int": to_int,
                    "float": to_float,
                    "str": to_str,
                }
            ),
            "int: 11, float: 11.11, str: hello",
            {"int": 11, "float": 11.11, "str": "hello"},
        ),
    ),
)
def test_conversion(converter, serialized, deserialized):
    assert converter.deserialize(serialized) == deserialized
