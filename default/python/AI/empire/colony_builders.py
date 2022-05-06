from typing import Dict, List, Mapping, Sequence, Union

import AIDependencies
from common.fo_typing import PlanetId, SpeciesName
from empire.survey_lock import survey_universe_lock
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


@survey_universe_lock
def can_build_colony_for_species(species_name: Union[SpeciesName, str]):
    return species_name in get_colony_builders()


@survey_universe_lock
def get_colony_builder_locations(species_name: SpeciesName) -> List[PlanetId]:
    return get_colony_builders()[species_name]


@survey_universe_lock
def can_build_only_sly_colonies():
    """
    Return true if empire could build only SP_SLY colonies.

    This could be possible only on early stage, when no other races are conquered
    and techs like EXOBOTS are not invented.

    This race has poor supply and live only on gas giants.
    """
    return list(get_colony_builders()) == ["SP_SLY"]


@survey_universe_lock
def get_colony_builders() -> Mapping[SpeciesName, List[PlanetId]]:
    """
    Return map from the species to list of the planet where you could build a colony ship with it.
    """
    return _get_colony_builders()


@cache_for_current_turn
def get_extra_colony_builders() -> List[str]:
    """
    Returns species the empire can build without having a colony, i.e. Exobots plus
    extinct species that has been enabled.
    """
    ret = ["SP_EXOBOT"]
    for spec_name in AIDependencies.EXTINCT_SPECIES:
        if tech_is_complete("TECH_COL_" + spec_name):
            ret.append("SP_" + spec_name)
    return ret


@cache_for_current_turn
def _get_colony_builders() -> Dict[SpeciesName, List[PlanetId]]:
    """
    Return mutable state.
    """
    colony_build_locations = {}

    # get it into colonizer list even if no colony yet
    for species in get_extra_colony_builders():
        colony_build_locations[species] = []
    return colony_build_locations
