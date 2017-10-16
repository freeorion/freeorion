"""This modules defines a json-based encoder to represent (complex) python objects in a single string.

For savegames in FreeOrion, the singleton AIstate instance will be encoded and the returned string
sent to the server to include in the savegame file. On reload, that string is used to reconstruct
the AIstate instance with all its information.

The following types are currently supported for encoding/decoding:
    * bool, int, float, string, None
    * list, set, tuple, dict
    * instances of trusted new-style __dict__-based classes

Additional effort is taken to encode even primitive types which are supported by json.
This is because json-decoding does not support non-string dictionary types. For example,
integer keys will be converted to strings which is not acceptable for our use.

For example,
json.loads(json.dumps({1: 2})) => {"1": 2}


Classes are trusted based on their module. Non-recognized modules will not be imported.
"""

import json

import EnumsAI
from freeorion_tools import profile

# a list of trusted modules - classes from other modules will not be loaded
try:
    import AIFleetMission
    import fleet_orders
    import character.character_module
    import AIstate
    _trusted_classes = {"%s.%s" % (cls.__module__, cls.__name__): cls for cls in [
        AIFleetMission.AIFleetMission,
        fleet_orders.AIFleetOrder,
        fleet_orders.OrderMilitary,
        fleet_orders.OrderAttack,
        fleet_orders.OrderDefend,
        fleet_orders.OrderColonize,
        fleet_orders.OrderOutpost,
        fleet_orders.OrderInvade,
        fleet_orders.OrderMove,
        fleet_orders.OrderRepair,
        fleet_orders.OrderResupply,
        fleet_orders.OrderSplitFleet,
        character.character_module.Trait,
        character.character_module.Aggression,
        character.character_module.EmpireIDTrait,
        character.character_module.Character,
        AIstate.AIstate,
    ]}
except RuntimeError:
    # unit test throws this at the moment  TODO handle cleaner
    _trusted_classes = {}

# prefixes to encode types not supported by json
# or not fully supported as dictionary key
ENUM_PREFIX = '__ENUM__'
CLASS_PREFIX = '__foAI__'
INT_PREFIX = '__INT__'
FLOAT_PREFIX = '__FLOAT__'
TRUE = '__TRUE__'
FALSE = '__FALSE__'
NONE = '__NONE__'
SET_PREFIX = '__SET__'
TUPLE_PREFIX = '__TUPLE__'


# placeholder char to represent quotes in nested containers
# which would break json decoding if present.
PLACEHOLDER = '$'


class CanNotSaveGameException(Exception):
    """Exception raised when constructing the savegame string failed."""
    pass


class InvalidSaveGameException(Exception):
    """Exception raised if the savegame could not be loaded."""
    pass


class FreeOrionAISaveGameEncoder(object):
    """Encoder support for some classes not supported by json."""

    def encode(self, o):
        """Encode the passed object as json-based string.

        :param o: object to be encoded
        :return: String representation of the object state
        :rtype: str
        """
        if isinstance(o, basestring):
            pass
        elif o is None:
            o = NONE
        elif o is True:
            o = TRUE
        elif o is False:
            o = FALSE
        elif isinstance(o, EnumsAI.EnumItem):
            o = "%s%s" % (ENUM_PREFIX, str(o))
        elif isinstance(o, (int, long)):
            o = "%s%d" % (INT_PREFIX, o)
        elif isinstance(o, float):
            o = "%s%s" % (FLOAT_PREFIX, repr(o))
        elif isinstance(o, dict):
            return self._dict_encoder(o)
        elif isinstance(o, list):
            return self._list_encoder(o)
        elif isinstance(o, tuple):
            o = self._tuple_encoder(o)
        elif isinstance(o, set):
            o = self._set_encoder(o)
        else:
            return self._object_encoder(o)
        # add quotes around the object if not a dict or list
        # - this allows parsing with the default JSON decoder
        return "\"%s\"" % o

    def _object_encoder(self, obj):
        """Get a string representation of state of an object which is not handled by a specialized encoder."""
        try:
            class_name = "%s.%s" % (obj.__class__.__module__, obj.__class__.__name__)
            if class_name not in _trusted_classes:
                raise CanNotSaveGameException("Class %s is not trusted" % class_name)
        except AttributeError:
            # obj does not have a class or class has no module
            raise CanNotSaveGameException("Encountered unsupported object %s (%s)" % (obj, type(obj)))

        # results in, e.g. __foAI__AIstate.AIstate
        key = "%s%s.%s" % (CLASS_PREFIX, obj.__class__.__module__, obj.__class__.__name__)

        # if possible, use getstate method to query the state, otherwise use the object's __dict__
        try:
            getstate = obj.__getstate__
        except AttributeError:
            value = obj.__dict__
        else:
            # only call now to avoid catching exceptions raised during the getstate call
            value = getstate()
        return self._dict_encoder({key: value})

    def _list_encoder(self, o):
        """Get a string representation of a list with its encoded content."""
        return "[%s]" % (', '.join([self.encode(v) for v in o]))

    def _tuple_encoder(self, o):
        """Get a string representation of a tuple with its encoded content."""
        retval = "%s(%s)" % (TUPLE_PREFIX, self._list_encoder(list(o)))
        return retval.replace('\"', PLACEHOLDER)

    def _set_encoder(self, o):
        """Get a string representation of a set with its encoded content."""
        retval = "%s(%s)" % (SET_PREFIX, self._list_encoder(list(o)))
        return retval.replace('\"', PLACEHOLDER)

    def _dict_encoder(self, o):
        """Get a string representation of a dict with its encoded content."""
        return "{%s}" % (', '.join(['%s: %s' % (self.encode(k), self.encode(v)) for k, v in o.items()]))


class FreeOrionAISaveGameDecoder(json.JSONDecoder):

    def __init__(self, **kwargs):
        super(FreeOrionAISaveGameDecoder, self).__init__(strict=True,  # do not allow control characters
                                                         **kwargs)

    def decode(self, s, _w=None):
        # use the default JSONDecoder to parse the string into a dict
        # then interpret the dict content according to our encoding
        retval = super(FreeOrionAISaveGameDecoder, self).decode(s)
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
        cls = _trusted_classes.get(full_name)
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


def encode(obj):
    return FreeOrionAISaveGameEncoder().encode(obj)


def decode(obj):
    return FreeOrionAISaveGameDecoder().decode(obj)


@profile
def build_savegame_string():
    import zlib
    import FreeOrionAI as foAI
    return zlib.compress(encode(foAI.foAIstate))


@profile
def load_savegame_string(string):
    import zlib
    string = zlib.decompress(string)
    return decode(string)


def _replace_quote_placeholders(s):
    """Replace PLACEHOLDER with quotes if not nested within another encoded container"""
    n = 0  # counts nesting level (i.e. number of opened but not closed parentheses)
    start = 0  # starting point for string replacement
    for i in range(len(s)):
        if s[i] == '(':
            # if this is an outer opening parenthesis, then replace placeholder from last parenthesis to here
            if n == 0:
                s = s[:start] + s[start:i].replace(PLACEHOLDER, '\"') + s[i:]
            n += 1
        elif s[i] == ')':
            n -= 1
            if n == 0:
                start = i
    s = s[:start] + s[start:].replace(PLACEHOLDER, '\"')
    return s


def assert_content(state, key, expected_type, may_be_none=True):
    if key not in state:
        raise InvalidSaveGameException("Expected key '%s' was not found in savegame state." % key)
    value = state[key]
    if not ((value is None and may_be_none) or type(value) is expected_type):
        raise InvalidSaveGameException("Expected type %s%s for key but got %s" % (
            expected_type, "or None" if may_be_none else "", type(value)))
