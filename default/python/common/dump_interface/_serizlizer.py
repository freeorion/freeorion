from abc import ABC
from typing import Callable, Dict, Sequence, TypeVar

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
        return self._deserializer(value)

    def serialize(self, value: Serializable) -> str:
        return self._serializer(value)


class IntSerializer(Serializer):
    def __init__(self):
        super().__init__(serializer=str, deserializer=int)


class FloatSerializer(Serializer):
    def __init__(self):
        super().__init__(serializer=str, deserializer=float)


class StrSerializer(Serializer):
    def __init__(self):
        super().__init__(serializer=str, deserializer=str.strip)


class TupleSerializer(Serializer):
    def __init__(self, items: Sequence[Serializer]):
        def serializer(value: Sequence[Serializable]):
            assert len(value) == len(items)
            return ", ".join(s.serialize(v) for v, s in zip(value, items))

        def deserializer(serialized: str):
            elements = serialized.split(",")
            assert len(elements) == len(items)
            return tuple(s.deserialize(v) for v, s in zip(elements, items))

        super().__init__(serializer=serializer, deserializer=deserializer)


class DictSerializer(Serializer):
    def __init__(self, items: Dict[str, Serializer]):
        def serializer(value: Dict[str, Serializable]) -> str:
            assert set(value) == set(items), "Keys should match"
            serialized_dict = {}
            for key, val in value.items():  # preserve order of values dict
                serializer_instance = items[key]
                serialized_dict[key] = serializer_instance.serialize(val)

            return ", ".join(f"{k}: {v}" for k, v in serialized_dict)

        def deserializer(serialized: str) -> Serializable:
            elements = serialized.split(",")
            assert len(elements) == len(items)

            def extract_pair(pair_string: str):
                key, val = pair_string.split(":")
                return key.strip(), val.strip()

            pairs = (extract_pair(x) for x in elements)
            return dict((k, items[k].deserialize(v)) for (k, v) in pairs)

        super().__init__(serializer=serializer, deserializer=deserializer)
