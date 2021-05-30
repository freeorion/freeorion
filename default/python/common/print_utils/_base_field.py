from abc import ABC, abstractmethod
from typing import Any


class Field(ABC):
    def __init__(self, name: str, align='<', description="", placeholder=""):
        """
        Cell specification.
        :param name: name of the column
        :param align: "<" | ">" | "=" | "^" https://docs.python.org/2/library/string.html#format-specification-mini-language
        :param description: description for column name, will be printed in table legend,
                            specify it if you use abbr as name: ``name``="PP", ``description``="production points"
        :param placeholder: placeholder for empty value
        """

        self.name = name
        self.align = align
        self.description = description
        self.placeholder = placeholder
        if description:
            self.name += '*'

    def __repr__(self):
        return '%s(%s)' % (self.__class__.__name__, self.name)

    def format_cell(self, item, width):
        return '{:{align}{width}}'.format(item, width=width, align=self.align)

    def format_header(self, width):
        return '{: <{width}}'.format(self.name, width=width)

    def make_cell_string(self, val):
        if self.placeholder and not val:
            return self.placeholder
        else:
            return self.convert_value_to_string(val)

    @abstractmethod
    def convert_value_to_string(self, val: Any) -> str:
        ...
