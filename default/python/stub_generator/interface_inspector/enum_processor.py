from typing import Dict


class EnumInfo:
    def __init__(self, name: str, attributes: Dict[str, int]):
        self.name = name
        self.attributes = attributes


def inspect_enum(name, obj):
    return EnumInfo(name, {k: v.numerator for k, v in obj.names.items()})
