import os
from inspect import getdoc, isroutine
from logging import warning, error, debug
from typing import Any

from stub_generator.constants import TYPE, NAME, ATTRS, DOC, PARENTS, CLASS_NAME, ENUM_PAIRS


def _get_member_info(name, member):
    info = {
        'type': str(type(member)),
    }
    if isinstance(member, property):
        info['getter'] = getdoc(member.fget)
    elif isroutine(member):
        info['routine'] = (member.__name__, getdoc(member))
    elif isinstance(member, (int, str, float, list, tuple, dict, set, frozenset)):
        pass  # we don't need any
    elif 'freeOrionAIInterface' in info['type'] or "freeorion" in info['type']:
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
        TYPE: 'instance',
        ATTRS: {},
        PARENTS: [str(parent.__name__) for parent in parents],
        "location": location,
    }
    for attr_name, member in _getmembers(instance):
        if attr_name not in parent_attrs + ['__module__']:
            info['attrs___'][attr_name] = _get_member_info('%s.%s' % (instance.__class__.__name__, attr_name), member)
    return info


def _inspect_boost_class(class_name, obj):
    parents = obj.mro()[1:-2]
    parent_attrs = sum((dir(parent) for parent in obj.mro()[1:]), [])

    info = {
        TYPE: "boost_class",
        NAME: class_name,
        ATTRS: {},
        DOC: getdoc(obj),
        PARENTS: [str(parent.__name__) for parent in parents]
    }
    for attr_name, member in _getmembers(obj):
        if attr_name not in parent_attrs + ['__module__', '__instance_size__']:
            info[ATTRS][attr_name] = _get_member_info('%s.%s' % (class_name, attr_name), member)
    return info


def _inspect_boost_function(name, value):
    return {
        TYPE: 'function',
        NAME: name,
        DOC: getdoc(value)
    }


def _inspect_type(name, obj):
    return {
        TYPE: "enum",
        NAME: name,
        ENUM_PAIRS: [(v.numerator, k) for k, v in obj.names.items()],
    }


_SWITCHER = {
    "<class 'type'>": _inspect_type,
    "<class 'Boost.Python.class'>": _inspect_boost_class,
    "<class 'Boost.Python.function'>": _inspect_boost_function,
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


def get_module_info(obj, instances, dump=False):
    data = []
    for name, member in _getmembers(obj):
        if name in _NAMES_TO_IGNORE:
            continue

        object_handler = _SWITCHER.get(str(type(member)), None)
        if object_handler:
            data.append(object_handler(name, member))
        else:
            warning("Unknown: '%s' of type '%s': %s" % (name, type(member), member))

    for location, instance in instances:
        if is_built_in(instance):
            warning("Argument at %s is builtin python instance: (%s) %s", location, type(instance), instance)
            continue
        if instance.__class__.__name__ in _INVALID_CLASSES:
            warning("Argument at %s is not allowed: (%s) %s", location, type(instance),
                    instance)
            continue
        try:

            # TODO add isntances location some where

            data.append((_inspect_instance(instance, location)))
        except Exception as e:
            error("Error inspecting: '%s' with '%s': %s", type(instance), type(e), e, exc_info=True)
    if dump:

        dump_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), '%s_info.json' % obj.__name__)
        with open(dump_path, 'w') as f:
            import json
            json.dump(sorted(data, key=lambda x: (x.get(NAME, '') or x[CLASS_NAME], x.get(TYPE, ''))), f, indent=4, sort_keys=True)
            pass
        debug("File with interface info is dumped: %s" % dump_path)
    return data
