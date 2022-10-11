from stub_generator.stub_generator.result_builder import Import, merge_imports, sort_imports, ResultBuilder
from io import StringIO


def test_merge_imports_same_imports_are_merged():
    imports = [
        Import("typing", ["Dict"]),
        Import("typing", ["List"]),
    ]
    result = merge_imports(imports)
    assert result == [Import("typing", ["Dict", "List"])]


def test_merge_imports_different_imports_are_not_merged():
    imports = [
        Import("collections", ["Dict"]),
        Import("typing", ["List"]),
    ]
    result = merge_imports(imports)
    assert result == imports


def test_sort_imports():
    imports = [
        Import("b", ["b", "a"]),
        Import("a", ["a"]),
    ]
    sort_imports(imports)
    assert imports == [
        Import("a", ["a"]),
        Import("b", ["a", "b"]),
    ]


expected_result = '''# header
from typing import Dict, List

from common.fo_typing import (
    BuildingId,
    EmpireId,
    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX,
)

A = 1
B = 2

class Foo: ...

class Foo(Enum): ...

def foo(): ...
def boo():
    """doc"""

def goo():
    """doc"""
'''


def test_resource_builder():
    builder = ResultBuilder("# header")
    builder.add_built_in_import(Import("typing", ["List", "Dict"]))
    builder.add_import(Import("common.fo_typing", ["EmpireId", "BuildingId", "X" * 50]))
    builder.add_extra_declaration("\nA = 1")
    builder.add_extra_declaration("B = 2")
    builder.add_classes(["class Foo: ..."])
    builder.add_enums(["class Foo(Enum): ..."])
    builder.add_functions(["def foo(): ...", 'def boo():\n    """doc"""\n', 'def goo():\n    """doc"""\n'])

    res = StringIO()
    builder.write(res)
    res.seek(0)

    assert res.read() == expected_result
