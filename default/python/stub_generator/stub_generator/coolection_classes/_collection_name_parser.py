from typing import Iterator, Iterable, Optional, List


class UnmatchedType(Exception):
    """
    Excaption raised when type could not be matched.
    """


class Token:
    def __init__(self, value: str, head: str, tail: str, arg_count: int):
        self.value = value
        self.head = head
        self.tail = tail
        self.arg_count = arg_count

    def wrap(self, *args: str):
        return f"{self.head}{', '.join(args)}{self.tail}"

    def __len__(self):
        return len(self.value)


class ScalarToken(Token):
    def __init__(self, value: str, type_: str):
        super().__init__(value, type_, "", 0)


class SimpleCollectionToken(Token):
    def __init__(self, value: str, type_):
        super().__init__(value, f"{type_}[", "]", 1)


class PairToken(SimpleCollectionToken):
    def __init__(self, value: str):
        super().__init__(value, "Tuple")

    def wrap(self, res: str):
        return f"{self.head}{res}, {res}{self.tail}"


class MapToken(Token):
    def __init__(self, value: str):
        super().__init__(value, "Map[", "]", 2)


_tokens: List[Token] = [
    SimpleCollectionToken("Vec", "Vec"),
    SimpleCollectionToken("Set", "Set"),
    MapToken("Map"),
    PairToken("Pair"),
    ScalarToken("Int", "int"),
    ScalarToken("Bool", "bool"),
    ScalarToken("Visibility", "visibility"),
    ScalarToken("UnlockableItem", "UnlockableItem"),
    ScalarToken("MeterType", "meterType"),
    ScalarToken("AccountingInfo", "AccountingInfo"),
    ScalarToken("ShipPart", "shipPart"),
    ScalarToken("ShipSlotType", "shipSlotType"),
    ScalarToken("Meter", "meter"),
    ScalarToken("Float", "float"),
    ScalarToken("Double", "float"),
    ScalarToken("String", "str"),
]


def _split_once(string):
    for token in _tokens:
        if string.endswith(token.value):
            size = len(token)
            return string[:-size], token
    else:
        raise UnmatchedType(f"Unsupported string: {string}")


def _iter_arguments(string) -> Iterator[Token]:
    string = string.replace("_", "")
    while string:
        string, token = _split_once(string)
        yield token


def _fetch_group(stream: Iterable[Token]):
    first = next(stream)
    if first.arg_count == 0:
        return [first, []]
    if first.arg_count == 1:
        return [first, [_fetch_group(stream)]]
    if first.arg_count == 2:
        val = _fetch_group(stream)
        key = _fetch_group(stream)
        return [first, [key, val]]


def make_type(string: Optional[str]):
    if not string:
        return ""
    try:
        token, args = _fetch_group(_iter_arguments(string))
    except UnmatchedType:
        return string

    def wrap(token, args):
        if not args:
            return token.wrap("")
        else:
            new_args = [wrap(t, a) for t, a in args]
            return token.wrap(*new_args)

    return wrap(token, args)


def is_collection_type(type_name: str) -> bool:
    type_ = make_type(type_name)
    return type_.startswith(("Vec", "Set", "Map", "Tuple"))
