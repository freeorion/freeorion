# coding=utf-8
"""


===== Columns =====



Process finished with exit code 0
"""
from io import StringIO

from common.print_utils import Number, Sequence, Table, Text, print_in_columns

EXPECTED_COLUMNS = """a   c
b   d
"""

EXPECTED_SIMPLE_TABLE = """Wooho
============================================================================
| name*            | value*     | zzz                 | zzzzzzzzzzzzzzzzzz |
============================================================================
| hello            |  144444.00 | a, b, c, f, f, f, f | a                  |
| Plato aaβ III    |      21.00 | d, e                | a                  |
| Plato β III      |      21.00 | d, e                | a                  |
| Plato B III      |      21.00 | d                   | a                  |
| Plato Bddddd III |      21.00 | d                   | a                  |
| Plato III        |      21.00 | d                   | a                  |
----------------------------------------------------------------------------
*name   Name for first column
*value  VValue
"""

EXPECTED_EMPTY_TABLE = """Wooho
=============================================
| name* | value* | zzz | zzzzzzzzzzzzzzzzzz |
=============================================
---------------------------------------------
*name   Name for first column
*value  VValue
"""


# https://pytest.org/latest/capture.html#accessing-captured-output-from-a-test-function
def test_print_in_columns(capfd):
    print_in_columns(["a", "b", "c", "d"], 2)
    out, err = capfd.readouterr()
    assert out == EXPECTED_COLUMNS


def make_table():
    t = Table(
        Text("name", description="Name for first column"),
        Number("value", description="VValue"),
        Sequence("zzz"),
        Sequence("zzzzzzzzzzzzzzzzzz"),
        table_name="Wooho",
    )
    t.add_row("hello", 144444, "abcffff", "a")
    t.add_row("Plato aa\u03b2 III", 21, "de", "a")
    t.add_row("Plato \u03b2 III", 21, "de", "a")
    t.add_row("Plato B III", 21, "d", "a")
    t.add_row("Plato Bddddd III", 21, "d", "a")
    t.add_row("Plato III", 21, "d", "a")
    return t


def test_table_is_printed():
    table = make_table()
    io = StringIO()

    def writer(row):
        io.write(row)
        io.write("\n")

    table.print_table(writer)
    assert io.getvalue() == EXPECTED_SIMPLE_TABLE


def test_table_is_converted_to_str():
    io = StringIO()

    def writer(row):
        io.write(row)
        io.write("\n")

    table = make_table()
    table.print_table(writer)
    assert io.getvalue() == EXPECTED_SIMPLE_TABLE


def test_empty_table():
    empty = Table(
        Text("name", description="Name for first column"),
        Number("value", description="VValue"),
        Sequence("zzz"),
        Sequence("zzzzzzzzzzzzzzzzzz"),
        table_name="Wooho",
    )

    io = StringIO()

    def writer(row):
        io.write(row)
        io.write("\n")

    empty.print_table(writer)

    assert io.getvalue() == EXPECTED_EMPTY_TABLE


def test_number_column():
    field = Number("name", placeholder="-")
    assert field.make_cell_string(0) == "-"


def test_total_is_calculated():
    table = Table(
        Number("A", precession=0, total=True),
        Number("B", precession=0, total=True),
    )
    table.add_row(1, 10)
    table.add_row(2, 20)
    assert list(table) == [
        "============",
        "| A  | B   |",
        "============",
        "|  1 |  10 |",
        "|  2 |  20 |",
        "============",
        "|  3 |  30 |",
        "============",
    ]
