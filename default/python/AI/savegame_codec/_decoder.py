"""This module defines the decoding for the FreeOrion AI savegames.

The decoder is subclassed from the standard library json decoder and
uses its string parsing. However, the resulting objects will be interpreted
differently according to the encoding used in FreeOrion AI savegames.

The decoder will only load trusted classes as defined in _definitions.py,
if an unknown/untrusted object is encountered, it will raise a InvalidSaveGameException.

When classes are loaded, their __setstate__ method will be invoked if available or
the __dict__ content will be set directly. It is the responsiblity of the trusted classes
to provide a __setstate__ method to verify and possibly sanitize the content of the passed state.
"""
import json

import EnumsAI
from freeorion_tools import profile
from logging import debug

from _definitions import (ENUM_PREFIX, FALSE, FLOAT_PREFIX, INT_PREFIX, InvalidSaveGameException, NONE, PLACEHOLDER,
                          SET_PREFIX, TRUE, TUPLE_PREFIX, trusted_classes, )


@profile
def load_savegame_string(string):
    """
    :rtype: AIstate
    """
    import base64
    import zlib

    new_string = string
    try:
        new_string = base64.b64decode(string)
    except TypeError as e:
        # The base64 module docs only mention a TypeError exception, for wrong padding
        # Older save files won't be base64 encoded, but seemingly that doesn't trigger
        # an exception here;
        debug("When trying to base64 decode savestate got exception: %s" % e)
    try:
        new_string = zlib.decompress(new_string)
    except zlib.error:
        pass  # probably an uncompressed (or wrongly base64 decompressed) string
    try:
        decoded_state = decode(new_string)
        debug("Decoded a zlib-compressed and apparently base64-encoded save-state string.")
        return decoded_state
    except (InvalidSaveGameException, ValueError, TypeError) as e:
        debug("Base64/zlib decoding path for savestate failed: %s" % e)

    try:
        string = zlib.decompress(string)
        debug("zlib-decompressed a non-base64-encoded save-state string.")
    except zlib.error:
        # probably an uncompressed string
        debug("Will try decoding savestate string without base64 or zlib compression.")
    return decode(string)


def decode(obj):
    return _FreeOrionAISaveGameDecoder().decode(obj)


class _FreeOrionAISaveGameDecoder(json.JSONDecoder):

    def __init__(self, **kwargs):
        # do not allow control characters
        super(_FreeOrionAISaveGameDecoder, self).__init__(strict=True, **kwargs)

    def decode(self, s, _w=None):
        # use the default JSONDecoder to parse the string into a dict
        # then interpret the dict content according to our encoding
        retval = super(_FreeOrionAISaveGameDecoder, self).decode(s)
        return self.__interpret(retval)

    def __interpret_dict(self, obj):
        # if the dict does not contain the class-encoding keys,
        # then it is a standard dictionary.
        if not all(key in obj for key in ('__class__', '__module__')):
            return {self.__interpret(key): self.__interpret(value)
                    for key, value in obj.iteritems()}

        # pop and verify class and module name, then parse the class content
        class_name = obj.pop('__class__')
        module_name = obj.pop('__module__')
        full_name = '%s.%s' % (module_name, class_name)
        cls = trusted_classes.get(full_name)
        if cls is None:
            raise InvalidSaveGameException("DANGER DANGER - %s not trusted"
                                           % full_name)

        parsed_content = self.__interpret_dict(obj)

        # create a new instance without calling the actual __new__or __init__
        # function of the class (so we avoid any side-effects from those)
        new_instance = object.__new__(cls)

        # Set the content trying to use the __setstate__ method if defined
        # Otherwise, directly set the dict content.
        try:
            setstate = new_instance.__setstate__
        except AttributeError:
            if not type(parsed_content) == dict:
                raise InvalidSaveGameException("Could not set content for %s"
                                               % new_instance)
            new_instance.__dict__ = parsed_content
        else:
            # only call now to not catch exceptions in the setstate method
            setstate(parsed_content)
        return new_instance

    def __interpret(self, x):
        """Interpret an object that was just decoded."""
        # primitive types do not have to be interpreted
        if isinstance(x, (int, float)):
            return x

        # special handling for dicts as they could encode our classes
        if isinstance(x, dict):
            return self.__interpret_dict(x)

        # for standard containers, interpret each element
        if isinstance(x, list):
            return list(self.__interpret(element) for element in x)

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
                # ignore surrounding parentheses
                content = x[len(TUPLE_PREFIX) + 1:-1]
                content = _replace_quote_placeholders(content)
                result = self.decode(content)
                return tuple(result)

            # does it encode a set?
            if x.startswith(SET_PREFIX):
                # ignore surrounding parentheses
                content = x[len(SET_PREFIX) + 1:-1]
                content = _replace_quote_placeholders(content)
                result = self.decode(content)
                return set(result)

            # does it encode an enum?
            if x.startswith(ENUM_PREFIX):
                full_name = x[len(ENUM_PREFIX):]
                partial_names = full_name.split('.')
                if not len(partial_names) == 2:
                    raise InvalidSaveGameException("Could not decode Enum %s"
                                                   % x)
                enum_name = partial_names[0]
                enum = getattr(EnumsAI, enum_name, None)
                if enum is None:
                    raise InvalidSaveGameException("Invalid enum %s"
                                                   % enum_name)
                retval = getattr(enum, partial_names[1], None)
                if retval is None:
                    raise InvalidSaveGameException("Invalid enum value %s"
                                                   % full_name)
                return retval

            if x == TRUE:
                return True

            if x == FALSE:
                return False

            if x == NONE:
                return None

            # no special cases apply at this point, should be a standard string
            return x

        raise TypeError("Unexpected type %s (%s)" % (type(x), x))


def _replace_quote_placeholders(s):
    """Replace PLACEHOLDER with quotes if not nested within another encoded container.

    To be able to use tuples as dictionary keys, to use standard json decoder,
    the entire tuple with its content must be encoded as a single string.
    The inner objects may no longer be quoted as that would prematurely terminate
    the strings. Inner quotes are therefore replaced with the PLACEHOLDER char.

    Example:
        output = encode(tuple(["1", "string"]))
        "__TUPLE__([$1$, $string$])"

    To be able to decode the inner content, the PLACEHOLDER must be converted
    to quotes again.
    """
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
