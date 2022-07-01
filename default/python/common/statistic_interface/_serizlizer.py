from abc import ABC
from typing import Callable, Dict, List, TypeVar

Serializable = TypeVar("Serializable")


class Serializer(ABC):
    def __init__(
        self,
        *,
        serializer: Callable[[Serializable], str],
        deserializer: Callable[[str], Serializable],
    ):
        self._deserializer = deserializer
        self._serializer = serializer

    def deserialize(self, value: str) -> Serializable:
        try:
            return self._deserializer(value)
        except Exception as e:
            raise ValueError(f"{self.__class__.__name__} fail to deserialize {value}: {e}") from e

    def serialize(self, value: Serializable) -> str:
        return self._serializer(value)


class _IntSerializer(Serializer):
    def __init__(self):
        super().__init__(serializer=str, deserializer=int)


class _FloatSerializer(Serializer):
    def __init__(self):
        super().__init__(serializer=str, deserializer=float)


class _StrSerializer(Serializer):
    def __init__(self):
        super().__init__(serializer=str, deserializer=str.strip)


class ListSerializer(Serializer):
    def __init__(self, item_serializer: Serializer):
        def serializer(value: List[Serializable]) -> str:
            return ", ".join(item_serializer.serialize(val) for val in value)

        def deserializer(serialized: str) -> Serializable:
            elements = serialized.split(",")
            return [item_serializer.deserialize(v) for v in elements]

        super().__init__(serializer=serializer, deserializer=deserializer)


class DictSerializer(Serializer):
    def __init__(self, items: Dict[str, Serializer]):
        def serializer(value: Dict[str, Serializable]) -> str:
            assert set(value) == set(items), f"Keys should match {sorted(value)} != {sorted(items)}"
            serialized_dict = {}
            for key, val in value.items():  # preserve order of values dict
                serializer_instance = items[key]
                serialized_dict[key] = serializer_instance.serialize(val)

            return ", ".join(f"{k}: {v}" for k, v in serialized_dict.items())

        def deserializer(serialized: str) -> Serializable:
            elements = serialized.split(",")
            assert len(elements) == len(items)

            def extract_pair(pair_string: str):
                key, val = pair_string.split(":")
                return key.strip(), val.strip()

            pairs = (extract_pair(x) for x in elements)
            return dict((k, items[k].deserialize(v)) for (k, v) in pairs)

        super().__init__(serializer=serializer, deserializer=deserializer)


to_int = _IntSerializer()
to_str = _StrSerializer()
to_float = _FloatSerializer()
