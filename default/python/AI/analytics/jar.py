from logging import error
from typing import Dict, List, NamedTuple

import freeOrionAIInterface as fo


class CallInfo(NamedTuple):
  args: tuple
  result: bool


class Jar:
  def __init__(self, name: str):
    self.name = name
    self._jar: Dict[int, List[CallInfo]] = {}

  def set_order(self, args: tuple, result: int):
    if result == 0:
      result = False
    elif result == 1:
      result = True
    else:
      error("Unexpected result from issuing order, expect {0, 1} got " % result, stack_info=True)
      result = True

    self._jar.setdefault(fo.currentTurn(), []).append(CallInfo(args, result))

  def get_turn(self, turn: int) -> List[CallInfo]:
    return self._jar.get(turn, [])

  def get_total(self):
    return sum(len(item) for item in self._jar.values())

  def __repr__(self):
    return "Jar({})".format(self.name)
