"""
##EmpireID:str:2 Name: Binding_2_pid_2_AI_1_RIdx_4_Aggressive Turn: 2
##EmpireColors:str:0 255 0 255
##CapitalID:str:927 Name: Imperial Arbol I Species: SP_REPLICON
"""
from typing import NamedTuple

from dump_interface import DumpKey, DumpType


class RGBA(NamedTuple):
    R: int
    G: int
    B: int
    A: int


class Empire(NamedTuple):
    id: int
    name: str
    color: RGBA


class Capital(NamedTuple):
    id: int
    name: str
    species: str


class LogData(NamedTuple):
    empire: Empire
    capital: Capital


def is_log_line(line):
    """
    Return True if its line with log

    Sample line:

    """
    return line.find("##")


def get_token_from_line(line: str):
    """
    Return token from line, if not token present return None.

    Line sample:
    18:27:15.422733 {0x000026c8} [debug] python : dumper.py:30 : ##EmpireID:str:2 Name: Binding_2_pid_2_AI_1_RIdx_4_Aggressive Turn: 2
    """

    index = line.find("##", 45, 80)
    if index == -1:
        return None
    line = line[index + 2 :]
    key_, type_, val = line.split(":", 2)
    return DumpKey(key_), DumpType(type_), val[:-1]


def parse_file(path):
    with open(path) as f:
        for line in f:
            token = get_token_from_line(line)
            if not token:
                continue
            yield token


def parse_log(path):
    tokens = parse_file(path)
    return tokens
