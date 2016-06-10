"""
Print utils.

For test proposes this module is not import any freeorion runtime libraries.
If you improve it somehow, add usage example to __main__ section.
"""

from itertools import izip_longest
from math import ceil


def print_in_columns(items, columns=2):
    """
    Prints collection as columns.

    >>> print_in_columns(['a', 'b', 'c', 'd', 2])
    >>> a   c
        b   d

    :param items: any ordered collection (list, tuple)
    :type items: list|tuple
    :param columns: number of columns
    :type columns: int
    :return None:
    """
    row_count = int(ceil((float(len(items)) / columns)))
    text_columns = list(izip_longest(*[iter(items)] * row_count, fillvalue=''))
    column_widths = (max(len(x) for x in word) for word in text_columns)
    template = '   '.join('%%-%ss' % w for w in column_widths)

    for row in zip(*text_columns):
        print template % row


class Base(object):
    header_fmt = ''
    fmt = None

    def __init__(self, name, align='>', description=None, **kwargs):
        """
        Header cell describe how to format column.

        :param name: name of the column
        :type name: str
        :param align: "<" | ">" | "=" | "^" https://docs.python.org/2/library/string.html#format-specification-mini-language
        :type align: str
        :param description: description for column name, will be printed in table legend,
                            specify it if you use abbr as name: ``name``="PP", ``description``="production points"

        :type description: str
        :param kwargs: column specific arguments, like ``precession`` for ``Float``
        :type kwargs: dict
        """

        self.name = name
        self.align = align
        self.description = description
        self.kwargs = kwargs
        if description:
            self.name += '*'

    def __repr__(self):
        return u'%s(%s)' % (self.__class__.__name__, self.name)

    def format_cell(self, item, width):
        return u'{:{align}{width}}'.format(item, width=width, align=self.align)

    def format_header(self, width):
        return u'{: <{width}}'.format(self.name, width=width)

    def to_unicode(self, val):
        return self.fmt.format(val, **self.kwargs)


class Text(Base):
    fmt = u'{}'

    def __init__(self, name, description=None, align='>'):
        super(Text, self).__init__(name, align=align, description=description)

    def to_unicode(self, val):
        if not isinstance(val, unicode):
            if not isinstance(val, str):
                val = str(val)
            val = val.decode('utf-8')
        return self.fmt.format(val, **self.kwargs)


class Float(Base):
    fmt = u'{: .{precession}f}'

    def __init__(self, name, align='>', precession=2, description=None):
        super(Float, self).__init__(name, align=align, precession=precession, description=description)


class Bool(Base):
    fmt = u'{}'

    def __init__(self, name, no_yes='-+', description=None):
        self.no_yes = no_yes
        assert len(no_yes) == 2
        super(Bool, self).__init__(name, description=description)

    def to_unicode(self, val):
        return self.no_yes[val].decode('utf-8')


class Sequence(Text):
    fmt = u'{}'

    def to_unicode(self, vals):
        vals = [super(Sequence, self).to_unicode(val) for val in vals]
        return self.fmt.format(', '.join(vals), **self.kwargs)


class Table(object):

    def __init__(self, headers, vertical_sep='|', header_sep='=', bottom_sep='-', table_name=None):
        """
        Table layout for print data.

        - specify headers in constructor
        - add rows
        - print

        :param headers: headers for table should be list of subclasses of Base.
        :type headers: [Base]
        :param vertical_sep: character to print vertical border
        :type vertical_sep: str
        :param header_sep: character to print horizontal border around header
        :type header_sep: str
        :param bottom_sep: character to print horizontal border at the table bottom
        :type bottom_sep: str
        :param table_name: if specified will be printed before table
        :type table_name: str
        """
        self.__table_name = table_name
        self.__bottom_sep = bottom_sep
        self.__header_sep = header_sep
        self.__vertical_sep = vertical_sep
        self.__rows = []
        self.__headers = headers

    def add_row(self, row):
        self.__rows.append(tuple(h.to_unicode(cell) for h, cell in zip(self.__headers, row)))

    def __get_row_separator(self, char, column_widthes):
        return char * (2 +
                       (len(column_widthes) - 1) * 3 +
                       sum(column_widthes) +
                       2
                       )

    def print_table(self):
        print self.get_table()

    def get_table(self):
        columns = [x for x in zip(*self.__rows)]
        column_widths = [max([max(len(i) for i in x), len(h.name)]) for h, x in zip(self.__headers, columns)]
        result = []

        if self.__table_name:
            result.append(self.__table_name)

        result.append(self.__get_row_separator(self.__header_sep, column_widths))
        inner_separator = ' %s ' % self.__vertical_sep
        result.append('%s %s %s' % (
            self.__vertical_sep,
            inner_separator.join(h.format_header(width) for h, width in zip(self.__headers, column_widths)),
            self.__vertical_sep
        ))
        result.append(self.__get_row_separator(self.__header_sep, column_widths))

        for row in self.__rows:
            text = '%s %s %s' % (self.__vertical_sep,
                                 inner_separator.join(
                                     h.format_cell(item, width) for h, item, width in
                                     zip(self.__headers, row, column_widths)
                                 ),
                                 self.__vertical_sep)
            result.append(text)
        result.append(self.__get_row_separator(self.__bottom_sep, column_widths))

        # print legend
        legend = [x for x in self.__headers if x.description]
        if legend:
            name_width = max(len(x.name) for x in legend)
            for header in legend:
                result.append(('*%-*s %s' % (name_width, header.name[:-1], header.description)))
        return '\n'.join(x.encode('utf-8') for x in result)


# Tests
if __name__ == '__main__':
    t = Table(
        [Text('name', align='<', description='Name for first column'),
         Float('value', description='VValue'),
         Sequence('zzz'),
         Sequence('zzzzzzzzzzzzzzzzzz'),
         ],
        table_name='Wooho')
    t.add_row(['hello', 144444, 'abcffff', 'a'])
    t.add_row([u'Plato aa\u03b2 III', 21, 'de', 'a'])
    t.add_row([u'Plato \u03b2 III', 21, 'de', 'a'])
    t.add_row(['Plato B III', 21, 'd', 'a'])
    t.add_row(['Plato Bddddd III', 21, 'd', 'a'])
    t.add_row(['Plato III', 21, 'd', 'a'])
    t.print_table()

    print
    print '=' * 5, 'Columns', '=' * 5
    print_in_columns(['a', 'b', 'c', 'd'], 2)
