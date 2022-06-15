from operator import attrgetter
from typing import List

from stub_generator.interface_inspector import EnumInfo
from stub_generator.stub_generator.base_generator import BaseGenerator


def _handle_enum(info: EnumInfo):
    def attr_sort_key(pair):
        """
        Make key to sort attributes.

        We sort enum items by is value and after that by name.
        Enum item could have aliases with the same value and different names.
        """
        name, val = pair
        return val, name

    pairs = sorted(info.attributes.items(), key=attr_sort_key)
    result = [
        "class %s(IntEnum):" % info.name,
    ]

    for text, value in pairs:
        result.append("    %s = %s" % (text, value))
    result.append("")
    yield "\n".join(result)


class EnumGenerator(BaseGenerator):
    def __init__(self, enums: List[EnumInfo]):
        super().__init__()

        for enum in sorted(enums, key=attrgetter("name")):
            self.body.extend(_handle_enum(enum))
