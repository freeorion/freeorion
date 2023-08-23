from collections.abc import Sequence

from common.fo_typing import (
    BuildingId,
    BuildingName,
    EmpireId,
    FleetId,
    ObjectId,
    PartName,
    PlanetId,
    PlayerId,
    ShipId,
    SpeciesName,
    SystemId,
    Turn,
)
from stub_generator.stub_generator.collection_classes import make_type
from stub_generator.stub_generator.rtype.mapper import Mapper
from stub_generator.stub_generator.rtype.utils import get_name_for_mapping

_property_map = Mapper(
    "property",
    {
        ("shipIDs", "IntVec"): Sequence[ShipId],
        ("buildingIDs", "IntVec"): Sequence[BuildingId],
        ("planetIDs", "IntVec"): Sequence[PlanetId],
        ("fleetIDs", "IntVec"): Sequence[FleetId],
        ("systemIDs", "IntVec"): Sequence[SystemId],
        ("empireID", "int"): EmpireId,
        ("capitalID", "int"): PlanetId,
        ("locationID", "int"): PlanetId,
        ("speciesName", "str"): SpeciesName,
        ("designedOnTurn", "int"): Turn,
        ("buildingTypeName", "str"): BuildingName,
        ("parts", "StringVec"): Sequence[PartName],
        ("recipient", "int"): PlayerId,
        ("sender", "int"): PlayerId,
        ("id", ""): ObjectId,
        ("systemID", ""): SystemId,
        ("name", ""): str,
        ("owner", ""): EmpireId,
        ("empire1", ""): EmpireId,
        ("empire2", ""): EmpireId,
        ("status", ""): "diplomaticStatus",
        ("specials", ""): SpeciesName,
        ("producedByEmpireID", "int"): EmpireId,
        ("planetID", "int"): PlanetId,
    },
)


def update_property_rtype(attr_name: str, rtype: str):
    if rtype.startswith("<type"):
        return rtype[7:-2]

    rtype = rtype.split(".")[-1].strip("'>")

    key = (attr_name, rtype)
    if key in _property_map:
        property_class = _property_map[key]

        return get_name_for_mapping(property_class)
    else:
        return make_type(rtype)
