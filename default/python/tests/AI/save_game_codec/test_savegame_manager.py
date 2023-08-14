import pytest

import savegame_codec
from CombatRatingsAI import ShipCombatStats


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


@pytest.fixture()
def trusted_scope(monkeypatch):
    test_scope = dict(savegame_codec._decoder.trusted_classes)
    test_scope.update(
        {
            __name__ + ".GetStateTester": GetStateTester,
            __name__ + ".SetStateTester": SetStateTester,
        }
    )
    monkeypatch.setattr(savegame_codec._decoder, "trusted_classes", test_scope)
    monkeypatch.setattr(savegame_codec._definitions, "trusted_classes", test_scope)
    monkeypatch.setattr(savegame_codec._encoder, "trusted_classes", test_scope)
    return test_scope


class Success(Exception):
    pass


class GetStateTester:
    def __getstate__(self):
        raise Success


class SetStateTester:
    def __setstate__(self, state):
        raise Success


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


def test_class_with_protected_attribute(trusted_scope):
    foo = ShipCombatStats()
    retval = savegame_codec.encode(foo)
    assert retval
    assert isinstance(retval, str)

    restored_obj = savegame_codec.decode(retval)
    assert type(restored_obj) == type(foo)
    assert restored_obj == foo


def check_encoding(obj):
    retval = savegame_codec.encode(obj)
    assert retval
    assert isinstance(retval, str)

    restored_obj = savegame_codec.decode(retval)
    assert type(restored_obj) == type(obj)
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


def test_class_encoding(trusted_scope):
    obj = DummyTestClass()
    with pytest.raises(
        savegame_codec.CanNotSaveGameException, match="Class test_savegame_manager.DummyTestClass is not trusted"
    ):
        savegame_codec.encode(obj)
        pytest.fail("Could save game even though test module should not be trusted")

    trusted_scope[__name__ + ".DummyTestClass"] = DummyTestClass
    retval = savegame_codec.encode(obj)

    assert retval
    assert isinstance(retval, str)

    loaded_obj = savegame_codec.decode(retval)
    assert isinstance(loaded_obj, DummyTestClass)
    assert type(loaded_obj) == type(obj)

    original_dict = loaded_obj.__dict__
    loaded_dict = loaded_obj.__dict__

    assert loaded_dict == original_dict

    del trusted_scope[__name__ + ".DummyTestClass"]

    with pytest.raises(
        savegame_codec.InvalidSaveGameException,
        match="DANGER DANGER - test_savegame_manager.DummyTestClass not trusted",
    ):
        savegame_codec.decode(retval)
        pytest.fail("Could load object from untrusted module")


@pytest.mark.usefixtures("trusted_scope")
def test_getstate_call():
    with pytest.raises(Success):
        savegame_codec.encode(GetStateTester())
        pytest.fail("__getstate__ was not called during encoding.")


@pytest.mark.usefixtures("trusted_scope")
def test_setstate_call():
    with pytest.raises(Success):
        savegame_codec.decode(savegame_codec.encode(SetStateTester()))
        pytest.fail("__setstate__ was not called during decoding.")


def test_enums():
    import EnumsAI

    test_obj = {EnumsAI.MissionType.COLONISATION: EnumsAI.MissionType.INVASION}
    restored_obj = savegame_codec.decode(savegame_codec.encode(test_obj))
    assert test_obj == restored_obj
