from collections import defaultdict
from enum import Enum
from logging import warning
from typing import List, Tuple

from stub_generator.interface_inspector.class_processor import ClassInfo, inspect_class
from stub_generator.interface_inspector.enum_processor import EnumInfo, inspect_enum
from stub_generator.interface_inspector.function_processor import (
    FunctionInfo,
    inspect_function,
)
from stub_generator.interface_inspector.inspection_helpers import _getmembers
from stub_generator.interface_inspector.instance_processing import (
    InstanceInfo,
    inspect_instances,
)


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

_MODULE_NAMES_TO_IGNORE = {
    "__doc__",
    "__package__",
    "__name__",
    "INVALID_GAME_TURN",
    "to_str",
    "__loader__",
    "__spec__",
}


def get_module_members(obj):
    module_members = defaultdict(list)

    for name, member in _getmembers(obj):
        if name in _MODULE_NAMES_TO_IGNORE:
            continue
        type_key = get_type(member)
        if type_key == MemberType.UNKNOWN:
            warning(f"Unknown: '{name}' of type '{type(member)}': {member}")
        else:
            type_inspector = _OBJECT_HANDLERS[type_key]
            module_members[type_key].append(type_inspector(name, member))
    return module_members


def get_module_info(obj, instances) -> Tuple[List[ClassInfo], List[EnumInfo], List[FunctionInfo], List[InstanceInfo]]:
    module_members = get_module_members(obj)
    return (
        module_members[MemberType.CLASS],
        module_members[MemberType.ENUM],
        module_members[MemberType.FUNCTION],
        list(inspect_instances(instances)),
    )
