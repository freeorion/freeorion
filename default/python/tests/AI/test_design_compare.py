from freeorion_tools.design_compare import recursive_dict_diff


def test_compare_empty_dicts():
    assert recursive_dict_diff({}, {}, diff_level_threshold=1) == {}


def test_compare_dict_returns_intersection():
    dict_a = {1: 2, 2: {2: 3, 3: 4}}
    dict_b = {2: {2: 3, 3: 3}}
    assert recursive_dict_diff(dict_a, dict_b, diff_level_threshold=1) == {1: 2, 2: {3: 4}}
