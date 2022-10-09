from stub_generator.stub_generator.rtype._base_rtype_mapping import make_type


_property_map = {
    ("shipIDs", "IntSet"): "Set[ShipId]",
    ("shipIDs", "IntVec"): "Sequence[ShipId]",
    ("buildingIDs", "IntSet"): "Set[BuildingId]",
    ("buildingIDs", "IntVec"): "Sequence[BuildingId]",
    ("planetIDs", "IntSet"): "Set[PlanetId]",
    ("planetIDs", "IntVec"): "Sequence[PlanetId]",
    ("fleetIDs", "IntVec"): "Sequence[FleetId]",
    ("fleetIDs", "IntSet"): "Set[FleetId]",
    ("systemIDs", "IntVec"): "Sequence[SystemId]",
    ("empireID", "int"): "EmpireId",
    ("capitalID", "int"): "PlanetId",
    ("locationID", "int"): "PlanetId",
    ("owner", "int"): "EmpireId",
    ("speciesName", "str"): "SpeciesName",
    ("designedOnTurn", "int"): "Turn",
    ("buildingTypeName", "str"): "BuildingName",
    ("parts", "StringVec"): "Sequence[PartName]",
}


def update_property_rtype(attr_name: str, rtype: str):
    if rtype.startswith("<type"):
        return rtype[7:-2]

    rtype = rtype.split(".")[-1].strip("'>")

    key = (attr_name, rtype)
    if key in _property_map:
        return _property_map[key]
    else:
        return make_type(rtype)
