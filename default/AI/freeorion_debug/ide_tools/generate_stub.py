from itertools import groupby
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
    for attr_name, attr in attrs.items():
        if attr['type'] == "<type 'property'>":
            properties.append((attr_name, attr.get('rtype', '')))
        elif attr['type'] == "<type 'instancemethod'>":
            instance_methods.append(attr['rutine'])
        else:
            print "!!!", name, attr

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

    for rutine_name, rutine_docs in instance_methods:
        if rutine_name == 'error_stub':
            continue

        docs = Docs(rutine_docs, 2)

        if docs.rtype == 'VisibilityIntMap':
            return_string = 'return dict()'
        elif docs.rtype == 'None':
            return_string = 'return None'
        else:
            return_string = 'return %s()' % docs.rtype

        argument_string = docs.get_argument_string(is_class=True)
        if argument_string:
            argument_string = ', ' + argument_string

        doc_string = docs.get_doc_string(is_class=True)
        result.append('    def %s(self%s):' % (rutine_name, argument_string))
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


ENUM_STUB = 'class Enum(int):\n    """Enum stub for docs, not really present in fo"""\n    pass'


def handle_enum(info):
    name = info['name']
    enum_dicts = info['enum_dicts']
    result = ['class %s(Enum):' % name,
              '    def __init__(self, numerator, name):',
              '        self.name = name',
              '        self.numerator = numerator',
              ''
              ]

    for _, (value, text) in sorted(enum_dicts.items()):
        result.append('    %s = None  # %s(%s, "%s")' % (text, name, value, text))
    result.append('')

    for _, (value, text) in sorted(enum_dicts.items(), key=lambda x: x[1][0]):
        result.append('%s.%s = %s(%s, "%s")' % (name, text, name, value, text))
    return '\n'.join(result)

known_types = {'boost_class',
               'enum',
               'function',
               'instance'}


def make_stub(data, result_path):
    groups = {}
    for info in data:
        if info['type'] in known_types:
            groups.setdefault(info['type'], []).append(info)
        else:
            print info['type']
    classes = [x for x in groups['boost_class'] if not x['name'].startswith('map_indexing_suite_')]
    clases_map = {x['name']: x for x in classes}
    instance_names = {instance['class_name'] for instance in groups.get('instance', [])}

    enums = sorted(groups['enum'], key=itemgetter('name'))
    enums_names = [x['name'] for x in enums]

    excludes = ([u'IntSet', u'StringSet', u'IntIntMap', u'ShipSlotVec',  u'VisibilityIntMap', u'IntDblMap',
                 u'IntBoolMap', u'ItemSpecVec', u'PairIntInt_IntMap', u'IntSetSet', u'StringVec',
                 u'IntPairVec'])

    print "Classes with out instance", instance_names.symmetric_difference(clases_map).difference(excludes)

    for instance in groups.get('instance', []):
        class_name = instance['class_name']
        if class_name in enums_names:
            print "skipping enum instance: %s" % class_name
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
                print "Unknown class attribute type", v['type']

    res = []
    classes = sorted(classes, key=lambda x:len(x['parents']))  # put classes with no parents on first place
    class_groups = groupby(classes, key=lambda x:  x['parents'] and x['parents'][0] or '')

    for name, group in class_groups:
        for cls in sorted(group, key=itemgetter('name')):
            res.append(handle_class(cls))

    res.append(ENUM_STUB)

    for enum in enums:
        res.append(handle_enum(enum))

    for function in sorted(groups['function'], key=itemgetter('name')):
        res.append(handle_function(function))

    with open(result_path, 'w') as f:
        f.write('\n\n\n'.join(res))
        f.write('\n')
