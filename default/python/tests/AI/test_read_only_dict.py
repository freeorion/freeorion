import pytest
from freeorion_tools import ReadOnlyDict
from pytest import raises

dict_content = {
    1: -1,
    1.09: 1.1,
    "abc": "xyz",
    (1, 2, 3): ("a", 1, (1, 2)),
}


def test_readonly_dict_created_from_dict_behaves_the_same():
    test_dict = ReadOnlyDict(dict_content)
    assert set(test_dict.keys()) == set(dict_content.keys())
    assert set(test_dict.values()) == set(dict_content.values())
    assert set(test_dict.items()) == set(dict_content.items())
    assert len(test_dict) == len(dict_content)


def test_readonly_dict_created_from_dict_iterates_over_same_keys():
    test_dict = ReadOnlyDict(dict_content)
    assert set(test_dict) == set(dict_content)


def test_readonly_dict_get_existing_key_works_same_as_for_dict():
    test_dict = ReadOnlyDict(dict_content)
    # check for membership checks and retrieval
    for key, value in dict_content.items():
        assert key in test_dict
        assert test_dict[key] == value
        assert test_dict.get(key, -99999) == value


def test_readonly_dict_get_non_existiong_key_works_same_as_for_dict():
    test_dict = ReadOnlyDict(dict_content)
    # check correct functionality if keys not in dict
    assert "INVALID_KEY" not in test_dict
    assert test_dict.get("INVALID_KEY", -99999) == -99999
    with pytest.raises(KeyError, match="'INVALID_KEY'"):
        # noinspection PyStatementEffect
        test_dict["INVALID_KEY"]
        pytest.fail("Invalid key lookup didn't raise a KeyError")


def test_readonly_dict_evaluated_to_bool_same_as_collection():
    test_dict = ReadOnlyDict(dict_content)
    assert bool(test_dict)
    assert test_dict
    empty_dict = ReadOnlyDict()
    assert not len(empty_dict)
    assert not empty_dict


def test_readonly_dict_created_from_dict_can_be_converted_to_the_same_dict():
    read_only_dict = ReadOnlyDict(dict_content)
    normal_dict = dict(read_only_dict)
    assert len(normal_dict) == len(dict_content)
    assert set(normal_dict.items()) == set(dict_content.items())


def test_readonly_dict_remove_value_raises_error():
    test_dict = ReadOnlyDict(dict_content)
    with raises(TypeError):
        del test_dict[1]
        raise AssertionError("Can delete items from the dict.")


def test_readonly_dict_set_value_raises_error():
    test_dict = ReadOnlyDict(dict_content)
    with raises(TypeError, match="'ReadOnlyDict' object does not support item assignment"):
        test_dict["INVALID_KEY"] = 1
        pytest.fail("Can add items to the dict.")
    assert "INVALID_KEY" not in test_dict


def test_readonly_create_from_dict_with_hashable_values_raises_error():
    with raises(TypeError, match="unhashable type: 'list'"):
        ReadOnlyDict({1: [1]})
        pytest.fail("Can store mutable items in dict")
