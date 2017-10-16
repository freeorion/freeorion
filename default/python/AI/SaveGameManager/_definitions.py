# a list of trusted classes - other classes will not be loaded
try:
    import AIFleetMission
    import fleet_orders
    import character.character_module
    import AIstate
    trusted_classes = {"%s.%s" % (cls.__module__, cls.__name__): cls for cls in [
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
    # unit test throws this at the moment during imports  TODO handle cleaner
    trusted_classes = {}

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


def assert_content(state, key, expected_type, may_be_none=True):
    if key not in state:
        raise InvalidSaveGameException("Expected key '%s' was not found in savegame state." % key)
    value = state[key]
    if not ((value is None and may_be_none) or type(value) is expected_type):
        raise InvalidSaveGameException("Expected type %s%s for key but got %s" % (
            expected_type, "or None" if may_be_none else "", type(value)))
