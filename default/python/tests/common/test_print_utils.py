# coding=utf-8
'''


===== Columns =====



Process finished with exit code 0
'''

from common.print_utils import print_in_columns, Table, Text, Float, Sequence

EXPECTED_COLUMNS = '''a   c
b   d
'''

EXPECTED_SIMPLE_TABLE = '''Wooho
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
*value  VValue'''

EXPECTED_EMPTY_TABLE = '''Wooho
=============================================
| name* | value* | zzz | zzzzzzzzzzzzzzzzzz |
=============================================
---------------------------------------------
*name   Name for first column
*value  VValue'''


# https://pytest.org/latest/capture.html#accessing-captured-output-from-a-test-function
def test_print_in_columns(capfd):
    print_in_columns(['a', 'b', 'c', 'd'], 2)
    out, err = capfd.readouterr()
    assert out == EXPECTED_COLUMNS


def test_simple_table():
    t = Table(
        [Text('name', description='Name for first column'), Float('value', description='VValue'),
         Sequence('zzz'), Sequence('zzzzzzzzzzzzzzzzzz')],
        table_name='Wooho')
    t.add_row(['hello', 144444, 'abcffff', 'a'])
    t.add_row([u'Plato aa\u03b2 III', 21, 'de', 'a'])
    t.add_row([u'Plato \u03b2 III', 21, 'de', 'a'])
    t.add_row(['Plato B III', 21, 'd', 'a'])
    t.add_row(['Plato Bddddd III', 21, 'd', 'a'])
    t.add_row(['Plato III', 21, 'd', 'a'])
    assert t.get_table() == EXPECTED_SIMPLE_TABLE


def test_empty_table():
    empty = Table(
        [Text('name', description='Name for first column'), Float('value', description='VValue'),
         Sequence('zzz'), Sequence('zzzzzzzzzzzzzzzzzz')],
        table_name='Wooho'
    )
    assert empty.get_table() == EXPECTED_EMPTY_TABLE
