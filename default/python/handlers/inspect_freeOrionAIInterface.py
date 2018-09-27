from common.listeners import register_pre_handler
from stub_generator import inspect


def inspect_ai_interface():
    # Put all related imports inside
    import PlanetUtilsAI
    import freeOrionAIInterface as fo

    capital_id = PlanetUtilsAI.get_capital()
    universe = fo.getUniverse()
    fleets_int_vector = universe.fleetIDs
    fleet = universe.getFleet(list(fleets_int_vector)[0])
    ship = universe.getShip(list(universe.shipIDs)[0])
    design = fo.getShipDesign(ship.designID)
    empire = fo.getEmpire()

    tech = fo.getTech('SHP_WEAPON_2_1')
    tech_spec = list(tech.unlockedItems)[0]

    part_id = list(empire.availableShipParts)[0]
    part_type = fo.getPartType(part_id)

    prod_queue = empire.productionQueue
    fo.issueEnqueueShipProductionOrder(list(empire.availableShipDesigns)[0], capital_id)

    research_queue = empire.researchQueue

    fo.issueEnqueueTechOrder('SHP_WEAPON_1_2', -1)

    planet = universe.getPlanet(capital_id)

    building = list(planet.buildingIDs)[0]

    color = empire.colour

    part_meters = ship.partMeters

    meter = planet.getMeter(fo.meterType.population)

    inspect(
        fo,
        instances=[
            meter,
            part_meters,
            color,
            universe,
            fleet,
            planet,
            universe.getSystem(planet.systemID),
            ship,
            empire,
            design,
            tech,
            tech_spec,
            fo.getFieldType('FLD_ION_STORM'),
            fo.getBuildingType('BLD_SHIPYARD_BASE'),
            fo.getGalaxySetupData(),
            fo.getHullType('SH_XENTRONIUM'),
            fo.getPartType('SR_WEAPON_1_1'),
            fo.getSpecial('MODERATE_TECH_NATIVES_SPECIAL'),
            fo.getSpecies('SP_ABADDONI'),
            fo.getTech('SHP_WEAPON_4_1'),
            fo.diplomaticMessage(1, 2, fo.diplomaticMessageType.acceptPeaceProposal),
            fleets_int_vector,
            part_type,
            prod_queue,
            prod_queue.allocatedPP,
            prod_queue[0],
            research_queue,
            research_queue[0],
            empire.getSitRep(0),
            universe.getBuilding(building)
        ],
        classes_to_ignore=(
            'IntSet', 'StringSet', 'IntIntMap', 'ShipSlotVec', 'VisibilityIntMap', 'IntDblMap',
            'IntBoolMap', 'ItemSpecVec', 'PairIntInt_IntMap', 'IntSetSet', 'StringVec',
            'IntPairVec', 'IntFltMap', 'MeterTypeStringPair', 'MeterTypeMeterMap', 'universeObject',
            # this item cannot be get from generate orders
            'diplomaticStatusUpdate',
        ),
        path='AI'
    )
    exit(1)  # exit game to main menu no need to play anymore.


register_pre_handler('generateOrders', inspect_ai_interface)
