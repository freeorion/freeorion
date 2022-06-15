from operator import attrgetter
from typing import List

from common.fo_typing import EmpireId, Turn
from stub_generator.interface_inspector import FunctionInfo
from stub_generator.parse_docs import Docs
from stub_generator.stub_generator.base_generator import BaseGenerator

rtypes_map = {
    "currentTurn": Turn.__name__,
    "empireID": EmpireId.__name__,
    "allEmpireIDs": f"Sequence[{EmpireId.__name__}]",
}


def _get_function_rtype(name: str, rtype: str) -> str:
    if name in rtypes_map:
        return rtypes_map[name]
    return rtype if rtype else ""


def _handle_function(fun: FunctionInfo):
    function = Docs(fun.doc, 1)
    return_annotation = " -> %s" % _get_function_rtype(fun.name, function.rtype)
    docstring = function.get_doc_string()
    if docstring:
        docstring = "\n" + docstring + "\n"
        end = ""
    else:
        end = " ..."
    arg_strings = list(function.get_argument_strings())
    if len(arg_strings) == 1:
        yield "def %s(%s)%s:%s%s" % (fun.name, arg_strings[0], return_annotation, docstring, end)
    else:
        for arg_string in arg_strings:
            yield "@overload\ndef %s(%s)%s: ..." % (fun.name, arg_string, return_annotation)

        yield "def %s(*args)%s:%s%s" % (fun.name, return_annotation, docstring, end)


class FunctionGenerator(BaseGenerator):
    def __init__(self, functions: List[FunctionInfo]):
        super().__init__()
        for function in sorted(functions, key=attrgetter("name")):
            self.body.extend(_handle_function(function))
