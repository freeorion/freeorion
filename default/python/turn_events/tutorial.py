import freeorion as fo


def tutorial_events():
    current_turn = fo.current_turn()
    shipyards_built = tutorial_find_shipyards()
    for empire_id in fo.get_all_empires():
        empire = fo.get_empire(empire_id)
        sent = 0
        if current_turn == 1:
            sent = tutorial_fleet_intro(empire)
        sent += tutorial_production_intro(empire)
        if empire_id in shipyards_built or sent == 0:
            shown = tutorial_find_shown(empire)
        if empire_id in shipyards_built:
            sent += tutorial_production_ship(
                empire, shipyards_built[empire_id] == current_turn)
            if "colony_pod" not in shown:
                sent += tutorial_colony_pod(empire)
        if sent == 0 and "focus" not in shown:
            fo.generate_sitrep(
                empire_id,
                "SITREP_TUTORIAL_FOCUS",
                "icons/sitrep/beginner_hint.png",
                "TUTORIAL_HINTS"
            )


def tutorial_find_shipyards():
    """Find the oldest basic shipyard built by each empire."""
    shipyards_built = {}
    universe = fo.get_universe()
    for building_id in universe.buildingIDs:
        building = universe.getBuilding(building_id)
        if building.buildingTypeName != "BLD_SHIPYARD_BASE":
            continue
        empire_id = building.producedByEmpireID
        if empire_id not in shipyards_built or \
                shipyards_built[empire_id] > building.creationTurn:
            print "tutorial: empire", empire_id, \
                "built shipyard in turn", building.creationTurn
            shipyards_built[empire_id] = building.creationTurn
    return shipyards_built


def tutorial_fleet_intro(empire):
    planet = fo.get_universe().getPlanet(empire.capitalID)
    if planet is None:
        print >> sys.stderr, "fleet_intro:", empire.name, \
            "capital planet", empire.capitalID, "does not exist"
        return 0
    system_id = planet.systemID
    fleet_id = fo.create_fleet("", system_id, empire.empireID)
    if fleet_id == fo.invalid_object():
        print >> sys.stderr, "fleet_intro:", \
            "couldn't create initial scout fleet for", empire.name, \
            "in system", system_id
        return 0
    if fo.create_ship("", "SD_SCOUT", "", fleet_id) == fo.invalid_object():
        print >> sys.stderr, "fleet_intro:", \
            "couldn't create initial scout ship for", empire.name, \
            "in fleet", fleet_id
        return 0
    print "fleet_intro: initial scout for", empire.name, \
        "system =", system_id, "fleet =", fleet_id
    fo.generate_sitrep(
        empire.empireID,
        "SITREP_TUTORIAL_FLEET_INTRO",
        "icons/sitrep/beginner_hint.png",
        "TUTORIAL_HINTS"
    )
    return 1


def tutorial_production_intro(empire):
    # all techs that unlock a building but do not require another tech
    # that also unlocks a building
    building_techs = [
        "GRO_GENETIC_ENG",
        "LRN_PHYS_BRAIN",
        "SHP_CONSTRUCTION",
        "SHP_DOMESTIC_MONSTER",
        "CON_ASYMP_MATS",
        "PRO_INDUSTRY_CENTER_I",
        "SPY_DETECT_2",
        "PRO_ORBITAL_GEN",
        "GRO_TERRAFORM",
        "LRN_GRAVITONICS",
        "PRO_EXOBOTS",
        "LRN_SPATIAL_DISTORT_GEN",
        "PRO_NDIM_ASSMB",
        "CON_STARGATE",
        "CON_PLANET_DRIVE",
    ]
    current_turn = fo.current_turn()
    researched_techs = empire.researchedTechs
    turn = current_turn + 1
    for tech in building_techs:
        if tech not in researched_techs:
            continue
        turn = researched_techs[tech]
        if turn < current_turn:
            print "production_intro:", \
                empire.name, "knows", tech, "since turn", turn
            break
        tech_first = tech
    if turn == current_turn:
        print "production_intro sending sitrep:", \
            empire.name, "just discovered", tech_first
        fo.generate_sitrep(
            empire.empireID,
            "SITREP_TUTORIAL_PRODUCTION_INTRO",
            {"tech": tech_first},
            "icons/sitrep/beginner_hint.png",
            "TUTORIAL_HINTS"
        )
        return 1
    if "SHP_CONSTRUCTION" in researched_techs and \
            researched_techs["SHP_CONSTRUCTION"] == current_turn:
        print "production_intro sending sitrep:", \
            empire.name, "just discovered shipyard"
        fo.generate_sitrep(
            empire.empireID,
            "SITREP_TUTORIAL_PRODUCTION_YARD",
            "icons/sitrep/beginner_hint.png",
            "TUTORIAL_HINTS"
        )
        return 1
    return 0


def tutorial_find_shown(empire):
    """Check which sitreps were already shown to this empire."""
    shown = {}
    num_sitreps = empire.numSitReps(fo.INVALID_GAME_TURN)
    sitrep_index = 0
    while sitrep_index < num_sitreps:
        sitrep = empire.getSitRep(sitrep_index)
        if sitrep.typeString == "SITREP_TUTORIAL_OUTPOST_POD" or \
                sitrep.typeString == "SITREP_TUTORIAL_COLONY_POD":
            print "tutorial_colony_pod shown to", empire.name, \
                "on turn", sitrep.getTurn
            shown["colony_pod"] = sitrep.getTurn
        elif sitrep.typeString == "SITREP_TUTORIAL_FOCUS":
            print "tutorial_focus shown to", empire.name, \
                "on turn", sitrep.getTurn
            shown["focus"] = sitrep.getTurn
        if len(shown) == 2:
            break
        sitrep_index = sitrep_index + 1
    return shown


def first_tech(empire, wanted_techs):
    """Figure out which of the wanted_techs is known longest to the empire
    and return the turn it was researched and its name,
    or current_turn+1 and NONE if none of wanted_techs is known.
    """
    turn_first = fo.current_turn() + 1
    tech_first = "NONE"
    for tech in wanted_techs:
        if tech not in empire.researchedTechs:
            print "production_ship:", empire.name, "does not know", tech
            continue
        turn = empire.researchedTechs[tech]
        if turn_first > turn:
            print "production_ship:", empire.name, "knows", tech, \
                "since turn", turn
            turn_first = turn
            tech_first = tech
    return turn_first, tech_first


def tutorial_production_ship(empire, shipyard_is_new):
    hull_techs = [
        "SHP_BASIC_HULL_1",
        "SHP_SMALLORG_HULL",
        "SHP_SPACE_FLUX_BUBBLE",
        "SHP_ASTEROID_HULLS",
        "SHP_FRC_ENRG_COMP",
        "SHP_XENTRONIUM_HULL",
    ]
    hull_techs_internal = [
        "SHP_BASIC_HULL_2",
        "SHP_SMALLORG_HULL",
        "SHP_SPACE_FLUX_BUBBLE",
        "SHP_ASTEROID_HULLS",
        "SHP_ENRG_FRIGATE",
    ]
    colony_techs = [
        "CON_OUTPOST",
        "CON_REMOTE_COL",
    ]
    current_turn = fo.current_turn()

    # sitrep about ship production in general
    hull_turn, hull_tech = first_tech(empire, hull_techs)
    if hull_turn > current_turn:
        print "production_ship sending no ship sitrep:", \
            empire.name, "does not knows any hulls"
        return 0

    sent = 0
    if hull_turn == current_turn:
        print "production_ship sending ship sitrep:", \
            empire.name, "just discovered", hull_tech
        fo.generate_sitrep(
            empire.empireID,
            "SITREP_TUTORIAL_PRODUCTION_SHIP_HULL",
            {"tech": hull_tech},
            "icons/sitrep/beginner_hint.png",
            "TUTORIAL_HINTS"
        )
        sent = 1
    elif shipyard_is_new:
        print "production_ship sending ship sitrep:", \
            empire.name, "just built the first shipyard"
        fo.generate_sitrep(
            empire.empireID,
            "SITREP_TUTORIAL_PRODUCTION_SHIP_YARD",
            "icons/sitrep/beginner_hint.png",
            "TUTORIAL_HINTS"
        )
        sent = 1

    # sitrep about colony/outpost ship production
    colony_turn, colony_tech = first_tech(empire, colony_techs)
    if colony_turn > current_turn:
        print "production_ship sending no colony sitrep:", \
            empire.name, "does not knows any colonization tech"
        return sent

    hull_turn, hull_tech = first_tech(empire, hull_techs_internal)
    if hull_turn > current_turn:
        print "production_ship sending no colony sitrep:", \
            empire.name, "does not knows any hulls with internal slots"
        return sent

    if colony_turn == current_turn:
        print "production_ship sending colony sitrep:", \
            empire.name, "just discovered", colony_tech
        fo.generate_sitrep(
            empire.empireID,
            "SITREP_TUTORIAL_PRODUCTION_COLONY_TECH",
            {"tech": colony_tech},
            "icons/sitrep/beginner_hint.png",
            "TUTORIAL_HINTS"
        )
        return sent + 1
    if hull_turn == current_turn:
        print "production_ship sending colony sitrep:", \
            empire.name, "just discovered", hull_tech
        fo.generate_sitrep(
            empire.empireID,
            "SITREP_TUTORIAL_PRODUCTION_COLONY_TECH",
            {"tech": hull_tech},
            "icons/sitrep/beginner_hint.png",
            "TUTORIAL_HINTS"
        )
        return sent + 1
    if shipyard_is_new:
        print "production_ship sending colony sitrep:", \
            empire.name, "just built the first shipyard"
        fo.generate_sitrep(
            empire.empireID,
            "SITREP_TUTORIAL_PRODUCTION_COLONY_YARD",
            "icons/sitrep/beginner_hint.png",
            "TUTORIAL_HINTS"
        )
        return sent + 1
    return sent


def tutorial_colony_pod(empire):
    owned = empire.shipPartTypesOwned
    if "CO_OUTPOST_POD" in owned:
        print "sending sitrep:", empire.name, "has an outpost pod"
        fo.generate_sitrep(
            empire.empireID,
            "SITREP_TUTORIAL_OUTPOST_POD",
            "icons/sitrep/beginner_hint.png",
            "TUTORIAL_HINTS"
        )
        return 1
    if "CO_COLONY_POD" in owned or "CO_SUSPEND_ANIM_POD" in owned:
        print "sending sitrep:", empire.name, "has a colony pod"
        fo.generate_sitrep(
            empire.empireID,
            "SITREP_TUTORIAL_COLONY_POD",
            "icons/sitrep/beginner_hint.png",
            "TUTORIAL_HINTS"
        )
        return 1
    print "not sending sitrep:", empire.name, \
        "has neither colony nor outpost pods"
    return 0
