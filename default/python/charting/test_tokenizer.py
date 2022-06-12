from log_tokenizer import _get_token_from_line
from statistic_interface import StatKey


def test_token_parser():
    line = "12:06:13.179169 {0x00002e80} [debug] python : statistic.py:30 : ##EmpireID:empire_id: 2, name: Binding_2_pid_2_AI_1_RIdx_4_Aggressive, turn: 1"
    token = _get_token_from_line(line)
    assert len(token) == 2
    key, val = token
    assert key == StatKey.EmpireID
    assert val == "empire_id: 2, name: Binding_2_pid_2_AI_1_RIdx_4_Aggressive, turn: 1"
