import SaveGameManager


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
        reload(SaveGameManager)  # just to be sure
        SaveGameManager._trusted_classes.update({
            __name__ + ".DummyTestClass": DummyTestClass,
            __name__ + ".GetStateTester": GetStateTester,
            __name__ + ".SetStateTester": SetStateTester,
        })

    def __exit__(self, exc_type, exc_val, exc_tb):
        SaveGameManager._trusted_classes.clear()

    def __del__(self):
        SaveGameManager._trusted_classes.clear()


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


def test_simple_object_encoding():
    def test_encoding(obj):
        retval = SaveGameManager.encode(obj)
        assert retval
        assert isinstance(retval, str)

        restored_obj = SaveGameManager.decode(retval)
        assert type(restored_obj) == type(obj)
        assert restored_obj == obj

    for obj in (
            int(), float(), str(), bool(),
            1, 1.2, 1.2e4, "TestString", True, False, None,
            tuple(), set(), list(), dict(),
            (1, 2, 3), [1, 2, 3], {1, 2, 3},
            {'a': 1, True: False, (1, 2): {1, 2, 3}, (1, 2, (3, (4,))): [1, 2, (3, 4), {1, 2, 3}]},
            ):
        test_encoding(obj)

    for obj in (lambda x: 1, test_encoding, OldStyleClass, DummyTestClass):
        try:
            test_encoding(obj)
            raise AssertionError("Could save unsupported, potentially dangerous object.")
        except SaveGameManager.CanNotSaveGameException:
            pass


def test_class_encoding():
    obj = DummyTestClass()
    try:
        retval = SaveGameManager.encode(obj)
        raise AssertionError("Could save game eventhough test module should not be trusted!")
    except SaveGameManager.CanNotSaveGameException:
        pass

    with TrustedScope():
        retval = SaveGameManager.encode(obj)

        assert retval
        assert isinstance(retval, str)

        loaded_obj = SaveGameManager.decode(retval)
        assert isinstance(loaded_obj, DummyTestClass)
        assert type(loaded_obj) == type(obj)

        original_dict = loaded_obj.__dict__
        original_keys = original_dict.keys().sort()
        original_values = original_dict.values().sort()

        loaded_dict = loaded_obj.__dict__
        loaded_keys = loaded_dict.keys().sort()
        loaded_values = loaded_dict.values().sort()

        assert original_keys == loaded_keys
        assert loaded_values == original_values
        assert loaded_dict == original_dict

    try:
        loaded_obj = SaveGameManager.decode(retval)
        raise AssertionError("Could load object from untrusted module")
    except SaveGameManager.InvalidSaveGameException:
        pass


def test_getstate_call():
    with TrustedScope():
        try:
            SaveGameManager.encode(GetStateTester())
            raise AssertionError("__getstate__ was not called during encoding.")
        except Success:
            pass


def test_setstate_call():
    with TrustedScope():
        try:
            retval = SaveGameManager.decode(SaveGameManager.encode(SetStateTester()))
            raise AssertionError("__setstate__ was not called during decoding.")
        except Success:
            pass


def test_enums():
    import EnumsAI

    test_obj = {EnumsAI.MissionType.COLONISATION: EnumsAI.MissionType.INVASION}
    restored_obj = SaveGameManager.decode(SaveGameManager.encode(test_obj))
    assert test_obj == restored_obj
