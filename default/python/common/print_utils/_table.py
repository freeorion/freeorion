"""
Print utils.

For test proposes this module is not import any freeorion runtime libraries.
If you improve it somehow, add usage example to __main__ section.
"""
from collections import defaultdict
from collections.abc import Collection
from itertools import zip_longest
from math import ceil
from typing import Any, Union

from common.print_utils._base_field import Field


def as_columns(items: Collection[Any], columns=2) -> str:
    """
    Split flat list to columns and print them.

    >>> as_columns(['a', 'b', 'c', 'd'], 2)
    a   c
    b   d
    """
    row_count = int(ceil(len(items) / columns))
    text_columns = list(zip_longest(*[iter(items)] * row_count, fillvalue=""))
    column_widths = (max(len(x) for x in word) for word in text_columns)
    template = "   ".join("%%-%ss" % w for w in column_widths)

    return "\n".join(template % row for row in zip(*text_columns))


class Table:
    def __init__(
        self,
        *fields: Union[Field, Collection[Field]],
        vertical_sep="|",
        header_sep="=",
        bottom_sep="-",
        table_name="",
        hide_header=False,
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
        self._notes = []  # each row has a not attached for it
        self._headers = fields
        self._hide_headers = hide_header
        self.totals = defaultdict(int)

    def __str__(self):
        return self.get_table()

    def add_row(self, *row, note: str = ""):
        table_row = []
        for field, val in zip(self._headers, row):
            if field.total:
                self.totals[field] += val
            table_row.append(field.make_cell_string(val))
        self._rows.append(table_row)
        self._notes.append(note)

    def _get_row_separator(self, char, column_widthes):
        return char * (2 + (len(column_widthes) - 1) * 3 + sum(column_widthes) + 2)

    def _get_rows_width(self) -> list[int]:
        if self._rows:
            return [max([len(y) for y in x], default=0) for x in zip(*self._rows)]
        else:
            return [1] * len(self._headers)

    def _get_headers_width(self) -> list[int]:
        if self._hide_headers:
            return [1] * len(self._headers)
        else:
            return [len(x.name) for x in self._headers]

    def _get_table_column_width(self) -> list[int]:
        columns_width = self._get_rows_width()
        header_width = self._get_headers_width()
        return [max(a, b) for a, b in zip(columns_width, header_width)]

    def __iter__(self):
        column_widths = self._get_table_column_width()

        if self._table_name:
            yield self._table_name

        inner_separator = " %s " % self._vertical_sep

        if not self._hide_headers:
            yield self._get_row_separator(self._header_sep, column_widths)

            yield "{} {} {}".format(
                self._vertical_sep,
                inner_separator.join(h.format_header(width) for h, width in zip(self._headers, column_widths)),
                self._vertical_sep,
            )

        yield self._get_row_separator(self._header_sep, column_widths)

        for row, note in zip(self._rows, self._notes):
            if note:
                note = f"  {note.strip()}"

            row_content = inner_separator.join(
                h.format_cell(item, width) for h, item, width in zip(self._headers, row, column_widths)
            )
            yield f"{self._vertical_sep} {row_content} {self._vertical_sep}{note}"

        if self.totals:
            yield self._get_row_separator(self._header_sep, column_widths)

            inner = inner_separator.join(
                h.format_cell(self.totals.get(h, " "), width) for h, width in zip(self._headers, column_widths)
            )

            yield f"{self._vertical_sep} {inner} {self._vertical_sep}"
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

        return "\n".join(self)
