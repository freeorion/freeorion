from stub_generator.stub_generator.collection_classes import make_type
from stub_generator.stub_generator.rtype.mapper import Mapper

_method_map = Mapper(
    {"supplyProjections": "Map[SystemId, int]", "status": "fo.diplomaticStatus", "turnPolicyAdopted": "Turn"}
)


def update_method_rtype(method_name: str, rtype: str) -> str:
    if method_name in _method_map:
        return _method_map[method_name]
    else:
        return make_type(rtype)
