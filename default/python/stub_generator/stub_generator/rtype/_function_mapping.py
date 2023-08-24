from collections.abc import Sequence

from common.fo_typing import EmpireId, PlayerId, Turn
from stub_generator.stub_generator.collection_classes import make_type
from stub_generator.stub_generator.rtype.mapper import Mapper
from stub_generator.stub_generator.rtype.utils import get_name_for_mapping

_rtypes_map = Mapper(
    "global_functions",
    {
        "currentTurn": Turn,
        "empireID": EmpireId,
        "allEmpireIDs": Sequence[EmpireId],
        "empirePlayerID": PlayerId,
        "getOptionsDBOptionBool": bool,
        "getOptionsDBOptionDouble": float,
        "getOptionsDBOptionInt": int,
        "getOptionsDBOptionStr": str,
    },
)


def update_function_rtype(name: str, rtype: str) -> str:
    key = name
    if key in _rtypes_map:
        return get_name_for_mapping(_rtypes_map[key])
    else:
        return make_type(rtype)
