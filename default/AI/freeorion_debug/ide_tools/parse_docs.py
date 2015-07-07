import re

normalization_dict = {'empire': 'empire_object',
                      'int': 'number',
                      'str': 'string',
                      'float': 'floating_number',
                      'object': 'obj',
                      'IntBoolMap': 'int_bool_map',
                      'IntDblMap': 'int_dbl_map',
                      'universeObject': 'base_object',
                      'meterType': 'meter_type',
                      'bool': 'boolean',
                      'StringVec': 'string_list',
                      'shipDesign': 'ship_design',
                      'universe': 'universe_object',
                      'researchQueue': 'research_queue',
                      'resPoolMap': 'res_pool',
                      'productionQueue': 'production_queue',
                      'diplomaticMessage': 'diplomatic_message',
                      'ship': 'ship_object',
                      'species': 'species_object',
                      'planetType': 'planet_type',
                      'system': 'system_object',
                      'tech': 'tech_object',
                      'list': 'item_list',
                      'planet': 'planet_object',
                      'partType': 'part_type',
                      'resPool': 'res_pool',
                      'researchQueueElement': 'research_queue_element',
                      'shipSlotType': 'ship_slot_type',
                      'sitrep': 'sitrep_object',
                      'IntIntMap': 'int_int_map',
                      'IntPairVec': 'int_pair_list',
                      'IntSet': 'int_set',
                      'IntSetSet': 'int_set_set',
                      'IntVec': 'int_list',
                      'IntVisibilityMap': 'int_visibility_map',
                      'ItemSpecVec': 'item_spec_vec',
                      'StringSet': 'string_set',
                      'VisibilityIntMap': 'visibility_int_map',
                      'buildingType': 'buildingType',
                      'productionQueueElement': 'production_queue_element',
                      'resourceType': 'resource_type',
                      'buildType': 'build_type',
                      'field': 'field',
                      'hullType': 'hull_type',
                      }


def normalize_name(tp):
    if not tp in normalization_dict:
        return 'arg'
    else:
        return normalization_dict[tp]


def get_argument_names(argument_types):
    counts = {}
    names = []
    arg_names = [normalize_name(tp) for tp in argument_types]
    for tp, arg_name in zip(argument_types, arg_names):
        if arg_names.count(arg_name) == 1:
            suffix = ''
        else:
            if arg_name in counts:
                counts[arg_name] += 1
            else:
                counts[arg_name] = 1
            suffix = str(counts[arg_name])
        names.append('%s%s' % (arg_name, suffix))
    return names


def parse_name(txt):
    match = re.match('\w+\((.*)\) -> (.+) :', txt)
    args, return_type = match.group(1, 2)
    args = [x.strip(' (').split(')') for x in args.split(',') if x]
    return [x[0] for x in args], return_type


def merge_args(arg_types):
    with_arguments = filter(None, arg_types)
    if len(with_arguments) > 1:
        keywords = True
        new_types = []
        for types in arg_types:
            new_types.extend(types)
    else:
        new_types = with_arguments and with_arguments[0]
        keywords = False
    argument_names = get_argument_names(new_types)
    if keywords:
        argument_strings = ['%s=None' % x for x in argument_names]
    else:
        argument_strings = argument_names[:]
    return argument_strings, zip(argument_names, new_types)


def normilize_rtype(rtype):
    if rtype == 'iterator':
        return 'iter'
    return rtype


def process_docstring(info):
    """
    Make linux and windows stabs look alike.
    """

    return info.replace('class ', '').replace('struct ', '').replace('enum ', '')\
        .replace('basic_string<char,std::char_traits<char>,std::allocator<char> >', 'string')\
        .replace(", ", ',')


class Docs(object):
    def __init__(self, text, indent):
        self.indent = indent

        if not text:
            self.rtype = 'unknown'
            self.args = ['*args']
            self.header = ''
            return

        self.text = text

        lines = filter(None, [x.strip() for x in self.text.split('\n')])

        is_header = True
        is_signature = False

        headers = []
        docs = []
        signatures = []

        for line in lines:
            if is_header:
                headers.append(line)
                docs.append([])
                is_header = False
            else:
                if is_signature:
                    signatures.append(line)
                    is_signature = False
                    is_header = True

                elif line == 'C++ signature :':
                    is_signature = True

                elif line:
                    docs[-1].append(line)
        res = []
        for name, doc, info in zip(headers, docs, signatures):
            arg_types, rtype = parse_name(name)
            # This signatures differes on linux and windows, just ignore them
            if name.startswith(('__delitem__', '__len__', '__iter__', '__getitem__', '__contains__', '__setitem__')):
                res.append((arg_types, rtype, 'platform dependant'))
            else:
                res.append((arg_types, rtype, info))

        self.resources = res
        arg_types, rtypes, infos = zip(*res)
        rtypes = set(rtypes)
        assert len(rtypes) == 1, "Different rtypes for: %s" % text
        self.rtype = normilize_rtype(rtypes.pop())

        if doc:
            doc.append('')
        self.header = doc + ['C++ signature%s:' % ('' if len(res) == 1 else 's')]
        self.header.extend('    %s' % process_docstring(info) for info in infos)

        # TODO dont ignore second implementation for arguments.
        _, args = merge_args(arg_types[:1])
        self.args = args

    def get_argument_string(self, is_class=False):
        return ', '.join([arg_name for arg_name, arg_type in self.args[is_class:]])

    def get_doc_string(self, is_class=False):
        doc = ['"""']
        doc.extend(self.header)
        if self.args:
            doc.append('')
        for arg_name, arg_type in self.args[is_class:]:
            doc.append(':param %s:' % arg_name)
            doc.append(':type %s: %s' % (arg_name, arg_type))

        doc.append(':rtype %s' % self.rtype)
        doc.append('"""')
        return '\n'.join('%s%s' % (' ' * 4 * self.indent, x) for x in doc)


if __name__ == '__main__':
    example1 = """getPlanet( (universe)arg1, (int)arg2) -> planet :\n    User defined docstring.\n\n    C++ signature :\n        class Planet const * getPlanet(class Universe,int)"""
    info = Docs(example1, 1)
    print "!!!"
    print info.get_argument_string()
    print "!!!"
    print info.get_doc_string()
