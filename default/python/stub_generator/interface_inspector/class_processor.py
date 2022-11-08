from inspect import getdoc
from typing import Any, Dict, List

from stub_generator.interface_inspector.inspection_helpers import (
    _get_member_info,
    _getmembers,
)


class ClassInfo:
    def __init__(self, name: str, attributes: Dict[str, Any], doc: str, parents: List[str]):
        self.name = name
        self.attributes = attributes
        self.doc = doc
        self.parents = parents


def inspect_class(class_name, obj):
    parents = obj.mro()[1:-2]
    parent_attrs = sum((dir(parent) for parent in obj.mro()[1:]), [])

    attrs = {}

    for attr_name, member in _getmembers(obj):
        if attr_name not in parent_attrs + ["__module__", "__instance_size__"]:
            attrs[attr_name] = _get_member_info(f"{class_name}.{attr_name}", member)
    return ClassInfo(class_name, attrs, getdoc(obj), [str(parent.__name__) for parent in parents])
