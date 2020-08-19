import freeOrionAIInterface as fo
from freeorion_tools import chat_human
from freeorion_tools.chat_handler.chat_formatter import ChatFormatter
from freeorion_tools.chat_handler.base_chat_handler import ChatHandlerBase
from freeorion_tools.chat_handler.interpreter import DebugInterpreter
from freeorion_tools.chat_handler.shell_variable import ShellVariable


class DebugChatHandler(ChatHandlerBase):
    """
    Handle console messages to provide debug chat.

    This handler allows executing arbitrary python code for the AIs players.
    Mainly it is supposed to be used for debugging purposed, but can be used for cheating too.
    """
    _entering_debug_message = 'Entering debug mode'
    _exit_debug_message = 'Exiting debug mode'

    _shell_locals = [
        ShellVariable(
            variable="fo",
            expression="freeOrionAIInterface",
            description="fo",
            imports=('import freeOrionAIInterface', ),
        ),
        ShellVariable(
            variable='ai',
            expression='get_aistate()',
            description='aistate',
            imports=("from aistate_interface import get_aistate",),
        ),
        ShellVariable(
            variable='u',
            expression='fo.getUniverse()',
            description='universe',
        ),
        ShellVariable(
            variable='e',
            expression='fo.getEmpire()',
            description='empire',
        ),
    ]

    def __init__(self, empire_id):
        self._formatter = ChatFormatter()
        self._empire_id = empire_id
        self._debug_mode = False
        self._interpreter = DebugInterpreter(self._shell_locals)

    def process_message(self, sender_id: int, message: str) -> bool:
        host_id = [x for x in fo.allPlayerIDs() if
                   fo.playerIsHost(x)][0]

        if sender_id != host_id:
            return False  # only user can send debug message

        if self._debug_mode:
            return self._handle_debug_commands(message)
        else:
            return self._handle_chat_commands(message)

    def _handle_chat_commands(self, message):
        if message.startswith('start'):
            self._handle_start(message)
        elif message == 'help' and not self._debug_mode:
            self._handle_help()
        elif message == 'stop':
            pass
        else:
            return False
        return True

    def _handle_debug_commands(self, message):
        if message == 'stop':
            self._handle_stop()
        else:
            self._handle_shell_input(message)
        return True

    # message handlers
    def _handle_stop(self):
        chat_human(self._exit_debug_message)
        self._debug_mode = False

    def _handle_shell_input(self, message):
        print('>', message, end='')
        out, err = self._interpreter.eval(message)
        if out:
            chat_human(self._formatter.white(out))
        if err:
            chat_human(self._formatter.red(err))

    def _handle_start(self, message):
        try:
            player_id = int(message[5:].strip())
        except ValueError:
            if self._is_first_ai():
                message = self._formatter.red("Invalid empire id, please input valid number")
                chat_human(message)
                self._handle_help()
            return
        # This message is not for that AI
        if player_id != fo.playerID():
            return
        self._start_debug_mode()

    def _start_debug_mode(self):
        self._debug_mode = True

        start_message = [
            self._entering_debug_message,
            "Print %s to exit." % self._formatter.blue('stop'),
            "Local variables:",
        ]

        variables_description = self._interpreter.set_locals()
        for var, name in variables_description:
            start_message.append(
                '  %s%s%s' % (
                    self._formatter.underline(self._formatter.yellow(var)),
                    ' ' * (3 - len(var)),  # cannot use normal formatting because of tags
                    name
                )
            )
        chat_human("\n".join(self._formatter.white(x) for x in start_message))

    @staticmethod
    def _is_first_ai():
        """
        Return true first AI handles message.
        :return:
        """
        ais = [x for x in fo.allPlayerIDs() if
               not fo.playerIsHost(x)]
        return ais[0] == fo.playerID()

    def _handle_help(self):
        if not self._is_first_ai():
            return

        help_message = [
            self._formatter.white("Chat commands:"),
            self._formatter.white(
                "  " + self._formatter.underline(
                    self._formatter.blue('start <id>')) + ": start debug for selected empire"),
        ]
        for player_id in fo.allPlayerIDs():
            if not fo.playerIsHost(player_id):

                help_message.append(
                    self._get_empire_string(player_id)
                )
        self._formatter.white(
            "  " + self._formatter.underline(self._formatter.blue('stop')) + ": stop debug"),
        chat_human('\n'.join(help_message))

    def _get_empire_string(self, player_id):
        empire = fo.getEmpire(fo.playerEmpireID(player_id))

        empire_color = (empire.colour.r, empire.colour.g, empire.colour.b, empire.colour.a)

        return (
                "    " +
                self._formatter.underline(self._formatter.blue(empire.empireID)) +
                self._formatter.colored(empire_color, " %s" % empire.name) +
                self._formatter.white(
                    ' by %s' % fo.playerName(player_id))
        )
