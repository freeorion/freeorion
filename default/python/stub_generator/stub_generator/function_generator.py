from operator import attrgetter
from typing import List

from stub_generator.interface_inspector import FunctionInfo
from stub_generator.parse_docs import Docs
from stub_generator.stub_generator.rtype import update_function_rtype


def _handle_function(fun: FunctionInfo):
    function = Docs(fun.doc, 1)
    return_annotation = " -> %s" % update_function_rtype(fun.name, function.rtype)
    docstring = function.get_doc_string()
    if docstring:
        docstring = f"\n{docstring}\n\n"
        end = ""
    else:
        end = " ...\n"
    arg_strings = list(function.get_argument_strings())
    if len(arg_strings) == 1:
        yield "def %s(%s)%s:%s%s" % (fun.name, arg_strings[0], return_annotation, docstring, end)
    else:
        for arg_string in arg_strings:
            yield "@overload\ndef %s(%s)%s: ...\n" % (fun.name, arg_string, return_annotation)

        yield "def %s(*args)%s:%s%s" % (fun.name, return_annotation, docstring, end)


class FunctionGenerator:
    def __init__(self, functions: List[FunctionInfo]):
        self._functions = functions

    def __iter__(self):
        super().__init__()
        for function in sorted(self._functions, key=attrgetter("name")):
            yield "".join(_handle_function(function))
