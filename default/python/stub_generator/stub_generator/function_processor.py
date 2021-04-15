from operator import attrgetter
from typing import List

from stub_generator.interface_inspector import FunctionInfo
from stub_generator.parse_docs import Docs
from stub_generator.stub_generator.processor import BaseProcessor


def _handle_function(fun: FunctionInfo):
    function = Docs(fun.doc, 1)
    return_annotation = ' -> %s' % function.rtype if function.rtype else ''
    docstring = function.get_doc_string()
    if docstring:
        docstring = '\n' + docstring
        end = ''
    else:
        end = '\n    ...'
    res = 'def %s(%s) %s:%s%s' % (fun.name, function.get_argument_string(), return_annotation, docstring, end)
    return res


class FunctionProcessor(BaseProcessor):
    def __init__(self, functions: List[FunctionInfo]):
        super().__init__()
        self.functions = functions

    def _process(self):
        for function in sorted(self.functions, key=attrgetter("name")):
            self.body.append(_handle_function(function))
