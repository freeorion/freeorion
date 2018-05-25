"""This module defines the encoding for the FreeOrion AI savegames.

The encoding is json-based with custom prefixes to support some objects
and dictionary keys of types which are not supported in standard json.

This module encodes the integer as string with prefix so that the information
about its integer key will be kept throughout json decoding.

The encoding implementation is recursive
    1) Identify the type of object to encode and call the correct encoder
    2) If it is a non-trivial type, first encode all its content
    3) Finally, encode the object itself

For class instances, the __getstate__ method is invoked to get its content.
If not defined, its __dict__ will be encoded instead.

If an object could not be encoded, raise a CanNotSaveGameException.
"""
import collections

import EnumsAI
from freeorion_tools import profile

from _definitions import (CanNotSaveGameException, ENUM_PREFIX, FALSE, FLOAT_PREFIX, INT_PREFIX, NONE, PLACEHOLDER,
                          SET_PREFIX, TRUE, TUPLE_PREFIX, trusted_classes, )


@profile
def build_savegame_string(use_compression=True):
    """Encode the AIstate and compress the resulting string with zlib.

    To decode the string, first call zlib.decompress() on it.

    :return: compressed savegame string
    :rtype: str
    """
    from aistate_interface import get_aistate
    savegame_string = encode(get_aistate())
    if use_compression:
        import base64
        import zlib
        savegame_string = base64.b64encode(zlib.compress(savegame_string))
    return savegame_string


def encode(o):
    """Encode the passed object as json-based string.

    :param o: object to be encoded
    :return: String representation of the object state
    :rtype: str
    """
    # Find and call the correct encoder based
    # on the type of the object to encode
    o_type = type(o)
    try:
        encoder = _encoder_table[o_type]
    except KeyError:
        if issubclass(o_type, EnumsAI.EnumItem):
            return '"%s%s"' % (ENUM_PREFIX, str(o))
        else:
            return _encode_object(o)
    else:
        # only call now to not catch KeyError withing the encoder call
        return encoder(o)


def _encode_str(o):
    return '"%s"' % o


def _encode_none(o):
    return '"%s"' % NONE


def _encode_int(o):
    return '"%s%s"' % (INT_PREFIX, str(o))


def _encode_float(o):
    return '"%s%s"' % (FLOAT_PREFIX, repr(o))


def _encode_bool(o):
    return '"%s"' % (TRUE if o else FALSE)


def _encode_object(obj):
    """Get a string representation of state of an object which is not handled by a specialized encoder."""
    try:
        class_name = "%s.%s" % (obj.__class__.__module__, obj.__class__.__name__)
        if class_name not in trusted_classes:
            raise CanNotSaveGameException("Class %s is not trusted" % class_name)
    except AttributeError:
        # obj does not have a class or class has no module
        raise CanNotSaveGameException("Encountered unsupported object %s (%s)" % (obj, type(obj)))

    # if possible, use getstate method to query the state, otherwise use the object's __dict__
    try:
        getstate = obj.__getstate__
    except AttributeError:
        value = obj.__dict__
    else:
        # only call now to avoid catching exceptions raised during the getstate call
        value = getstate()

    # encode information about class
    value.update({'__class__': obj.__class__.__name__,
                  '__module__': obj.__class__.__module__})
    return _encode_dict(value)


def _encode_list(o):
    """Get a string representation of a list with its encoded content."""
    return "[%s]" % (', '.join([encode(v) for v in o]))


def _encode_tuple(o):
    """Get a string representation of a tuple with its encoded content."""
    return '"%s(%s)"' % (TUPLE_PREFIX, _encode_list(list(o)).replace('"', PLACEHOLDER))


def _encode_set(o):
    """Get a string representation of a set with its encoded content."""
    return '"%s(%s)"' % (SET_PREFIX, _encode_list(list(o)).replace('"', PLACEHOLDER))


def _encode_dict(o):
    """Get a string representation of a dict with its encoded content."""
    return "{%s}" % (', '.join(['%s: %s' % (encode(k), encode(v)) for k, v in o.iteritems()]))


_encoder_table = {
    str: _encode_str,
    unicode: _encode_str,
    bool: _encode_bool,
    int: _encode_int,
    float: _encode_float,
    dict: _encode_dict,
    collections.OrderedDict: _encode_dict,
    list: _encode_list,
    set: _encode_set,
    tuple: _encode_tuple,
    type(None): _encode_none,
}
