import os
from typing import Generator


def get_code_location(generator: Generator) -> str:
    return f"{os.path.basename(generator.gi_code.co_filename)}:{generator.gi_frame.f_lineno}"


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

    yield fo.getSpecies("SP_ABADDONI")

    fleets_int_vector = universe.fleetIDs

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

    ship = universe.getShip(list(universe.shipIDs)[0])
    yield ship

    design = fo.getShipDesign(ship.designID)
    yield design

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
    "popCenter",  # parent class, it's not possible to get instance
    "resourceCenter",  # parent class, it's not possible to get instance
    "diplomaticStatusUpdate",  # this item is not used in generate orders/universe
    "universeObject",  # parent class, it's not possible to get instance
}

classes_to_exclude_from_universe = {
    "productionQueueElement",  # not applicable
    "researchQueueElement",  # not applicable
    *common_classes_to_exclude,
}

classes_to_exclude_from_ai = {
    *common_classes_to_exclude,
}
