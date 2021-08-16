import freeOrionAIInterface as fo
from functools import wraps

from aistate_interface import get_aistate


def cache_for_session(func):
    """
    Cache a function value for current session.

    Wraps only functions with hashable arguments.
    Use this only if the called function return value is constant throughout the game.
    """
    _cache = {}

    @wraps(func)
    def wrapper(*args, **kwargs):
        key = (func, args, tuple(kwargs.items()))
        if key in _cache:
            return _cache[key]
        res = func(*args, **kwargs)
        _cache[key] = res
        return res

    wrapper._cache = _cache
    return wrapper


def cache_for_current_turn(func):
    """
    Cache a function value updated each turn.

    The cache is non-persistent through loading a game.
    Wraps only functions with hashable arguments.
    """
    _cache = {}

    @wraps(func)
    def wrapper(*args, **kwargs):
        key = (func, args, tuple(kwargs.items()))
        this_turn = fo.currentTurn()
        if key in _cache and _cache[key][0] == this_turn:
            return _cache[key][1]
        res = func(*args, **kwargs)
        _cache[key] = (this_turn, res)
        return res

    wrapper._cache = _cache
    return wrapper


def cache_by_turn_persistent(func):
    """
    Cache a function value by turn, persistent through loading a game.

    It will also provides a history that may be analysed.
    The cache is keyed by the original function name. It only wraps functions without arguments.

    As the result is stored in AIstate, its type must be trusted by the savegame_codec module.
    """

    @wraps(func)
    def wrapper():
        if get_aistate() is None:
            return func()
        else:
            cache = get_aistate().misc.setdefault("caches", {}).setdefault(func.__name__, {})
            this_turn = fo.currentTurn()
            return cache[this_turn] if this_turn in cache else cache.setdefault(this_turn, func())

    return wrapper
