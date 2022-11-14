import freeOrionAIInterface as fo
from logging import debug, info

from common.print_utils import Sequence, Table, Text
from EnumsAI import EmpireProductionTypes
from turn_state import get_all_empire_planets


def print_building_list():
    debug("Buildings present on all owned planets:")
    universe = fo.getUniverse()
    for pid in get_all_empire_planets():
        planet = universe.getPlanet(pid)
        if planet:
            debug("%30s: %s" % (planet.name, [universe.getBuilding(bldg).name for bldg in planet.buildingIDs]))
    debug("")


def print_production_queue(after_turn=False):
    """Print production queue content with relevant info in table format."""
    universe = fo.getUniverse()
    s = "after" if after_turn else "before"
    title = "Production Queue Turn %d %s ProductionAI calls" % (fo.currentTurn(), s)
    prod_queue_table = Table(
        Text("Object"),
        Text("Location"),
        Text("Quantity"),
        Text("Progress"),
        Text("Allocated PP"),
        Text("Turns left"),
        table_name=title,
    )
    for element in fo.getEmpire().productionQueue:
        if element.buildType == EmpireProductionTypes.BT_SHIP:
            item = fo.getShipDesign(element.designID)
        elif element.buildType == EmpireProductionTypes.BT_BUILDING:
            item = fo.getBuildingType(element.name)
        else:
            continue
        cost = item.productionCost(fo.empireID(), element.locationID)

        prod_queue_table.add_row(
            element.name,
            universe.getPlanet(element.locationID),
            "%dx %d" % (element.remaining, element.blocksize),
            f"{element.progress * cost:.1f} / {cost:.1f}",
            "%.1f" % element.allocation,
            "%d" % element.turnsLeft,
        )
    info(prod_queue_table)


def print_capital_info(homeworld):
    table = Table(
        Text("Id", description="Building id"),
        Text("Name"),
        Text("Type"),
        Sequence("Tags"),
        Sequence("Specials"),
        Text("Owner Id"),
        table_name="Buildings present at empire Capital in Turn %d" % fo.currentTurn(),
    )

    universe = fo.getUniverse()

    for building_id in homeworld.buildingIDs:
        building = universe.getBuilding(building_id)

        table.add_row(
            building_id,
            building.name,
            "_".join(building.buildingTypeName.split("_")[-2:]),
            sorted(building.tags),
            sorted(building.specials),
            building.owner,
        )

    info(table)
