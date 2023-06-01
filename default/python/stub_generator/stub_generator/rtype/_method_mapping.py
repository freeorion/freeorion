from stub_generator.stub_generator.collection_classes import make_type
from stub_generator.stub_generator.rtype.mapper import Mapper

_method_map = Mapper(
    {
        ("empire", "supplyProjections"): "Map[SystemId, int]",
    }
)


def update_method_rtype(class_: str, method_name: str, rtype: str) -> str:
    key = (class_, method_name)
    if key in _method_map:
        return _method_map[key]
    else:
        return make_type(rtype)
