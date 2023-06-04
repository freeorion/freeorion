from collections.abc import Iterator, Mapping
from typing import TypeVar

KEY = TypeVar("KEY")
VAL = TypeVar("VAL")

_mapper_registry = []


class Mapper(Mapping[KEY, VAL]):
    def __init__(self, mapping: dict):
        self._dict = mapping
        self._used = set()
        _mapper_registry.append(self)

    def __getitem__(self, key: KEY) -> VAL:
        self._used.add(key)
        return self._dict[key]

    def __len__(self) -> int:
        return len(self._dict)

    def __iter__(self) -> Iterator[KEY]:
        return iter(self._dict)

    def unused_items(self) -> set[KEY]:
        return set(self._dict) - self._used


def report_unused_mapping():
    for mapper in _mapper_registry:
        unused_items = mapper.unused_items()
        if unused_items:
            print(f"Mapper has unused items for this stub, might be used for other: {sorted(unused_items)}")
