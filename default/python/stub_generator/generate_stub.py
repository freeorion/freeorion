from logging import error, warning
from operator import attrgetter, itemgetter
from typing import Iterable, List, Set

from common.print_utils import Table, Text
from stub_generator.interface_inspector import ClassInfo, EnumInfo, FunctionInfo, InstanceInfo
from stub_generator.parse_docs import Docs


def _handle_class(info: ClassInfo):
    assert not info.doc, "Got docs need to handle it"
    parents = [x for x in info.parents if x != 'object']

    result = []
    if parents:
        result.append('class %s(%s):' % (info.name, ', '.join(info.parents)))
    else:
        result.append('class %s:' % info.name)

    properties = []
    instance_methods = []
    for attr_name, attr in sorted(info.attributes.items()):
        if attr['type'] == "<class 'property'>":
            properties.append((attr_name, attr.get('rtype', '')))
        elif attr['type'] in ("<class 'Boost.Python.function'>", "<class 'function'>"):
            instance_methods.append(attr['routine'])
        else:
            warning("Skipping '%s' (%s): %s" % ((info.name), attr['type'], attr))

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


def _handle_function(fun: FunctionInfo):
    name = fun.name
    function = Docs(fun.doc, 1)
    return_annotation = ' -> %s' % function.rtype if function.rtype else ''
    docstring = function.get_doc_string()
    if docstring:
        docstring = '\n' + docstring
        end = ''
    else:
        end = '\n    ...'
    res = 'def %s(%s) %s:%s%s' % (name, function.get_argument_string(), return_annotation, docstring, end)
    return res


ENUM_STUB = ('class Enum(int):\n'
             '    """Enum stub for docs, not really present in fo"""\n'
             '    def __new__(cls, *args, **kwargs):\n'
             '        return super(Enum, cls).__new__(cls, args[0])')


def _handle_enum(info: EnumInfo):
    name = info.name
    pairs = sorted(info.attributes.items(), key=itemgetter(1))
    result = ['class %s(Enum):' % name,
              '    def __init__(self, numerator, name):',
              '        self.name = name',
              ''
              ]

    for text, value in pairs:
        result.append('    %s = None  # %s(%s, "%s")' % (text, name, value, text))
    result.append('')
    result.append('')  # two empty lines between enum and its items declaration

    for text, value in pairs:
        result.append('%s.%s = %s(%s, "%s")' % (name, text, name, value, text))
    return '\n'.join(result)


def _report_classes_without_instances(classes_map: Iterable[str], instance_names, classes_to_ignore: Set[str]):
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


def make_stub(classes: List[ClassInfo], enums: List[EnumInfo], functions: List[FunctionInfo], instances: List[InstanceInfo], result_path, classes_to_ignore: set):

    # exclude technical Map classes that are prefixed with map_indexing_suite_ classes
    classes = [x for x in classes if not x.name.startswith('map_indexing_suite_')]
    classes_map = {x.name: x for x in classes}

    _report_classes_without_instances(
        classes_map.keys(),
        {instance.class_name for instance in instances},
        classes_to_ignore)


    enums_names = {x.name for x in enums}

    # enrich class data with the instance data
    # class properties does not provide any useful info, so we use instance to find return type of the properties
    for instance in instances:
        if instance.class_name in enums_names:
            warning("skipping enum instance: %s" % instance.class_name)
            continue
        try:
            class_attrs = classes_map[instance.class_name].attributes
        except KeyError:
            error("Instance class was not found in classes generated by C++ interface,"
                  " please check instance at %s with class: %s" % (instance["location"], (instance.class_name),))
            continue

        for attribute_name, class_attibute in class_attrs.items():
            inst = instance.attributes.get(attribute_name)
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
                type_, instance.class_name, attribute_name, class_attibute))

    res = [
        '# Autogenerated do not modify manually!\n'
        '# This is a type-hinting python stub file, used by python IDEs to provide type hints. For more information\n'
        '# about stub files, see https://www.python.org/dev/peps/pep-0484/#stub-files\n'
        '# During execution, the actual module is made available via\n'
        '# a C++ Boost-python process as part of the launch.\n'
        'from typing import Dict'
    ]
    classes = sorted(classes, key=lambda class_: (
        len(class_.parents), class_.parents and class_.parents[0] or '',
        class_.name))  # put classes with no parents on first place

    for cls in classes:
        res.append(_handle_class(cls))

    res.append(ENUM_STUB)

    for enum in sorted(enums, key=attrgetter("name")):
        res.append(_handle_enum(enum))

    for function in sorted(functions, key=attrgetter("name")):
        res.append(_handle_function(function))

    with open(result_path, 'w') as f:
        f.write('\n\n\n'.join(res))
        f.write('\n')
