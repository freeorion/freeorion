from io import StringIO
from typing import Any, Tuple

# Tests classes
import pytest
from freeorion_tools.chat_handler.chat_formatter import ChatFormatter
from freeorion_tools.chat_handler.debug_chat_handler import DebugChatHandler
from freeorion_tools.chat_handler.shell_variable import ShellVariable


class Printer:
    def __init__(self):
        self.input = StringIO()

    def __call__(self, message):
        self.input.write(str(message))
        self.input.write("\n")


class DummyProvider:
    def __init__(self, player_id):
        self._player_id = player_id

    def get_player_ids(self):
        return [1, 2, 3]

    def is_host(self, player_id):
        return player_id == 1

    def player_id(self):
        return self._player_id

    def player_name(self, player_id):
        return "Player_%s" % player_id

    def get_empire_id(self, player_id):
        return player_id

    def get_empire_name(self, player_id):
        return "Empire_%s" % player_id

    def get_empire_color(self, player_id):
        return 0, 0, 0, 0


class DummyFormatter(ChatFormatter):
    def _get_tag_rgb(self, color: (int, int, int, int)) -> (str, str):
        return "", ""

    def _get_underline_tag(self) -> (str, str):
        return "", ""

    def _wrap_to_tag(self, tag: str, message: Any) -> str:
        return str(message)


@pytest.fixture
def debug_handler():
    _shell_locals = [
        ShellVariable(
            variable="variable_a",
            expression="'value_a'",
            description="description for a",
        ),
        ShellVariable(
            variable="variable_b",
            expression="'value_b'",
            description="description for b",
        ),
        ShellVariable(
            variable="variable_c",
            expression="'value_c'",
            description="variable with import",
            imports=("from glob import has_magic",),
        ),
        ShellVariable(
            variable="variable_a_plus_b",
            expression="variable_a + variable_b",
            description="description for a+b",
        ),
    ]

    provider = DummyProvider(2)
    return DebugChatHandler(
        2,
        provider=provider,
        shell_locals=_shell_locals,
        formatter=DummyFormatter(),
        chat_printer=Printer(),
        log_printer=Printer(),
    )


@pytest.fixture
def debug_handler_for_muted_ai():
    provider = DummyProvider(3)
    chat_printer = Printer()
    return chat_printer.input, DebugChatHandler(
        3,
        provider=provider,
        shell_locals=[],
        formatter=DummyFormatter(),
        chat_printer=chat_printer,
        log_printer=Printer(),
    )


def get_output_from_handler(handler):
    chat = handler._chat_printer.input.getvalue()
    log = handler._log_printer.input.getvalue()
    return chat, log


@pytest.fixture
def test_caller(debug_handler: DebugChatHandler):
    def function(message):
        # clear previous logs
        debug_handler._chat_printer.input.truncate(0)
        debug_handler._log_printer.input.truncate(0)
        assert debug_handler.process_message(1, message)
        chat, log = get_output_from_handler(debug_handler)
        assert "SyntaxError" not in chat
        assert "SyntaxError" not in log
        return chat, log

    return function


class TestNonDebugModeForAiWhoResponds:
    def test_send_help_prints_user_names(self, test_caller):
        chat, log = test_caller("help")
        assert "player_1" not in chat
        assert "Empire_2 by Player_2" in chat
        assert "Empire_3 by Player_3" in chat

    def test_sending_unknown_message_is_ignored_by_handler(self, debug_handler: DebugChatHandler):
        handled = debug_handler.process_message(1, "hello world")
        assert handled is False
        chat, log = get_output_from_handler(debug_handler)
        assert chat == ""
        assert log == ""

    def test_sending_stop_before_start_is_swallowed_by_handler(self, debug_handler: DebugChatHandler):
        result = debug_handler.process_message(1, "stop")
        assert result is True
        chat, log = get_output_from_handler(debug_handler)
        assert chat == ""
        assert log == ""


class TestNonDebugModeForAiWhoMuted:
    def test_send_help_is_ignored_by_second_ai(self, debug_handler_for_muted_ai: Tuple[StringIO, DebugChatHandler]):
        chat_io, handler = debug_handler_for_muted_ai
        assert handler.process_message(1, "help")
        assert chat_io.getvalue() == ""


class TestDebugModStart:
    def test_sending_message_from_ai_is_propagated_by_handler(self, debug_handler: DebugChatHandler):
        # ignore messages not from human
        result = debug_handler.process_message(3, "start 2")
        assert result is False
        chat, log = get_output_from_handler(debug_handler)
        assert chat == ""
        assert log == ""

    def test_starting_with_not_existing_ai_is_swallowed_by_handler(self, debug_handler: DebugChatHandler):
        result = debug_handler.process_message(1, "start 10")
        assert result is True
        chat, log = get_output_from_handler(debug_handler)
        assert chat == ""
        assert log == ""

    def test_starting_with_nonnumerical_ai_is_swallowed_by_handler(self, debug_handler: DebugChatHandler):
        assert debug_handler.process_message(1, "start xxx")
        chat, log = get_output_from_handler(debug_handler)
        assert "Invalid empire id, please input valid number" in chat

    def test_start_start_debug_mode_for_respond_ai(self, test_caller):
        chat, log = test_caller("start 2")
        assert "Entering debug mode" in chat

    def test_start_start_debug_mode_for_muted_ai(self, debug_handler_for_muted_ai: Tuple[StringIO, DebugChatHandler]):
        chat_io, handler = debug_handler_for_muted_ai
        handler.process_message(1, "start 3")
        assert "Entering debug mode" in chat_io.getvalue()

    def test_sending_start_prints_intro_message(self, test_caller):
        chat, log = test_caller("start 2")
        assert "variable_a" in chat
        assert "description for a" in chat
        assert "variable_b" in chat
        assert "description for b" in chat
        assert "variable_a_plus_b" in chat
        assert "scription for a+b" in chat


@pytest.fixture
def start_debug(test_caller):
    return test_caller("start 2")


@pytest.mark.usefixtures("start_debug")
class TestInDebugMode:
    def test_sending_stop_pauses_debug(self, test_caller):
        chat, log = test_caller("stop")
        assert "Exiting debug mode" in chat

    def test_evaluate_initial_variables_prints_them_to_chat(self, test_caller):
        chat, log = test_caller("variable_a")
        assert "value_a" in chat
        chat, log = test_caller("variable_b")
        assert "value_b" in chat
        chat, log = test_caller("variable_a_plus_b")
        assert "value_avalue_b" in chat

    def test_evaluating_command_prints_command_to_log(self, test_caller):
        chat, log = test_caller("'free' + 'orion'")
        assert "'free' + 'orion'" in log

    def test_evaluating_command_prints_result(self, test_caller):
        chat, log = test_caller("'free' + 'orion'")
        assert "freeorion" in chat

    def test_evaluating_assignment_assigns_variable(self, test_caller):
        test_caller("variable_c = 'value_c'")
        chat, log = test_caller("variable_c")
        assert "value_c" in chat

    def test_evaluating_erroneous_statement_prints_exception_to_console(self, debug_handler: DebugChatHandler):
        assert debug_handler.process_message(1, "1/0")
        chat, log = get_output_from_handler(debug_handler)
        assert "ZeroDivisionError: division by zero" in chat
        assert "ZeroDivisionError: division by zero" in log
        assert debug_handler.process_message(1, "variable_a")
        chat, log = get_output_from_handler(debug_handler)
        assert "value_a" in chat

    def test_start_with_initial_imports_evaluates_imports(self, test_caller):
        chat, log = test_caller("has_magic")
        assert "function has_magic at" in chat

    def test_help_call_is_ignored(self, test_caller):
        chat, log = test_caller("help")
        assert "Type help() for interactive help" in chat

    def test_start_call_is_ignored(self, test_caller):
        chat, log = test_caller("start")
        assert " name 'start' is not defined" in chat

    def test_start_with_number_call_is_ignored(self, debug_handler: DebugChatHandler):
        assert debug_handler.process_message(1, "start 2")
        chat, log = get_output_from_handler(debug_handler)
        assert "SyntaxError" in log
        assert "SyntaxError" in chat


class TestRestart:
    def test_modified_initial_variables_are_restored(self, test_caller):
        test_caller("start 2")
        test_caller("variable_b = 'value_d'")
        chat, log = test_caller("variable_b")
        assert "value_d" in chat
        test_caller("stop")
        test_caller("start 2")
        chat, log = test_caller("variable_b")
        assert "value_b" in chat
        assert "value_d" not in chat

    def test_not_initial_variables_are_preserved(self, test_caller):
        test_caller("start 2")
        test_caller("variable_d = 'value_d'")
        chat, log = test_caller("variable_d")
        assert "value_d" in chat
        test_caller("stop")
        test_caller("start 2")
        chat, log = test_caller("variable_d")
        assert "value_d" in chat
