from collections.abc import Mapping

from common.fo_typing import SystemId, Turn
from stub_generator.stub_generator.collection_classes import make_type
from stub_generator.stub_generator.rtype.mapper import Mapper
from stub_generator.stub_generator.rtype.utils import get_name_for_mapping

_method_map = Mapper(
    "methods",
    {
        "supplyProjections": Mapping[SystemId, int],
        "turnPolicyAdopted": Turn,
        "name": str,
    },
)


def update_method_rtype(method_name: str, rtype: str) -> str:
    if method_name in _method_map:
        return get_name_for_mapping(_method_map[method_name])
    else:
        return make_type(rtype)
