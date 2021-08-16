from inspect import getdoc, isroutine
from logging import error, warning


def _get_member_info(name, member):
    type_ = str(type(member))

    info = {
        "type": type_,
    }
    if isinstance(member, property):
        info["getter"] = getdoc(member.fget)
    elif isroutine(member):
        info["routine"] = (member.__name__, getdoc(member))
    elif isinstance(member, (int, str, float, list, tuple, dict, set, frozenset)):
        pass  # we don't need any
    elif "freeOrionAIInterface" in type_ or "freeorion" in type_:
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
            error("\n".join(message))
            continue
        if not predicate or predicate(value):
            results.append((key, value))
    results.sort()
    return results
