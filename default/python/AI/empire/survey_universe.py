import freeOrionAIInterface as fo
from logging import info

import AIDependencies
import AIstate
from AIDependencies import Tags
from aistate_interface import get_aistate
from colonization import rate_piloting, special_is_nest
from colonization.calculate_planet_colonization_rating import empire_metabolisms
from colonization.calculate_population import active_growth_specials
from common.listeners import register_pre_handler
from common.print_utils import Bool, Sequence, Table, Text
from empire.buildings_locations import set_building_locations
from empire.colony_builders import can_build_colony_for_species, set_colony_builders
from empire.colony_status import (
    set_colonies_is_under_attack,
    set_colonies_is_under_treat,
)
from empire.growth_specials import set_growth_special
from empire.pilot_rating import set_pilot_rating_for_planet
from empire.ship_builders import get_ship_builder_locations, set_ship_builders
from empire.survey_lock import survey_universe_lock
from EnumsAI import FocusType
from freeorion_tools import get_partial_visibility_turn, get_species_tag_grade
from freeorion_tools.timers import AITimer
from turn_state import (
    get_empire_planets_by_species,
    get_owned_planets,
    set_have_asteroids,
    set_have_gas_giant,
    set_have_nest,
)

register_pre_handler("generateOrders", survey_universe_lock.lock)

survey_timer = AITimer("empire.surver_universe()")


def survey_universe():  # noqa: C901
    survey_timer.start("Categorizing Visible Planets")
    universe = fo.getUniverse()
    empire_id = fo.empireID()
    current_turn = fo.currentTurn()

    AIstate.empireStars.clear()
    empire_metabolisms.clear()
    active_growth_specials.clear()

    # var setup done
    aistate = get_aistate()
    for sys_id in universe.systemIDs:
        system = universe.getSystem(sys_id)
        if not system:
            continue
        local_ast = False
        local_gg = False
        empire_has_qualifying_planet = False
        if sys_id in AIstate.colonyTargetedSystemIDs:
            empire_has_qualifying_planet = True
        for pid in system.planetIDs:
            planet = universe.getPlanet(pid)
            if not planet:
                continue
            if planet.size == fo.planetSize.asteroids:
                local_ast = True
            elif planet.size == fo.planetSize.gasGiant:
                local_gg = True
            spec_name = planet.speciesName
            this_spec = fo.getSpecies(spec_name)
            owner_id = planet.owner
            planet_population = planet.currentMeterValue(fo.meterType.population)
            buildings_here = [universe.getBuilding(bldg).buildingTypeName for bldg in planet.buildingIDs]
            weapons_grade = "WEAPONS_0.0"
            if owner_id == empire_id:
                if planet_population > 0.0 and this_spec:
                    empire_has_qualifying_planet = True
                    for metab in [tag for tag in this_spec.tags if tag in AIDependencies.metabolismBoostMap]:
                        empire_metabolisms[metab] = empire_metabolisms.get(metab, 0.0) + planet.habitableSize
                    if this_spec.canProduceShips:
                        pilot_val = rate_piloting(spec_name)
                        weapons_grade = "WEAPONS_%.1f" % pilot_val
                        set_pilot_rating_for_planet(pid, pilot_val)
                        yard_here = []
                        if "BLD_SHIPYARD_BASE" in buildings_here:
                            set_ship_builders(spec_name, pid)
                            yard_here = [pid]
                        if this_spec.canColonize and planet.currentMeterValue(fo.meterType.targetPopulation) >= 3:
                            set_colony_builders(spec_name, yard_here)
                set_building_locations(weapons_grade, buildings_here, pid)

                for special in planet.specials:
                    if special_is_nest(special):
                        set_have_nest()
                    if special in AIDependencies.metabolismBoosts:
                        set_growth_special(special, pid)
                        if planet.focus == FocusType.FOCUS_GROWTH:
                            active_growth_specials.setdefault(special, []).append(pid)
            elif owner_id != -1:
                if get_partial_visibility_turn(pid) >= current_turn - 1:  # only interested in immediately recent data
                    aistate.misc.setdefault("enemies_sighted", {}).setdefault(current_turn, []).append(pid)

        if empire_has_qualifying_planet:
            if local_ast:
                set_have_asteroids()
            elif local_gg:
                set_have_gas_giant()

        if sys_id in get_owned_planets():
            AIstate.empireStars.setdefault(system.starType, []).append(sys_id)
            sys_status = aistate.systemStatus.setdefault(sys_id, {})
            if sys_status.get("fleetThreat", 0) > 0:
                set_colonies_is_under_attack()
            if sys_status.get("neighborThreat", 0) > 0:
                set_colonies_is_under_treat()

    survey_universe_lock.unlock()
    survey_timer.stop()

    # lock must be released before this, since it uses the locked functions
    _print_empire_species_roster()


def _print_empire_species_roster():
    """Print empire species roster in table format to log."""
    grade_map = {
        "ULTIMATE": "+++",
        "GREAT": "++",
        "GOOD": "+",
        "AVERAGE": "o",
        "BAD": "-",
        "VERY_BAD": "--",
        "EXTREMELY_BAD": "---",
        "NO": "x",
    }

    grade_tags = [
        Tags.INDUSTRY,
        Tags.RESEARCH,
        Tags.INFLUENCE,
        Tags.POPULATION,
        Tags.SUPPLY,
        Tags.WEAPONS,
        Tags.ATTACKTROOPS,
    ]

    grade_tags_names = {
        Tags.INDUSTRY: "Ind.",
        Tags.RESEARCH: "Res.",
        Tags.INFLUENCE: "Infl.",
        Tags.POPULATION: "Pop.",
        Tags.SUPPLY: "Supply",
        Tags.WEAPONS: "Pilot",
        Tags.ATTACKTROOPS: "Troop",
    }

    species_table = Table(
        Text("species"),
        Sequence("PIDs"),
        Bool("Colonize"),
        Text("Shipyards"),
        *[Text(grade_tags_names[v]) for v in grade_tags],
        Sequence("Tags"),
        table_name="Empire species roster Turn %d" % fo.currentTurn(),
    )
    for species_name, planet_ids in get_empire_planets_by_species().items():
        species_tags = fo.getSpecies(species_name).tags
        number_of_shipyards = len(get_ship_builder_locations(species_name))
        species_table.add_row(
            species_name,
            planet_ids,
            can_build_colony_for_species(species_name),
            number_of_shipyards,
            *[grade_map.get(get_species_tag_grade(species_name, tag).upper(), "o") for tag in grade_tags],
            [tag for tag in species_tags if not any(s in tag for s in grade_tags) and "PEDIA" not in tag],
        )
    info(species_table)
