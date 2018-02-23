from stub_generator import inspect


def inspect_universe_generation():
    """
    Inspect function for universe generation interface.

    We have no way to run it without modifying of game code.

    To generate interface
    put call before return section of universe_generation.universe_generator.create_universe function
    """
    import freeorion as fo
    techs = fo.techs()
    universe = fo.get_universe()
    planet_ids = universe.planetIDs

    system_ids = universe.systemIDs
    system = universe.getSystem(system_ids[0])

    building_ids = universe.buildingIDs
    building = universe.getBuilding(building_ids[0])
    empire_ids = fo.get_all_empires()
    empire = fo.get_empire(empire_ids[0])
    fields_ids = universe.fieldIDs
    field = universe.getField(fields_ids[0])

    fleets_ids = universe.fleetIDs
    fleet = universe.getFleet(fleets_ids[0])
    tech = fo.getTech(techs[0])
    unlocked_items = tech.unlockedItems

    ship = universe.getShip(list(fleet.shipIDs)[0])
    # meter = ship.getMeter(fo.meterType.maxFuel)

    design = ship.design

    universe_object = universe.getObject(universe.systemIDs[0])

    # fo.getHullType ?

    species = None
    special = None
    planets = [universe.getPlanet(pid) for pid in planet_ids]
    for planet in planets:
        species_name = planet.speciesName
        if species_name:
            species = fo.getSpecies(species_name)
        specials = list(planet.specials)
        if specials:
            special = fo.getSpecial(specials[0])

    inspect(
        fo,
        instances=[
            fo.get_galaxy_setup_data(),
            universe,
            planet_ids,
            universe.getPlanet(planet_ids[0]),
            system,
            techs,
            tech,
            tech.unlockedTechs,
            building,
            fo.getBuildingType(building.buildingTypeName),
            empire,
            field,
            fo.getFieldType(field.fieldTypeName),
            fleet,
            ship,
            # meter,
            design,
            fleet.shipIDs,  # int set wtf?
            universe_object,
            system.starlanesWormholes,
            empire.systemSupplyRanges,
            empire.supplyProjections(),
            empire.obstructedStarlanes(),
            empire.planetsWithWastedPP,
            unlocked_items,
            species,
            special
        ],
        classes_to_ignore=(
            'FleetPlan', 'GGColor', 'MonsterFleetPlan', 'PlayerSetupData', 'ShipPartMeterMap', 'ShipSlotVec',
            'VisibilityIntMap', 'diplomaticMessage', 'diplomaticStatusUpdate', 'meter',
        ),
        path=''
    )
    # fo.sys_get_star_type(system),
    # fo.planet_get_size(pid),
    # fo.planet_get_type(planet),
    # fo.species_get_planet_environment(species, planet_type),
    # fo.species_preferred_focus(species),

    # fo.getBuildingType(string)
    # fo.getFieldType(string)
    # fo.getHullType(string)
    # fo.getPartType(string)
    # fo.getShipDesign(number)
    # fo.getSpecial(string)
    # fo.getSpecies(string)
    # fo.getTech(string)
    # fo.getTechCategories(obj)
    # fo.get_empire(number)
    # fo.planet_get_species(number)
    # fo.techsInCategory(string)
    exit(1)

# Errors

# meters
# Traceback (most recent call last):
# File "f:\projects\FreeOrion\default/python/universe_generation\universe_generator.py", line 163, in create_universe
#  inspect_universe_generation()
# File "f:\projects\FreeOrion\default/python\interface_stub\stub_functions.py", line 27, in inspect_universe_generation
#  meter = ship.getMeter(fo.meterType.maxFuel)
# Boost.Python.ArgumentError: Python argument types in
#  ship.getMeter(ship, meterType)
# did not match C++ signature:
# getMeter(class Ship {lvalue}, enum MeterType, class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >)


# fleet.shipIDs is intSet, why?
