import os
from typing import Generator


def get_code_location(generator: Generator) -> str:
    return "%s:%s" % (os.path.basename(generator.gi_code.co_filename), generator.gi_frame.f_lineno)


def get_item_with_location(generator: Generator) -> Generator:
    for x in generator:
        yield get_code_location(generator), x


def handle_interfaces_mismatch():
    """
    Handle cases when AI and universe generation interfaces mismatch.
    """

    try:
        import freeorion as fo

        universe = fo.get_universe()
        empire_of_first_ai = fo.get_empire(2)
        galaxy_data = fo.get_galaxy_setup_data()
        return fo, universe, empire_of_first_ai, galaxy_data
    except ImportError:
        pass

    try:
        import freeOrionAIInterface as fo

        universe = fo.getUniverse()
        empire_of_first_ai = fo.getEmpire(2)
        galaxy_data = fo.getGalaxySetupData()
        return fo, universe, empire_of_first_ai, galaxy_data

    except ImportError:
        pass


def get_common_instances() -> Generator:
    """
    Get instances for objects that are the same to AI and universe generation interfaces.
    """
    fo, universe, empire_of_first_ai, galaxy_data = handle_interfaces_mismatch()

    yield universe
    yield galaxy_data

    planet = universe.getPlanet(empire_of_first_ai.capitalID)
    yield planet

    yield universe.getSystem(planet.systemID)

    tech = fo.getTech("SHP_WEAPON_2_1")
    yield tech
    yield tech.unlockedItems[0]

    yield fo.getGameRules()
    ship_hull = fo.getShipHull("SH_XENTRONIUM")
    yield ship_hull
    yield ship_hull.slots

    yield fo.getSpecies("SP_ABADDONI")

    fleets_int_vector = universe.fleetIDs
    yield fleets_int_vector

    fleet = universe.getFleet(list(fleets_int_vector)[0])
    yield fleet

    fields_ids = universe.fieldIDs
    field = universe.getField(fields_ids[0])
    yield field

    yield fo.getFieldType("FLD_ION_STORM")
    yield fo.getBuildingType("BLD_SHIPYARD_BASE")

    yield fo.getShipPart("SR_WEAPON_1_1")
    yield fo.getSpecial("MODERATE_TECH_NATIVES_SPECIAL")
    yield fo.getShipHull("SH_XENTRONIUM")

    yield universe.effectAccounting
    yield universe.buildingIDs

    ship = universe.getShip(list(universe.shipIDs)[0])
    design = fo.getShipDesign(ship.designID)
    part_meters = ship.partMeters
    yield ship
    yield design
    yield part_meters

    yield fo.diplomaticMessage(1, 2, fo.diplomaticMessageType.acceptPeaceProposal)
    yield empire_of_first_ai
    yield empire_of_first_ai.productionQueue
    yield empire_of_first_ai.researchQueue

    planet = universe.getPlanet(empire_of_first_ai.capitalID)
    yield planet

    meter = planet.getMeter(fo.meterType.population)
    yield meter

    building = list(planet.buildingIDs)[0]
    yield universe.getBuilding(building)

    yield fo.getPolicy("PLC_LIBERTY")


common_classes_to_exclude = {
    "IntBoolMap",
    "IntDblMap",
    "IntFltMap",
    "IntFltMap",
    "IntPairVec",
    "IntSetSet",
    "MeterTypeMeterMap",
    "MeterTypeStringPair",
    "PairIntInt_IntMap",
    "VisibilityIntMap",
    "IntSet",
    "StringSet",
    "StringSet2",
    "StringVec",
    "IntIntDblMapMap",
    "IntStringMap",
    "String_IntStringMap_Map",
    "StringIntMap",
    "StringsMap",
    "IntIntMap",
    "AccountingInfoVec",
    "StatRecordsMap",
    "MeterTypeAccountingInfoVecPair",
    "MeterTypeAccountingInfoVecMap",
    "popCenter",
    "resourceCenter",
}

classes_to_exclude_from_universe = {
    "MeterTypeAccountingInfoVecMap",
    "MonsterFleetPlan",
    "RuleValueStringStringPair",
    "ShipPartMeterMap",
    "AccountingInfoVec",
}

classes_to_exclude_from_ai = {
    "UnlockableItemVec",
    "universeObject",
    # this item cannot be get from generate orders
    "diplomaticStatusUpdate",
}

classes_to_exclude_from_ai.update(common_classes_to_exclude)
classes_to_exclude_from_universe.update(common_classes_to_exclude)
