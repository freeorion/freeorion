from abc import ABC, abstractmethod


class BaseProcessor(ABC):
    def __init__(self):
        self.imports = []
        self.before = []
        self.body = []

    @abstractmethod
    def _process(self):
        ...

    def process(self):
        return self._process()
