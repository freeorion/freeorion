import copy
import freeOrionAIInterface as fo
from logging import debug, error, info, warning

from AIDependencies import INVALID_ID
from freeorion_tools import (
    assertion_fails,
    get_ship_part,
)
from freeorion_tools.design_compare import recursive_dict_diff
from turn_state import get_inhabited_planets


def build_cache_key(hullname: str, partlist: list[str]) -> str:
    """
    This reference name is used to identify existing designs and is mapped
    by Cache.map_reference_design_name to the ingame design name. Order of components are ignored.

    :param hullname: hull name
    :param partlist: list of part names
    :return: reference name
    """
    return "{}-{}".format(hullname, "-".join(sorted(partlist)))  # "Hull-Part1-Part2-Part3-Part4"


class ShipDesignCache:
    """This class handles the caching of information used to assess and build shipdesigns in this module.

    Important methods:
    update_for_new_turn(self): Updates the cache for the current turn, to be called once at the beginning of each turn.

    Important members:
    design_id_by_name          # {"designname": designid}
    part_by_partname           # {"partname": part object}
    map_reference_design_name  # {"reference_designname": "ingame_designname"}, cf. _build_reference_name()
    hulls_for_planets          # buildable hulls per planet {planetID: ["buildableHull1", "buildableHull2", ...]}
    parts_for_planets          # buildable parts per planet and slot: {planetID: {slottype1: ["part1", "part2"]}}
    best_designs               # {shipclass: {reqTup: {species: {available_parts: {hull: (rating, best_parts)}}}}}
    production_cost            # {planetID: {"partname1": local_production_cost, "hullname1": local_production_cost}}
    production_time            # {planetID: {"partname1": local_production_time, "hullname1": local_production_time}}

    Debug methods:
    print_CACHENAME(self), e.g. print_hulls_for_planets: prints content of the cache in some nicer format
    print_all(self): calls all the printing functions
    """

    def __init__(self):
        """Cache is empty on creation"""
        self.design_id_by_name = {}
        self.part_by_partname = {}
        self.map_reference_design_name = {}
        self.hulls_for_planets = {}
        self.parts_for_planets = {}
        self.best_designs = {}
        self.production_cost = {}
        self.production_time = {}
        self.last_printed = {}

    def update_for_new_turn(self):
        """Update the cache for the current turn.

        Make sure this function is called once at the beginning of the turn,
        i.e. before any other function of this module is used.
        """
        info(10 * "=" + "Updating ShipDesignCache for new turn" + 10 * "=")
        if not self.map_reference_design_name:
            self._build_cache_after_load()
        self._check_cache_for_consistency()
        self.update_cost_cache()
        self._update_buildable_items_this_turn()

    def print_design_id_by_name(self):
        """Print the design_id_by_name cache."""
        debug("DesignID cache: %s" % self.design_id_by_name)

    def print_part_by_partname(self):
        """Print the part_by_partname cache."""
        debug("Parts cached by name: %s" % self.part_by_partname)

    def print_map_reference_design_name(self):
        """Print the ingame, reference name map of shipdesigns."""
        debug("Design name map: %s" % self.map_reference_design_name)

    def print_hulls_for_planets(self, pid=None):
        """Print the hulls buildable on each planet.

        :param pid: None, int or list of ints
        """
        if pid is None:
            planets = list(self.hulls_for_planets)
        elif isinstance(pid, int):
            planets = [pid]
        elif isinstance(pid, list):
            planets = pid
        else:
            error("Invalid parameter 'pid' for 'print_hulls_for_planets'. Expected int, list or None.")
            return
        debug("Hull-cache:")
        get_planet = fo.getUniverse().getPlanet
        for pid in planets:
            debug(f"{get_planet(pid).name}: {self.hulls_for_planets[pid]}")

    def print_parts_for_planets(self, pid=None):
        """Print the parts buildable on each planet.

        :param pid: int or list of ints
        """
        if pid is None:
            planets = list(self.parts_for_planets)
        elif isinstance(pid, int):
            planets = [pid]
        elif isinstance(pid, list):
            planets = pid
        else:
            error("Invalid parameter 'pid' for 'print_parts_for_planets'. Expected int, list or None.")
            return
        debug("Available parts per planet:")
        get_planet = fo.getUniverse().getPlanet

        for pid in planets:
            debug("  %s:" % get_planet(pid).name)
            for slot in self.parts_for_planets[pid]:
                debug(f"    {slot}: {self.parts_for_planets[pid][slot]}")

    def print_best_designs(self, print_diff_only: bool = True):
        """Print the best designs that were previously found.

        :param print_diff_only: Print only changes to cache since last print
        """
        debug("Currently cached best designs:")
        if print_diff_only:
            print_dict = recursive_dict_diff(self.best_designs, self.last_printed, diff_level_threshold=1)
        else:
            print_dict = self.best_designs
        for classname in print_dict:
            debug(classname)
            cache_name = print_dict[classname]
            for consider_fleet in cache_name:
                debug(4 * " " + str(consider_fleet))
                cache_upkeep = cache_name[consider_fleet]
                for req_tuple in cache_upkeep:
                    debug(8 * " " + str(req_tuple))
                    cache_reqs = cache_upkeep[req_tuple]
                    for tech_tuple in cache_reqs:
                        debug(12 * " " + str(tech_tuple) + " # relevant tech upgrades")
                        cache_techs = cache_reqs[tech_tuple]
                        for species_tuple in cache_techs:
                            debug(16 * " " + str(species_tuple) + " # relevant species stats")
                            cache_species = cache_techs[species_tuple]
                            for av_parts in cache_species:
                                debug(20 * " " + str(av_parts))
                                cache_parts = cache_species[av_parts]
                                for hullname in sorted(cache_parts, reverse=True, key=lambda x: cache_parts[x][0]):
                                    debug(24 * " " + hullname + ":" + str(cache_parts[hullname]))
        self.last_printed = copy.deepcopy(self.best_designs)

    def print_production_cost(self):
        """Print production_cost cache."""
        universe = fo.getUniverse()
        debug("Cached production cost per planet:")
        for pid in self.production_cost:
            debug(f"  {universe.getPlanet(pid).name}: {self.production_cost[pid]}")

    def print_production_time(self):
        """Print production_time cache."""
        universe = fo.getUniverse()
        debug("Cached production cost per planet:")
        for pid in self.production_time:
            debug(f"  {universe.getPlanet(pid).name}: {self.production_time[pid]}")

    def print_all(self):
        """Print the entire ship design cache."""
        debug("Printing the ShipDesignAI cache...")
        self.print_design_id_by_name()
        self.print_part_by_partname()
        self.print_map_reference_design_name()
        self.print_hulls_for_planets()
        self.print_parts_for_planets()
        self.print_best_designs()
        self.print_production_cost()
        self.print_production_time()

    def update_cost_cache(self, partnames=None, hullnames=None):
        """Cache the production cost and time for each part and hull for each inhabited planet for this turn.

        If partnames and hullnames are both None, rebuild Cache with available parts.
        Otherwise, update cache for the specified items.

        :param partnames: iterable
        :param hullnames: iterable
        """
        empire = fo.getEmpire()
        empire_id = empire.empireID

        parts_to_update = set()
        hulls_to_update = set()
        if partnames is None and hullnames is None:
            # clear cache and rebuild with available parts, called at the beginning of the turn
            self.production_cost.clear()
            self.production_time.clear()
            parts_to_update.update(list(empire.availableShipParts))
            hulls_to_update.update(list(empire.availableShipHulls))
        if partnames:
            parts_to_update.update(partnames)
        if hullnames:
            hulls_to_update.update(hullnames)

        # no need to update items we already cached in this turn
        pids = list(get_inhabited_planets())
        if self.production_cost and pids:
            cached_items = set(self.production_cost[pids[0]].keys())
            parts_to_update -= cached_items
            hulls_to_update -= cached_items

        for partname in parts_to_update:
            part = get_ship_part(partname)
            for pid in pids:
                self.production_cost.setdefault(pid, {})[partname] = part.productionCost(empire_id, pid, INVALID_ID)
                self.production_time.setdefault(pid, {})[partname] = part.productionTime(empire_id, pid, INVALID_ID)
        for hullname in hulls_to_update:
            hull = fo.getShipHull(hullname)
            for pid in pids:
                self.production_cost.setdefault(pid, {})[hullname] = hull.productionCost(empire_id, pid, INVALID_ID)
                self.production_time.setdefault(pid, {})[hullname] = hull.productionTime(empire_id, pid, INVALID_ID)

    def _build_cache_after_load(self):
        """Build cache after loading or starting a game.

        This function is supposed to be called after a reload or at the first turn.
        It reads out all the existing ship designs and then updates the following cache:
        - map_reference_design_name
        - design_id_by_name
        """
        if self.map_reference_design_name or self.design_id_by_name:
            warning("ShipDesignAI.Cache._build_cache_after_load() called but cache is not empty.")
        for design_id in fo.getEmpire().allShipDesigns:
            design = fo.getShipDesign(design_id)
            reference_name = build_cache_key(design.hull, design.parts)
            self.map_reference_design_name[reference_name] = design.name
            self.design_id_by_name[design.name] = design_id

    def _check_cache_for_consistency(self):  # noqa: C901
        """Check if the persistent cache is consistent with the gamestate and fix it if not.

        This function should be called once at the beginning of the turn (before update_shipdesign_cache()).
        Especially (only?) in multiplayer games, the shipDesignIDs may sometimes change across turns.
        """
        debug("Checking persistent cache for consistency...")
        try:
            for partname in self.part_by_partname:
                cached_name = self.part_by_partname[partname].name
                if cached_name != partname:
                    self.part_by_partname[partname] = fo.getShipPart(partname)
                    error(f"Part cache corrupted. Expected: {partname}, got: {cached_name}. Cache was repaired.")
        except Exception as e:
            self.part_by_partname.clear()
            error(e, exc_info=True)

        corrupted = []
        # create a copy of the dict-keys so we can alter the dict
        for designname in list(self.design_id_by_name):
            # dropping invalid designs from cache
            if self.design_id_by_name[designname] == INVALID_ID:
                del self.design_id_by_name[designname]
                continue
            try:
                cached_name = fo.getShipDesign(self.design_id_by_name[designname]).name
                if cached_name != designname:
                    warning(f"ShipID cache corrupted. Expected: {designname}, got: {cached_name}.")
                    design_id = next(
                        iter(
                            [
                                shipDesignID
                                for shipDesignID in fo.getEmpire().allShipDesigns
                                if designname == fo.getShipDesign(shipDesignID).name
                            ]
                        ),
                        None,
                    )
                    if design_id is not None:
                        self.design_id_by_name[designname] = design_id
                    else:
                        corrupted.append(designname)
            except AttributeError:
                warning("ShipID cache corrupted. Could not get cached shipdesign. Repairing Cache.", exc_info=True)
                design_id = next(
                    iter(
                        [
                            shipDesignID
                            for shipDesignID in fo.getEmpire().allShipDesigns
                            if designname == fo.getShipDesign(shipDesignID).name
                        ]
                    ),
                    None,
                )
                if design_id is not None:
                    self.design_id_by_name[designname] = design_id
                else:
                    corrupted.append(designname)
        for corrupted_entry in corrupted:
            del self.design_id_by_name[corrupted_entry]
            bad_ref = next(
                iter([_key for _key, _val in self.map_reference_design_name.items() if _val == corrupted_entry]), None
            )
            if bad_ref is not None:
                del self.map_reference_design_name[bad_ref]

    def _update_buildable_items_this_turn(self):
        """Calculate which parts and hulls can be built on each planet this turn."""
        self.hulls_for_planets.clear()
        self.parts_for_planets.clear()
        empire = fo.getEmpire()
        all_hulls = list(empire.availableShipHulls)
        all_parts = list(empire.availableShipParts)

        for pid in get_inhabited_planets():
            for hull_name in all_hulls:
                hull = fo.getShipHull(hull_name)
                if assertion_fails(hull is not None):
                    continue

                if hull.productionLocation(pid):
                    self.hulls_for_planets.setdefault(pid, []).append(hull_name)

            for part_name in all_parts:
                ship_part = get_ship_part(part_name)
                if assertion_fails(ship_part is not None):
                    continue

                slot_types = ship_part.mountableSlotTypes
                if ship_part.productionLocation(pid):
                    for slot_type in slot_types:
                        self.parts_for_planets.setdefault(pid, {}).setdefault(slot_type, []).append(part_name)
