from enum import Enum
from typing import Iterator, Iterable, Optional


class UnmatchedType(Exception):
    """
    Excaption raised when type could not be matched.
    """


class TokenType:
    def __init__(self, value: str, head: str, tail: str, arg_count: int):
        self.value = value
        self.head = head
        self.tail = tail
        self.arg_count = arg_count

    def wrap(self, *res: str):
        return f"{self.head}{', '.join(res)}{self.tail}"


class ScalarToken(TokenType):
    def __init__(self, value: str, type_: str):
        super(ScalarToken, self).__init__(value, type_, "", 0)


class SimpleCollectionToken(TokenType):
    def __init__(self, value: str, type_):
        super(SimpleCollectionToken, self).__init__(value, f"{type_}[", "]", 1)


class PairCollectionToken(SimpleCollectionToken):
    def __init__(self, value: str):
        super(PairCollectionToken, self).__init__(value, "Tuple")

    def wrap(self, res: str):
        return f"{self.head}{res}, {res}{self.tail}"


class MapToken(TokenType):
    def __init__(self, value: str):
        super(MapToken, self).__init__(value, "Map[", "]", 2)


class Tokens(Enum):
    VEC = SimpleCollectionToken("Vec", "Sequence")
    SET = SimpleCollectionToken("Set", "Set")
    MAP = MapToken("Map")
    PAIR = PairCollectionToken("Pair")
    INT = ScalarToken("Int", "int")
    FLOAT = ScalarToken("Flt", "float")
    STRING = ScalarToken("String", "str")

    def __len__(self):
        return len(self.value.value)

    def wrap(self, *args: str):
        return self.value.wrap(*args)


def _split_once(string):
    for token in Tokens:
        if string.endswith(token.value.value):
            size = len(token)
            return string[:-size], token
    else:
        raise UnmatchedType(f"Unsupported string: {string}")


def _iter_arguments(string) -> Iterator[Tokens]:
    while string:
        string, token = _split_once(string)
        yield token


def fetch_group(stream: Iterable[Tokens]):
    first = next(stream)
    if first.value.arg_count == 0:
        return [first, []]
    if first.value.arg_count == 1:
        return [first, [fetch_group(stream)]]
    if first.value.arg_count == 2:
        val = fetch_group(stream)
        key = fetch_group(stream)
        return [first, [key, val]]


def make_type(string: Optional[str]):
    if not string:
        return ""
    try:
        token, args = fetch_group(_iter_arguments(string))
    except UnmatchedType:
        return string

    def wrap(token, args):
        if not args:
            return token.wrap("")
        else:
            new_args = [wrap(t, a) for t, a in args]
            return token.value.wrap(*new_args)

    return wrap(token, args)
