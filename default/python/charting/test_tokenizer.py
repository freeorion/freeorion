from dump_interface import DumpKey, DumpType
from log_tokenizer import _get_token_from_line


def test_token_parser():
    line = "12:06:13.179169 {0x00002e80} [debug] python : dumper.py:30 : ##EmpireID:str:2 Name: Binding_2_pid_2_AI_1_RIdx_4_Aggressive Turn: 1"
    token = _get_token_from_line(line)
    assert len(token) == 3
    key_, type_, val = token

    assert key_ == DumpKey.EmpireID
    assert type_ == DumpType.str
    assert val == "2 Name: Binding_2_pid_2_AI_1_RIdx_4_Aggressive Turn: 1"
