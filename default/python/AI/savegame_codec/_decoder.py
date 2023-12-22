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
import binascii
import json
from typing import Union

import EnumsAI
from AIstate import AIstate

from ._definitions import (
    ENUM_PREFIX,
    FALSE,
    FLOAT_PREFIX,
    INT_PREFIX,
    NONE,
    PLACEHOLDER,
    SET_PREFIX,
    TRUE,
    TUPLE_PREFIX,
    InvalidSaveGameException,
    trusted_classes,
)


class SaveDecompressException(Exception):
    """
    Exception class for troubles with decompressing save game string.
    """


def _starts_with_prefix(prefix: str, candidate: str) -> bool:
    return candidate.startswith(prefix)


def _extract_value(prefix: str, value: str):
    return value[len(prefix) :]


def _extract_collection(prefix: str, value: str):
    return value[len(prefix) + 1 : -1]


def load_savegame_string(string: Union[str, bytes]) -> AIstate:
    """
    :raises: SaveDecompressException, InvalidSaveGameException
    """
    import base64
    import zlib

    try:
        new_string = base64.b64decode(string)
    except (binascii.Error, ValueError, TypeError) as e:
        raise SaveDecompressException("Fail to decode base64 savestate %s" % e) from e
    try:
        new_string = zlib.decompress(new_string)
    except zlib.error as e:
        raise SaveDecompressException("Fail to decompress savestate %s" % e) from e
    return decode(new_string.decode("utf-8"))


def decode(obj):
    return _FreeOrionAISaveGameDecoder().decode(obj)


class _FreeOrionAISaveGameDecoder(json.JSONDecoder):
    def __init__(self, **kwargs):
        # do not allow control characters
        super().__init__(strict=True, **kwargs)

    def decode(self, s, _w=None):
        # use the default JSONDecoder to parse the string into a dict
        # then interpret the dict content according to our encoding
        retval = super().decode(s)
        return self.__interpret(retval)

    def __interpret_dict(self, obj):
        # if the dict does not contain the class-encoding keys,
        # then it is a standard dictionary.
        if not all(key in obj for key in ("__class__", "__module__")):
            return {self.__interpret(key): self.__interpret(value) for key, value in obj.items()}

        # pop and verify class and module name, then parse the class content
        class_name = obj.pop("__class__")
        module_name = obj.pop("__module__")
        full_name = f"{module_name}.{class_name}"
        cls = trusted_classes.get(full_name)
        if cls is None:
            raise InvalidSaveGameException("DANGER DANGER - %s not trusted" % full_name)

        parsed_content = self.__interpret_dict(obj)

        # create a new instance without calling the actual __new__or __init__
        # function of the class (so we avoid any side-effects from those)
        new_instance = object.__new__(cls)

        # Set the content trying to use the __setstate__ method if defined
        # Otherwise, directly set the dict content.
        try:
            setstate = new_instance.__setstate__
        except AttributeError:
            if not type(parsed_content) == dict:  # noqa: E721
                raise InvalidSaveGameException("Could not set content for %s" % new_instance)
            new_instance.__dict__ = parsed_content
        else:
            # only call now to not catch exceptions in the setstate method
            setstate(parsed_content)
        return new_instance

    def __interpret(self, x):  # noqa: C901
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

        # if it is a string, check if it encodes another data type
        if isinstance(x, str):
            # does it encode an integer?
            if _starts_with_prefix(INT_PREFIX, x):
                x = _extract_value(INT_PREFIX, x)
                return int(x)

            # does it encode a float?
            if _starts_with_prefix(FLOAT_PREFIX, x):
                x = _extract_value(FLOAT_PREFIX, x)
                return float(x)

            # does it encode a tuple?
            if _starts_with_prefix(TUPLE_PREFIX, x):
                # ignore surrounding parentheses
                content = _extract_collection(TUPLE_PREFIX, x)
                content = _replace_quote_placeholders(content)
                result = self.decode(content)
                return tuple(result)

            # does it encode a set?
            if _starts_with_prefix(SET_PREFIX, x):
                # ignore surrounding parentheses
                content = _extract_collection(SET_PREFIX, x)
                content = _replace_quote_placeholders(content)
                result = self.decode(content)
                return set(result)

            # does it encode an enum?
            if _starts_with_prefix(ENUM_PREFIX, x):
                full_name = _extract_value(ENUM_PREFIX, x)
                partial_names = full_name.split(".")
                if not len(partial_names) == 2:
                    raise InvalidSaveGameException("Could not decode Enum %s" % x)
                enum_name = partial_names[0]
                enum = getattr(EnumsAI, enum_name, None)
                if enum is None:
                    raise InvalidSaveGameException("Invalid enum %s" % enum_name)
                retval = getattr(enum, partial_names[1], None)
                if retval is None:
                    raise InvalidSaveGameException("Invalid enum value %s" % full_name)
                return retval

            if _starts_with_prefix(TRUE, x):
                return True

            if _starts_with_prefix(FALSE, x):
                return False

            if _starts_with_prefix(NONE, x):
                return None

            # no special cases apply at this point, should be a standard string
            return x

        raise TypeError(f"Unexpected type {type(x)} ({x})")


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
        if s[i] == "(":
            # if this is an outer opening parenthesis, then replace placeholder from last parenthesis to here
            if n == 0:
                s = s[:start] + s[start:i].replace(PLACEHOLDER, '"') + s[i:]
            n += 1
        elif s[i] == ")":
            n -= 1
            if n == 0:
                start = i
    s = s[:start] + s[start:].replace(PLACEHOLDER, '"')
    return s
