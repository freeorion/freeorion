import re
from collections.abc import Iterator
from logging import error

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
        names.append(f"{arg_name}{suffix}")
    if is_class:
        names.insert(0, "self")
    return names, types


def _normalize_rtype(rtype):
    if rtype == "iterator":
        return "Iterator"
    return rtype


def _parse_signature(line):
    expre = re.compile(r"(\w+)\((.*)\) -> (\w+)")
    name, args, rtype = expre.match(line).group(1, 2, 3)
    args = tuple(re.findall(r"\((\w+)\) *(\w+)", args))
    return name, args, rtype


class Docs:
    def __init__(self, text, indent, is_class=False):  # noqa: C901
        self.indent = indent
        self.is_class = is_class

        if not text:
            self._set_unknown()
            return

        self.text = text
        lines = [x.strip() for x in self.text.split("\n")]

        name, self.resources = self._get_resources(lines)
        args, rtypes, infos = zip(*self.resources)
        self.rtype = _normalize_rtype(self._get_final_rtype(name, rtypes))

        self._header = self._make_doclines(infos, name)

        self.args_sets = self._get_argument_sets(args, is_class)

    def _get_argument_sets(self, args, is_class):
        args_sets = []
        for argument_set in args:
            names, types = get_argument_names(argument_set, is_class)
            args_sets.append(list(zip(names, types)))
        return args_sets

    def _make_doclines(self, infos, name):
        """
        Make the Python docstring from multiple infos.

        Cut of first and last string if they are empty.
        We cant cut off all empty lines, because it can be inside docstring.
        """
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

        if len(doc_lines) > 1:
            error("Too many doclines for %s %s", name, doc_lines)

        return doc_lines

    def _get_final_rtype(self, name, rtypes):
        if len(set(rtypes)) != 1:
            error("[%s] Different rtypes", name)
        return rtypes[0]

    def _get_resources(self, lines):
        res = []
        name, args, rtype = _parse_signature(lines[0])
        res.append((args, rtype, []))
        for line in lines[1:]:
            if self._is_extra_signature(line, name):
                name, args, rtype = _parse_signature(line)
                res.append((args, rtype, []))
            else:
                res[-1][2].append(line)
        return name, res

    def _set_unknown(self):
        self.rtype = "unknown"
        self.args = ["*args"]
        self._header = ""

    def _is_extra_signature(self, line, name):
        return line.startswith(f"{name}(")

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
            args.extend(f"{arg_name}: {arg_type}" for arg_name, arg_type in arg_set[self.is_class :])
            yield self._format_arg_string(args)

    def get_doc_string(self):
        doc = []
        if self._header:
            doc.append('"""')
            doc.extend(self._header)
            doc.append('"""')

        return "\n".join("{}{}".format(" " * 4 * self.indent if x else "", x) for x in doc)
