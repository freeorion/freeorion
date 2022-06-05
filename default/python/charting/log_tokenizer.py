"""
Log entry samples.

##EmpireID:str:2 Name: Binding_2_pid_2_AI_1_RIdx_4_Aggressive Turn: 2
##EmpireColors:str:0 255 0 255
##CapitalID:str:927 Name: Imperial Arbol I Species: SP_REPLICON
"""
from typing import Optional, Tuple

from dump_interface import LOG_PREFIX, DumpKey, DumpType


def _get_token_from_line(line: str) -> Optional[Tuple[DumpKey, DumpType, str]]:
    """
    Return token from line, if not token present return None.

    Line sample:
    12:06:13.179169 {0x00002e80} [debug] python : dumper.py:30 : ##EmpireID:str:2 Name: Binding_2_pid_2_AI_1_RIdx_4_Aggressive Turn: 1
    """

    index = line.find(LOG_PREFIX, 45, 80)
    if index == -1:
        return None
    line = line[index + 2 :]
    key_, type_, val = line.split(":", 2)
    return DumpKey(key_), DumpType(type_), val


def tokenize_log(path):
    with open(path) as f:
        for line in f:
            token = _get_token_from_line(line)
            if not token:
                continue
            yield token
