import re
from logging import error
from typing import Iterator

from stub_generator.stub_generator import normalize_name


def get_argument_names(arguments, is_class):
    counts = {}
    names = []

    types = [x[0] for x in arguments]

    if is_class:
        arguments = arguments[1:]

    arg_names = [normalize_name(tp) for tp in arguments]
    for tp, arg_name in zip(arguments, arg_names):
        if arg_names.count(arg_name) == 1:
            suffix = ""
        else:
            if arg_name in counts:
                counts[arg_name] += 1
            else:
                counts[arg_name] = 1
            suffix = str(counts[arg_name])
        names.append("%s%s" % (arg_name, suffix))
    if is_class:
        names.insert(0, "self")
    return names, types


def normalize_rtype(rtype):
    if rtype == "iterator":
        return "Iterator"
    return rtype


class Docs:
    def __init__(self, text, indent, is_class=False):
        self.indent = indent
        self.is_class = is_class

        if not text:
            self.rtype = "unknown"
            self.args = ["*args"]
            self.header = ""
            return

        self.text = text

        lines = [x.strip() for x in self.text.split("\n")]

        def parse_signature(line):
            expre = re.compile(r"(\w+)\((.*)\) -> (\w+)")
            name, args, rtype = expre.match(line).group(1, 2, 3)
            args = tuple(re.findall(r"\((\w+)\) *(\w+)", args))
            return name, args, rtype

        res = []
        name, args, rtype = parse_signature(lines[0])
        res.append((args, rtype, []))
        for line in lines[1:]:
            if line.startswith("%s(" % name):
                name, args, rtype = parse_signature(line)
                res.append((args, rtype, []))
            else:
                res[-1][2].append(line)

        self.resources = res

        args, rtypes, infos = zip(*res)
        if len(set(rtypes)) != 1:
            error("[%s] Different rtypes", name)
        self.rtype = normalize_rtype(rtypes[0])

        # cut of first and last string if they are empty
        # we cant cut off all empty lines, because it can be inside docstring
        doc_lines = []

        for doc_part in infos:
            if not doc_part:
                continue
            else:
                # cut first and last empty strings
                if not doc_part[0]:
                    doc_part = doc_part[1:]
                if not doc_part:
                    continue
                if not doc_part[-1]:
                    doc_part = doc_part[:-1]
                doc_lines.append("\n".join(doc_part))

        # if docs are equals show only one of them
        self.header = sorted(doc_lines)

        self.args_sets = []

        for argument_set in args:
            names, types = get_argument_names(argument_set, is_class)
            self.args_sets.append(list(zip(names, types)))

    def _format_arg_string(self, args):
        arg_string = ", ".join(args)

        # Black limit is 120, this includes
        # function definition including name and return type
        # Lets guess the best option
        # Adding trailing coma will stop black from reformatting lines,
        # so in case of long names we could just reduce this number.
        arg_size = 90

        if len(arg_string) <= arg_size:
            return arg_string
        else:
            if self.is_class:
                indent = " " * 8
            else:
                indent = " " * 4
            result = ["\n"]
            for arg in args:
                result.append(f"{indent}{arg},\n")
            result.append(indent[:-4])
            return "".join(result)

    def get_argument_strings(self) -> Iterator[str]:
        for arg_set in self.args_sets:
            if self.is_class:
                args = ["self"]
            else:
                args = []
            args.extend("%s: %s" % (arg_name, arg_type) for arg_name, arg_type in arg_set[self.is_class :])
            yield self._format_arg_string(args)

    def get_doc_string(self):
        doc = []
        if self.header:
            doc.append('"""')
            if self.header:
                doc.extend(self.header)
            doc.append('"""')

        return "\n".join("%s%s" % (" " * 4 * self.indent if x else "", x) for x in doc)


if __name__ == "__main__":
    # example1 = """__delitem__( (IntBoolMap)arg1, (object)arg2) -> None"""
    example1 = """getEmpire() -> empire\n\ngetEmpire((int)star_name, (int)arg2, (int)arg3) -> empire"""

    # example1 = ("""getUserDataDir() -> str :\n
    #     Returns path to directory where FreeOrion stores user specific data (saves, etc.).
    #
    #     getUserDataDir((int)args1) -> str :\n
    #         Unicorns.
    #     """)
    #
    # example1 = """getUserDataDir() -> str :\n    Returns path to directory where FreeOrion stores user specific data (config files, saves, etc.)."""

    info = Docs(example1, 1)
    print("=" * 100)
    print("Arg string:", info.get_argument_string())
    print("=" * 100)
    print("Doc string:\n", info.get_doc_string())

    # double standards
    # canBuild ['empire', 'buildType', 'str', 'int'], ['empire', 'buildType', 'int', 'int']
    # inField ['field', 'universeObject'], ['field', 'float', 'float']
    # validShipDesign ['str', 'StringVec']
