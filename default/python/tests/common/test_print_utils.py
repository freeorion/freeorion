"""


===== Columns =====



Process finished with exit code 0
"""
from common.print_utils import Number, Sequence, Table, Text, as_columns

EXPECTED_ITEMS_MULTIPLE_COLUMNS = """a   c
b   d"""


EXPECTED_ITEMS_NOT_MULTIPLE_COLUMNS = """a   c
b    """

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
*value  VValue"""

EXPECTED_EMPTY_TABLE = """Wooho
=============================================
| name* | value* | zzz | zzzzzzzzzzzzzzzzzz |
=============================================
---------------------------------------------
*name   Name for first column
*value  VValue"""


def test_as_columns_with_items_multiple_columns():
    out = as_columns(["a", "b", "c", "d"], 2)
    assert out == EXPECTED_ITEMS_MULTIPLE_COLUMNS


def test_as_columns_with_items_not_multiple_columns():
    out = as_columns(["a", "b", "c"], 2)
    assert out == EXPECTED_ITEMS_NOT_MULTIPLE_COLUMNS


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


def test_table_is_converted_to_str():
    table = make_table()
    assert str(table) == EXPECTED_SIMPLE_TABLE


def test_empty_table():
    empty = Table(
        Text("name", description="Name for first column"),
        Number("value", description="VValue"),
        Sequence("zzz"),
        Sequence("zzzzzzzzzzzzzzzzzz"),
        table_name="Wooho",
    )
    assert str(empty) == EXPECTED_EMPTY_TABLE


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
