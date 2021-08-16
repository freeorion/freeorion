import freeOrionAIInterface as fo
from typing import Any

from freeorion_tools import chat_human
from freeorion_tools.chat_handler.base_chat_handler import ChatHandlerBase
from freeorion_tools.chat_handler.chat_formatter import ChatFormatter
from freeorion_tools.chat_handler.interpreter import DebugInterpreter
from freeorion_tools.chat_handler.shell_variable import ShellVariable


def _chat_printer(message: Any):
    return chat_human(message, send_to_logs=False)


debug_chat_handler_local_vars = _shell_locals = [
    ShellVariable(
        variable="fo",
        expression="freeOrionAIInterface",
        description="fo",
        imports=("import freeOrionAIInterface",),
    ),
    ShellVariable(
        variable="ai",
        expression="get_aistate()",
        description="aistate",
        imports=("from aistate_interface import get_aistate",),
    ),
    ShellVariable(
        variable="u",
        expression="fo.getUniverse()",
        description="universe",
    ),
    ShellVariable(
        variable="e",
        expression="fo.getEmpire()",
        description="empire",
    ),
]


class Provider:
    def get_player_ids(self):
        return fo.allPlayerIDs()

    def is_host(self, player_id):
        return fo.playerIsHost(player_id)

    def player_id(self):
        return fo.playerID()

    def player_name(self, player_id):
        return fo.playerName(player_id)

    def get_empire_id(self, player_id):
        empire = fo.getEmpire(fo.playerEmpireID(player_id))
        return empire.empireID

    def get_empire_name(self, player_id):
        empire = fo.getEmpire(fo.playerEmpireID(player_id))
        return empire.name

    def get_empire_color(self, player_id):
        empire = fo.getEmpire(fo.playerEmpireID(player_id))
        return empire.colour


class DebugChatHandler(ChatHandlerBase):
    """
    Handle console messages to provide debug chat.

    This handler allows executing arbitrary python code for the AIs players.
    Mainly it is supposed to be used for debugging purposed, but can be used for cheating too.
    """

    _entering_debug_message = "Entering debug mode"
    _exit_debug_message = "Exiting debug mode"

    def __init__(
        self,
        empire_id: int,
        chat_printer=_chat_printer,
        log_printer=print,
        formatter=ChatFormatter(),
        shell_locals=None,
        provider=None,
    ):
        if not shell_locals:
            shell_locals = debug_chat_handler_local_vars

        if not provider:
            provider = Provider()
        self._provider = provider
        self._shell_locals = shell_locals
        self._formatter = formatter
        self._empire_id = empire_id
        self._debug_mode = False
        self._interpreter = DebugInterpreter(self._shell_locals)
        self._log_printer = log_printer
        self._chat_printer = chat_printer

    def send_notification(
        self,
        chat_message: Any = None,
        log_message: Any = None,
    ):
        """Handle user notifications wia logs and chat."""
        if chat_message is not None:
            self._chat_printer(chat_message)
        if log_message is not None:
            self._log_printer(log_message)

    def process_message(self, sender_id: int, message: str) -> bool:
        host_id = [x for x in self._provider.get_player_ids() if self._provider.is_host(x)][0]

        if sender_id != host_id:
            return False  # only user can send debug message

        if self._debug_mode:
            return self._handle_debug_commands(message)
        else:
            return self._handle_chat_commands(message)

    def _handle_chat_commands(self, message):
        if message.startswith("start"):
            self._handle_start(message)
        elif message == "help" and not self._debug_mode:
            self._handle_help()
        elif message == "stop":
            pass
        else:
            return False
        return True

    def _handle_debug_commands(self, message):
        if message == "stop":
            self._handle_stop()
        else:
            self._handle_shell_input(message)
        return True

    # message handlers
    def _handle_stop(self):
        self.send_notification(
            chat_message=self._exit_debug_message,
            log_message=self._exit_debug_message,
        )
        self._debug_mode = False

    def _handle_shell_input(self, message):
        """Handle message and prints it to chat and logs."""
        self.send_notification(log_message=f">>> {message}")
        out, err = self._interpreter.eval(message)
        if out:
            self.send_notification(
                chat_message=self._formatter.white(out),
                log_message=out,
            )
        if err:
            self.send_notification(
                chat_message=self._formatter.red(err),
                log_message=err,
            )

    def _handle_start(self, message):
        try:
            player_id = int(message[5:].strip())
        except ValueError:
            if self._ai_should_respond():
                text = "Invalid empire id, please input valid number"
                self.send_notification(
                    chat_message=self._formatter.red(text),
                )
                self._handle_help()
            return
        # This message is not for that AI
        if player_id != self._provider.player_id():
            return
        self._start_debug_mode()

    def _start_debug_mode(self):
        self._debug_mode = True

        start_message = [
            self._entering_debug_message,
            "Print %s to exit." % self._formatter.blue("stop"),
            "Local variables:",
        ]

        variables_description = self._interpreter.set_locals()
        for var, name in variables_description:
            start_message.append(
                "  %s%s%s"
                % (
                    self._formatter.underline(self._formatter.yellow(var)),
                    " " * (3 - len(var)),  # cannot use normal formatting because of tags
                    name,
                )
            )
        self.send_notification(chat_message="\n".join(self._formatter.white(x) for x in start_message))

    def _ai_should_respond(self):
        """
        Return true if AI should respond to message.

        To avoid chat pollution only one AI should respond to the message.
        AI player with the smallest id is selected.
        """
        ais = [x for x in self._provider.get_player_ids() if not self._provider.is_host(x)]
        return ais[0] == self._provider.player_id()

    def _handle_help(self):
        if not self._ai_should_respond():
            return

        help_message = [
            self._formatter.white("Chat commands:"),
            self._formatter.white(
                "  "
                + self._formatter.underline(self._formatter.blue("start <id>"))
                + ": start debug for selected empire"
            ),
        ]
        for player_id in self._provider.get_player_ids():
            if not self._provider.is_host(player_id):
                help_message.append(self._get_empire_string(player_id))
        self._formatter.white("  " + self._formatter.underline(self._formatter.blue("stop")) + ": stop debug"),
        self.send_notification(chat_message="\n".join(help_message))

    def _get_empire_string(self, player_id):
        empire_id = self._provider.get_empire_id(player_id)
        name = self._provider.get_empire_name(player_id)
        color = self._provider.get_empire_color(player_id)

        return (
            "    "
            + self._formatter.underline(self._formatter.blue(empire_id))
            + self._formatter.colored(color, " %s" % name)
            + self._formatter.white(" by %s" % self._provider.player_name(player_id))
        )
