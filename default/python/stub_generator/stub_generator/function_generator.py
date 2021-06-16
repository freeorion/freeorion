from operator import attrgetter
from typing import List

from stub_generator.interface_inspector import FunctionInfo
from stub_generator.parse_docs import Docs
from stub_generator.stub_generator.base_generator import BaseGenerator


def _handle_function(fun: FunctionInfo):
    function = Docs(fun.doc, 1)
    return_annotation = ' -> %s' % function.rtype if function.rtype else ''
    docstring = function.get_doc_string()
    if docstring:
        docstring = '\n' + docstring
        end = ''
    else:
        end = ' ...'
    arg_strings = list(function.get_argument_strings())
    if len(arg_strings) == 1:
        yield 'def %s(%s)%s:%s%s' % (fun.name, arg_strings[0], return_annotation, docstring, end)
    else:
        for arg_string in arg_strings:
            yield '@overload\ndef %s(%s) %s: ...' % (fun.name, arg_string, return_annotation)

        yield 'def %s(*args)%s:%s%s' % (fun.name, return_annotation, docstring, end)


class FunctionGenerator(BaseGenerator):
    def __init__(self, functions: List[FunctionInfo]):
        super().__init__()
        for function in sorted(functions, key=attrgetter("name")):
            self.body.extend(_handle_function(function))
