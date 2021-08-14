"""
Print utils.

For test proposes this module is not import any freeorion runtime libraries.
If you improve it somehow, add usage example to __main__ section.
"""
from collections import defaultdict
from itertools import zip_longest
from math import ceil
from typing import Any, Callable, Collection, Union

from common.print_utils._base_field import Field


def print_in_columns(items: Collection[Any], columns=2, printer=print):
    """
    Split flat list to columns and print them.

    >>> print_in_columns(['a', 'b', 'c', 'd', 2])
    a   c
    b   d
    """
    row_count = int(ceil((len(items) / columns)))
    text_columns = list(zip_longest(*[iter(items)] * row_count, fillvalue=""))
    column_widths = (max(len(x) for x in word) for word in text_columns)
    template = "   ".join("%%-%ss" % w for w in column_widths)

    for row in zip(*text_columns):
        printer(template % row)


class Table:
    def __init__(
        self, *fields: Union[Field, Collection[Field]], vertical_sep="|", header_sep="=", bottom_sep="-", table_name=""
    ):
        """
        Table layout for print data.

        - specify headers in constructor
        - add rows
        - print
        """
        self._table_name = table_name
        self._bottom_sep = bottom_sep
        self._header_sep = header_sep
        self._vertical_sep = vertical_sep
        self._rows = []
        self._headers = fields
        self.totals = defaultdict(int)

    def __str__(self):
        return self.get_table()

    def add_row(self, *row):
        table_row = []
        for filed, val in zip(self._headers, row):
            if filed.total:
                self.totals[filed] += val
            table_row.append(filed.make_cell_string(val))
        self._rows.append(table_row)

    def _get_row_separator(self, char, column_widthes):
        return char * (2 + (len(column_widthes) - 1) * 3 + sum(column_widthes) + 2)

    def __iter__(self):
        columns = [[len(y) for y in x] for x in zip(*self._rows)]
        # join size of headers and columns, since columns can be empty
        header_and_columns = [
            [h] + x for h, x in zip_longest([len(x.name) for x in self._headers], columns, fillvalue=[])
        ]
        column_widths = [max(x) for x in header_and_columns]

        if self._table_name:
            yield self._table_name

        yield self._get_row_separator(self._header_sep, column_widths)
        inner_separator = " %s " % self._vertical_sep
        yield "%s %s %s" % (
            self._vertical_sep,
            inner_separator.join(h.format_header(width) for h, width in zip(self._headers, column_widths)),
            self._vertical_sep,
        )

        yield self._get_row_separator(self._header_sep, column_widths)

        for row in self._rows:
            yield "%s %s %s" % (
                self._vertical_sep,
                inner_separator.join(
                    h.format_cell(item, width) for h, item, width in zip(self._headers, row, column_widths)
                ),
                self._vertical_sep,
            )

        if self.totals:
            yield self._get_row_separator(self._header_sep, column_widths)

            inner = inner_separator.join(
                h.format_cell(self.totals.get(h, " "), width) for h, width in zip(self._headers, column_widths)
            )

            yield "%s %s %s" % (self._vertical_sep, inner, self._vertical_sep)
            yield self._get_row_separator(self._header_sep, column_widths)
        else:
            yield self._get_row_separator(self._bottom_sep, column_widths)

        # print legend
        legend = [x for x in self._headers if x.description]
        if legend:
            name_width = max(len(x.name) for x in legend)
            for header in legend:
                yield "*%-*s %s" % (name_width, header.name[:-1], header.description)

    def get_table(self) -> str:
        """
        Return table as text.

        This method is deprecated, since long output will be truncated by logger.
        Use  print_table instead.
        """

        return "\n".join(list(self))

    def print_table(self, printer: Callable[[str], None]) -> None:
        """
        Pass table row by row to printer.
        """
        for line in self:
            printer(line)
