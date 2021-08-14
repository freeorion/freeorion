class ShellVariable:
    """
    Variable initialization and description for an interpreter.
    """

    def __init__(self, *, variable: str, expression: str, description: str, imports=tuple()):
        self.name = variable
        self.expression = expression
        self.description = description
        self.imports = imports

    def get_evaluation_command(self) -> str:
        yield from self.imports
        yield "%s = %s" % (self.name, self.expression)
