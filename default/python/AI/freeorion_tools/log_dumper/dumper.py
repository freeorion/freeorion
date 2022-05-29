from typing import Callable, Union

from common.dump_interface import LOG_PREFIX, DumpKey, DumpType

DumpValue = Union[str, int]


class DumpException(Exception):
    pass


class Dumper:
    _converters = {
        DumpType.str: str,
        DumpType.int: str,
    }

    def __init__(self, writer: Callable[[str], None]):
        self._writer = writer

    def dump(self, key: DumpKey, value: DumpValue):
        try:
            type_ = self._get_type(value)
            value_as_string = self._value_to_string(type_, value)
            self._dump_raw(key, type_, value_as_string)
        except DumpException as e:
            raise DumpException(f"Fail to dump {key} with {value}") from e

    def _dump_raw(self, key: DumpKey, type_: DumpType, value_as_string):
        self._writer(f"{LOG_PREFIX}{key.value}:{type_.value}:{value_as_string}")

    def _get_type(self, value) -> DumpType:
        if isinstance(value, int):
            return DumpType.int
        elif isinstance(value, str):
            return DumpType.str
        else:
            raise DumpException(f"Unsupported type '{type(value)}'")

    def _value_to_string(self, type_, value):
        return self._converters[type_](value)
