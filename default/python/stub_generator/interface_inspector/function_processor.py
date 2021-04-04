from inspect import getdoc


class FunctionInfo:
    def __init__(self, name: str, doc: str):
        self.name = name
        self.doc = doc


def inspect_function(name, value) -> FunctionInfo:
    return FunctionInfo(name, getdoc(value))
