import pytest
from dump_interface._serizlizer import (
    DictSerializer,
    FloatSerializer,
    IntSerializer,
    StrSerializer,
    TupleSerializer,
)


@pytest.mark.parametrize(
    ("converter", "serizlized", "desirizlized"),
    (
        (IntSerializer(), "11", 11),
        (FloatSerializer(), "11.11", 11.11),
        (StrSerializer(), "hello", "hello"),
        (
            TupleSerializer([IntSerializer(), FloatSerializer(), StrSerializer()]),
            "11, 11.11, hello",
            (11, 11.11, "hello"),
        ),
        (
            DictSerializer(
                {
                    "int": IntSerializer(),
                    "float": FloatSerializer(),
                    "str": StrSerializer(),
                }
            ),
            "int: 11, float: 11.11, str: hello",
            {"int": 11, "float": 11.11, "str": "hello"},
        ),
    ),
)
def test_conversion(converter, serizlized, desirizlized):
    assert converter.deserialize(serizlized) == desirizlized
