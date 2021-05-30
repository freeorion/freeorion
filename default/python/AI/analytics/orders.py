from collections import Counter
from functools import wraps
from logging import error, info
from operator import attrgetter
from typing import Callable, Dict, Iterator, List, NamedTuple, Tuple

import freeOrionAIInterface as fo
from common.print_utils import Number, Table, Text


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

        for name, attr in get_issuers():
            jar = Jar(name)
            self.jars.append(jar)
            tracker = CommandTracker(jar, attr)
            tracker.patch()
        self.jars.sort(key=attrgetter("name"))

    def report_turn(self):
        turn = fo.currentTurn()
        table = Table(
            Text('Order'),
            Number("I", precession=0, description="Issued orders", placeholder=" "),
            Number("F", precession=0, description="Failed orders", placeholder=" "),
            Number("Turn", precession=0, description="Total this turn", placeholder=" "),
            Number("Total", precession=0, description="Grand total", placeholder=" "),
            table_name="Issuing orders analytics for turn {}".format(turn),
        )

        for jar in self.jars:
            results = Counter(item.result for item in jar.get_turn(turn))
            success = results[True]
            fail = results[False]
            total = success + fail
            table.add_row(
                (jar.name, success, fail, total, jar.get_total())
            )
        table.print_table(info)
