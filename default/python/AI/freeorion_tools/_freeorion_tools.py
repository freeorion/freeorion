# This Python file uses the following encoding: utf-8
import StringIO
import cProfile
import logging
import pstats
import re
import traceback
from collections import Mapping
from functools import wraps
from logging import debug, error

import freeOrionAIInterface as fo  # pylint: disable=import-error

# color wrappers for chat:
RED = '<rgba 255 0 0 255>%s</rgba>'
WHITE = '<rgba 255 255 255 255>%s</rgba>'


def dict_from_map(thismap):
    """Convert C++ map to python dict."""
    return {el.key(): el.data() for el in thismap}


def get_ai_tag_grade(tag_list, tag_type):
    """
    Accepts a list of string tags and a tag_type (like 'WEAPONS').
    Checks for the first tag in the list (if any), for tag_type "TYPE",
    having the structure X_TYPE
    and then returns 'X'
    X is most commonly (but not necessarily) one of [NO, BAD, AVERAGE, GOOD, GREAT, ULTIMATE]
    If no matching tags, returns empty string (which for most types should be considered equivalent to AVERAGE)
    """
    for tag in [tag for tag in tag_list if tag.count("_") > 0]:
        parts = tag.split("_", 1)
        if parts[1] == tag_type.upper():
            return parts[0]
    return ""


# this name left with C naming style for compatibility with translation assistance procedures
def UserString(label, default=None):  # pylint: disable=invalid-name
    """
    A translation assistance tool is intended to search for this method to identify translatable strings.

    :param label: a UserString key
    :param default: a default value to return if there is a key error
    :return: a translated string for the label
    """

    table_string = fo.userString(label)

    if "ERROR: " + label in table_string:  # implement test for string lookup not found error
        return default or table_string
    else:
        return table_string


# this name left with C naming style for compatibility with translation assistance procedures
def UserStringList(label):  # pylint: disable=invalid-name
    """
    A translation assistance tool is intended to search for this method to identify translatable strings.

    :param label: a UserString key
    :return: a python list of translated strings from the UserString list identified by the label
    """

    return fo.userStringList(label)


def tech_is_complete(tech):
    """
    Return if tech is complete.
    """
    return fo.getEmpire().techResearched(tech)


def ppstring(foo):
    """
    Returns a string version of lists, dicts, sets, such that entries with special characters will be
    printed in legible string format rather than as hex escape characters, i.e.,
    ['Asimov α'] rather than ['Asimov \xce\xb1']."""

    if isinstance(foo, list):
        return "[" + ",".join(map(ppstring, foo)) + "]"
    elif isinstance(foo, dict):
        return "{" + ",".join([ppstring(k) + ":" + ppstring(v) for k, v in foo.iteritems()]) + "}"
    elif isinstance(foo, tuple):
        return "(" + ",".join(map(ppstring, foo)) + ")"
    elif isinstance(foo, set) or isinstance(foo, frozenset):
        return "{" + ",".join(map(ppstring, foo)) + "}"
    elif isinstance(foo, str):
        return "'" + foo + "'"
    else:
        return str(foo)


class ConsoleLogHandler(logging.Handler):
    """A log handler to send errors to the console. """
    def emit(self, record):
        """Emit a record.

        If a formatter is specified, it is used to format the record and then sent to human players. """
        try:
            human_ids = [x for x in fo.allPlayerIDs() if fo.playerIsHost(x)]
            if not human_ids:
                return
            msg = self.format(record)

            for human_id in human_ids:
                fo.sendChatMessage(human_id, msg)
        except (KeyboardInterrupt, SystemExit):
            raise
        # Hide errors from within the ConsoleLogHandler
        except:
            self.handleError(record)


# Create the log handler, format it and attach it to the root logger
console_handler = ConsoleLogHandler()

console_handler.setFormatter(
    logging.Formatter(RED % ('%s : %%(filename)s:%%(funcName)s():%%(lineno)d  - %%(message)s'
                             % fo.userString('AI_ERROR_MSG'))))

console_handler.setLevel(logging.ERROR)

logging.getLogger().addHandler(console_handler)


def remove_tags(message):
    """Remove tags described in Font.h from message."""
    expr = '</?(i|u|(rgba ([0-1]\.)?\d+ ([0-1]\.)?\d+ ([0-1]\.)?\d+ ([0-1]\.)?\d+)|rgba|left|center|right|pre)>'
    return re.sub(expr, '', message)


def chat_human(message):
    """
    Send chat message to human and print it to log.
    Log message cleared form tags.
    """
    human_id = [x for x in fo.allPlayerIDs() if fo.playerIsHost(x)][0]
    message = str(message)
    fo.sendChatMessage(human_id, message)
    debug("Chat Message to human: %s", remove_tags(message))


def cache_by_session(func):
    """
    Cache a function value by session.
    Wraps only functions with hashable arguments.
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


def cache_by_session_with_turnwise_update(func):
    """
    Cache a function value during session, updated each turn.
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


def cache_by_turn(func):
    """
    Cache a function value by turn, stored in foAIstate so also provides a history that may be analysed. The cache
    is keyed by the original function name.  Wraps only functions without arguments.
    Cache result is stored in savegame, will crash with picle error if result contains any boost object.
    """
    # avoid circular import
    from aistate_interface import get_aistate

    @wraps(func)
    def wrapper():
        if get_aistate() is None:
            return func()
        else:
            cache = get_aistate().misc.setdefault('caches', {}).setdefault(func.__name__, {})
            this_turn = fo.currentTurn()
            return cache[this_turn] if this_turn in cache else cache.setdefault(this_turn, func())
    return wrapper


def dict_to_tuple(dic):
    return tuple(dic.iteritems())


def tuple_to_dict(tup):
    try:
        return dict(tup)
    except TypeError:
        try:
            return {k: v for k, v in [tup]}
        except:
            error("Can't convert tuple_list to dict: %s", tup)
            return {}


def profile(func):

    @wraps(func)
    def wrapper(*args, **kwargs):
        pr = cProfile.Profile()
        pr.enable()
        retval = func(*args, **kwargs)
        pr.disable()
        s = StringIO.StringIO()
        sortby = 'cumulative'
        ps = pstats.Stats(pr, stream=s).sort_stats(sortby)
        ps.print_stats()
        debug(s.getvalue())
        return retval

    return wrapper


def get_partial_visibility_turn(obj_id):
    """Return the last turn an object had at least partial visibility.

    :type obj_id: int
    :return: Last turn an object had at least partial visibility, -9999 if never
    :rtype: int
    """
    visibility_turns_map = fo.getUniverse().getVisibilityTurnsMap(obj_id, fo.empireID())
    return visibility_turns_map.get(fo.visibility.partial, -9999)


class ReadOnlyDict(Mapping):
    """A dict that offers only read access.

     Note that if the values of the ReadOnlyDict are mutable,
     then those objects may actually be changed.

     It is strongly advised to store only immutable objects.
     A slight protection is offered by checking for hashability of the values.

      Example usage:
      my_dict = ReadOnlyDict({1:2, 3:4})
      print my_dict[1]
      for k in my_dict:
          print my_dict.get(k, -1)
      for k in my_dict.keys():
          print my_dict[k]
      for k, v in my_dict.iteritems():
          print k, v
      my_dict[5] = 4  # throws TypeError
      del my_dict[1]  # throws TypeError
     """

    def __init__(self, *args, **kwargs):
        self._data = dict(*args, **kwargs)
        for k, v in self._data.iteritems():
            try:
                hash(v)
            except TypeError:
                error("Tried to store a non-hashable value in ReadOnlyDict")
                raise

    def __getitem__(self, item):
        return self._data[item]

    def __iter__(self):
        return iter(self._data)

    def __len__(self):
        return len(self._data)

    def __str__(self):
        return str(self._data)


def dump_universe():
    """Dump the universe but not more than once per turn."""
    cur_turn = fo.currentTurn()

    if (not hasattr(dump_universe, "last_dump") or
            dump_universe.last_dump < cur_turn):
        dump_universe.last_dump = cur_turn
        fo.getUniverse().dump()  # goes to debug logger


class LogLevelSwitcher(object):
    """A context manager class which controls the log level within its scope.

    Example usage:
    logging.getLogger().setLevel(logging.INFO)

    debug("Some message")  # not printed because of log level
    with LogLevelSwitcher(logging.DEBUG):
        debug("foo")  # printed because we set to DEBUG level

    debug("baz")  # not printed, we are back to INFO level
    """
    def __init__(self, log_level):
        self.target_log_level = log_level
        self.old_log_level = 0

    def __enter__(self):
        self.old_log_level = logging.getLogger().level
        logging.getLogger().setLevel(self.target_log_level)

    def __exit__(self, exc_type, exc_val, exc_tb):
        logging.getLogger().setLevel(self.old_log_level)


def with_log_level(log_level):
    """A decorator to set a specific logging level for the function call.

    This decorator is useful to selectively activate debugging for a specific function
    while the rest of the code base only logs at a higher level.

    If functions are called within the decorated function, then those will be
    executed with the same logging level. However, if those functions use this
    decorator as well, then that logging level will be respected for its scope.

    Example usage:
    @log_with_specific_log_level(logging.DEBUG)
    def foo():
        debug("debug stuff")
    """
    def decorator(func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            with LogLevelSwitcher(log_level):
                return func(*args, **kwargs)

        return wrapper
    return decorator


def assertion_fails(cond, msg="", logger=logging.error):
    """
    Check if condition fails and if so, log a traceback but raise no Exception.

    This is a useful functions for generic sanity checks and may be used to
    replace manual error logging with more context provided by the traceback.

    :param bool cond: condition to be asserted
    :param str msg: additional info to be logged
    :param func logger: may be used to override default log level (error)
    :return: True if assertion failed, otherwise false.
    """
    if cond:
        return False

    if msg:
        header = "Assertion failed: %s." % msg
    else:
        header = "Assertion failed!"
    stack = traceback.extract_stack()[:-1]  # do not log this function
    logger("%s Traceback (most recent call last): %s", header,
           ''.join(traceback.format_list(stack)))
    return True
