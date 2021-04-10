from abc import ABC, abstractmethod


class BaseProcessor(ABC):
    def __init__(self):
        self._headings = []
        self._items = []

    @abstractmethod
    def _process(self, items):
        ...

    def process(self, items):
        return self._process(items)

    def get_body(self):
        yield from self._items

    def get_heading(self):
        yield from self._headings