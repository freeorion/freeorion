from common.configure_logging import redirect_logging_to_freeorion_logger

# Logging is redirected before other imports so that import errors appear in log files.
redirect_logging_to_freeorion_logger()

import sys
from random import random, uniform, choice
from math import sin, cos, pi, hypot

import freeorion as fo
from universe_tables import MONSTER_FREQUENCY


def execute_turn_events():
    print "Executing turn events for turn", fo.current_turn()

    # creating fields
    systems = fo.get_systems()
    radius = fo.get_universe_width() / 2.0
    if random() < max(0.0003 * radius, 0.03):
        if random() < 0.4:
            field_type = "FLD_MOLECULAR_CLOUD"
            size = 5.0
        else:
            field_type = "FLD_ION_STORM"
            size = 5.0

        x = y = radius
        dist_from_center = 0.0
        while (dist_from_center < radius) or any(hypot(fo.get_x(s) - x, fo.get_y(s) - y) < 50.0 for s in systems):
            angle = random() * 2.0 * pi
            dist_from_center = radius + uniform(min(max(radius * 0.02, 10), 50.0), min(max(radius * 0.05, 20), 100.0))
            x = radius + (dist_from_center * sin(angle))
            y = radius + (dist_from_center * cos(angle))

        print "...creating new", field_type, "field, at distance", dist_from_center, "from center"
        if fo.create_field(field_type, x, y, size) == fo.invalid_object():
            print >> sys.stderr, "Turn events: couldn't create new field"

    # creating monsters
    gsd = fo.get_galaxy_setup_data()
    monster_freq = MONSTER_FREQUENCY[gsd.monsterFrequency]
    # monster freq ranges from 1/30 (= one monster per 30 systems) to 1/3 (= one monster per 3 systems)
    # (example: low monsters and 150 Systems results in 150 / 30 * 0.01 = 0.05)
    if monster_freq > 0 and random() < len(systems) * monster_freq * 0.01:
        # only spawn Krill at the moment, other monsters can follow in the future
        if random() < 1:
            monster_type = "SM_KRILL_1"
        else:
            monster_type = "SM_FLOATER"

        # search for systems without planets or fleets
        candidates = [s for s in systems if len(fo.sys_get_planets(s)) <= 0 and len(fo.sys_get_fleets(s)) <= 0]
        if not candidates:
            print >> sys.stderr, "Turn events: unable to find system for monster spawn"
        else:
            system = choice(candidates)
            print "...creating new", monster_type, "at", fo.get_name(system)

            # create monster fleet
            monster_fleet = fo.create_monster_fleet(system)
            # if fleet creation fails, report an error
            if monster_fleet == fo.invalid_object():
                print >> sys.stderr, "Turn events: unable to create new monster fleet"
            else:
                # create monster, if creation fails, report an error
                monster = fo.create_monster(monster_type, monster_fleet)
                if monster == fo.invalid_object():
                    print >> sys.stderr, "Turn events: unable to create monster in fleet"

    if fo.get_galaxy_setup_data().startingEra == fo.startingEra.prewarp:
        tutorial_events()

    return True


def tutorial_events():
    current_turn = fo.current_turn()
    shipyards_built = tutorial_find_shipyards()
    for empire_id in fo.get_all_empires():
        empire = fo.get_empire(empire_id)
        sent = 0
        if current_turn == 1:
            tutorial_fleet_intro(empire)
            sent = 1
        sent += tutorial_production_intro(empire)
        if empire_id in shipyards_built or sent == 0:
            shown = tutorial_find_shown(empire)
        if empire_id in shipyards_built:
            sent += tutorial_production_ship(
                empire, shipyards_built[empire_id] == current_turn)
            if "colony_pod" not in shown:
                sent += tutotial_colony_pod(empire)
        if sent == 0 and "focus" not in shown:
            fo.generate_sitrep(
                empire_id,
                "SITREP_TUTORIAL_FOCUS",
                "icons/sitrep/beginner_hint.png",
                "TUTORIAL_HINTS"
            )


# Find the oldest basic shipyard built by each empire.
def tutorial_find_shipyards():
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
    system_id = fo.get_universe().getPlanet(empire.capitalID).systemID
    fleet_id = fo.create_fleet("", system_id, empire.empireID)
    fo.create_ship("", "SD_SCOUT", "", fleet_id)
    print "tutorial: initial scout for", empire.name, \
        "system =", system_id, "fleet =", fleet_id
    fo.generate_sitrep(
        empire.empireID,
        "SITREP_TUTORIAL_FLEET_INTRO",
        "icons/sitrep/beginner_hint.png",
        "TUTORIAL_HINTS"
    )


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
    res_techs = empire.researchedTechs
    turn = current_turn + 1
    for tech in building_techs:
        if tech not in res_techs:
            continue
        turn = res_techs[tech]
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
    if "SHP_CONSTRUCTION" in res_techs and \
            res_techs["SHP_CONSTRUCTION"] == current_turn:
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


# Check which sitreps were already shown to this empire.
def tutorial_find_shown(empire):
    shown = {}
    num_sitreps = empire.numSitReps(fo.INVALID_GAME_TURN)
    sitrep_index = 0
    while sitrep_index < num_sitreps:
        sitrep = empire.getSitRep(sitrep_index)
        if sitrep.typeString == "SITREP_TUTORIAL_OUTPOST_POD" or \
                sitrep.typeString == "SITREP_TUTORIAL_COLONY_POD":
            print "tutotial_colony_pod shown to", empire.name, \
                "on turn", sitrep.getTurn
            shown["colony_pod"] = sitrep.getTurn
        elif sitrep.typeString == "SITREP_TUTORIAL_FOCUS":
            print "tutotial_focus shown to", empire.name, \
                "on turn", sitrep.getTurn
            shown["focus"] = sitrep.getTurn
        if len(shown) == 2:
            break
        sitrep_index = sitrep_index + 1
    return shown


# Figure out which of the wanted_techs is known longest to the empire
# and return the turn it was researched and its name,
# or current_turn+1 and NONE if none of wanted_techs is known.
def first_tech(empire, wanted_techs):
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


def tutotial_colony_pod(empire):
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
