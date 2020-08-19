from typing import Any

# Tests classes
from pytest import fixture

import freeOrionAIInterface as fo
from freeorion_tools.chat_handler import debug_chat_handler
from freeorion_tools.chat_handler.debug_chat_handler import DebugChatHandler
from freeorion_tools.chat_handler.chat_formatter import ChatFormatter
from freeorion_tools.chat_handler.shell_variable import ShellVariable


class _ChatFormatter(ChatFormatter):
    def _get_tag_rgb(self, color: (int, int, int, int)) -> (str, str):
        return '', ''

    def _get_underline_tag(self) -> (str, str):
        return '', ''

    def _wrap_to_tag(self, tag: str, message: Any) -> str:
        return str(message)


@fixture
def debug_handler(monkeypatch):
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
            imports=("from glob import magic_check",),
        ),
        ShellVariable(
            variable="variable_a_plus_b",
            expression="variable_a + variable_b",
            description="description for a+b",
        ),
    ]

    monkeypatch.setattr(debug_chat_handler, 'chat_human', print)
    monkeypatch.setattr(debug_chat_handler.DebugChatHandler, '_shell_locals', _shell_locals)
    monkeypatch.setattr(debug_chat_handler.DebugChatHandler, '_get_empire_string', lambda self, player_id: "player_%s" % player_id)
    monkeypatch.setattr(fo, 'allPlayerIDs', lambda: [1, 2, 3], raising=False)
    monkeypatch.setattr(fo, 'playerIsHost', lambda player_id: player_id == 1, raising=False)
    monkeypatch.setattr(fo, 'playerID', lambda: 2, raising=False)
    monkeypatch.setattr(fo, 'playerName', lambda player_id: "Player_%s" % player_id, raising=False)
    monkeypatch.setattr(debug_chat_handler, 'ChatFormatter', _ChatFormatter)
    return DebugChatHandler(2)


@fixture
def test_caller(debug_handler, capsys):
    def function(message):
        # clear previous logs
        capsys.readouterr()
        assert debug_handler.process_message(1, message)
        out, err = capsys.readouterr()
        assert err == ''
        assert "SyntaxError" not in out
        return out

    return function


class TestChat:
    def test_send_help_prints_user_names(self, test_caller):
        help_message = test_caller("help")
        assert "player_1" not in help_message
        assert "player_2" in help_message
        assert "player_3" in help_message

    def test_sending_unknown_message_is_ignored_by_handler(self, debug_handler, capsys):
        handled = debug_handler.process_message(1, "hello world")
        assert handled is False
        out, err = capsys.readouterr()
        assert err == ''
        assert out == ''

    def test_sending_stop_before_start_is_swallowed_by_handler(self, debug_handler, capsys):
        result = debug_handler.process_message(1, "stop")
        assert result is True
        out, err = capsys.readouterr()
        assert err == ''
        assert out == ''

    def test_sending_message_from_ai_is_propagated_by_handler(self, debug_handler, capsys):
        # ignore messages not from human
        result = debug_handler.process_message(3, "start 2")
        assert result is False
        out, err = capsys.readouterr()
        assert err == ''
        assert out == ''

    def test_starting_with_not_existing_ai_is_swallowed_by_handler(self, debug_handler, capsys):
        result = debug_handler.process_message(1, "start 10")
        assert result is True
        out, err = capsys.readouterr()
        assert err == ''
        assert out == ''

    def test_starting_with_nonnumerical_ai_is_swallowed_by_handler(self, debug_handler, capsys):
        result = debug_handler.process_message(1, "start xxx")
        assert result is True
        out, err = capsys.readouterr()
        assert not err
        assert "Invalid empire id, please input valid number" in out


class TestDebugMode:
    def test_sending_stop_pauses_debug(self, test_caller):
        out = test_caller("start 2")
        assert "Entering debug mode" in out
        assert "variable_b" in out
        out = test_caller("stop")
        assert "Exiting debug mode" in out

    def test_sending_start_prints_intro_message(self, test_caller):
        out = test_caller("start 2")
        assert "variable_a" in out
        assert "description for a" in out
        assert "variable_b" in out
        assert "description for b" in out
        assert "variable_a_plus_b" in out
        assert "scription for a+b" in out

    def test_evaluate_initial_variables_prints_them_to_chat(self, test_caller):
        test_caller("start 2")
        out = test_caller("variable_a")
        assert "value_a" in out
        assert "variable_a" in out
        out = test_caller("variable_b")
        assert "value_b" in out
        assert "variable_b" in out
        out = test_caller("variable_a_plus_b")
        assert "value_avalue_b" in out
        assert "variable_a_plus_b" in out

    def test_evaluating_command_prints_result(self, test_caller):
        test_caller("start 2")
        out = test_caller("'free' + 'orion'")
        assert "freeorion" in out

    def test_evaluating_assignment_assigns_variable(self, test_caller):
        test_caller("start 2")
        out = test_caller("variable_c = 'value_c'")
        assert "variable_c" in out
        out = test_caller("variable_c")
        assert "value_c" in out

    def test_evaluating_erroneous_statement_prints_exception_to_console(self, debug_handler, capsys):
        assert debug_handler.process_message(1, "start 2")
        assert debug_handler.process_message(1, '1/0')
        out, err = capsys.readouterr()
        assert err == ''
        assert 'ZeroDivisionError: division by zero' in out
        assert debug_handler.process_message(1, 'variable_a')
        out, err = capsys.readouterr()
        assert err == ''
        assert 'value_a' in out

    def test_start_with_initial_imports_evaluates_imports(self, test_caller):
        test_caller("start 2")
        result = test_caller("magic_check")
        assert "magic_checkre" in result


class TestRestart:
    def test_modified_initial_variables_are_restored(self, test_caller):
        test_caller("start 2")
        test_caller("variable_b = 'value_d'")
        out = test_caller("variable_b")
        assert 'value_d' in out
        test_caller("stop")
        test_caller("start 2")
        out = test_caller("variable_b")
        assert 'value_b' in out
        assert 'value_d' not in out

    def test_not_initial_variables_are_preserved(self, test_caller):
        test_caller("start 2")
        test_caller("variable_d = 'value_d'")
        out = test_caller("variable_d")
        assert 'value_d' in out
        test_caller("stop")
        test_caller("start 2")
        out = test_caller("variable_d")
        assert 'value_d' in out
