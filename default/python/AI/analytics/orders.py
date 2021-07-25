from collections import Counter
from functools import wraps
from logging import info
from operator import attrgetter
from typing import Callable, Iterator, List, Tuple

import freeOrionAIInterface as fo
from analytics.jar import Jar
from common.print_utils import Number, Table, Text


def wrap_order(fun: Callable, call_jar: Jar):
  @wraps(fun)
  def wrapper(*args):
    result = fun(*args)
    call_jar.set_order(args, result)
    return result

  return wrapper


class CommandTracker:
  def __init__(self, jar: Jar, fun_name: str):
    self.fun_name = fun_name
    self.jar = jar

  def patch(self) -> None:
    fun = getattr(fo, self.fun_name)
    wrapped = wrap_order(fun, self.jar)
    setattr(fo, self.fun_name, wrapped)


def get_issuers() -> Iterator[Tuple[str, str]]:
  for attr in dir(fo):
    if attr.startswith("issue") and attr.endswith("Order"):
      yield attr[5:-5], attr


class Tracker:
  def __init__(self):
    self.jars: List[Jar] = []
    for name, issuer in get_issuers():
      jar = Jar(name)
      self.jars.append(jar)
      tracker = CommandTracker(jar, issuer)
      tracker.patch()
    self.jars.sort(key=attrgetter("name"))

  def report_turn(self):
    turn = fo.currentTurn()
    table = Table(
      Text('Order'),
      Number("Issued", precession=0, placeholder=" ", total=True),
      Number("Failed", precession=0, placeholder=" ", total=True),
      Number("Total", precession=0, placeholder=" ", total=True),
      table_name="Issuing orders analytics for turn {}".format(turn),
    )

    for jar in self.jars:
      results = Counter(item.result for item in jar.get_turn(turn))
      success = results[True]
      fail = results[False]
      table.add_row(jar.name, success, fail, jar.get_total())
    table.print_table(info)
