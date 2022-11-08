from typing import Any, Collection

from common.print_utils._base_field import Field


class Text(Field):
    def __init__(self, name, description="", align="<"):
        super().__init__(name, align=align, description=description)

    def convert_value_to_string(self, val: Any) -> str:
        return str(val)


class Number(Field):
    def __init__(self, name, precession=2, align=">", description="", placeholder="", total=False):
        super().__init__(
            name,
            align=align,
            description=description,
            placeholder=placeholder,
            total=total,
        )
        self.precession = precession

    def convert_value_to_string(self, val: Any) -> str:
        return "{: .{precession}f}".format(val, precession=self.precession)


class Bool(Field):
    def __init__(self, name, no_yes=("-", "+"), description=""):
        self.no_yes = no_yes
        assert len(no_yes) == 2
        super().__init__(name, description=description)

    def convert_value_to_string(self, val: Any) -> str:
        return self.no_yes[val]


class Sequence(Text):
    def convert_value_to_string(self, vals: Collection[Any]) -> str:
        return ", ".join(str(val) for val in vals)
