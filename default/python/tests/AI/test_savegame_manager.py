import pytest

import savegame_codec
from pytest import fixture


class DummyTestClass(object):
    def __init__(self):
        self.some_int = 812
        self.some_negative_int = -712
        self.some_float = 1.2
        self.some_other_float = 1.248e3
        self.some_string = "TestString"
        self._some_private_var = "_Test"
        self.__some_more_private_var = "__Test"
        self.some_dict = {'ABC': 123.1, (1, 2.3, 'ABC', "zz"): (1, (2, (3, 4)))}
        self.some_list = [self.some_int, self.some_float, self.some_other_float, self.some_string]
        self.some_set = set(self.some_list)
        self.some_tuple = tuple(self.some_list)

    def dummy_class_function(self):
        pass


class TrustedScope(object):

    def __enter__(self):
        reload(savegame_codec)  # just to be sure
        savegame_codec._definitions.trusted_classes.update({
            __name__ + ".DummyTestClass": DummyTestClass,
            __name__ + ".GetStateTester": GetStateTester,
            __name__ + ".SetStateTester": SetStateTester,
        })

    def __exit__(self, exc_type, exc_val, exc_tb):
        savegame_codec._definitions.trusted_classes.clear()

    def __del__(self):
        savegame_codec._definitions.trusted_classes.clear()


class Success(Exception):
    pass


class GetStateTester(object):
    def __getstate__(self):
        raise Success


class SetStateTester(object):
    def __setstate__(self, state):
        raise Success


class OldStyleClass():
    pass


@fixture(
    params=[
        int(), float(), str(), bool(),
        1, 1.2, 1.2e4, "TestString", True, False, None,
        tuple(), set(), list(), dict(),
        (1, 2, 3), [1, 2, 3], {1, 2, 3},
        {'a': 1, True: False, (1, 2): {1, 2, 3}, (1, 2, (3, (4,))): [1, 2, (3, 4), {1, 2, 3}]}
    ],
    ids=str)
def simple_object(request, ):
    return request.param


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
    with pytest.raises(savegame_codec.CanNotSaveGameException,
                       message="Could save function",
                       match="Class __builtin__.function is not trusted"):
        check_encoding(lambda: 0)


def test_encoding_old_style_class():
    with pytest.raises(savegame_codec.CanNotSaveGameException,
                       message="Could save Old style class",
                       match="Encountered unsupported object test_savegame_manager.OldStyleClass \(<type 'classobj'>\)"):
        check_encoding(OldStyleClass)


def test_encoding_type():
    with pytest.raises(savegame_codec.CanNotSaveGameException,
                       message="Could save untrusted class",
                       match="Class __builtin__.type is not trusted"):
        check_encoding(list)


def test_class_encoding():
    obj = DummyTestClass()
    with pytest.raises(savegame_codec.CanNotSaveGameException,
                       message="Could save game even though test module should not be trusted",
                       match="Class test_savegame_manager.DummyTestClass is not trusted"):
        savegame_codec.encode(obj)

    with TrustedScope():
        retval = savegame_codec.encode(obj)

        assert retval
        assert isinstance(retval, str)

        loaded_obj = savegame_codec.decode(retval)
        assert isinstance(loaded_obj, DummyTestClass)
        assert type(loaded_obj) == type(obj)

        original_dict = loaded_obj.__dict__
        loaded_dict = loaded_obj.__dict__

        assert loaded_dict == original_dict

    with pytest.raises(savegame_codec.InvalidSaveGameException,
                       message="Could load object from untrusted module",
                       match="DANGER DANGER - test_savegame_manager.DummyTestClass not trusted"):
        savegame_codec.decode(retval)


def test_getstate_call():
    with TrustedScope():
        with pytest.raises(Success, message="__getstate__ was not called during encoding."):
            savegame_codec.encode(GetStateTester())


def test_setstate_call():
    with TrustedScope():
        with pytest.raises(Success, message="__setstate__ was not called during decoding."):
            savegame_codec.decode(savegame_codec.encode(SetStateTester()))


def test_enums():
    import EnumsAI

    test_obj = {EnumsAI.MissionType.COLONISATION: EnumsAI.MissionType.INVASION}
    restored_obj = savegame_codec.decode(savegame_codec.encode(test_obj))
    assert test_obj == restored_obj
