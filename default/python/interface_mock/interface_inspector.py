import os
from inspect import getdoc, isroutine
from generate_mock import make_mock

from common.configure_logging import convenience_function_references_for_logger
(debug, info, warn, error, fatal) = convenience_function_references_for_logger(__name__)


def get_member_info(member):
    info = {
        'type': str(type(member)),
    }

    if isinstance(member, property):
        info['getter'] = (member.fget.__name__, getdoc(member.fget))
    elif isroutine(member):
        info['rutine'] = (member.__name__, getdoc(member))
    elif 'freeOrionAIInterface' in info['type']:
        info['value'] = str(member)
    elif isinstance(member, int):
        if type(member) == int:
            info['value'] = member
        else:
            info['value'] = str(member)
    elif isinstance(member, (str, long, bool, float)):
        info['value'] = member
    elif isinstance(member, (list, tuple, dict, set, frozenset)):
        if not len(member):
            info['value'] = member
    else:
        warn('Unexpected member "%s":' % (type(member), member))
    return info


def getmembers(obj, predicate=None):
    """Return all members of an object as (name, value) pairs sorted by name.
    Optionally, only return members that satisfy a given predicate."""
    results = []
    for key in dir(obj):
        try:
            value = getattr(obj, key)
        except AttributeError:
            continue
        except Exception as e:
            error('Error in "%s.%s" with error: %s' % (obj.__class__.__name__, key, e))
            continue
        if not predicate or predicate(value):
            results.append((key, value))
    results.sort()
    return results


def inspect_instance(instance):
    parents = instance.__class__.mro()[1:-2]
    parent_attrs = sum((dir(parent) for parent in instance.__class__.mro()[1:]), [])

    info = dict(class_name=instance.__class__.__name__,
                type='instance',
                attrs={},
                parents=[str(parent.__name__) for parent in parents])
    for name, member in getmembers(instance):
        if name not in parent_attrs + ['__module__']:
            info['attrs'][name] = get_member_info(member)
    return info


def inspect_boost_class(name, obj):
    parents = obj.mro()[1:-2]
    parent_attrs = sum((dir(parent) for parent in obj.mro()[1:]), [])

    info = {'type': "boost_class",
            'name': name,
            'attrs': {},
            'doc': getdoc(obj),
            'parents': [str(parent.__name__) for parent in parents]
            }
    for name, member in getmembers(obj):
        if name not in parent_attrs + ['__module__', '__instance_size__']:
            info['attrs'][name] = get_member_info(member)
    return info


def inspect_boost_function(name, value):
    return {
        'type': 'function',
        'name': name,
        'doc': getdoc(value)
    }


def inspect_type(name, obj):
    enum_dict = {}
    for k, v in obj.names.items():
        enum_dict.setdefault(v, [None, None])[1] = k

    for k, v in obj.values.items():
        enum_dict.setdefault(v, [None, None])[0] = k
    return {'type': "enum",
            'name': name,
            'enum_dicts': enum_dict,
            }


switcher = {
    "<type 'type'>": inspect_type,
    "<type 'Boost.Python.class'>": inspect_boost_class,
    "<type 'Boost.Python.function'>": inspect_boost_function,
}


def _inspect(obj, instances):
    data = []

    for name, member in getmembers(obj):
        function = switcher.get(str(type(member)), None)
        if function:
            data.append(function(name, member))
        elif name in ('__doc__', '__package__', '__name__', 'INVALID_GAME_TURN', 'to_str'):
            pass
        else:
            warn("Unknown: '%s' of type '%s': %s" % (name, type(member), member))
    for i, instance in enumerate(instances, start=2):
        if isinstance(instance, (basestring, int, long, float)):
            warn("Argument number %s(1-based) is builtin python instance: (%s) %s" % (i, type(instance), instance))
            continue
        try:
            data.append(inspect_instance(instance))
        except Exception as e:
            error("Error inspecting: '%s' with '%s': %s" % (type(instance), type(e), e))
    return data


def inspect(obj, instances, classes_to_ignore=tuple()):
    """
    Inspect interface and generate mock. writes its logs to freeoriond.log

    :param obj: main interface module (freeOrionAIInterface for AI)
    :param instances:  list of instances, required to get more detailed information about them
    :param classes_to_ignore: classes that should not to be reported when check for missed instances done.
                              this argument required because some classes present in interface
                              but have no methods, to get their instances.
    """
    debug("\n\nStart generating skeleton for %s\n\n" % obj.__name__)
    folder_name = os.path.join(os.path.dirname(__file__), 'result')
    result_path = os.path.join(folder_name, '%s.py' % obj.__name__)
    make_mock(_inspect(obj, instances), result_path, classes_to_ignore)
    debug("Skeleton written to %s" % result_path)
