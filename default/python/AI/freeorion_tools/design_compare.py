import copy
from typing import TypeVar

KT = TypeVar("KT")
VT = TypeVar("VT")


def _recursive_dict_diff(
    dict_new: dict[KT, VT], dict_old: dict[KT, VT], dict_diff: dict[KT, VT], diff_level_threshold=0
) -> int:
    NO_DIFF = 9999
    min_diff_level = NO_DIFF
    for key, value in dict_new.items():
        if key not in dict_old:
            dict_diff[key] = copy.deepcopy(value)
            min_diff_level = 0
        elif isinstance(value, dict):
            this_diff_level = (
                _recursive_dict_diff(value, dict_old[key], dict_diff.setdefault(key, {}), diff_level_threshold) + 1
            )
            min_diff_level = min(min_diff_level, this_diff_level)
            if this_diff_level > NO_DIFF and min_diff_level > diff_level_threshold:
                del dict_diff[key]
        elif key not in dict_old or value != dict_old[key]:
            dict_diff[key] = copy.deepcopy(value)
            min_diff_level = 0
    return min_diff_level


def recursive_dict_diff(dict_new: dict[KT, VT], dict_old: dict[KT, VT], diff_level_threshold=0) -> dict[KT, VT]:
    """
    Find the entries in dict_new that are not present in dict_old and store them in dict_diff.

    Example usage:
    dict_a = {1:2, 2: {2: 3, 3: 4}}
    dict_b = {2: {2: 3, 3: 3}}
    recursive_dict_diff(dict_a, dict_b)
    >>> {1:2, 2:{3:4}}

    :param dict_diff: Difference between dict_old and dict_new, modified and filled within this function
    :param diff_level_threshold: Depth to next diff up to which non-diff entries are stored in dict_diff
    :return: recursive depth distance to entries differing in dict_new and dict_old
    """

    diff = {}
    _recursive_dict_diff(dict_new, dict_old, diff, diff_level_threshold=diff_level_threshold)
    return diff
