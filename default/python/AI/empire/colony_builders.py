from typing import Dict, List, Mapping, Sequence, Union

import AIDependencies
from common.fo_typing import PlanetId, SpeciesName
from freeorion_tools import tech_is_complete
from freeorion_tools.caching import cache_for_current_turn


def set_colony_builders(species_name: SpeciesName, yards: Sequence[PlanetId]):
    """
    Add planet where you can build colonies for species.

    Warning! Temporal coupling.
    All calls of this function should be done before using of this information.
    """
    empire_colonizers = _get_colony_builders()
    empire_colonizers.setdefault(species_name, []).extend(yards)


def can_build_colony_for_species(species_name: Union[SpeciesName, str]):
    return species_name in get_colony_builders()


def get_colony_builder_locations(species_name: SpeciesName) -> List[PlanetId]:
    return get_colony_builders()[species_name]


def can_build_only_sly_colonies():
    """
    Return true if empire could build only SP_SLY colonies.

    This could be possible only on early stage, when no other races are conquered
    and techs like EXOBOTS are not invented.

    This race has poor supply and live only on gas giants.
    """
    return list(get_colony_builders()) == ["SP_SLY"]


def get_colony_builders() -> Mapping[SpeciesName, List[PlanetId]]:
    """
    Return map from the species to list of the planet where you could build a colony ship with it.
    """
    return _get_colony_builders()


@cache_for_current_turn
def _get_colony_builders() -> Dict[SpeciesName, List[PlanetId]]:
    """
    Return mutable state.
    """
    colony_build_locations = {}

    # get it into colonizer list even if no colony yet
    if tech_is_complete(AIDependencies.EXOBOT_TECH_NAME):
        colony_build_locations["SP_EXOBOT"] = []

    # get it into colonizer list even if no colony yet
    for spec_name in AIDependencies.EXTINCT_SPECIES:
        if tech_is_complete("TECH_COL_" + spec_name):
            colony_build_locations["SP_" + spec_name] = []
    return colony_build_locations
