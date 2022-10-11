from typing import List, TextIO, Iterable

from dataclasses import dataclass


@dataclass
class Import:
    from_: str
    items: List[str]


def merge_imports(imports):
    import_map = {}
    for i in imports:
        if i.from_ in import_map:
            import_map[i.from_].items.extend(i.items)
        else:
            import_map[i.from_] = i
    return list(import_map.values())


def sort_imports(imports):
    imports.sort(key=lambda x: x.from_)
    for import_ in imports:
        import_.items.sort()


class ResultBuilder:
    def __init__(self, docstring: str):
        self._docstring = docstring
        self._built_in_imports = []
        self._imports = []
        self._extra_declaration = []
        self._classes = []
        self._enums = []
        self._functions = []

    def add_built_in_import(self, import_: Import):
        self._built_in_imports.append(import_)

    def add_import(self, import_: Import):
        self._imports.append(import_)

    def add_extra_declaration(self, declaration: str):
        self._extra_declaration.append(declaration)

    def add_classes(self, classes: Iterable[str]):
        self._classes.extend(classes)

    def add_enums(self, enums: Iterable[str]):
        self._enums.extend(enums)

    def add_functions(self, functions: Iterable[str]):
        self._functions.extend(functions)

    def _import_string(self, import_: Import):
        return f"from {import_.from_} import {', '.join(import_.items)}"

    def _write_imports(self, file: TextIO, imports: List[Import]):
        imports = merge_imports(imports)
        sort_imports(imports)
        for i in imports:
            oneliner = self._import_string(i)
            if len(oneliner) <= 100:
                file.write(oneliner)
                file.write("\n")
            else:
                file.write(f"from {i.from_} import (\n")
                for item in i.items:
                    file.write(f"    {item},\n")
                file.write(")\n")

    def write(self, file: TextIO):
        file.write(self._docstring)
        file.write("\n")
        self._write_imports(file, self._built_in_imports)
        file.write("\n")
        self._write_imports(file, self._imports)

        for declaraion in self._extra_declaration:
            file.write(declaraion)
            file.write("\n")

        for cls in self._classes:
            file.write("\n")
            file.write(cls)
            file.write("\n")

        for enum in self._enums:
            file.write("\n")
            file.write(enum)
            file.write("\n")

        file.write("\n")
        for function in self._functions[:-1]:
            file.write(function)
            file.write("\n")

        last_function = self._functions[-1]
        file.write(last_function)
        if not last_function.endswith("\n"):
            file.write("\n")
