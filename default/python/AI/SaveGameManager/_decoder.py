"""This module defines the decoding for the FreeOrion AI savegames.

The decoder is subclassed from the standard library json decoder and
uses its string parsing. However, the resulting objects will be interpreted
differently according to the encoding used in FreeOrion AI savegames.

The decoder will only load trusted classes as defined in _definitions.py,
if an unknown/untrusted object is encountered, it will raise a InvalidSaveGameException.

When classes are loaded, their __setstate__ method will be invoked if available or
the __dict__ content will be set directly. It is the responsiblity of the trusted classes
to provide a __setstate__ method to verify and possibly sanitize the content of the passed state.

Example usage:
    Importing this module directly is not advised.
    Use the exported functions when importing the package instead.
"""
import json

import EnumsAI
from freeorion_tools import profile

from _definitions import *


@profile
def load_savegame_string(string):
    import zlib
    string = zlib.decompress(string)
    return decode(string)


def decode(obj):
    return _FreeOrionAISaveGameDecoder().decode(obj)


class _FreeOrionAISaveGameDecoder(json.JSONDecoder):

    def __init__(self, **kwargs):
        super(_FreeOrionAISaveGameDecoder, self).__init__(strict=True,  # do not allow control characters
                                                          **kwargs)

    def decode(self, s, _w=None):
        # use the default JSONDecoder to parse the string into a dict
        # then interpret the dict content according to our encoding
        retval = super(_FreeOrionAISaveGameDecoder, self).decode(s)
        return self.__interpret(retval)

    def __interpret_dict(self, obj):
        # With our encoding, dicts with more than one entry must be a standard dict
        # because our custom encoded classes have only a single key which is its name
        # and its __dict__ content as value. In that case, interpret the dict content.
        if len(obj) != 1:
            return {self.__interpret(key): self.__interpret(value)
                    for key, value in obj.iteritems()}

        # check if this is an encoded class or a standard (1-entry) dictionary
        [(key, value)] = obj.items()
        if not key.startswith(CLASS_PREFIX):
            return {self.__interpret(key): self.__interpret(value)
                    for key, value in obj.iteritems()}

        # We now know this is an encoded class, check content for validity first.
        if not type(value) == dict:
            raise InvalidSaveGameException("Incorrect class encoding: Content not a dict.")
        parsed_content = self.__interpret_dict(value)

        # get the class if in list of trusted classes, otherwise do not load.
        full_name = key[len(CLASS_PREFIX):]
        cls = trusted_classes.get(full_name)
        if cls is None:
            raise InvalidSaveGameException("DANGER DANGER - %s not trusted" % full_name)

        # create a new instance without calling the actual __new__ or __init__ function of the class.
        new_instance = object.__new__(cls)

        # Set the content trying to use the __setstate__ method if defined
        # Otherwise, directly set the dict content.
        try:
            setstate = new_instance.__setstate__
        except AttributeError:
            if not type(parsed_content) == dict:
                raise InvalidSaveGameException("Could not set content for" % new_instance)
            new_instance.__dict__ = parsed_content
        else:
            # only call now to not catch exceptions in the setstate method
            setstate(parsed_content)
        return new_instance

    def __interpret(self, x):
        # primitive types do not have to be interpreted
        if x is None or isinstance(x, (int, float, bool)):
            return x

        # special handling for dicts as they could encode our classes
        if isinstance(x, dict):
            return self.__interpret_dict(x)

        # for standard containers, interpret each element
        if isinstance(x, (list, tuple, set)):
            return x.__class__(self.__interpret(element) for element in x)

        # encode a unicode str according to systems standard encoding
        if isinstance(x, unicode):
            x = x.encode('utf-8')

        # if it is a string, check if it encodes another data type
        if isinstance(x, str):

            # does it encode an integer?
            if x.startswith(INT_PREFIX):
                x = x[len(INT_PREFIX):]
                return int(x)

            # does it encode a float?
            if x.startswith(FLOAT_PREFIX):
                x = x[len(FLOAT_PREFIX):]
                return float(x)

            # does it encode a tuple?
            if x.startswith(TUPLE_PREFIX):
                content = x[len(TUPLE_PREFIX) + 1:-1]  # ignore surrounding parentheses
                content = _replace_quote_placeholders(content)
                result = self.decode(content)
                return tuple(result)

            # does it encode a set?
            if x.startswith(SET_PREFIX):
                content = x[len(SET_PREFIX) + 1:-1]  # ignore surrounding parentheses
                content = _replace_quote_placeholders(content)
                result = self.decode(content)
                return set(result)

            # does it encode an enum?
            if x.startswith(ENUM_PREFIX):
                full_name = x[len(ENUM_PREFIX):]
                partial_names = full_name.split('.')
                if not len(partial_names) == 2:
                    raise InvalidSaveGameException("Could not decode Enum %s" % x)
                enum_name = partial_names[0]
                enum = getattr(EnumsAI, enum_name, None)
                if enum is None:
                    raise InvalidSaveGameException("Could not find enum %s in EnumsAI" % enum_name)
                retval = getattr(enum, partial_names[1], None)
                if retval is None:
                    raise InvalidSaveGameException("Could not find enum value %s in EnumsAI" % full_name)
                return retval

            if x == TRUE:
                return True

            if x == FALSE:
                return False

            if x == NONE:
                return None

        # no special cases apply at this point, just return the value
        return x


def _replace_quote_placeholders(s):
    """Replace PLACEHOLDER with quotes if not nested within another encoded container"""
    n = 0  # counts nesting level (i.e. number of opened but not closed parentheses)
    start = 0  # starting point for string replacement
    for i in range(len(s)):
        if s[i] == '(':
            # if this is an outer opening parenthesis, then replace placeholder from last parenthesis to here
            if n == 0:
                s = s[:start] + s[start:i].replace(PLACEHOLDER, '"') + s[i:]
            n += 1
        elif s[i] == ')':
            n -= 1
            if n == 0:
                start = i
    s = s[:start] + s[start:].replace(PLACEHOLDER, '"')
    return s

