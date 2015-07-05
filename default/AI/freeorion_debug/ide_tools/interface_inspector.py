import os
from inspect import getmembers, getdoc, isroutine
from generate_stub import make_stub


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
        print '>>>', type(member), "of", member
        print
    return info


def getmembers(object, predicate=None):
    """Return all members of an object as (name, value) pairs sorted by name.
    Optionally, only return members that satisfy a given predicate."""
    results = []
    for key in dir(object):
        try:
            value = getattr(object, key)
        except AttributeError:
            continue
        except Exception as e:
            print 'Error in "%s.%s": %s' % (object.__class__.__name__, key, e)
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
        if not name in parent_attrs + ['__module__']:
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
        if not name in parent_attrs + ['__module__', '__instance_size__']:
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


def _inspect(obj, *instances):
    data = []

    for name, member in getmembers(obj):
        function = switcher.get(str(type(member)), None)
        if function:
            data.append(function(name, member))
        elif name in ('__doc__', '__package__', '__name__'):
            pass
        else:
            print "Unknown", name, type(member), member
    for instance in instances:
        try:
            data.append(inspect_instance(instance))
        except Exception as e:
            print "Error inspecting:", type(instance), type(e), e
            from traceback import print_exc
    return data


def inspect(obj, *instances):
    folder_name = os.path.join(os.path.dirname(__file__), 'result')
    result_path = os.path.join(folder_name, '%s.py' % obj.__name__)
    make_stub(_inspect(obj, *instances), result_path)
