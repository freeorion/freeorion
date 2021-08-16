from abc import ABC, abstractmethod
from typing import Any


class Field(ABC):
    def __init__(
        self,
        name: str,
        align="<",
        description="",
        placeholder="",
        total=False,
    ):
        """
        Cell specification.
        :param name: name of the column
        :param align: "<" | ">" | "=" | "^" https://docs.python.org/2/library/string.html#format-specification-mini-language
        :param description: description for column name, will be printed in table legend,
                            specify it if you use abbr as name: ``name``="PP", ``description``="production points"
        :param placeholder: placeholder for empty value
        :param total: add total string
        """
        self.name = name
        self.align = align
        self.description = description
        self.placeholder = placeholder
        if description:
            self.name += "*"
        self.total = total

    def __repr__(self):
        return f"{self.__class__.__name__}({self.name})"

    def format_cell(self, item, width):
        return f"{item:{self.align}{width}}"

    def format_header(self, width):
        return f"{self.name:<{width}}"

    def make_cell_string(self, val):
        if self.placeholder and not val:
            return self.placeholder
        else:
            return self.convert_value_to_string(val)

    def make_summary_value(self):
        return ""

    @abstractmethod
    def convert_value_to_string(self, val: Any) -> str:
        ...
