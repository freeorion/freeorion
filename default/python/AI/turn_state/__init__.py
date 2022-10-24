"""
This module contains tools to work with pre-aggregated information about AI state.
This information is not stored in the save game, so it recalculated each turn.

The main purpose of this module is to separate code that aggregates information and decision-making code.
Decision-making code is smart, and this is the dumb module,
which means that we will try to make this module API to cover all needs of decision-making code, in order to reduce extra manipulation.
If getting information requires more than one line it might be a candidate to a function in this module.

Module API:
Module API is represented as several functions.
All public functions are just lazy objects.
They are initialized upon the first request each turn, and the value is then cached until the end of that turn.

Avoiding classes that are exposed to outer scope requires some additional calls inside functions,
 but this price is not too high to get a simple API.

The module is split into sub-modules, based on the idea that each submodule should work with the same set of data.
This allows achieving the following: each submodule is relatively small and is therefore easy to understand and extend.

What should be put to the module:
If your code works with freeOrionAIInterface only and data can be queried each time, this code is a candidate for this module.

How to organize data inside that module:
Simple rules: keep files small and dependencies between sub-modules under control.
If you don't sure which sub-modules should be chosen - consider creating a new one.
If your code adds new sub-modules dependency to existing sub-modules - consider creating a new one.
"""

from turn_state._empire_resources import (
    computronium_candidates,
    have_asteroids,
    have_computronium,
    have_gas_giant,
    have_honeycomb,
    have_nest,
    have_ruins,
    have_worldtree,
    honeycomb_candidates,
    luxury_resources,
    owned_asteroid_coatings,
    population_with_industry_focus,
    population_with_research_focus,
    set_have_asteroids,
    set_have_gas_giant,
    set_have_nest,
)
from turn_state._planet_state import (
    get_all_empire_planets,
    get_colonized_planets,
    get_colonized_planets_in_system,
    get_empire_outposts,
    get_empire_planets_by_species,
    get_empire_planets_with_species,
    get_inhabited_planets,
    get_number_of_colonies,
    get_owned_planets,
    get_owned_planets_in_system,
    get_unowned_empty_planets,
)
from turn_state._supply_state import (
    get_distance_to_enemy_supply,
    get_supply_group,
    get_supply_group_id,
    get_system_supply,
    get_systems_by_supply_tier,
    supply_connected,
)
