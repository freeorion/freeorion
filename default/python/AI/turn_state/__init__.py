"""
This module contains tools to work with pre-aggregated information about AI state.
This information is not stored in the save game and recalculated each turn.

The main purpose of this module is to separate code that aggregates information and code that makes a decision.
Decision-making code is smart and this is the dumb module,
that means that we try to make this module API as comfortable as possible.

Module API:
Module API is represented as several functions.
All public functions just lazy objects.
They initialized with the first request and cache its value till the end of the turn.

Avoiding classes that are exposed to outer scope requires some additional calls inside functions,
 but this price is not too high to get a simple API.

The module is split into submodules by the idea that each submodule works with the same set of data.
This allows to achieve the following:
- Each submodule is relatively small, so it is easy to understand and easy to extended;
"""

from turn_state._dry_dock_state import get_empire_drydocks
from turn_state._empire_resources import (
    have_asteroids, have_computronium,
    have_gas_giant, have_nest, have_ruins,
    population_with_industry_focus, population_with_research_focus,
    set_have_asteroids, set_have_gas_giant, set_have_nest,
)
from turn_state._pilot_ratings import (
    best_pilot_rating, medium_pilot_rating, set_best_pilot_rating,
    set_medium_pilot_rating,
)
from turn_state._planet_state import (
    get_all_empire_planets, get_colonized_planets, get_colonized_planets_in_system,
    get_empire_outposts, get_empire_planets_by_species,
    get_empire_planets_with_species, get_inhabited_planets, get_number_of_colonies,
    get_owned_planets, get_owned_planets_in_system, get_unowned_empty_planets,
)
from turn_state._supply_state import get_distance_to_enemy_supply, get_system_supply, get_systems_by_supply_tier
