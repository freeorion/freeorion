from logging import error, warning
from operator import itemgetter

from common.print_utils import Table, Text
from stub_generator.constants import ATTRS, CLASS_NAME, DOC, ENUM_PAIRS, NAME, PARENTS, TYPE
from stub_generator.parse_docs import Docs


def _handle_class(info):
    name = info[NAME]
    docs = info[DOC]
    attrs = info[ATTRS]
    assert not docs, "Got docs need to handle it"
    parents = info[PARENTS]
    if not parents:
        parents = ['object']  # instance is boost wrapper
    result = []

    if 'object' in parents:
        parents.remove('object')
    if parents:
        result.append('class %s(%s):' % (name, ', '.join(parents)))
    else:
        result.append('class %s:' % name)

    properties = []
    instance_methods = []
    for attr_name, attr in sorted(attrs.items()):
        if attr['type'] == "<class 'property'>":
            properties.append((attr_name, attr.get('rtype', '')))
        elif attr['type'] in ("<class 'Boost.Python.function'>", "<class 'function'>"):
            instance_methods.append(attr['routine'])
        else:
            warning("Skipping '%s' (%s): %s" % (name, attr['type'], attr))

    for property_name, rtype in properties:
        if not rtype:
            return_annotation = ''
        elif rtype.startswith("<type"):
            return_annotation = '-> %s' % rtype[7:-2]
        else:
            return_annotation = '-> %s' % rtype.split('.')[-1].strip("'>")

        if property_name == 'class':
            result.append('    # cant define it via python in that way')
            result.append('    # @property')
            result.append('    # def %s(self)%s: ' % (property_name, return_annotation))
            result.append('    #    ...')
        else:
            result.append('    @property')
            result.append('    def %s(self)%s:' % (property_name, return_annotation))
            result.append('        ...')
        result.append('')

    for routine_name, routine_docs in instance_methods:
        docs = Docs(routine_docs, 2, is_class=True)
        # TODO: Subclass map-like classes from dict (or custom class) rather than this hack

        if docs.rtype in ('VisibilityIntMap', 'IntIntMap'):
            docs.rtype = 'Dict[int, int]'

        doc_string = docs.get_doc_string()
        if doc_string:
            doc_string = '\n' + doc_string
            end = ''
        else:
            end = '\n        ...'
        result.append(
            '    def %s(%s) -> %s:%s%s' % (routine_name, docs.get_argument_string(), docs.rtype, doc_string, end))
        result.append('')
    if not (properties or instance_methods):
        result.append('    ...')
    if not result[-1]:
        result.pop()
    return '\n'.join(result)


def _handle_function(doc):
    name = doc[NAME]
    doc = Docs(doc[DOC], 1)
    return_annotation = ' -> %s' % doc.rtype if doc.rtype else ''
    docstring = doc.get_doc_string()
    if docstring:
        docstring = '\n' + docstring
        end = ''
    else:
        end = '\n    ...'
    res = 'def %s(%s) %s:%s%s' % (name, doc.get_argument_string(), return_annotation, docstring, end)
    return res


ENUM_STUB = ('class Enum(int):\n'
             '    """Enum stub for docs, not really present in fo"""\n'
             '    def __new__(cls, *args, **kwargs):\n'
             '        return super(Enum, cls).__new__(cls, args[0])')


def _handle_enum(info):
    name = info[NAME]
    enum_items = info[ENUM_PAIRS]
    result = ['class %s(Enum):' % name,
              '    def __init__(self, numerator, name):',
              '        self.name = name',
              ''
              ]
    pairs = sorted(enum_items)
    for value, text in pairs:
        result.append('    %s = None  # %s(%s, "%s")' % (text, name, value, text))
    result.append('')
    result.append('')  # two empty lines between enum and its items declaration

    for value, text in pairs:
        result.append('%s.%s = %s(%s, "%s")' % (name, text, name, value, text))
    return '\n'.join(result)


_KNOWN_TYPES = {
    'boost_class',
    'enum',
    'function',
    'instance',
}


def _sort_by_type(data):
    groups = {}
    for info in data:
        if info[TYPE] in _KNOWN_TYPES:
            groups.setdefault(info[TYPE], []).append(info)
        else:
            error('Unknown type "%s" in "%s' % (info[TYPE], info))

    return tuple(groups.get(name, []) for name in ('boost_class', 'enum', 'instance', 'function'))


def _report_classes_without_instances(classes_map, instance_names, classes_to_ignore: set):
    missed_instances = instance_names.symmetric_difference(classes_map).difference(classes_to_ignore)

    if not missed_instances:
        return

    warning("")
    warning(
        "In order to get more information about the classes in API we need to process an instances of thea classes.")
    warning("Classes mentioned bellow does not have instances so their specs are not full.")
    warning("Please provide instances or add them to ignored,")
    warning("check generate_stub usage in the freeorion/default/python/handlers folder.")
    warning("")

    table = Table([Text("classes")], )

    for inst in sorted(missed_instances, key=str.lower):
        table.add_row((str(inst),))
    warning(table.get_table())


def make_stub(data, result_path, classes_to_ignore: set):
    classes, enums, instances, functions = _sort_by_type(data)
    # exclude technical Map classes that are prefixed with map_indexing_suite_ classes
    classes = [x for x in classes if not x[NAME].startswith('map_indexing_suite_')]
    classes_map = {x[NAME]: x for x in classes}

    _report_classes_without_instances(
        classes_map,
        {instance[CLASS_NAME] for instance in instances},
        classes_to_ignore)

    enums = sorted(enums, key=itemgetter(NAME))
    enums_names = [x[NAME] for x in enums]

    # enrich class data with the instance data
    # class properties does not provide any useful info, so we use instance to find return type of the properties
    for instance in instances:
        class_name = instance[CLASS_NAME]
        if class_name in enums_names:
            warning("skipping enum instance: %s" % class_name)
            continue
        try:
            class_attrs = classes_map[class_name][ATTRS]
        except KeyError:
            error("Instance class was not found in classes generated by C++ interface,"
                  " please check instance at %s with class: %s" % (instance["location"], class_name,))
            continue

        instance_attrs = instance[ATTRS]

        for attribute_name, class_attibute in class_attrs.items():
            inst = instance_attrs.get(attribute_name)
            if not inst:
                continue
            type_ = class_attibute['type']

            if type_ in ("<class 'Boost.Python.function'>", "<class 'function'>"):
                continue

            if type_ == "<class 'property'>":
                assert class_attibute['getter'] is None  # if we will have docs here, handle them
                class_attibute['rtype'] = inst['type'][8:-2]  # "<class 'str'>" - > str TODO extract to function
                continue
            error("Unknown class attribute type: '%s' for %s.%s: %s" % (
                type_, class_name, attribute_name, class_attibute))

    res = [
        '# Autogenerated do not modify manually!\n'
        '# This is a type-hinting python stub file, used by python IDEs to provide type hints. For more information\n'
        '# about stub files, see https://www.python.org/dev/peps/pep-0484/#stub-files\n'
        '# During execution, the actual module is made available via\n'
        '# a C++ Boost-python process as part of the launch.\n'
        'from typing import Dict'
    ]
    classes = sorted(classes, key=lambda class_: (
        len(class_[PARENTS]), class_[PARENTS] and class_[PARENTS][0] or '',
        class_[NAME]))  # put classes with no parents on first place

    for cls in classes:
        res.append(_handle_class(cls))

    res.append(ENUM_STUB)

    for enum in sorted(enums, key=itemgetter(NAME)):
        res.append(_handle_enum(enum))

    for function in sorted(functions, key=itemgetter(NAME)):
        res.append(_handle_function(function))

    with open(result_path, 'w') as f:
        f.write('\n\n\n'.join(res))
        f.write('\n')
