import freeOrionAIInterface as fo # pylint: disable=import-error
import FreeOrionAI as foAI
import ColonisationAI


def sys_name_ids(sys_ids):
    universe = fo.getUniverse()
    res=[]
    for sysID in sys_ids:
        sys = universe.getSystem(sysID)
        if sys:
            res.append( "%s:%d"%(sys.name, sysID ) )
        else:
            res.append("unknown:%d"%sysID )
    return res


def planet_name_ids(planet_ids):
    universe = fo.getUniverse()
    res=[]
    for pid in planet_ids:
        planet = universe.getPlanet(pid)
        if planet:
            res.append( "%s:%d"%(planet.name, pid ) )
        else:
            res.append("unknown:%d"%pid )
    return res


def get_capital(): # if no current capital returns planet with biggest pop
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    if empire is None:
        print "Danger Danger! FO can't find an empire for me!!!!"
        return -1
    empire_id = empire.empireID
    capital_id = empire.capitalID
    homeworld = universe.getPlanet(capital_id)
    if homeworld:
        if homeworld.owner==empire_id:
            return capital_id
        else:
            print "Nominal Capitol %s does not appear to be owned by empire %d  %s"%(homeworld.name,  empire_id,  empire.name)
    #exploredSystemIDs = empire.exploredSystemIDs
    #exploredPlanetIDs = PlanetUtilsAI.get_planets_in__systems_ids(exploredSystemIDs)
    empire_owned_planet_ids = get_owned_planets_by_empire(universe.planetIDs, empire_id)
    peopled_planets = get_populated_planet_ids(  empire_owned_planet_ids)
    if not peopled_planets:
        if empire_owned_planet_ids:
            return empire_owned_planet_ids[0]
        else:
            return -1
    try:
        for spec_list in [ list(ColonisationAI.empireColonizers),  list(ColonisationAI.empireShipBuilders),  None]:
            pop_map = []
            for planetID in peopled_planets:
                planet = universe.getPlanet(planetID)
                if (spec_list is not None) and planet.speciesName not in spec_list:
                    continue
                pop_map.append( ( planet.currentMeterValue(fo.meterType.population) ,  planetID) )
            if len(pop_map) > 0:
                pop_map.sort()
                return pop_map[-1][-1]
    except:
        pass
    return -1 #shouldn't ever reach here


def get_capital_sys_id():
    cap_id = get_capital()
    if cap_id is None or cap_id==-1:
        return -1
    else:
        return fo.getUniverse().getPlanet(cap_id).systemID


def get_planets_in__systems_ids(system_ids):
    """return list of planets in systems"""

    universe = fo.getUniverse()
    planet_ids = []

    for system_id in system_ids:
        these_planets=set(foAI.foAIstate.systemStatus.get(system_id, {}).get('planets', {}).keys())
        system = universe.getSystem(system_id)
        if system is not None:
            these_planets.update(list(system.planetIDs))
        planet_ids.extend(list(these_planets)) # added list

    return planet_ids


def get_owned_planets_by_empire(planetIDs, empireID):
    """return list of planets owned by empireID"""

    universe = fo.getUniverse()
    result = []

    for planetID in planetIDs:
        planet = universe.getPlanet(planetID)
        #even if our universe says we own it, if we can't see it we must have lost it
        if planet and (not planet.unowned) and planet.ownedBy(empireID) and (universe.getVisibility(planetID,  empireID) >= fo.visibility.partial):
            result.append(planetID)

    return result


def get_all_owned_planet_ids(planetIDs):
    """return list of all owned and populated planetIDs"""
    universe = fo.getUniverse()
    allOwnedPlanetIDs = []

    for planetID in planetIDs:
        planet = universe.getPlanet(planetID)
        if planet:
            planetPopulation = planet.currentMeterValue(fo.meterType.population)
            if not planet.unowned or planetPopulation > 0:
                allOwnedPlanetIDs.append(planetID)

    return allOwnedPlanetIDs


def get_populated_planet_ids(planetIDs):
    universe = fo.getUniverse()
    pops=[]
    for planetID in planetIDs:
        planet = universe.getPlanet(planetID)
        if planet.currentMeterValue(fo.meterType.population) >0:
            pops.append(planetID)
    return pops


def get_systems(planetIDs):
    """return list of systems containing planetIDs"""
    universe = fo.getUniverse()
    systemIDs = []

    for planetID in planetIDs:
        planet = universe.getPlanet(planetID)
        systemID = planet.systemID
        systemIDs.append(systemID)
    return systemIDs
