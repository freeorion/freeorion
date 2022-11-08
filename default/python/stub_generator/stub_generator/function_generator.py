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
        docstring = f"\n{docstring}\n"
        end = ""
    else:
        end = " ..."
    arg_strings = list(function.get_argument_strings())
    if len(arg_strings) == 1:
        yield f"def {fun.name}({arg_strings[0]}){return_annotation}:{docstring}{end}"
    else:
        for arg_string in arg_strings:
            yield f"@overload\ndef {fun.name}({arg_string}){return_annotation}: ...\n"

        yield f"def {fun.name}(*args){return_annotation}:{docstring}{end}"


def generate_functions(functions: List[FunctionInfo]):
    for function in sorted(functions, key=attrgetter("name")):
        yield "".join(_handle_function(function))
