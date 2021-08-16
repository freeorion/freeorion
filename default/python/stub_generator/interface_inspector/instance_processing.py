from logging import error, warning
from typing import Any, Dict, Iterator, List

from stub_generator.interface_inspector.inspection_helpers import (
    _get_member_info,
    _getmembers,
)

_INVALID_CLASSES = {"method"}


class InstanceInfo:
    def __init__(
        self,
        class_name: str,
        attributes: Dict[str, Any],
        parents: List[str],
    ):
        self.class_name = class_name
        self.attributes = attributes
        self.parents = parents


def _inspect_instance(instance):
    parents = instance.__class__.mro()[1:-2]
    parent_attrs = sum((dir(parent) for parent in instance.__class__.mro()[1:]), [])

    attrs = {}

    for attr_name, member in _getmembers(instance):
        if attr_name not in parent_attrs + ["__module__"]:
            attrs[attr_name] = _get_member_info("%s.%s" % (instance.__class__.__name__, attr_name), member)
    return InstanceInfo(instance.__class__.__name__, attrs, [str(parent.__name__) for parent in parents])


def is_built_in(instance: Any) -> bool:
    """
    Return is instance is one of built in type.

    Inherited classes from built in return False.
    """
    built_in_types = {str, float, int, tuple, tuple, list, dict, set}
    return type(instance) in built_in_types


def inspect_instances(instances) -> Iterator[InstanceInfo]:
    for location, instance in instances:
        if is_built_in(instance):
            warning("Argument at %s is builtin python instance: (%s) %s", location, type(instance), instance)
            continue
        if instance.__class__.__name__ in _INVALID_CLASSES:
            warning("Argument at %s is not allowed: (%s) %s", location, type(instance), instance)
            continue
        try:
            yield _inspect_instance(instance)
        except Exception as e:
            error("Error inspecting: '%s' with '%s': %s", type(instance), type(e), e, exc_info=True)
