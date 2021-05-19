from operator import attrgetter
from typing import List

from stub_generator.interface_inspector import EnumInfo
from stub_generator.stub_generator.base_generator import BaseGenerator

ENUM_STUB = ('class Enum(int):\n'
             '    """Enum stub for docs, not really present in fo"""\n'
             '    def __new__(cls, *args, **kwargs):\n'
             '        return super(Enum, cls).__new__(cls, args[0])')


def _handle_enum(info: EnumInfo):
    def attr_sort_key(pair):
        """
        Make key to sort attributes.

        We sort enum items by is value and after that by name.
        Enum item could have aliases with the same value and different names.
        """
        name, val = pair
        return val, name

    pairs = sorted(info.attributes.items(), key=attr_sort_key)
    result = ['class %s(Enum):' % info.name,
              '    def __init__(self, numerator, name):',
              '        self.name = name',
              ''
              ]

    for text, value in pairs:
        result.append('    %s = None  # %s(%s, "%s")' % (text, (info.name), value, text))
    result.append('')
    result.append('')  # two empty lines between enum and its items declaration

    for text, value in pairs:
        result.append('%s.%s = %s(%s, "%s")' % ((info.name), text, (info.name), value, text))
    yield '\n'.join(result)


class EnumGenerator(BaseGenerator):
    def __init__(self, enums: List[EnumInfo]):
        super().__init__()
        self.body.append(ENUM_STUB)
        for enum in sorted(enums, key=attrgetter("name")):
            self.body.extend(_handle_enum(enum))
