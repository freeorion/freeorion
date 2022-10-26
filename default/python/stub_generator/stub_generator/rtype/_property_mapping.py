from stub_generator.stub_generator.coolection_classes import make_type


_property_map = {
    ("shipIDs", "IntSet"): "Set[ShipId]",
    ("shipIDs", "IntVec"): "Vec[ShipId]",
    ("buildingIDs", "IntSet"): "Set[BuildingId]",
    ("buildingIDs", "IntVec"): "Vec[BuildingId]",
    ("planetIDs", "IntSet"): "Set[PlanetId]",
    ("planetIDs", "IntVec"): "Vec[PlanetId]",
    ("fleetIDs", "IntVec"): "Vec[FleetId]",
    ("fleetIDs", "IntSet"): "Set[FleetId]",
    ("systemIDs", "IntVec"): "Vec[SystemId]",
    ("empireID", "int"): "EmpireId",
    ("capitalID", "int"): "PlanetId",
    ("locationID", "int"): "PlanetId",
    ("owner", "int"): "EmpireId",
    ("speciesName", "str"): "SpeciesName",
    ("designedOnTurn", "int"): "Turn",
    ("buildingTypeName", "str"): "BuildingName",
    ("parts", "StringVec"): "Vec[PartName]",
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
