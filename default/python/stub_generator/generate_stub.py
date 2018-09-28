from logging import warn, error
from operator import itemgetter

from parse_docs import Docs


def handle_class(info):
    name = info['name']
    docs = info['doc']
    attrs = info['attrs']
    assert not docs, "Got docs need to handle it"
    parents = info['parents']
    if not parents:
        parents = ['object']  # instance is boost wrapper
    result = ['class %s(%s):' % (name, ', '.join(parents))]

    properties = []
    instance_methods = []
    for attr_name, attr in sorted(attrs.items()):
        if attr['type'] == "<type 'property'>":
            properties.append((attr_name, attr.get('rtype', '')))
        elif attr['type'] == "<type 'instancemethod'>":
            instance_methods.append(attr['routine'])
        else:
            warn("Skipping '%s': %s" % (name, attr))

    for property_name, rtype in properties:
        if not rtype:
            return_text = 'pass'
        elif rtype.startswith("<type"):
            return_text = 'return %s()' % rtype[7:-2]
        else:
            return_text = 'return %s()' % rtype.split('.')[-1].strip("'>")

        if property_name == 'class':
            result.append('    # cant define it via python in that way')
            result.append('    # @property')
            result.append('    # def %s(self): ' % property_name)
            result.append('    #    %s' % return_text)
        else:
            result.append('    @property')
            result.append('    def %s(self):' % property_name)
            result.append('        %s' % return_text)
        result.append('')

    for routine_name, routine_docs in instance_methods:
        docs = Docs(routine_docs, 2, is_class=True)
        # TODO: Subclass map-like classes from dict (or custom class) rather than this hack
        if docs.rtype in ('VisibilityIntMap', 'IntIntMap'):
            docs.rtype = 'dict[int, int]'
            return_string = 'return dict()'
        elif docs.rtype == 'None':
            return_string = 'return None'
        else:
            return_string = 'return %s()' % docs.rtype

        doc_string = docs.get_doc_string()
        result.append('    def %s(%s):' % (routine_name, docs.get_argument_string()))
        result.append(doc_string)
        result.append('        %s' % return_string)
        result.append('')
    if not (properties or instance_methods):
        result.append('    pass')
    if not result[-1]:
        result.pop()
    return '\n'.join(result)


def handle_function(doc):
    name = doc['name']
    doc = Docs(doc['doc'], 1)
    return_string = 'return %s%s' % (doc.rtype, '()' if doc.rtype != 'None' else '')

    res = 'def %s(%s):\n%s\n    %s' % (name, doc.get_argument_string(), doc.get_doc_string(), return_string)
    return res


ENUM_STUB = ('class Enum(int):\n'
             '    """Enum stub for docs, not really present in fo"""\n'
             '    def __new__(cls, *args, **kwargs):\n'
             '        return super(Enum, cls).__new__(cls, args[0])')


def handle_enum(info):
    name = info['name']
    enum_dicts = info['enum_dicts']
    result = ['class %s(Enum):' % name,
              '    def __init__(self, numerator, name):',
              '        self.name = name',
              ''
              ]

    for _, (value, text) in sorted(enum_dicts.items()):
        result.append('    %s = None  # %s(%s, "%s")' % (text, name, value, text))
    result.append('')
    result.append('')  # two empty lines between enum and its items declaration

    for _, (value, text) in sorted(enum_dicts.items(), key=lambda x: x[1][0]):
        result.append('%s.%s = %s(%s, "%s")' % (name, text, name, value, text))
    return '\n'.join(result)


known_types = {'boost_class',
               'enum',
               'function',
               'instance'}


def make_stub(data, result_path, classes_to_ignore):
    groups = {}
    for info in data:
        if info['type'] in known_types:
            groups.setdefault(info['type'], []).append(info)
        else:
            error('Unknown type "%s" in "%s' % (info['type'], info))
    classes = [x for x in groups['boost_class'] if not x['name'].startswith('map_indexing_suite_')]
    clases_map = {x['name']: x for x in classes}
    instance_names = {instance['class_name'] for instance in groups.get('instance', [])}

    enums = sorted(groups['enum'], key=itemgetter('name'))
    enums_names = [x['name'] for x in enums]

    missed_instances = instance_names.symmetric_difference(clases_map).difference(classes_to_ignore)
    warn(
        "Classes without instances (%s): %s",
        len(missed_instances),
        ', '.join(sorted(missed_instances, key=str.lower))
    )

    for instance in groups.get('instance', []):
        class_name = instance['class_name']
        if class_name in enums_names:
            warn("skipping enum instance: %s" % class_name)
            continue

        class_attrs = clases_map[class_name]['attrs']
        instance_attrs = instance['attrs']

        for k, v in class_attrs.items():
            inst = instance_attrs.get(k)
            if not inst:
                continue

            if v['type'] == "<type 'property'>":
                v['rtype'] = inst['type']
            elif v['type'] == "<type 'instancemethod'>":
                pass
            else:
                error("Unknown class attribute type: '%s'" % v['type'])
    res = [
        '# Autogenerated do not modify manually!\n'
        '# This is a type-hinting python stub file, used by python IDEs to provide type hints. For more information\n'
        '# about stub files, see https://www.python.org/dev/peps/pep-0484/#stub-files\n'
        '# During execution, the actual freeOrionAIInterface module is made available via\n'
        '# a C++ Boost-python process as part of the launch.'
    ]
    classes = sorted(classes, key=lambda class_: (len(class_['parents']), class_['parents'] and class_['parents'][0] or '', class_['name']))  # put classes with no parents on first place

    for cls in classes:
        res.append(handle_class(cls))

    res.append(ENUM_STUB)

    for enum in sorted(enums, key=itemgetter('name')):
        res.append(handle_enum(enum))

    for function in sorted(groups['function'], key=itemgetter('name')):
        res.append(handle_function(function))

    with open(unicode(result_path, 'utf-8'), 'w') as f:
        f.write('\n\n\n'.join(res))
        f.write('\n')
