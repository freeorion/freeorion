from operator import attrgetter, itemgetter
from typing import List

from stub_generator.interface_inspector import EnumInfo
from stub_generator.stub_generator.processor import BaseProcessor

ENUM_STUB = ('class Enum(int):\n'
             '    """Enum stub for docs, not really present in fo"""\n'
             '    def __new__(cls, *args, **kwargs):\n'
             '        return super(Enum, cls).__new__(cls, args[0])')


def _handle_enum(info: EnumInfo):
    pairs = sorted(info.attributes.items(), key=itemgetter(1))
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
    return '\n'.join(result)


class EnumProcessor(BaseProcessor):
    def __init__(self, enums: List[EnumInfo]):
        super().__init__()
        self.body.append(ENUM_STUB)
        for enum in sorted(enums, key=attrgetter("name")):
            self.body.append(_handle_enum(enum))
