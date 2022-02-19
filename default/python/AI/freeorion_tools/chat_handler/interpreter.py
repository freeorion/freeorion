import logging
import sys
from code import InteractiveInterpreter
from io import StringIO
from typing import List

from freeorion_tools.chat_handler.shell_variable import ShellVariable


class DebugInterpreter:
    def __init__(self, shell_locals: List[ShellVariable]):
        self._shell_locals = shell_locals
        self._interpreter = InteractiveInterpreter()

    def eval(self, msg):
        old_stdout = sys.stdout
        old_stderr = sys.stderr

        sys.stdout = StringIO()
        sys.stderr = StringIO()
        if msg.endswith(";"):
            msg = msg.replace(";", "\n")
        handler = logging.StreamHandler(sys.stdout)
        logging.getLogger().addHandler(handler)
        self._interpreter.runsource(msg)
        logging.getLogger().removeHandler(handler)

        sys.stdout.seek(0)
        out = sys.stdout.read()
        sys.stderr.seek(0)
        err = sys.stderr.read()
        sys.stdout = old_stdout
        sys.stderr = old_stderr
        return out.strip("\n"), err.strip("\n")

    def set_locals(self):
        initial_code = []

        variables_description = []
        for shellLocal in self._shell_locals:
            initial_code.extend(shellLocal.get_evaluation_command())
            result = shellLocal.name, shellLocal.description
            variables_description.append(result)
        for line in initial_code:
            self.eval(line)

        return variables_description
