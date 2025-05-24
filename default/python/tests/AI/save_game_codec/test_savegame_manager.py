import pytest

import savegame_codec
from CombatRatingsAI import ShipCombatStats
from savegame_codec._definitions import TrustedClasses


class DummyTestClass:
    def __init__(self):
        self.some_int = 812
        self.some_negative_int = -712
        self.some_float = 1.2
        self.some_other_float = 1.248e3
        self.some_string = "TestString"
        self._some_private_var = "_Test"
        self.__some_more_private_var = "__Test"
        self.some_dict = {"ABC": 123.1, (1, 2.3, "ABC", "zz"): (1, (2, (3, 4)))}
        self.some_list = [self.some_int, self.some_float, self.some_other_float, self.some_string]
        self.some_set = set(self.some_list)
        self.some_tuple = tuple(self.some_list)

    def dummy_class_function(self):
        pass


@pytest.fixture(
    params=[
        0,
        0.0,
        "",
        False,
        1,
        1.2,
        1.2e4,
        "TestString",
        True,
        False,
        None,
        tuple(),
        set(),
        list(),
        dict(),
        (1, 2, 3),
        [1, 2, 3],
        {1, 2, 3},
        {"a": 1, True: False, (1, 2): {1, 2, 3}, (1, 2, (3, (4,))): [1, 2, (3, 4), {1, 2, 3}]},
    ],
    ids=str,
)
def simple_object(
    request,
):
    return request.param


def test_class_with_protected_attribute():
    foo = ShipCombatStats()
    trusted_classes = TrustedClasses()
    retval = savegame_codec.encode(foo, TrustedClasses())
    assert retval
    assert isinstance(retval, str)

    restored_obj = savegame_codec.decode(retval, trusted_classes)
    assert type(restored_obj) is type(foo)
    assert restored_obj == foo


def check_encoding(obj):
    trusted_classes = TrustedClasses()
    retval = savegame_codec.encode(obj, trusted_classes)
    assert retval
    assert isinstance(retval, str)

    restored_obj = savegame_codec.decode(retval, trusted_classes)
    assert type(restored_obj) is type(obj)
    assert restored_obj == obj


def test_encoding_simple_object(simple_object):
    check_encoding(simple_object)


def test_encoding_function():
    match = "Class builtins.function is not trusted"

    with pytest.raises(savegame_codec.CanNotSaveGameException, match=match):
        check_encoding(lambda: 0)
        pytest.fail("Could save function")


def test_encoding_type():
    match = "Class builtins.type is not trusted$"

    with pytest.raises(savegame_codec.CanNotSaveGameException, match=match):
        check_encoding(list)
        pytest.fail("Could save untrusted class")


def test_class_encoding():
    trusted_scope = TrustedClasses()
    obj = DummyTestClass()
    with pytest.raises(
        savegame_codec.CanNotSaveGameException, match="Class test_savegame_manager.DummyTestClass is not trusted"
    ):
        savegame_codec.encode(obj, trusted_scope)
        pytest.fail("Could save game even though test module should not be trusted")

    new_trusted_scope = TrustedClasses({__name__ + ".DummyTestClass": DummyTestClass})

    retval = savegame_codec.encode(obj, new_trusted_scope)

    assert retval
    assert isinstance(retval, str)

    loaded_obj = savegame_codec.decode(retval, new_trusted_scope)
    assert isinstance(loaded_obj, DummyTestClass)
    assert type(loaded_obj) is type(obj)

    original_dict = loaded_obj.__dict__
    loaded_dict = loaded_obj.__dict__

    assert loaded_dict == original_dict


def test_enums():
    import EnumsAI

    test_obj = {EnumsAI.MissionType.COLONISATION: EnumsAI.MissionType.INVASION}
    trusted_classes = TrustedClasses()
    restored_obj = savegame_codec.decode(savegame_codec.encode(test_obj, TrustedClasses()), trusted_classes)
    assert test_obj == restored_obj
