import pytest
from freeorion_tools import ReadOnlyDict

dict_content = {
    1: -1,
    1.09: 1.1,
    'abc': 'xyz',
    (1, 2, 3): ('a', 1, (1, 2)),
}


def test_dict_content():
    test_dict = ReadOnlyDict(dict_content)
    assert test_dict.keys() == dict_content.keys()
    assert test_dict.values() == dict_content.values()
    assert test_dict.items() == dict_content.items()
    assert len(test_dict) == len(dict_content)


def test_membership():
    test_dict = ReadOnlyDict(dict_content)
    # check for membership checks and retrieval
    for key, value in dict_content.iteritems():
        assert key in test_dict
        assert test_dict[key] == value
        assert test_dict.get(key, -99999) == value


def test_non_existing_keys():
    test_dict = ReadOnlyDict(dict_content)
    # check correct functionality if keys not in dict
    assert 'INVALID_KEY' not in test_dict
    assert test_dict.get('INVALID_KEY', -99999) == -99999
    with pytest.raises(KeyError, message="Invalid key lookup didn't raise a KeyError", match="'INVALID_KEY'"):
        # noinspection PyStatementEffect
        test_dict['INVALID_KEY']


def test_str_conversion():
    # check bool and str conversions
    test_dict = ReadOnlyDict(dict_content)
    assert str(test_dict) == str(dict_content)


def test_bool():
    test_dict = ReadOnlyDict(dict_content)
    assert bool(test_dict)
    assert test_dict
    empty_dict = ReadOnlyDict()
    assert not len(empty_dict)
    assert not empty_dict


def test_conversion_to_dict():
    read_only_dict = ReadOnlyDict(dict_content)
    normal_dict = dict(read_only_dict)
    assert len(normal_dict) == len(dict_content)
    assert normal_dict.items() == dict_content.items()


def test_deletion():
    test_dict = ReadOnlyDict(dict_content)
    # check that dict can not be modified
    try:
        del test_dict[1]
        raise AssertionError("Can delete items from the dict.")
    except TypeError:
        pass


def test_setitem():
    test_dict = ReadOnlyDict(dict_content)
    with pytest.raises(TypeError, message='Can add items to the dict.',
                       match="'ReadOnlyDict' object does not support item assignment"):
        test_dict['INVALID_KEY'] = 1
    assert 'INVALID_KEY' not in test_dict


def test_nonhashable_values():
    # make sure can't store unhashable items (as heuristic for mutable items)
    with pytest.raises(TypeError, message='Can store mutable items in dict', match="unhashable type: 'list'"):
        ReadOnlyDict({1: [1]})
