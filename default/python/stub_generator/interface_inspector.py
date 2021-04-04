from collections import defaultdict
from enum import Enum
from inspect import getdoc, isroutine
from logging import error, warning
from typing import Any

from stub_generator.constants import ATTRS, CLASS_NAME, DOC, ENUM_PAIRS, NAME, PARENTS


def _get_member_info(name, member):
    type_ = str(type(member))

    info = {
        'type': type_,
    }
    if isinstance(member, property):
        info['getter'] = getdoc(member.fget)
    elif isroutine(member):
        info['routine'] = (member.__name__, getdoc(member))
    elif isinstance(member, (int, str, float, list, tuple, dict, set, frozenset)):
        pass  # we don't need any
    elif 'freeOrionAIInterface' in type_ or "freeorion" in type_:
        pass  # TODO we got some instance here, probably we should inspect it too.
    else:
        # instance properties will be already resolved
        warning('[%s] Unexpected member "%s"(%s)', name, type(member), member)
    return info


def _getmembers(obj, predicate=None):
    """Return all members of an object as (name, value) pairs sorted by name.
    Optionally, only return members that satisfy a given predicate."""
    results = []
    for key in dir(obj):
        try:
            value = getattr(obj, key)
        except AttributeError:
            continue
        except Exception as e:
            message = [
                "-" * 20,
                'Error in "%s.%s" with error' % (obj.__class__.__name__, key),
                "..." * 20,
                "Error info:",
                str(e),
                "..." * 20,
            ]
            error('\n'.join(message))
            continue
        if not predicate or predicate(value):
            results.append((key, value))
    results.sort()
    return results


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


def _inspect_class(class_name, obj):
    parents = obj.mro()[1:-2]
    parent_attrs = sum((dir(parent) for parent in obj.mro()[1:]), [])

    info = {
        NAME: class_name,
        ATTRS: {},
        DOC: getdoc(obj),
        PARENTS: [str(parent.__name__) for parent in parents]
    }
    for attr_name, member in _getmembers(obj):
        if attr_name not in parent_attrs + ['__module__', '__instance_size__']:
            info[ATTRS][attr_name] = _get_member_info('%s.%s' % (class_name, attr_name), member)
    return info


def _inspect_function(name, value):
    return {
        NAME: name,
        DOC: getdoc(value)
    }


def _inspect_enum(name, obj):
    return {
        NAME: name,
        ENUM_PAIRS: [(v.numerator, k) for k, v in obj.names.items()],
    }


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
    MemberType.ENUM: _inspect_enum,
    MemberType.CLASS: _inspect_class,
    MemberType.FUNCTION: _inspect_function,
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


def get_module_info(obj, instances):
    module_members = get_module_members(obj)
    return module_members[MemberType.CLASS],  module_members[MemberType.ENUM], module_members[MemberType.FUNCTION], list(inspect_instances(instances))
