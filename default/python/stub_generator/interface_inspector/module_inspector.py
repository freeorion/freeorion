from collections import defaultdict
from enum import Enum
from logging import error, warning
from typing import Any, List, Tuple

from stub_generator.constants import ATTRS, CLASS_NAME, PARENTS
from stub_generator.interface_inspector.class_processor import ClassInfo, inspect_class
from stub_generator.interface_inspector.enum_processor import EnumInfo, inspect_enum
from stub_generator.interface_inspector.function_processor import FunctionInfo, inspect_function
from stub_generator.interface_inspector.inspection_helpers import _get_member_info, _getmembers


def _inspect_instance(instance, location):
    parents = instance.__class__.mro()[1:-2]
    parent_attrs = sum((dir(parent) for parent in instance.__class__.mro()[1:]), [])

    info = {
        CLASS_NAME: instance.__class__.__name__,
        ATTRS: {},
        PARENTS: [str(parent.__name__) for parent in parents],
        "location": location,
    }
    for attr_name, member in _getmembers(instance):
        if attr_name not in parent_attrs + ['__module__']:
            info[ATTRS][attr_name] = _get_member_info('%s.%s' % (instance.__class__.__name__, attr_name), member)
    return info


class MemberType(Enum):
    ENUM = 1
    CLASS = 2
    FUNCTION = 3
    UNKNOWN = -1


def get_type(member):
    type_ = str(type(member))
    if type_ == "<class 'type'>":
        return MemberType.ENUM
    elif type_ == "<class 'Boost.Python.class'>":
        return MemberType.CLASS
    elif type_ == "<class 'Boost.Python.function'>":
        return MemberType.FUNCTION
    else:
        return MemberType.UNKNOWN


_OBJECT_HANDLERS = {
    MemberType.ENUM: inspect_enum,
    MemberType.CLASS: inspect_class,
    MemberType.FUNCTION: inspect_function,
}

_NAMES_TO_IGNORE = {'__doc__', '__package__', '__name__', 'INVALID_GAME_TURN', 'to_str', '__loader__', '__spec__'}
_INVALID_CLASSES = {"method"}


def is_built_in(instance: Any) -> bool:
    """
    Return is instance is one of built in type.

    Inherited classes from built in return False.
    """
    built_in_types = {str, float, int, tuple, tuple, list, dict, set}
    return type(instance) in built_in_types


def get_module_members(obj):
    module_members = defaultdict(list)

    for name, member in _getmembers(obj):
        if name in _NAMES_TO_IGNORE:
            continue
        type_key = get_type(member)
        if type_key == MemberType.UNKNOWN:
            warning("Unknown: '%s' of type '%s': %s" % (name, type(member), member))
        else:
            type_inspector = _OBJECT_HANDLERS[type_key]
            module_members[type_key].append(type_inspector(name, member))
    return module_members


def inspect_instances(instances):
    for location, instance in instances:
        if is_built_in(instance):
            warning("Argument at %s is builtin python instance: (%s) %s", location, type(instance), instance)
            continue
        if instance.__class__.__name__ in _INVALID_CLASSES:
            warning("Argument at %s is not allowed: (%s) %s", location, type(instance),
                    instance)
            continue
        try:
            yield _inspect_instance(instance, location)
        except Exception as e:
            error("Error inspecting: '%s' with '%s': %s", type(instance), type(e), e, exc_info=True)


def get_module_info(obj, instances) -> Tuple[List[ClassInfo], List[EnumInfo], List[FunctionInfo], Any]:
    module_members = get_module_members(obj)
    return (
        module_members[MemberType.CLASS],
        module_members[MemberType.ENUM],
        module_members[MemberType.FUNCTION],
        list(inspect_instances(instances))
    )
