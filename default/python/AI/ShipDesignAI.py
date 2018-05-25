"""
This module deals with the autonomous shipdesign from the AI. The design process is class-based.
The basic functionality is defined in the class ShipDesigner. The more specialised classes mostly
implement the rating function and some additional information for the optimizing algorithms to improve performance.

Example usage of this module:
import ShipDesignAI
myDesign = ShipDesignAI.WarShipDesigner()
myDesign.additional_specifications.enemy_mine_dmg = 10
best_military_designs = myDesign.optimize_design()  # best designs per planet: (rating,planetID,design_id,cost) tuples


Available ship classes:
- WarShipDesigner: basic military ship
- CarrierShipDesigner: military ship that includes fighter-related parts such as hangars and launch bays
- OrbitalTroopShipDesigner: Troop ships for invasion in the same system
- StandardTroopShipDesigner: Troop ships for invasion of other systems
- OrbitalColonisationShipDesigner: Ships for colonization in the same system
- StandardColonisationShipDesigner: Ships for colonization of other systems
- OrbitalOutpostShipDesigner: Ships for outposting in the same system
- StandardOutpostShipDesigner: Ships for outposting in other systems
- OrbitalDefenseShipDesigner: Ships for stationary defense

Internal use only:
Classes:
- ShipDesignCache: caches information used in this module. Only use the defined instance (variable name "Cache")
- ShipDesigner: base class for all designs. Provides basic and general functionalities
- MilitaryShipDesigner: base class for all military-related ships.
- ColonisationShipDesignerBaseClass: base class for all colonisation ships which provides common functionalities
- OutpostShipDesignerBaseClass: same but for outpost ships
- TroopShipDesignerBaseClass: same but for troop ships
- AdditionalSpecifications:  Defines all requirements we have for our designs such as minimum fuel or minimum speed.

global variables:
- Cache: Instance of the ShipDesignCache class - all cached information is stored in here.
"""

# TODO: Add limit on production cost based on the total PP output of the empire if possible (only military?)
# TODO: Use distance to colonisable planets as a modifier to the rating of colonization/outposting ships
# TODO: Use the distance to next colonisable planets as base for speed modifiers for troops/colo/outpost ships
# TODO: For stable release, comment out the profiling functionality
# TODO: Implement a better system for the new weapon upgrade functionality:
#       - _calculate_weapon_strength() may be removed
#       - Filtering the weapon parts must be updated: current cache does not consider tech upgrades, weapons are ignored
import copy
import math
from collections import Counter, defaultdict
from logging import debug, info, warn, error

import freeOrionAIInterface as fo
from aistate_interface import get_aistate
import AIDependencies
import CombatRatingsAI
import FleetUtilsAI
from AIDependencies import INVALID_ID
from freeorion_tools import UserString, tech_is_complete
from turn_state import state

# Define meta classes for the ship parts  TODO storing as set may not be needed anymore
ARMOUR = frozenset({fo.shipPartClass.armour})
SHIELDS = frozenset({fo.shipPartClass.shields})
DETECTION = frozenset({fo.shipPartClass.detection})
STEALTH = frozenset({fo.shipPartClass.stealth})
FUEL = frozenset({fo.shipPartClass.fuel})
COLONISATION = frozenset({fo.shipPartClass.colony})
ENGINES = frozenset({fo.shipPartClass.speed})
TROOPS = frozenset({fo.shipPartClass.troops})
WEAPONS = frozenset({fo.shipPartClass.shortRange})
GENERAL = frozenset({fo.shipPartClass.general})
FIGHTER_BAY = frozenset({fo.shipPartClass.fighterBay})
FIGHTER_HANGAR = frozenset({fo.shipPartClass.fighterHangar})
ALL_META_CLASSES = frozenset({WEAPONS, ARMOUR, DETECTION, FUEL, STEALTH, SHIELDS,
                              COLONISATION, ENGINES, TROOPS, GENERAL})

# Prefixes for the test ship designs
TESTDESIGN_NAME_BASE = "AI_TESTDESIGN"
TESTDESIGN_NAME_HULL = TESTDESIGN_NAME_BASE + "_HULL"
TESTDESIGN_NAME_PART = TESTDESIGN_NAME_BASE + "_PART"

# Hardcoded preferred hullname for testdesigns, should be a hull without conditions but with maximum different slottypes
TESTDESIGN_PREFERRED_HULL = "SH_BASIC_MEDIUM"

MISSING_REQUIREMENT_MULTIPLIER = -1000
INVALID_DESIGN_RATING = -999  # this needs to be negative but greater than MISSING_REQUIREMENT_MULTIPLIER

# For Trooper designs, set a maximum number of troopers per ship that will be considered for ratings purposes, to
# prevent the AI from settling on trooper designs with the absolute largest hulls, which may nominally have the lowest
# per-unit-cost but which then result in much trooper overkill/waste when actually deployed.  The application of this
# limit does (in normal use) take into account shipbuilder species modifiers.
# Current limit set to 48 to allow up to a grav hull filled with Advanced Troop Pods and Good offensive troopers (or
# partly filled with Great advanced offensive troopers)
MAX_TROOPERS_PER_SHIP = 48

# Potentially, not adding techs to AIDependencies is intended for testing purposes.
# Therefore, chat the player only once to inform him about the issue to prevent spam.
_raised_warnings = set()

# string constants for better readability of the cache
WITH_UPKEEP = "considering fleet upkeep"
WITHOUT_UPKEEP = "not considering fleet upkeep"


def _get_capacity(x):
    return x.capacity


class ShipDesignCache(object):
    """This class handles the caching of information used to assess and build shipdesigns in this module.

    Important methods:
    update_for_new_turn(self): Updates the cache for the current turn, to be called once at the beginning of each turn.

    Important members:
    testhulls:                 # set of all hullnames used for testdesigns
    design_id_by_name          # {"designname": designid}
    part_by_partname           # {"partname": part object}
    map_reference_design_name  # {"reference_designname": "ingame_designname"}, cf. _build_reference_name()
    strictly_worse_parts       # strictly worse parts: {"part": ["worsePart1", "worsePart2"]}
    hulls_for_planets          # buildable hulls per planet {planetID: ["buildableHull1", "buildableHull2", ...]}
    parts_for_planets          # buildable parts per planet and slot: {planetID: {slottype1: ["part1", "part2"]}}
    best_designs               # {shipclass: {reqTup: {species: {available_parts: {hull: (rating, best_parts)}}}}}
    production_cost            # {planetID: {"partname1": local_production_cost, "hullname1": local_production_cost}}
    production_time            # {planetID: {"partname1": local_production_time, "hullname1": local_production_time}}

    Debug methods:
    print_CACHENAME(self), e.g. print_testhulls: prints content of the cache in some nicer format
    print_all(self): calls all the printing functions
    """

    def __init__(self):
        """Cache is empty on creation"""
        self.testhulls = set()
        self.design_id_by_name = {}
        self.part_by_partname = {}
        self.map_reference_design_name = {}
        self.strictly_worse_parts = {}
        self.hulls_for_planets = {}
        self.parts_for_planets = {}
        self.best_designs = {}
        self.production_cost = {}
        self.production_time = {}
        self.last_printed = {}

    def update_for_new_turn(self):
        """ Update the cache for the current turn.

        Make sure this function is called once at the beginning of the turn,
        i.e. before any other function of this module is used.
        """
        info(10 * "=" + "Updating ShipDesignCache for new turn" + 10 * "=")
        if not self.map_reference_design_name:
            self._build_cache_after_load()
        self._check_cache_for_consistency()
        self.update_cost_cache()
        self._update_buildable_items_this_turn(verbose=False)

    def print_testhulls(self):
        """Print the testhulls cache."""
        debug("Testhull cache: %s" % self.testhulls)

    def print_design_id_by_name(self):
        """Print the design_id_by_name cache."""
        debug("DesignID cache: %s" % self.design_id_by_name)

    def print_part_by_partname(self):
        """Print the part_by_partname cache."""
        debug("Parts cached by name: %s" % self.part_by_partname)

    def print_strictly_worse_parts(self):
        """Print the strictly_worse_parts cache."""
        debug("List of strictly worse parts (ignoring slots):")
        for part in self.strictly_worse_parts:
            debug("  %s: %s" % (part, self.strictly_worse_parts[part]))

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
            debug("%s: %s" % (get_planet(pid).name, self.hulls_for_planets[pid]))

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
                debug("    %s: %s" % (slot, self.parts_for_planets[pid][slot]))

    def print_best_designs(self, print_diff_only=True):
        """Print the best designs that were previously found.

        :param print_diff_only: Print only changes to cache since last print
        :type print_diff_only: bool
        """
        debug("Currently cached best designs:")
        if print_diff_only:
            print_dict = {}
            recursive_dict_diff(self.best_designs, self.last_printed, print_dict, diff_level_threshold=1)
        else:
            print_dict = self.best_designs
        for classname in print_dict:
            debug(classname)
            cache_name = print_dict[classname]
            for consider_fleet in cache_name:
                debug(4*" " + str(consider_fleet))
                cache_upkeep = cache_name[consider_fleet]
                for req_tuple in cache_upkeep:
                    debug(8*" " + str(req_tuple))
                    cache_reqs = cache_upkeep[req_tuple]
                    for tech_tuple in cache_reqs:
                        debug(12*" " + str(tech_tuple) + " # relevant tech upgrades")
                        cache_techs = cache_reqs[tech_tuple]
                        for species_tuple in cache_techs:
                            debug(16*" " + str(species_tuple) + " # relevant species stats")
                            cache_species = cache_techs[species_tuple]
                            for av_parts in cache_species:
                                debug(20*" " + str(av_parts))
                                cache_parts = cache_species[av_parts]
                                for hullname in sorted(cache_parts, reverse=True, key=lambda x: cache_parts[x][0]):
                                    debug(24*" " + hullname + ":" + str(cache_parts[hullname]))
        self.last_printed = copy.deepcopy(self.best_designs)

    def print_production_cost(self):
        """Print production_cost cache."""
        universe = fo.getUniverse()
        debug("Cached production cost per planet:")
        for pid in self.production_cost:
            debug("  %s: %s" % (universe.getPlanet(pid).name, self.production_cost[pid]))

    def print_production_time(self):
        """Print production_time cache."""
        universe = fo.getUniverse()
        debug("Cached production cost per planet:")
        for pid in self.production_time:
            debug("  %s: %s" % (universe.getPlanet(pid).name, self.production_time[pid]))

    def print_all(self):
        """Print the entire ship design cache."""
        debug("Printing the ShipDesignAI cache...")
        self.print_testhulls()
        self.print_design_id_by_name()
        self.print_part_by_partname()
        self.print_strictly_worse_parts()
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
        pids = list(state.get_inhabited_planets())
        if self.production_cost and pids:
            cached_items = set(self.production_cost[pids[0]].keys())
            parts_to_update -= cached_items
            hulls_to_update -= cached_items

        for partname in parts_to_update:
            part = get_part_type(partname)
            for pid in pids:
                self.production_cost.setdefault(pid, {})[partname] = part.productionCost(empire_id, pid)
                self.production_time.setdefault(pid, {})[partname] = part.productionTime(empire_id, pid)
        for hullname in hulls_to_update:
            hull = fo.getHullType(hullname)
            for pid in pids:
                self.production_cost.setdefault(pid, {})[hullname] = hull.productionCost(empire_id, pid)
                self.production_time.setdefault(pid, {})[hullname] = hull.productionTime(empire_id, pid)

    def _build_cache_after_load(self):
        """Build cache after loading or starting a game.

        This function is supposed to be called after a reload or at the first turn.
        It reads out all the existing ship designs and then updates the following cache:
        - map_reference_design_name
        - design_id_by_name
        """
        if self.map_reference_design_name or self.design_id_by_name:
            warn("ShipDesignAI.Cache._build_cache_after_load() called but cache is not empty.")
        for design_id in fo.getEmpire().allShipDesigns:
            design = fo.getShipDesign(design_id)
            if TESTDESIGN_NAME_BASE in design.name:
                continue
            reference_name = _build_reference_name(design.hull, design.parts)
            self.map_reference_design_name[reference_name] = design.name
            self.design_id_by_name[design.name] = design_id

    def _check_cache_for_consistency(self):
        """Check if the persistent cache is consistent with the gamestate and fix it if not.

        This function should be called once at the beginning of the turn (before update_shipdesign_cache()).
        Especially (only?) in multiplayer games, the shipDesignIDs may sometimes change across turns.
        """
        debug("Checking persistent cache for consistency...")
        try:
            for partname in self.part_by_partname:
                cached_name = self.part_by_partname[partname].name
                if cached_name != partname:
                    self.part_by_partname[partname] = fo.getPartType(partname)
                    error("Part cache corrupted. Expected: %s, got: %s. Cache was repaired." % (partname, cached_name))
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
                    warn("ShipID cache corrupted. Expected: %s, got: %s." % (designname, cached_name))
                    design_id = next(iter([shipDesignID for shipDesignID in fo.getEmpire().allShipDesigns
                                          if designname == fo.getShipDesign(shipDesignID).name]), None)
                    if design_id is not None:
                        self.design_id_by_name[designname] = design_id
                    else:
                        corrupted.append(designname)
            except AttributeError:
                warn("ShipID cache corrupted. Could not get cached shipdesign. Repairing Cache.", exc_info=True)
                design_id = next(iter([shipDesignID for shipDesignID in fo.getEmpire().allShipDesigns
                                      if designname == fo.getShipDesign(shipDesignID).name]), None)
                if design_id is not None:
                    self.design_id_by_name[designname] = design_id
                else:
                    corrupted.append(designname)
        for corrupted_entry in corrupted:
            del self.design_id_by_name[corrupted_entry]
            bad_ref = next(iter([_key for _key, _val in self.map_reference_design_name.iteritems()
                                 if _val == corrupted_entry]), None)
            if bad_ref is not None:
                del self.map_reference_design_name[bad_ref]

    def _update_buildable_items_this_turn(self, verbose=False):
        """Calculate which parts and hulls can be built on each planet this turn.

        :param verbose: toggles detailed debugging output.
        :type verbose: bool
        """
        # TODO: Refactor this function
        # The AI currently has no way of checking building requirements of individual parts and hulls directly.
        # It can only check if we can build a design. Therefore, we use specific testdesigns to check if we can
        # build a hull or part.
        # The building requirements are constant so calculate this only once at the beginning of each turn.
        #
        # Code structure:
        #   1. Update hull test designs
        #   2. Get a list of buildable ship hulls for each planet
        #   3. Update ship part test designs
        #   4. Cache the list of buildable ship parts for each planet
        #
        self.hulls_for_planets.clear()
        self.parts_for_planets.clear()
        inhabited_planets = state.get_inhabited_planets()
        if not inhabited_planets:
            debug("No inhabited planets found. The design process was aborted.")
            return
        get_shipdesign = fo.getShipDesign
        get_hulltype = fo.getHullType
        empire = fo.getEmpire()
        empire_id = empire.empireID
        universe = fo.getUniverse()
        available_hulls = list(empire.availableShipHulls)   # copy so we can sort it locally
        # Later on in the code, we need to find suitable testhulls, i.e. buildable hulls for all slottypes.
        # To reduce the number of lookups, move the hardcoded TESTDESIGN_PREFERED_HULL to the front of the list.
        # This hull should be buildable on each planet and also cover the most common slottypes.
        try:
            idx = available_hulls.index(TESTDESIGN_PREFERRED_HULL)
            available_hulls[0], available_hulls[idx] = available_hulls[idx], available_hulls[0]
        except ValueError:
            warn("Tried to use '%s' as testhull but not in available_hulls." % TESTDESIGN_PREFERRED_HULL)
            warn("Please update ShipDesignAI.py according to the new content.")
        testdesign_names = [get_shipdesign(design_id).name for design_id in empire.allShipDesigns
                            if get_shipdesign(design_id).name.startswith(TESTDESIGN_NAME_BASE)]
        testdesign_names_hull = [name for name in testdesign_names if name.startswith(TESTDESIGN_NAME_HULL)]
        testdesign_names_part = [name for name in testdesign_names if name.startswith(TESTDESIGN_NAME_PART)]
        available_slot_types = {slottype for slotlist in [get_hulltype(hull).slots for hull in available_hulls]
                                for slottype in slotlist}
        new_parts = [get_part_type(part) for part in empire.availableShipParts
                     if part not in self.strictly_worse_parts]
        pid = self.production_cost.keys()[0]  # as only location invariant parts are considered, use arbitrary planet.
        for new_part in new_parts:
            self.strictly_worse_parts[new_part.name] = []
            if new_part.partClass in WEAPONS:
                continue  # TODO:  Update cache-functionality to handle tech upgrades
            if not new_part.costTimeLocationInvariant:
                debug("new part %s not location invariant!" % new_part.name)
                continue
            for part_class in ALL_META_CLASSES:
                if new_part.partClass in part_class:
                    for old_part in [get_part_type(part) for part in self.strictly_worse_parts
                                     if part != new_part.name]:
                        if not old_part.costTimeLocationInvariant:
                            debug("old part %s not location invariant!" % old_part.name)
                            continue
                        if old_part.partClass in part_class:
                            if new_part.capacity >= old_part.capacity:
                                a = new_part
                                b = old_part
                            else:
                                a = old_part
                                b = new_part
                            if (self.production_cost[pid][a.name] <= self.production_cost[pid][b.name]
                                    and {x for x in a.mountableSlotTypes} >= {x for x in b.mountableSlotTypes}
                                    and self.production_time[pid][a.name] <= self.production_time[pid][b.name]):
                                self.strictly_worse_parts[a.name].append(b.name)
                                debug("Part %s is strictly worse than part %s" % (b.name, a.name))
                    break
        available_parts = sorted(self.strictly_worse_parts.keys(),
                                 key=lambda item: get_part_type(item).capacity, reverse=True)

        # in case of a load, we need to rebuild our Cache.
        if not self.testhulls:
            debug("Testhull cache not found. This may happen only at first turn after game start or load.")
            for hullname in available_hulls:
                des = [des for des in testdesign_names_part if des.endswith(hullname)]
                if des:
                    self.testhulls.add(hullname)
            if verbose:
                debug("Rebuilt Cache. The following hulls are used in testdesigns for parts: %s" % self.testhulls)

        # 1. Update hull test designs
        debug("Updating Testdesigns for hulls...")
        if verbose:
            debug("Available Hulls: %s" % available_hulls)
            debug("Existing Designs (prefix: %s): %s" % (
                TESTDESIGN_NAME_HULL, [x.replace(TESTDESIGN_NAME_HULL, "") for x in testdesign_names_hull]))
        for hull in [get_hulltype(hullname) for hullname in available_hulls
                     if "%s_%s" % (TESTDESIGN_NAME_HULL, hullname) not in testdesign_names_hull]:
            partlist = len(hull.slots) * [""]
            testdesign_name = "%s_%s" % (TESTDESIGN_NAME_HULL, hull.name)
            _create_ship_design(testdesign_name, hull.name, partlist,
                                description="TESTPURPOSE ONLY", verbose=verbose)

        # 2. Cache the list of buildable ship hulls for each planet
        debug("Caching buildable hulls per planet...")
        testname = "%s_%s" % (TESTDESIGN_NAME_HULL, "%s")
        for pid in inhabited_planets:
            self.hulls_for_planets[pid] = []
        for hullname in available_hulls:
            testdesign = _get_design_by_name(testname % hullname)
            if testdesign:
                for pid in inhabited_planets:
                    if _can_build(testdesign, empire_id, pid):
                        self.hulls_for_planets[pid].append(hullname)
            else:
                warn("Missing testdesign for hull %s!" % hullname)

        # 3. Update ship part test designs
        #     Because there are different slottypes, we need to find a hull that can host said slot.
        #     However, not every planet can build every hull. Thus, for each inhabited planet:
        #       I. Check which parts do not have a testdesign yet with a hull we can build on this planet
        #       II. If there are parts, find out which slots we need
        #       III. For each slot type, try to find a hull we can build on this planet
        #            and use this hull for all the parts hostable in this type.
        debug("Updating test designs for ship parts...")
        if verbose:
            debug("Available parts: %s" % available_parts)
            debug("Existing Designs (prefix: %s): %s" % (
                TESTDESIGN_NAME_PART, [x.replace(TESTDESIGN_NAME_PART, "") for x in testdesign_names_part]))
        for pid in inhabited_planets:
            planetname = universe.getPlanet(pid).name
            local_hulls = self.hulls_for_planets[pid]
            needs_update = [get_part_type(partname) for partname in available_parts
                            if not any(["%s_%s_%s" % (TESTDESIGN_NAME_PART, partname, hullname) in testdesign_names_part
                                       for hullname in local_hulls])]
            if not needs_update:
                if verbose:
                    debug("Planet %s: Test designs are up to date" % planetname)
                continue
            if verbose:
                debug("Planet %s: The following parts appear to need a new design: %s" % (
                    planetname, [part.name for part in needs_update]))
            for slot in available_slot_types:
                testhull = next((hullname for hullname in local_hulls if slot in get_hulltype(hullname).slots), None)
                if testhull is None:
                    if verbose:
                        debug("Failure: Could not find a hull with slots of type '%s' for this planet" % slot.name)
                    continue
                else:
                    if verbose:
                        debug("Using hull %s for slots of type '%s'" % (testhull, slot.name))
                    self.testhulls.add(testhull)
                slotlist = [s for s in get_hulltype(testhull).slots]
                slot_index = slotlist.index(slot)
                num_slots = len(slotlist)
                for part in [part for part in needs_update if slot in part.mountableSlotTypes]:
                    partlist = num_slots * [""]
                    partlist[slot_index] = part.name
                    testdesign_name = "%s_%s_%s" % (TESTDESIGN_NAME_PART, part.name, testhull)
                    res = _create_ship_design(testdesign_name, testhull, partlist,
                                              description="TESTPURPOSE ONLY", verbose=verbose)
                    if res:
                        testdesign_names_part.append(testdesign_name)
                    else:
                        continue
                    needs_update.remove(part)  # We only need one design per part, not for every possible slot

        #  later on in the code, we will have to check multiple times if the test hulls are in
        #  the list of buildable hulls for the planet. As the ordering is preserved, move the
        #  testhulls to the front of the availableHull list to save some time in the checks.
        for i, s in enumerate(self.testhulls):
            try:
                idx = available_hulls.index(s)
                if i != idx:
                    available_hulls[i], available_hulls[idx] = available_hulls[idx], available_hulls[i]
            except ValueError:
                error("hull in testhull cache not in available_hulls even though it is supposed to be a proper subset.",
                      exc_info=True)

        # 4. Cache the list of buildable ship parts for each planet
        debug("Caching buildable ship parts per planet...")
        for pid in inhabited_planets:
            local_testhulls = [hull for hull in self.testhulls
                               if hull in self.hulls_for_planets[pid]]
            this_planet = universe.getPlanet(pid)
            if verbose:
                debug("Testhulls for %s are %s" % (this_planet, local_testhulls))
            self.parts_for_planets[pid] = {}
            local_ignore = set()
            local_cache = self.parts_for_planets[pid]
            for slot in available_slot_types:
                local_cache[slot] = []
            for partname in available_parts:
                if partname in local_ignore:
                    continue
                part_slottypes = get_part_type(partname).mountableSlotTypes
                ship_design = None
                for hullname in local_testhulls:
                    if not any((slot in get_hulltype(hullname).slots for slot in part_slottypes)):
                        continue
                    ship_design = _get_design_by_name("%s_%s_%s" % (TESTDESIGN_NAME_PART, partname, hullname))
                    if ship_design:
                        if _can_build(ship_design, empire_id, pid):
                            for slot in part_slottypes:
                                local_cache.setdefault(slot, []).append(partname)
                                local_ignore.update(self.strictly_worse_parts[partname])
                        break
                if verbose and not ship_design:
                    planetname = universe.getPlanet(pid).name
                    debug("Failure: Couldn't find a testdesign for part %s on planet %s." % (partname, planetname))
            # make sure we do not edit the list later on this turn => tuple: immutable
            # This also allows to shallowcopy the cache.
            for slot in local_cache:
                local_cache[slot] = tuple(local_cache[slot])

            if verbose:
                debug("Parts for Planet: %s: " % universe.getPlanet(pid).name, self.parts_for_planets[pid])


Cache = ShipDesignCache()


class AdditionalSpecifications(object):
    """This class is a container for all kind of additional information
    and requirements we may want to use when assessing ship designs.

    methods for external use:
    convert_to_tuple(): Builds and returns a tuple of the class attributes
    update_enemy(enemy): updates enemy stats
    """

    def __init__(self):
        # TODO: Extend this framework according to needs of future implementations
        self.minimum_fuel = 0
        self.minimum_speed = 0
        self.orbital = False
        self.minimum_structure = 1
        self.minimum_fighter_launch_rate = 0
        self.enemy_shields = 1  # to avoid spamming flak cannons
        self.max_enemy_weapon_strength = 0
        self.avg_enemy_weapon_strength = 0
        self.expected_turns_till_fight = 2
        current_turn = fo.currentTurn()
        if current_turn < 80:
            self.enemy_mine_dmg = 0  # TODO: Implement the detection of actual enemy mine damage
        elif current_turn < 150:
            self.enemy_mine_dmg = 2
        elif current_turn < 230:
            self.enemy_mine_dmg = 6
        else:
            self.enemy_mine_dmg = 14
        self.enemy = None
        self.update_enemy(get_aistate().get_standard_enemy())

    def update_enemy(self, enemy):
        """Read out the enemies stats and save them.

        :param enemy:
        :type enemy: CombatRatingsAI.ShipCombatStats
        """
        self.enemy = enemy
        enemy_attack_stats, enemy_structure, self.enemy_shields = enemy.get_basic_stats()
        self.enemy_shields += 1  # add bias against weak weapons to account to allow weapons to stay longer relevant.
        if enemy_attack_stats:
            self.max_enemy_weapon_strength = max(enemy_attack_stats.keys())
            n = 0
            d = 0
            for dmg, count in enemy_attack_stats.iteritems():
                d += dmg*count
                n += count
            self.avg_enemy_weapon_strength = d/n

    def convert_to_tuple(self):
        """Create a tuple of this class' attributes (e.g. to use as key in dict).

        :returns: tuple (minFuel,minSpeed,enemyDmg,enemyShield,enemyMineDmg)
        """
        return ("minFuel: %s" % self.minimum_fuel, "minSpeed: %s" % self.minimum_speed,
                "enemyDmg: %s" % self.max_enemy_weapon_strength, "enemyShields: %s" % self.enemy_shields,
                "enemyMineDmg: %s" % self.enemy_mine_dmg)


class DesignStats(object):

    def __init__(self):
        self.attacks = {}  # {damage: shots_per_round}
        self.reset()  # this call initiates remaining instance variables!

    # noinspection PyAttributeOutsideInit
    def reset(self):
        self.attacks.clear()
        self.structure = 0
        self.shields = 0
        self.fuel = 0
        self.speed = 0
        self.stealth = 0
        self.detection = 0
        self.troops = 0
        self.colonisation = -1  # -1 since 0 indicates an outpost (capacity = 0)
        self.fuel_per_turn = 0
        self.organic_growth = 0
        self.maximum_organic_growth = 0
        self.repair_per_turn = 0
        self.asteroid_stealth = 0
        self.solar_stealth = 0
        self.fighter_capacity = 0
        self.fighter_launch_rate = 0
        self.fighter_damage = 0

    def convert_to_combat_stats(self):
        """Return a tuple as expected by CombatRatingsAI"""
        return (self.attacks, self.structure, self.shields,
                self.fighter_capacity, self.fighter_launch_rate, self.fighter_damage)


class ShipDesigner(object):
    """This class and its subclasses implement the building of a ship design and its rating.
     Specialised Designs with their own rating system or optimizing algorithms should inherit from this class.

    Member functions intended for external use:
    optimize_design(): Returns the estimated optimum design according to the rating function
    evaluate(): Returns a rating for the design as of the current state
    update_hull(hullname): sets the hull used in the design
    update_parts(partname_list): sets the parts used in the design
    update_species(species): sets the piloting species
    update_stats(): calculates the stats of the design based on hull+parts+species
    add_design(): Adds the shipdesign in the C++ part of the game

    Functions which are to be overridden in inherited classes:
    _rating_function()
    _class_specific_filter()
    _starting_guess()
    _calc_rating_for_name()

    For improved performance, maybe override _filling_algorithm() with a more specialised algorithm as well.
    """
    basename = "Default - Do not build"     # base design name
    description = "Base class Ship type"    # design description
    useful_part_classes = ALL_META_CLASSES  # only these parts are considered in the design process

    filter_useful_parts = True              # removes any part not belonging to self.useful_part_classes
    filter_inefficient_parts = False        # removes cost-inefficient parts (less capacity and less capacity/cost)

    consider_fleet_count = True             # defines if we consider fleet upkeep cost

    NAMETABLE = "AI_SHIPDESIGN_NAME_INVALID"
    NAME_THRESHOLDS = []                    # list of rating thresholds to choose a different name
    design_name_dict = {}                   # {min_rating: basename}: based on rating, the highest unlocked name is used
    running_index = {}                      # {basename: int}: a running index per design name

    def __init__(self):
        """Make sure to call this constructor in each subclass."""
        self.species = None         # name of the piloting species (string)
        self.hull = None            # hull object (not hullname!)
        self.partnames = []         # list of partnames (string)
        self.parts = []             # list of actual part objects
        self.design_stats = DesignStats()
        self.production_cost = 9999
        self.production_time = 1
        self.pid = INVALID_ID               # planetID for checks on production cost if not LocationInvariant.
        self.additional_specifications = AdditionalSpecifications()
        self.design_name_dict = {k: v for k, v in zip(self.NAME_THRESHOLDS,
                                                      UserString(self.NAMETABLE, self.basename).splitlines())}

    def evaluate(self):
        """ Return a rating for the design.

        First, check if the minimum requirements are met. If so, return the _rating_function().
        Otherwise, return a large negative number scaling with distance to the requirements.

        :returns: float - rating of the current part/hull combo
        """
        self.update_stats()

        # If we do not meet the requirements, we want to return a negative rating.
        # However, we also need to make sure, that the closer we are to requirements,
        # the better our rating is so the optimizing heuristic finds "the right way".
        rating = MISSING_REQUIREMENT_MULTIPLIER * sum([
            max(0., self._minimum_fuel() - self.design_stats.fuel),
            max(0, self._minimum_speed() - self.design_stats.speed),
            max(0, self._minimum_structure() - (self.design_stats.structure + self._expected_organic_growth())),
            max(0, self._minimum_fighter_launch_rate() - self.design_stats.fighter_launch_rate)
        ])
        if self._orbital() and self.design_stats.speed > 0:
            rating = min(-100, rating - 99999)
        return rating if rating < 0 else self._rating_function()

    def _minimum_fuel(self):
        return self.additional_specifications.minimum_fuel

    def _minimum_speed(self):
        return self.additional_specifications.minimum_speed

    def _orbital(self):
        return self.additional_specifications.orbital

    def _minimum_structure(self):
        return self.additional_specifications.minimum_structure

    def _minimum_fighter_launch_rate(self):
        return self.additional_specifications.minimum_fighter_launch_rate

    def _rating_function(self):
        """Rate the design according to current hull/part combo.

        :returns: float - rating
        """
        error("WARNING: Rating function not overloaded for class %s!" % self.__class__.__name__)
        raise NotImplementedError

    def _set_stats_to_default(self):
        """Set stats to default.

        Call this if design is invalid to avoid miscalculation of ratings."""
        self.design_stats.reset()
        self.production_cost = 9999
        self.production_time = 1

    def update_hull(self, hullname):
        """Set hull of the design.

        :param hullname:
        :type hullname: str
        """
        self.hull = fo.getHullType(hullname)

    def update_parts(self, partname_list):
        """Set both partnames and parts attributes.

        :param partname_list: contains partnames as strings
        :type partname_list: list"""
        self.partnames = partname_list
        self.parts = [get_part_type(part) for part in partname_list if part]

    def update_species(self, species):
        """Set the piloting species.

        :param species:
        :type species: str
        """
        self.species = species

    def update_stats(self, ignore_species=False):
        """Calculate and update all stats of the design.

        Default stats if no hull in design.

        :param ignore_species: toggles whether species piloting grades are considered in the stats.
        :type ignore_species: bool
        """
        self._set_stats_to_default()

        if not self.hull:
            warn("Tried to update stats of design without hull. Reset values to default.")
            return

        local_cost_cache = Cache.production_cost[self.pid]
        local_time_cache = Cache.production_time[self.pid]

        # read out hull stats
        self.design_stats.structure = self.hull.structure
        self.design_stats.fuel = self.hull.fuel
        self.design_stats.speed = self.hull.speed
        self.design_stats.stealth = self.hull.stealth
        self.production_cost = local_cost_cache.get(self.hull.name, self.hull.productionCost(fo.empireID(), self.pid))
        self.production_time = local_time_cache.get(self.hull.name, self.hull.productionTime(fo.empireID(), self.pid))

        # read out part stats
        shield_counter = cloak_counter = detection_counter = colonization_counter = engine_counter = 0  # to deal with Non-stacking parts
        hangar_part_names = set()
        bay_parts = list()
        for part in self.parts:
            self.production_cost += local_cost_cache.get(part.name, part.productionCost(fo.empireID(), self.pid))
            self.production_time = max(self.production_time,
                                       local_time_cache.get(part.name, part.productionTime(fo.empireID(), self.pid)))
            partclass = part.partClass
            capacity = part.capacity if partclass not in WEAPONS else self._calculate_weapon_strength(part)
            if partclass in FUEL:
                self.design_stats.fuel += capacity
            elif partclass in ENGINES:
                engine_counter += 1
                if engine_counter == 1:
                    self.design_stats.speed += capacity
                else:
                    self.design_stats.speed = 0
            elif partclass in COLONISATION:
                colonization_counter += 1
                if colonization_counter == 1:
                    self.design_stats.colonisation = capacity
                else:
                    self.design_stats.colonisation = -1
            elif partclass in DETECTION:
                detection_counter += 1
                if detection_counter == 1:
                    self.design_stats.detection += capacity
                else:
                    self.design_stats.detection = 0
            elif partclass in ARMOUR:
                self.design_stats.structure += capacity
            elif partclass in WEAPONS:
                shots = self._calculate_weapon_shots(part)
                self.design_stats.attacks[capacity] = self.design_stats.attacks.get(capacity, 0) + shots
            elif partclass in SHIELDS:
                shield_counter += 1
                if shield_counter == 1:
                    self.design_stats.shields = capacity
                else:
                    self.design_stats.shields = 0
            elif partclass in TROOPS:
                self.design_stats.troops += capacity
            elif partclass in STEALTH:
                cloak_counter += 1
                if cloak_counter == 1:
                    self.design_stats.stealth += capacity
                else:
                    self.design_stats.stealth = 0
            elif partclass in FIGHTER_BAY:
                bay_parts.append(part)
            elif partclass in FIGHTER_HANGAR:
                hangar_part_names.add(part.name)
                if len(hangar_part_names) > 1:
                    # enforce only one hangar part per design
                    self.design_stats.fighter_capacity = 0
                    self.design_stats.fighter_damage = 0
                    self.design_stats.fighter_launch_rate = 0
                else:
                    self.design_stats.fighter_capacity += self._calculate_hangar_capacity(part)
                    self.design_stats.fighter_damage = self._calculate_hangar_damage(part)

        if len(bay_parts) > 0:
            hangar_part_name = None
            for hangar_part_name in hangar_part_names:
                break
            self.design_stats.fighter_launch_rate = self._calculate_fighter_launch_rate(bay_parts, hangar_part_name)

        self._apply_hardcoded_effects()

        if self.species and not ignore_species:
            shields_grade = CombatRatingsAI.get_species_shield_grade(self.species)
            self.design_stats.shields = CombatRatingsAI.weight_shields(self.design_stats.shields, shields_grade)
            if self.design_stats.troops:
                troops_grade = CombatRatingsAI.get_species_troops_grade(self.species)
                self.design_stats.troops = CombatRatingsAI.weight_attack_troops(self.design_stats.troops, troops_grade)

    def _apply_hardcoded_effects(self):
        """Update stats that can not be read out by the AI yet, i.e. applied by effects.

        This function should contain *all* hardcoded effects for hulls/parts to be considered by the AI
        to make sure we can easily adjust the values for future balance changes or after implementing a
        method to read out all stats.
        """

        def parse_complex_tokens(tup):
            """Parse complex tokens which have a value dependent on another value

            Example usage:
            Repair by 10% of max structure per turn. {REPAIR_PER_TURN: (STRUCTURE, 0.1)}
            :param tup: (dependency, fraction_of_value)
            :type tup: tuple
            :return: float
            """
            dependency, value = tup
            dep_val = 0
            if dependency == AIDependencies.STRUCTURE:
                dep_val = self.design_stats.structure
            elif dependency == AIDependencies.FUEL:
                dep_val = self.design_stats.fuel
            else:
                warn("Can't parse dependent token:" + str(tup))
            return dep_val * value

        def parse_tokens(tokendict, is_hull=False):
            """Adjust design stats according to the token dict key-value pairs.

            :param tokendict: tokens and values
            :type tokendict: dict
            :param is_hull: tokendict is related to hull
            :type is_hull: bool
            """
            for token, value in tokendict.items():
                if isinstance(value, tuple) and token is not AIDependencies.ORGANIC_GROWTH:
                    value = parse_complex_tokens(value)
                if token == AIDependencies.REPAIR_PER_TURN:
                    self.design_stats.repair_per_turn += min(value, self.design_stats.structure)
                elif token == AIDependencies.FUEL_PER_TURN:
                    self.design_stats.fuel_per_turn = max(self.design_stats.fuel_per_turn + value, self.design_stats.fuel)
                elif token == AIDependencies.STEALTH_MODIFIER:
                    if not (AIDependencies.NO_EFFECT_WITH_CLOAKS in tokendict.get(AIDependencies.STACKING_RULES, [])
                            and self._partclass_in_design(STEALTH)):
                        self.design_stats.stealth += value
                # STACKING_RULES configures NO_EFFECT_WITH_CLOAKS -> ignore
                elif token == AIDependencies.STACKING_RULES:
                    continue
                elif token == AIDependencies.ASTEROID_STEALTH:
                    self.design_stats.asteroid_stealth += value
                elif token == AIDependencies.SHIELDS:
                    self.design_stats.shields += value
                elif token == AIDependencies.DETECTION:
                    self.design_stats.detection += value
                elif token == AIDependencies.ORGANIC_GROWTH:
                    self.design_stats.organic_growth += value[0]
                    self.design_stats.maximum_organic_growth += value[1]
                elif token == AIDependencies.SOLAR_STEALTH:
                    self.design_stats.solar_stealth = max(self.design_stats.solar_stealth, value)
                elif token == AIDependencies.SPEED:
                    self.design_stats.speed += value
                elif token == AIDependencies.FUEL:
                    self.design_stats.fuel += value
                elif token == AIDependencies.STRUCTURE:
                    self.design_stats.structure += value
                else:
                    warn("Failed to parse token: %s" % token)
            # if the hull has no special detection specified, then it has base detection.
            if is_hull and AIDependencies.DETECTION not in tokendict:
                self.design_stats.detection += AIDependencies.BASE_DETECTION

        # TODO establish framework for conditional effects and get rid of this
        if self.hull.name == "SH_SPATIAL_FLUX":
            self.design_stats.stealth += 10
            relevant_stealth_techs = [
                "SPY_STEALTH_PART_1", "SPY_STEALTH_PART_2",
                "SPY_STEALTH_PART_3", "SPY_STEALTH_4",
            ]
            completed_techs = [tech for tech in relevant_stealth_techs
                               if tech_is_complete(tech)]
            self.design_stats.stealth += 10 * len(completed_techs)

        if self.hull.name not in AIDependencies.HULL_EFFECTS:
            self.design_stats.detection += AIDependencies.BASE_DETECTION
        else:
            parse_tokens(AIDependencies.HULL_EFFECTS[self.hull.name], is_hull=True)

        for partname in set(self.partnames):
            if partname in AIDependencies.PART_EFFECTS:
                parse_tokens(AIDependencies.PART_EFFECTS[partname])

        for tech in AIDependencies.TECH_EFFECTS:
            if tech_is_complete(tech):
                parse_tokens(AIDependencies.TECH_EFFECTS[tech])

    def add_design(self, verbose=True):
        """Add a real (i.e. gameobject) ship design of the current configuration.

        :param verbose: toggles detailed debugging output
        :type verbose: bool
        """
        # First build a name. We want to have a safe way to reference the design
        # And to find out whether it is a duplicate of an existing one.
        # Therefore, we build a reference name using the hullname and all parts.
        # The real name that is shown in the game AI differs from that one. Current
        # implementation is a simple running index that gets counted up in addition
        # to a base name. The real name is mapped using a dictionary.
        # For now, abbreviating the Empire name to uppercase first and last initials

        design_name = self._build_design_name()
        reference_name = _build_reference_name(self.hull.name, self.partnames)  # "Hull-Part1-Part2-Part3-Part4"

        if reference_name in Cache.map_reference_design_name:
            if verbose:
                debug("Design with reference name %s is cached: %s" % (
                    reference_name, Cache.map_reference_design_name[reference_name]))
            try:
                return _get_design_by_name(Cache.map_reference_design_name[reference_name]).id
            except AttributeError:
                cached_name = Cache.map_reference_design_name[reference_name]
                error("%s maps to %s in Cache.map_reference_design_name."
                      " But the design seems not to exist..." % (reference_name, cached_name), exc_info=True)
                return None

        if verbose:
            debug("Trying to add Design... %s" % design_name)
        res = _create_ship_design(design_name, self.hull.name, self.partnames,
                                  description=self.description, verbose=verbose)
        if not res:
            return None
        new_design = _get_design_by_name(design_name)
        if new_design:
            Cache.map_reference_design_name[reference_name] = design_name
            return new_design.id
        else:
            warn("Tried to get just created design %s but got None" % design_name)
            return None

    def _class_specific_filter(self, partname_dict):
        """Add additional filtering to _filter_parts().

        To be implemented in subclasses.
        """
        pass

    def optimize_design(self, additional_parts=(), additional_hulls=(),
                        loc=None, verbose=False, consider_fleet_count=True):
        """Try to find the optimimum designs for the shipclass for each planet and add it as gameobject.

        Only designs with a positive rating (i.e. matching the minimum requirements) will be returned.

        :return: list of (rating, planet_id, design_id, cost, design_stats) tuples, i.e. best available design for each planet
        :param loc: int or list of ints (optional) - planet ids where the designs are to be built. Default: All planets.
        :param verbose: Toggles detailed logging for debugging.
        :type verbose: bool
        :param additional_hulls: additional unavailable hulls to consider in the design process
        :type additional_hulls: list
        :param additional_parts: additional unavailable parts to consider in the design process
        :type additional_parts: list
        :param consider_fleet_count: Toggles whether fleet upkeep should be reflected in the rating.
        :type consider_fleet_count: bool
        """
        if loc is None:
            planets = state.get_inhabited_planets()
        elif isinstance(loc, int):
            planets = [loc]
        elif isinstance(loc, list):
            planets = loc
        else:
            error("Invalid loc parameter for optimize_design(). Expected int or list but got %s" % loc)
            return []

        self.consider_fleet_count = consider_fleet_count

        Cache.update_cost_cache(partnames=additional_parts, hullnames=additional_hulls)

        additional_part_dict = {}
        for partname in additional_parts:
            for slot in get_part_type(partname).mountableSlotTypes:
                additional_part_dict.setdefault(slot, []).append(partname)

        # TODO: Rework caching to only cache raw stats of designs, then evaluate them
        design_cache_class = Cache.best_designs.setdefault(self.__class__.__name__, {})
        design_cache_fleet_upkeep = design_cache_class.setdefault(WITH_UPKEEP if consider_fleet_count else WITHOUT_UPKEEP, {})
        req_tuple = self.additional_specifications.convert_to_tuple()
        design_cache_reqs = design_cache_fleet_upkeep.setdefault(req_tuple, {})
        universe = fo.getUniverse()
        best_design_list = []

        if verbose:
            debug("Trying to find optimum designs for shiptype class %s" % self.__class__.__name__)
        for pid in planets:
            planet = universe.getPlanet(pid)
            self.pid = pid
            self.update_species(planet.speciesName)

            relevant_techs = []
            if WEAPONS & self.useful_part_classes:
                relevant_techs = [tech for tech in AIDependencies.WEAPON_UPGRADE_TECHS if tech_is_complete(tech)]
            relevant_techs += [tech for tech in AIDependencies.TECH_EFFECTS if tech_is_complete(tech)]
            relevant_techs = tuple(relevant_techs)
            design_cache_tech = design_cache_reqs.setdefault(relevant_techs, {})

            # The piloting species is only important if its modifiers are of any use to the design
            # Therefore, consider only those treats that are actually useful. Note that the
            # canColonize trait is covered by the parts we can build, so no need to consider it here.
            # The same is true for the canProduceShips trait which simply means no hull can be built.
            relevant_grades = []
            if WEAPONS & self.useful_part_classes:
                weapons_grade = CombatRatingsAI.get_pilot_weapons_grade(self.species)
                relevant_grades.append("WEAPON: %s" % weapons_grade)
            if SHIELDS & self.useful_part_classes:
                shields_grade = CombatRatingsAI.get_species_shield_grade(self.species)
                relevant_grades.append("SHIELDS: %s" % shields_grade)
            if TROOPS & self.useful_part_classes:
                troops_grade = CombatRatingsAI.get_species_troops_grade(self.species)
                relevant_grades.append("TROOPS: %s" % troops_grade)
            species_tuple = tuple(relevant_grades)
            design_cache_species = design_cache_tech.setdefault(species_tuple, {})

            available_hulls = list(Cache.hulls_for_planets[pid]) + list(additional_hulls)
            if verbose:
                debug("Evaluating planet %s" % planet.name)
                debug("Species: %s" % planet.speciesName)
                debug("Available Ship Hulls: %s" % available_hulls)
            available_parts = copy.copy(Cache.parts_for_planets[pid])  # this is a dict! {slottype:(partnames)}
            available_slots = set(available_parts.keys()) | set(additional_part_dict.keys())
            for slot in available_slots:
                available_parts[slot] = list(available_parts.get(slot, [])) + additional_part_dict.get(slot, [])

            self._filter_parts(available_parts, verbose=verbose)
            all_parts = []
            for partlist in available_parts.values():
                all_parts += partlist
            design_cache_parts = design_cache_species.setdefault(frozenset(all_parts), {})
            best_rating_for_planet = 0
            best_hull = None
            best_parts = None
            for hullname in available_hulls:
                if hullname in design_cache_parts:
                    cache = design_cache_parts[hullname]
                    best_hull_rating = cache[0]
                    current_parts = cache[1]
                    if verbose:
                        debug("Best rating for hull %s: %f (read from Cache) %s" % (
                            hullname, best_hull_rating, current_parts))
                else:
                    self.update_hull(hullname)
                    best_hull_rating, current_parts = self._filling_algorithm(available_parts)
                    design_cache_parts.update({hullname: (best_hull_rating, current_parts)})
                    if verbose:
                        debug("Best rating for hull %s: %f %s" % (hullname, best_hull_rating, current_parts))
                if best_hull_rating > best_rating_for_planet:
                    best_rating_for_planet = best_hull_rating
                    best_hull = hullname
                    best_parts = current_parts
            if verbose:
                debug("Best overall rating for this planet: %f (%s with %s)" % (
                    best_rating_for_planet, best_hull, best_parts))
            if best_hull:
                self.update_hull(best_hull)
                self.update_parts(best_parts)
                design_id = self.add_design(verbose=verbose)
                if verbose:
                    debug("For best design got got design id %s" % design_id)
                if design_id is not None:
                    best_design_list.append((best_rating_for_planet, pid, design_id,
                                             self.production_cost, copy.deepcopy(self.design_stats)))
                else:
                    error("The best design for %s on planet %d could not be added."
                          % (self.__class__.__name__, pid))
            elif verbose:
                debug("Could not find a suitable design of type %s for planet %s." % (self.__class__.__name__, planet))
        sorted_design_list = sorted(best_design_list, key=lambda x: x[0], reverse=True)
        return sorted_design_list

    def _filter_parts(self, partname_dict, verbose=False):
        """Filter the partname_dict.

        This function filters a list of parts according to the following criteria:

            1) filter out parts not in self.useful_part_classes
            2) filter_inefficient_parts (optional): filters out parts that are weaker and have a worse effect/cost ratio
            3) ship class specific filter as defined in _class_specific_filter

        Each filter can be turned on/off by setting the correspondig class attribute to true/false.
        WARNING: The dict passed as parameter is modified inside this function and entries are removed!

        :param partname_dict: keys: slottype, values: list of partnames. MODIFIED INSIDE THIS FUNCTION!
        :param verbose: toggles verbose logging
        :type verbose: bool
        """
        empire_id = fo.empireID()
        if verbose:
            debug("Available parts:")
            for x in partname_dict:
                debug("  %s: %s" % (x, partname_dict[x]))

        part_dict = {slottype: zip(partname_dict[slottype], map(get_part_type, partname_dict[slottype]))
                     for slottype in partname_dict}  # {slottype: [(partname, parttype_object)]}

        for slottype in part_dict:
            part_dict[slottype] = [tup for tup in part_dict[slottype] if tup[1].partClass in self.useful_part_classes]

        if self.filter_inefficient_parts:
            local_cost_cache = Cache.production_cost[self.pid]
            # TODO: Check for redundance of weapons with new tech upgrade system
            # TODO: Check for redundance of hangars
            # TODO Remember to use secondaryStat as well for weapons/hangars
            check_for_redundance = (ARMOUR | ENGINES | FUEL | SHIELDS
                                    | STEALTH | DETECTION | TROOPS | FIGHTER_BAY) & self.useful_part_classes
            for slottype in part_dict:
                partclass_dict = defaultdict(list)
                for tup in part_dict[slottype]:
                    partclass = tup[1].partClass
                    if partclass in check_for_redundance:
                        partclass_dict[partclass].append(tup[1])
                for shipPartsPerClass in partclass_dict.itervalues():
                    for a in shipPartsPerClass:
                        if a.capacity == 0:  # TODO: Modify this if effects of items get hardcoded
                            part_dict[slottype].remove((a.name, a))
                            if verbose:
                                debug("removing %s because capacity is zero." % a.name)
                            continue
                        if len(shipPartsPerClass) == 1:
                            break
                        # cost_a = local_cost_cache[a.name]
                        cost_a = local_cost_cache.get(a.name, a.productionCost(empire_id, self.pid))
                        for b in shipPartsPerClass:
                            cost_b = local_cost_cache.get(b.name, b.productionCost(empire_id, self.pid))
                            if (b is not a
                                    and (b.capacity/cost_b - a.capacity/cost_a) > -1e-6
                                    and b.capacity >= a.capacity):
                                if verbose:
                                    debug("removing %s because %s is better." % (a.name, b.name))
                                part_dict[slottype].remove((a.name, a))
                                break
        for slottype in part_dict:
            partname_dict[slottype] = [tup[0] for tup in part_dict[slottype]]
        self._class_specific_filter(partname_dict)
        if verbose:
            debug("Available parts after filtering:")
            for x in partname_dict:
                debug("  %s: %s" % (x, partname_dict[x]))

    def _starting_guess(self, available_parts, num_slots):
        """Return an initial guess for the filling of the slots.

        The closer the guess to the final result, the less time the optimizing algorithm takes to finish.
        In order to improve performance it thus makes sense to state a very informed guess so only a few
        parts if any have to be changed.

        If not overloaded in the subclasses, the initial guess is an empty design.

        :param available_parts: name of the available parts including "" for empty slot (last position of the list)
        :param num_slots: number of slots to fill
        :return: list of int: the number of parts used in the design (indexed in order as in available_parts)
        """
        return len(available_parts)*[0] + [num_slots]  # corresponds to an entirely empty design

    def _combinatorial_filling(self, available_parts):
        """Fill the design using a combinatorial approach.

        This generic filling algorithm considers the problem of filling the slots as combinatorial problem.
        We are interested in the best combination of parts without considering order.
        In general, this yields (n+k-1) choose k possibilities in total per slottype where
        n is the number of parts and k is the number of slots.
        For s different slottypes, we thus end up with Product((n_i+k_i-1) choose k_i, i=1..s)
        combinations to test for. As this is still quite too much for brute force, we use the following heuristic:

        find some _starting_guess() for the filling
        until no more improvement:
            for each slottype in the hull:
                until no more improvement:
                    swap single parts if this increases the rating

        So basically optimize the filling for each slottype sequentially. If one slottype was improved, we need to
        check the already optimized slottypes again as the new parts could have affected the value of some parts.
        The same logic holds true for the parts: If we tried to exchange two parts earlier which did not improve
        the rating, we can't be sure that still isn't the case if we swapped some other parts. For example, consider
        a military ship without weapons: The rating will always be zero. So swapping a bad armour part for a good one
        will not yield an improvement. Once we add a single weapon however, the rating will be increased by exchanging
        the armour parts.

        This heuristic will always find a local maximum. For simple enough (convex) rating functions this is also
        the global maximum. More intrigued functions might require a different approach, however. Another problem might
        occur if we have a non-stacking part available for both the external and internal slot. We will never exchange
        these parts in this algorithm so if future oontent has this situation, we need to either specify a very distinct
        _starting_guess() or change the algorithm.

        :param available_parts: dict, indexed by slottype, containing a list of partnames for the slot
        :return: best rating, corresponding list of partnames
        """
        number_of_slots_by_slottype = Counter()
        for slot in self.hull.slots:
            number_of_slots_by_slottype[slot] += 1
        parts = {}                # indexed by slottype, contains list of partnames (list of strings)
        total_filling = {}        # indexed by slottype, contains the number of parts (ordered as in parts)
        for slot in number_of_slots_by_slottype:
            parts[slot] = available_parts.get(slot, []) + [""]
            total_filling[slot] = self._starting_guess(available_parts.get(slot, []), number_of_slots_by_slottype[slot])
        last_changed_slottype = None
        exit_outer_loop = False
        while True:
            # Try to optimize each slottype iteratively until no more improvement.
            if exit_outer_loop or not number_of_slots_by_slottype:
                break
            for slot in number_of_slots_by_slottype:
                if last_changed_slottype is None:  # first run of the loop
                    last_changed_slottype = slot
                elif slot == last_changed_slottype:
                    exit_outer_loop = True
                    break
                current_filling = total_filling[slot]
                num_parts = len(current_filling)
                range_parts = range(num_parts)
                current_parts = []
                other_parts = []
                for s in number_of_slots_by_slottype:
                    if s is slot:
                        for j in range_parts:
                            current_parts += current_filling[j] * [parts[s][j]]
                    else:
                        for j in xrange(len(total_filling[s])):
                            other_parts += total_filling[s][j] * [parts[s][j]]
                self.update_parts(other_parts + current_parts)
                current_rating = self.evaluate()
                last_changed = None
                exit_loop = False
                while not exit_loop:
                    # Try to increase the count of one part and decreasing the other parts
                    # as long as this yields a better rating. Repeat until no more improvement.
                    for i in range_parts:
                        if i == last_changed:
                            exit_loop = True
                            break
                        elif last_changed is None:  # first run of the loop
                            last_changed = i
                        for j in range_parts:
                            if j == i:
                                continue
                            while current_filling[j] > 0:
                                current_parts[current_parts.index(parts[slot][j])] = parts[slot][i]  # exchange parts
                                self.update_parts(other_parts + current_parts)
                                new_rating = self.evaluate()
                                if new_rating > current_rating:  # keep the new config as it is better.
                                    current_filling[j] -= 1
                                    current_filling[i] += 1
                                    current_rating = new_rating
                                    last_changed = i
                                    last_changed_slottype = slot
                                else:  # undo the change as the rating is worse, try next part.
                                    current_parts[current_parts.index(parts[slot][i])] = parts[slot][j]
                                    break
        # rebuild the partlist in the order of the slots of the hull
        partlist = []
        slot_filling = {}
        for slot in number_of_slots_by_slottype:
            slot_filling[slot] = []
            for j in xrange(len(total_filling[slot])):
                slot_filling[slot] += total_filling[slot][j] * [parts[slot][j]]
        for slot in self.hull.slots:
            partlist.append(slot_filling[slot].pop())
        self.update_parts(partlist)
        rating = self.evaluate()
        return rating, partlist

    def _filling_algorithm(self, available_parts, verbose=False):
        """Fill the slots of the design using some optimizing algorithm.

        Default algorithm is _combinatorial_filling().

        :param available_parts: dict, indexed by slottype, containing a list of partnames for the slot
        """
        rating, parts = self._combinatorial_filling(available_parts)
        return rating, parts

    def _total_dmg_vs_shields(self):
        """Sum up and return the damage of weapon parts vs a shielded enemy as defined in additional_specifications.

        :return: summed up damage vs shielded enemy
        """
        total_dmg = 0
        for dmg, count in self.design_stats.attacks.items():
            total_dmg += max(0, dmg - self.additional_specifications.enemy_shields) * count
        return total_dmg

    def _total_dmg(self):
        """Sum up and return the damage of all weapon parts.

        :return: Total damage of the design (against no shields)
        """
        total_dmg = 0
        for dmg, count in self.design_stats.attacks.items():
            total_dmg += dmg * count
        return total_dmg

    def _build_design_name(self):
        """Build the ingame design name.

        The design name is based on empire name, shipclass and, if a design_name_dict is implemented for this class,
        on the strength of the design.
        :return: string
        """
        name_template = "%s %s Mk. %d"  # e.g. "EmpireAbbreviation Warship Mk. 1"
        empire_name = fo.getEmpire().name.upper()
        empire_initials = empire_name[:1] + empire_name[-1:]
        rating = self._calc_rating_for_name()
        basename = next((name for (maxRating, name) in sorted(self.design_name_dict.items(), reverse=True)
                        if rating >= maxRating), self.__class__.basename)

        def design_name():
            """return the design name based on the name_template"""
            return name_template % (empire_initials, basename, self.running_index[basename])
        self.__class__.running_index.setdefault(basename, 1)
        while _get_design_by_name(design_name(), looking_for_new_design=True):
            self.__class__.running_index[basename] += 1
        return design_name()

    def _calc_rating_for_name(self):
        """Return a rough rating for the design independent of special requirements and species.

         The design name should not depend on the strength of the enemy or upon some additional requests we have
         for the design but only compare the basic functionality. If not overloaded in the subclass, this function
         returns the structure of the design.

         :return: float - a rough approximation of the strength of this design
         """
        self.update_stats(ignore_species=True)
        return self.design_stats.structure

    def _adjusted_production_cost(self):
        """Return a production cost adjusted by the number of ships we have.

        If self.consider_fleet_count is False, then return the base cost without fleet upkeep modifier.

        Building one warship of rating 2R and cost 2C is effectively cheaper than building 2 warships of rating R and
        cost C due to fleet upkeep considerations even if they have the same raw rating/cost ratio.
        The significance of this fleet upkeep efficiency increases with the raw fleet upkeep rate, with the expected
        lifetime of the ship under consideration (i.e., with a longer expected lifespan it will affect the future
        construction of a greater number of ships), and it also increases with the number of ships of this design
        expected to be created. In the calculation below, the empire's current total shipcount is taken as a rough proxy
        for both of the duration and extent factors.
        Note: This same sort of adjustment would be valid for troop ships, could theoretically have some applicability
        to scout ships since there could conceivably be a tradeoff for more scout ships of lower rating/range, but
        would really not be applicable to colony ships since in that case there is really not a potential tradeoff
        between number of ships and capacity.

        :return: adjusted production cost
        :rtype: float
        """
        # TODO: Consider total pp production output as additional factor
        # TODO: Rethink about math and maybe work out a more accurate formula
        if self.consider_fleet_count:
            return self.production_cost**(1 / FleetUtilsAI.get_fleet_upkeep())
        else:
            return self.production_cost / FleetUtilsAI.get_fleet_upkeep()  # base cost

    def _shield_factor(self):
        """Calculate the effective factor by which structure is increased by shields.

        :rtype: float
        """
        enemy_dmg = self.additional_specifications.max_enemy_weapon_strength
        return max(enemy_dmg / max(0.01, enemy_dmg - self.design_stats.shields), 1)

    def _effective_fuel(self):
        """Return the number of turns the ship can move without refueling.

        :rtype: float
        """
        return min(self.design_stats.fuel / max(1 - self.design_stats.fuel_per_turn, 0.001), 10)

    def _expected_organic_growth(self):
        """Get expected organic growth defined by growth rate and expected numbers till fight.

        :return: Expected organic growth
        :rtype: float
        """
        return min(self.additional_specifications.expected_turns_till_fight * self.design_stats.organic_growth,
                   self.design_stats.maximum_organic_growth)

    def _remaining_growth(self):
        """Get growth potential after _expected_organic_growth() took place.

        :return: Remaining growth after _expected_organic_growth()
        :rtype: float
        """
        return self.design_stats.maximum_organic_growth - self._expected_organic_growth()

    def _effective_mine_damage(self):
        """Return enemy mine damage corrected by self-repair-rate.

        :rtype: float
        """
        return self.additional_specifications.enemy_mine_dmg - self.design_stats.repair_per_turn

    def _partclass_in_design(self, partclass):
        """Check if partclass is used in current design.

        :param partclass: One of the meta partclasses
        :type partclass: frozenset
        :rtype: bool
        """
        return any(part.partClass in partclass for part in self.parts)

    def _calculate_weapon_strength(self, weapon_part, ignore_species=False):
        # base damage
        weapon_name = weapon_part.name
        base = weapon_part.capacity
        tech_bonus = _get_tech_bonus(AIDependencies.WEAPON_UPGRADE_DICT, weapon_name)
        # species modifiers
        if not ignore_species:
            weapons_grade = CombatRatingsAI.get_pilot_weapons_grade(self.species)
            species_modifier = AIDependencies.PILOT_DAMAGE_MODIFIER_DICT.get(weapons_grade, {}).get(weapon_name, 0)
        else:
            species_modifier = 0
        return base + tech_bonus + species_modifier

    def _calculate_weapon_shots(self, weapon_part, ignore_species=False):
        weapon_name = weapon_part.name
        base = weapon_part.secondaryStat
        if not base:
            warn("Queried weapon %s for number of shots but didn't return any." % weapon_name)
            base = 1
        tech_bonus = _get_tech_bonus(AIDependencies.WEAPON_ROF_UPGRADE_DICT, weapon_name)
        # species modifier
        if not ignore_species:
            weapons_grade = CombatRatingsAI.get_pilot_weapons_grade(self.species)
            species_modifier = AIDependencies.PILOT_ROF_MODIFIER_DICT.get(weapons_grade, {}).get(weapon_name, 0)
        else:
            species_modifier = 0
        return base + species_modifier + tech_bonus

    def _calculate_fighter_launch_rate(self, bay_parts, hangar_part_name):
        launch_rate = 0
        bays_bonus = AIDependencies.HANGAR_LAUNCH_CAPACITY_MODIFIER_DICT.get(hangar_part_name, {})
        for bay_part in bay_parts:
            launch_rate += bay_part.capacity
            launch_rate += bays_bonus.get(bay_part.name, 0)
        return launch_rate

    def _calculate_hangar_damage(self, hangar_part, ignore_species=False):
        hangar_name = hangar_part.name
        base = hangar_part.secondaryStat
        tech_bonus = _get_tech_bonus(AIDependencies.FIGHTER_DAMAGE_UPGRADE_DICT, hangar_name)
        # species modifier
        if not ignore_species:
            weapons_grade = CombatRatingsAI.get_pilot_weapons_grade(self.species)
            species_modifier = AIDependencies.PILOT_FIGHTERDAMAGE_MODIFIER_DICT.get(weapons_grade,
                                                                                    {}).get(hangar_name, 0)
        else:
            species_modifier = 0
        return base + species_modifier + tech_bonus

    def _calculate_hangar_capacity(self, hangar_part, ignore_species=False):
        hangar_name = hangar_part.name
        base = hangar_part.capacity
        tech_bonus = _get_tech_bonus(AIDependencies.FIGHTER_CAPACITY_UPGRADE_DICT, hangar_name)
        # species modifier
        if not ignore_species:
            weapons_grade = CombatRatingsAI.get_pilot_weapons_grade(self.species)
            species_modifier = AIDependencies.PILOT_FIGHTER_CAPACITY_MODIFIER_DICT.get(weapons_grade,
                                                                                       {}).get(hangar_name, 0)
        else:
            species_modifier = 0
        return base + species_modifier + tech_bonus

    def _combat_rating(self):
        combat_stats = CombatRatingsAI.ShipCombatStats(stats=self.design_stats.convert_to_combat_stats())
        return combat_stats.get_rating(enemy_stats=self.additional_specifications.enemy)


class MilitaryShipDesignerBaseClass(ShipDesigner):
    basename = "Military Ship (do not build me)"
    description = "Military Ship"

    def __init__(self):
        super(MilitaryShipDesignerBaseClass, self).__init__()

    def _adjusted_production_cost(self):
        # as military ships are grouped up in fleets, their power rating scales quadratic in numbers.
        # To account for this, we need to maximize rating/cost_squared not rating/cost as usual.
        exponent = get_aistate().character.warship_adjusted_production_cost_exponent()
        return super(MilitaryShipDesignerBaseClass, self)._adjusted_production_cost()**exponent

    def _effective_structure(self):
        effective_structure = self.design_stats.structure + self._expected_organic_growth() + self._remaining_growth() / 5
        effective_structure *= self._shield_factor()
        return effective_structure

    def _speed_factor(self):
        return 1 + 0.005*(self.design_stats.speed - 85)

    def _fuel_factor(self):
        return 1 + 0.03 * (self._effective_fuel() - self._minimum_fuel()) ** 0.5


class WarShipDesigner(MilitaryShipDesignerBaseClass):
    """Class that implements military designs.

    Extends __init__()
    Overrides _rating_function()
    Overrides _starting_guess()
    Overrides _calc_rating_for_name()
    """
    # TODO: Consider subclassing into small/big ships.
    # While big flagships are good in one-on-one situations, we may want to consider building small ships to support
    # them efficiently and act as decoys and anti-decoy-weapon in the fleet. Before doing so, changes need to be done
    # to how the AI assembles its military fleets.

    basename = "Warship"
    description = "Military Ship"
    useful_part_classes = ARMOUR | WEAPONS | SHIELDS | FUEL | ENGINES
    filter_useful_parts = True
    filter_inefficient_parts = True

    NAMETABLE = "AI_SHIPDESIGN_NAME_MILITARY"
    NAME_THRESHOLDS = sorted([0, 100, 250, 500, 1000, 2500, 5000, 7500, 10000,
                              15000, 20000, 25000, 30000, 35000, 40000, 50000, 70000, 1000000])

    def __init__(self):
        ShipDesigner.__init__(self)
        self.additional_specifications.minimum_fuel = 1
        self.additional_specifications.minimum_speed = 30
        self.additional_specifications.expected_turns_till_fight = 10 if fo.currentTurn() < 50 else 5

    def _rating_function(self):
        # TODO: Find a better way to determine the value of speed and fuel
        # Generally, the only relevant damage is that which gets through the enemy shields.
        # However, even if we do not get through the enemy shields at all, we want to make sure we have ships with
        # fighting capability to deal with weaker enemy ships, troopers and so on. But this is only meant as last resort
        # and any design that actually deals damage through shields is to be prefered, i.e. should get a better rating.
        total_dmg = max(self._total_dmg_vs_shields(), self._total_dmg() / 1000)
        if total_dmg <= 0:
            return INVALID_DESIGN_RATING
        combat_rating = self._combat_rating()
        speed_factor = self._speed_factor()
        fuel_factor = self._fuel_factor()
        return combat_rating * speed_factor * fuel_factor / self._adjusted_production_cost()

    def _starting_guess(self, available_parts, num_slots):
        # for military ships, our primary rating function is given by
        # [n*d * (a*(s-n) + h)] / [n*cw + (s-n) * ca + ch]
        # where:
        # s = number of slots
        # n = number of slots filled with weapons
        # d = damage of the weapon
        # a = structure value of armour parts
        # h = base structure of the hull
        # cw, ca, ch = cost of weapon, armour, hull respectively
        # As this is a simple rational function in n, the maximizing problem can be solved analytically.
        # The analytical solution (after rounding to the nearest integer)is a good starting guess for our best design.
        ret_val = (len(available_parts) + 1) * [0]
        parts = [get_part_type(part) for part in available_parts]
        weapons = [part for part in parts if part.partClass in WEAPONS]
        armours = [part for part in parts if part.partClass in ARMOUR]
        if weapons:
            weapon_part = max(weapons, key=self._calculate_weapon_strength)
            weapon = weapon_part.name
            idxweapon = available_parts.index(weapon)
            cw = Cache.production_cost[self.pid].get(weapon, weapon_part.productionCost(fo.empireID(), self.pid))
            if armours:
                armour_part = max(armours, key=_get_capacity)
                armour = armour_part.name
                idxarmour = available_parts.index(armour)
                a = get_part_type(armour).capacity
                ca = Cache.production_cost[self.pid].get(armour, armour_part.productionCost(fo.empireID(), self.pid))
                s = num_slots
                h = self.hull.structure
                ch = Cache.production_cost[self.pid].get(self.hull.name,
                                                         self.hull.productionCost(fo.empireID(), self.pid))
                if ca == cw:
                    n = (s+h/a)/2
                else:
                    p1 = a*s*ca + a*ch
                    p2 = math.sqrt(a * (ca*s + ch) * (a*s*cw+a*ch+h*cw-h*ca))
                    p3 = a*(ca-cw)
                    n = max((p1+p2)/p3, (p1-p2)/p3)
                n = int(round(n))
                n = max(n, 1)
                n = min(n, s)
                debug("estimated weapon slots for %s: %d" % (self.hull.name, n))
                ret_val[idxarmour] = s-n
                ret_val[idxweapon] = n
            else:
                ret_val[idxweapon] = num_slots
        elif armours:
            armour = max(armours, key=_get_capacity).name
            idxarmour = available_parts.index(armour)
            ret_val[idxarmour] = num_slots
        else:
            ret_val[-1] = num_slots
        return ret_val

    def _calc_rating_for_name(self):
        self.update_stats(ignore_species=True)
        return self.design_stats.structure*self._total_dmg()*(1+self.design_stats.shields/10)


class CarrierShipDesigner(MilitaryShipDesignerBaseClass):
    """Class that implements military designs with fighter parts.

    Extends __init__()
    Extends _filling_algorithm()
    Overrides _rating_function()
    Overrides _calc_rating_for_name()
    """
    basename = "Carrier"
    description = "Carrier"
    useful_part_classes = WEAPONS | ARMOUR | SHIELDS | FUEL | ENGINES | FIGHTER_HANGAR | FIGHTER_BAY
    filter_useful_parts = True
    filter_inefficient_parts = True

    NAMETABLE = "AI_SHIPDESIGN_NAME_MILITARY"
    NAME_THRESHOLDS = sorted([0, 1000])

    def __init__(self):
        ShipDesigner.__init__(self)
        self.additional_specifications.minimum_fuel = 1
        self.additional_specifications.minimum_speed = 30
        self.additional_specifications.expected_turns_till_fight = 10 if fo.currentTurn() < 50 else 5
        self.additional_specifications.minimum_fighter_launch_rate = 1

    def _rating_function(self):
        if self.design_stats.fighter_capacity < 1:
            return INVALID_DESIGN_RATING
        combat_rating = self._combat_rating()
        speed_factor = self._speed_factor()
        fuel_factor = self._fuel_factor()
        return combat_rating * speed_factor * fuel_factor / self._adjusted_production_cost()

    # TODO Implement _starting_guess() for faster convergence

    def _filling_algorithm(self, available_parts, verbose=False):
        # Currently, only one type of hangar part is allowed due to game mechanics.
        # However, in the generic _filling_algorithm(), only one part is exchanged per time.
        # Therefore, after using (multiple) entries of one hangar part, the algorithm won't consider different parts.
        # Workaround: Do multiple passes with only one hangar part each and choose the best rated one.
        if verbose:
            debug("Calling _filling_algorithm() for Carrier-Style ships!")
            debug("Available parts: %s" % available_parts)
        # first, get all available hangar parts.
        hangar_parts = set()
        for partlist in available_parts.values():
            for partname in partlist:
                part = get_part_type(partname)
                if part.partClass == fo.shipPartClass.fighterHangar:
                    hangar_parts.add(partname)
        if verbose:
            debug("Found the following hangar parts: %s" % hangar_parts)

        # now, call the standard-algorithm with only one hangar part at a time and choose the best rated one.
        best_rating = INVALID_DESIGN_RATING
        best_partlist = [""] * len(self.hull.slots)
        for this_hangar_part in hangar_parts:
            current_available_parts = {}
            forbidden_hangar_parts = {part for part in hangar_parts if part != this_hangar_part}
            for slot, partlist in available_parts.iteritems():
                current_available_parts[slot] = [part_ for part_ in partlist if part_ not in forbidden_hangar_parts]
            this_rating, this_partlist = ShipDesigner._filling_algorithm(self, current_available_parts)
            if verbose:
                debug("Best rating for part %s is %.2f with partlist %s" % (this_hangar_part, this_rating, this_partlist))
            if this_rating > best_rating:
                best_rating = this_rating
                best_partlist = this_partlist
        return best_rating, best_partlist

    def _calc_rating_for_name(self):
        base_rating = self.design_stats.structure*self._total_dmg()*(1+self.design_stats.shields/10)
        fighter_rating = self.design_stats.fighter_capacity * self.design_stats.fighter_launch_rate * (.1+self.design_stats.fighter_damage)
        return base_rating + fighter_rating


class TroopShipDesignerBaseClass(ShipDesigner):
    """Base class for troop ships. To be inherited from.

    Extends __init__()
    Overrides _rating_function()
    Overrides _starting_guess()
    Overrides _class_specific_filter
    """
    basename = "Troopers (Do not build me)"
    description = "Trooper."
    useful_part_classes = TROOPS
    filter_useful_parts = True
    filter_inefficient_parts = True

    def __init__(self):
        ShipDesigner.__init__(self)

    def _rating_function(self):
        if self.design_stats.troops == 0:
            return INVALID_DESIGN_RATING
        else:
            return min(MAX_TROOPERS_PER_SHIP, self.design_stats.troops)/self._adjusted_production_cost()

    def _starting_guess(self, available_parts, num_slots):
        # fill completely with biggest troop pods. If none are available for this slot type, leave empty.
        troop_pods = [get_part_type(part) for part in available_parts if get_part_type(part).partClass in TROOPS]
        ret_val = (len(available_parts)+1)*[0]
        if troop_pods:
            biggest_troop_pod = max(troop_pods, key=_get_capacity).name
            try:  # we could use an if-check here but since we usually have troop pods for the slot, try is faster
                idx = available_parts.index(biggest_troop_pod)
            except ValueError:
                idx = len(available_parts)
                warn("Found no valid troop pod.", exc_info=True)
        else:
            idx = len(available_parts)
        ret_val[idx] = num_slots
        return ret_val

    def _class_specific_filter(self, partname_dict):
        for slot in partname_dict:
            remaining_parts = [part for part in partname_dict[slot] if
                               get_part_type(part).partClass in TROOPS.union(ARMOUR)]
            partname_dict[slot] = remaining_parts


class OrbitalTroopShipDesigner(TroopShipDesignerBaseClass):
    """Class implementing orbital invasion designs

    Extends __init__()
    """
    basename = "SpaceInvaders"
    description = "Ship designed for local invasions of enemy planets"

    useful_part_classes = TROOPS
    NAMETABLE = "AI_SHIPDESIGN_NAME_TROOPER_ORBITAL"
    NAME_THRESHOLDS = sorted([0])

    def __init__(self):
        TroopShipDesignerBaseClass.__init__(self)
        self.additional_specifications.minimum_speed = 0
        self.additional_specifications.minimum_fuel = 0
        # require that the orbital trooper base have zero speed, otherwise once completed it won't be recognized as an
        # orbital trooper base
        self.additional_specifications.orbital = True


class StandardTroopShipDesigner(TroopShipDesignerBaseClass):
    """Class implementing standard troop ship designs.

    Extends __init__()
    """
    basename = "StormTroopers"
    description = "Ship designed for the invasion of enemy planets"
    useful_part_classes = TROOPS | ARMOUR
    NAMETABLE = "AI_SHIPDESIGN_NAME_TROOPER_STANDARD"
    NAME_THRESHOLDS = sorted([0, 6, 14])

    def __init__(self):
        TroopShipDesignerBaseClass.__init__(self)
        self.additional_specifications.minimum_speed = 30
        self.additional_specifications.minimum_fuel = 2

    def _minimum_structure(self):
        return self._effective_mine_damage() + 1


class ColonisationShipDesignerBaseClass(ShipDesigner):
    """Base class for colonization ships. To be inherited from.

    Extends __init__()
    Overrides _rating_function()
    Overrides _starting_guess()
    Overrides _class_specific_filter()
    """
    basename = "Seeder (Do not build me!)"
    description = "Unarmed Colony Ship"
    useful_part_classes = FUEL | COLONISATION | ENGINES | DETECTION

    filter_useful_parts = True
    filter_inefficient_parts = True

    def __init__(self):
        ShipDesigner.__init__(self)

    def _rating_function(self):
        if self.design_stats.colonisation <= 0:  # -1 indicates no pod, 0 indicates outpost
            return INVALID_DESIGN_RATING
        return self.design_stats.colonisation*(1+0.002*(self.design_stats.speed-75))/self.production_cost

    def _starting_guess(self, available_parts, num_slots):
        # we want to use one and only one of the best colo pods
        ret_val = (len(available_parts)+1)*[0]
        if num_slots == 0:
            return ret_val
        parts = [get_part_type(part) for part in available_parts]
        colo_parts = [part for part in parts if part.partClass in COLONISATION and part.capacity > 0]
        if colo_parts:
            colo_part = max(colo_parts, key=lambda x: x.capacity)
            idx = available_parts.index(colo_part.name)
            ret_val[idx] = 1
            ret_val[-1] = num_slots - 1
        else:
            ret_val[-1] = num_slots
        return ret_val

    def _class_specific_filter(self, partname_dict):
        # remove outpost pods
        for slot in partname_dict:
            parts = [get_part_type(part) for part in partname_dict[slot]]
            for part in parts:
                if part.partClass in COLONISATION and part.capacity == 0:
                    partname_dict[slot].remove(part.name)


class StandardColonisationShipDesigner(ColonisationShipDesignerBaseClass):
    """Class that implements standard colonisation ships.

    Extends __init__()
    """
    basename = "Seeder"
    description = "Unarmed ship designed for the colonisation of distant planets"
    useful_part_classes = FUEL | COLONISATION | ENGINES | DETECTION
    NAMETABLE = "AI_SHIPDESIGN_NAME_COLONISATION_STANDARD"
    NAME_THRESHOLDS = sorted([0])

    def __init__(self):
        ColonisationShipDesignerBaseClass.__init__(self)
        self.additional_specifications.minimum_speed = 30
        self.additional_specifications.minimum_fuel = 1


class OrbitalColonisationShipDesigner(ColonisationShipDesignerBaseClass):
    """Class implementing orbital colonisation ships.

    Extends __init__()
    Overrides _rating_function()
    """
    basename = "Orbital Seeder"
    description = "Unarmed ship designed for the colonisation of local planets"
    useful_part_classes = COLONISATION
    NAMETABLE = "AI_SHIPDESIGN_NAME_COLONISATION_ORBITAL"
    NAME_THRESHOLDS = sorted([0])

    def __init__(self):
        ColonisationShipDesignerBaseClass.__init__(self)
        self.additional_specifications.minimum_speed = 0
        self.additional_specifications.minimum_fuel = 0

    def _rating_function(self):
        if self.design_stats.colonisation <= 0:  # -1 indicates no pod, 0 indicates outpost
            return INVALID_DESIGN_RATING
        return self.design_stats.colonisation/self.production_cost


class OutpostShipDesignerBaseClass(ShipDesigner):
    """Base class for outposter designs. To be inherited.

    Extends __init__()
    Overrides _rating_function()
    Overrides _starting_guess()
    Overrides _class_specific_filter()
    """
    basename = "Outposter (do not build me!)"
    description = "Unarmed Outposter Ship"
    useful_part_classes = COLONISATION | FUEL | ENGINES | DETECTION

    filter_useful_parts = True
    filter_inefficient_parts = True

    def __init__(self):
        ShipDesigner.__init__(self)

    def _rating_function(self):
        if self.design_stats.colonisation != 0:
            return INVALID_DESIGN_RATING
        return (1+0.002*(self.design_stats.speed-75))/self.production_cost

    def _class_specific_filter(self, partname_dict):
        # filter all colo pods
        for slot in partname_dict:
            parts = [get_part_type(part) for part in partname_dict[slot]]
            for part in parts:
                if part.partClass in COLONISATION and part.capacity != 0:
                    partname_dict[slot].remove(part.name)

    def _starting_guess(self, available_parts, num_slots):
        # use one outpost pod as starting guess
        ret_val = (len(available_parts)+1)*[0]
        if num_slots == 0:
            return ret_val
        parts = [get_part_type(part) for part in available_parts]
        colo_parts = [part for part in parts if part.partClass in COLONISATION and part.capacity == 0]
        if colo_parts:
            colo_part = colo_parts[0]
            idx = available_parts.index(colo_part.name)
            ret_val[idx] = 1
            ret_val[-1] = num_slots - 1
        else:
            ret_val[-1] = num_slots
        return ret_val


class OrbitalOutpostShipDesigner(OutpostShipDesignerBaseClass):
    """Class that implements orbital outposting ships.

    Extends __init__()
    Overrides _rating_function()
    """
    basename = "OrbitalOutposter"
    description = "Unarmed ship designed for founding local outposts"
    useful_part_classes = COLONISATION
    filter_useful_parts = True
    filter_inefficient_parts = False
    NAMETABLE = "AI_SHIPDESIGN_NAME_OUTPOSTER_ORBITAL"
    NAME_THRESHOLDS = sorted([0])

    def __init__(self):
        OutpostShipDesignerBaseClass.__init__(self)
        self.additional_specifications.minimum_fuel = 0
        self.additional_specifications.minimum_speed = 0

    def _rating_function(self):
        if self.design_stats.colonisation != 0:
            return INVALID_DESIGN_RATING
        return 1/self.production_cost


class StandardOutpostShipDesigner(OutpostShipDesignerBaseClass):
    """Class that implements standard outposting ships.

    Extends __init__()
    """
    basename = "Outposter"
    description = "Unarmed ship designed for founding distant outposts"
    useful_part_classes = COLONISATION | FUEL | ENGINES | DETECTION
    NAMETABLE = "AI_SHIPDESIGN_NAME_OUTPOSTER_STANDARD"
    NAME_THRESHOLDS = sorted([0])

    def __init__(self):
        OutpostShipDesignerBaseClass.__init__(self)
        self.additional_specifications.minimum_fuel = 2
        self.additional_specifications.minimum_speed = 30


class OrbitalDefenseShipDesigner(ShipDesigner):
    """Class that implements orbital defense designs.

    Extends __init__()
    Overrides _rating_function()
    """
    basename = "Decoy"
    description = "Orbital Defense Ship"
    useful_part_classes = WEAPONS | ARMOUR
    NAMETABLE = "AI_SHIPDESIGN_NAME_ORBITAL_DEFENSE"
    NAME_THRESHOLDS = sorted([0, 1])

    filter_useful_parts = True
    filter_inefficient_parts = True

    def __init__(self):
        ShipDesigner.__init__(self)

    def _rating_function(self):
        if self.design_stats.speed > 10:
            return INVALID_DESIGN_RATING
        total_dmg = self._total_dmg_vs_shields()
        return (1+total_dmg*self.design_stats.structure)/self._adjusted_production_cost()

    def _calc_rating_for_name(self):
        self.update_stats(ignore_species=True)
        return self._total_dmg()


class ScoutShipDesigner(ShipDesigner):
    """Scout ship class"""
    basename = "Scout"
    description = "For exploration and reconnaissance"
    useful_part_classes = DETECTION | FUEL | ENGINES
    NAMETABLE = "AI_SHIPDESIGN_NAME_SCOUT"
    NAME_THRESHOLDS = sorted([0])
    filter_useful_parts = True
    filter_inefficient_parts = True

    def __init__(self):
        ShipDesigner.__init__(self)
        self.additional_specifications.minimum_fuel = 3
        self.additional_specifications.minimum_speed = 60

    def _rating_function(self):
        if not self.design_stats.detection:
            return INVALID_DESIGN_RATING
        detection_factor = self.design_stats.detection ** 2
        fuel_factor = self._effective_fuel()
        speed_factor = self.design_stats.speed ** 0.5
        return detection_factor * fuel_factor * speed_factor / self.production_cost


class KrillSpawnerShipDesigner(ShipDesigner):
    """Stealthy ship used for harassing the enemy using the Krill Spawner part."""
    basename = "Krill Spawner"
    description = "Stealthy ship that unleashes chaos in the enemy backlines."
    useful_part_classes = DETECTION | FUEL | ENGINES | GENERAL | ARMOUR  # cloak parts do not stack with Krillspawner
    NAMETABLE = "AI_SHIPDESIGN_NAME_KRILLSPAWNER"
    NAME_THRESHOLDS = sorted([0])
    filter_useful_parts = True
    filter_inefficient_parts = True

    def __init__(self):
        ShipDesigner.__init__(self)
        self.additional_specifications.minimum_speed = 30
        self.additional_specifications.minimum_fuel = 2

    def _rating_function(self):
        if AIDependencies.PART_KRILL_SPAWNER not in self.partnames:
            return INVALID_DESIGN_RATING
        structure_factor = (1 + self.design_stats.structure - self._minimum_structure())**0.03  # nice to have but not too important
        fuel_factor = self._effective_fuel()
        speed_factor = 1 + (self.design_stats.speed - self._minimum_speed())**0.1
        stealth_factor = 1 + (self.design_stats.stealth + self.design_stats.asteroid_stealth / 2)  # TODO: Adjust for enemy detection strength
        detection_factor = self.design_stats.detection**1.5
        return (structure_factor * fuel_factor * speed_factor *
                stealth_factor * detection_factor / self.production_cost)

    def _minimum_structure(self):
        return 2 * self._effective_mine_damage() + 1

    def _starting_guess(self, available_parts, num_slots):
        # starting guess is a design completely empty except for the krill spawner part.
        ret_val = (len(available_parts)+1)*[0]
        if num_slots == 0:
            return ret_val
        if AIDependencies.PART_KRILL_SPAWNER not in available_parts:
            ret_val[-1] = num_slots
        else:
            idx = available_parts.index(AIDependencies.PART_KRILL_SPAWNER)
            ret_val[idx] = 1
            ret_val[-1] = num_slots - 1
        return ret_val


def _create_ship_design(design_name, hull_name, part_names, model="fighter",
                        description="", icon="", name_desc_in_string_table=False,
                        verbose=False):
    """This is basically a wrapper around fo.issueCreateShipDesignOrder to
    also update the cache.

    :param design_name: the name of the design
    :type design_name: str
    :param description: a human readable description of the design
    :type description: str
    :param hull_name: the name of the hull to use
    :type hull_name: str
    :param part_names: list of partnames (string)
    :type part_names: list
    :param icon: an icon that is shown (for instance in the Production dialog)
    :type icon: str
    :param model: a modelname
    :type model: str
    :param name_desc_in_string_table: lookup the name in the stringstable
    :type name_desc_in_string_table: bool
    :param verbose: write some debugging output
    :type verbose: bool

    :returns: bool (True on success False on error)
    """

    res = bool(fo.issueCreateShipDesignOrder(design_name, description,
                                             hull_name, part_names, icon,
                                             model, name_desc_in_string_table))
    if res:
        if verbose:
            debug("Success: Added Design %s, with result %d" % (design_name, res))
        # update cache
        design = _update_design_by_name_cache(design_name, verbose=verbose)
        if design:
            if verbose:
                debug("Success: Design %s stored in design_by_name_cache" % design_name)
        else:
            warn("Tried to get just created design %s but got None" % design_name)
    else:
        warn("Tried to add design %s but returned %s, expected 1" % (design_name, res))

    return res


def _update_design_by_name_cache(design_name, verbose=False, cache_as_invalid=True):
    """Updates the design by name cache

    :param design_name: the name of the design that needs updating
    :type design_name: str
    :param verbose: write some debugging output
    :type verbose: bool

    :return: shipDesign object or None
    """

    design = None
    for design_id in fo.getEmpire().allShipDesigns:
        if verbose:
            debug("Checking design %s in search for %s" % (fo.getShipDesign(design_id).name, design_name))
        if fo.getShipDesign(design_id).name == design_name:
            design = fo.getShipDesign(design_id)
            break
    if design:
        Cache.design_id_by_name[design_name] = design.id
    elif cache_as_invalid:
        # invalid design
        debug("Shipdesign %s seems not to exist: Caching as invalid design." % design_name)
        Cache.design_id_by_name[design_name] = INVALID_ID
    return design


def _get_design_by_name(design_name, update_invalid=False, looking_for_new_design=False):
    """Return the shipDesign object of the design with the name design_name.

    Results are cached for performance improvements. The cache is to be
    checked for consistency with check_cache_for_consistency() once per turn
    as there appears to be a random bug in multiplayer, changing IDs. If a
    new design is created, this function must be called with update_invalid
    set to True to update its internal cache accordingly.

    :param design_name: string
    :param update_invalid: update a previously invalid design in cache
    :return: shipDesign object
    """
    # get shipdesign from fo if
    #  * not in cache or
    #  * if an design is invalid and update_invalid is true
    # otherwise use cache
    if design_name in Cache.design_id_by_name and not (
            update_invalid and (Cache.design_id_by_name[design_name] == INVALID_ID)):
        design = fo.getShipDesign(Cache.design_id_by_name[design_name])
    else:
        design = _update_design_by_name_cache(design_name, cache_as_invalid=not looking_for_new_design)

    return design


def get_part_type(partname):
    """Return the partType object (fo.getPartType(partname)) of the given partname.

    As the function in late game may be called some thousand times, the results are cached.

    :type partname: str
    :rtype: fo.partType
    """
    if not partname:
        return None
    if partname in Cache.part_by_partname:
        return Cache.part_by_partname[partname]
    else:
        parttype = fo.getPartType(partname)
        if parttype:
            Cache.part_by_partname[partname] = parttype
            return Cache.part_by_partname[partname]
        else:
            warn("Could not find part %s" % partname)
            return None


def _build_reference_name(hullname, partlist):
    """
    This reference name is used to identify existing designs and is mapped
    by Cache.map_reference_design_name to the ingame design name. Order of components are ignored.

    :param hullname: hull name
    :type hullname: str
    :param partlist: list of part names
    :type partlist: list
    :return: reference name
    :rtype: str
    """
    return "%s-%s" % (hullname, "-".join(sorted(partlist)))  # "Hull-Part1-Part2-Part3-Part4"


def _can_build(design, empire_id, pid):
    # TODO: Remove this function once we stop profiling this module
    """Check if a design can be built by an empire on a particular planet.

    This function only exists for profiling reasons to add an extra entry to cProfile.

    :param design: design object
    :param empire_id:
    :param pid: id of the planet for which the check is performed
    :return: bool
    """
    return design.productionLocationForEmpire(empire_id, pid)


def recursive_dict_diff(dict_new, dict_old, dict_diff, diff_level_threshold=0):
    """Find the entries in dict_new that are not present in dict_old and store them in dict_diff.

    Example usage:
    dict_a = {1:2, 2: {2: 3, 3: 4}}
    dict_b = {2: {2: 3, 3: 3}}
    diff = {}
    recursive_dict_diff(dict_a, dict_b, diff)
    --> diff = {1:2, 2:{3:4}}

    :type dict_new: dict
    :type dict  _old: dict
    :param dict_diff: Difference between dict_old and dict_new, modified and filled within this function
    :type dict_diff: dict
    :param diff_level_threshold: Depth to next diff up to which non-diff entries are stored in dict_diff
    :return: recursive depth distance to entries differing in dict_new and dict_old
    :rtype: int
    """
    NO_DIFF = 9999
    min_diff_level = NO_DIFF
    for key, value in dict_new.iteritems():
        if key not in dict_old:
            dict_diff[key] = copy.deepcopy(value)
            min_diff_level = 0
        elif isinstance(value, dict):
            this_diff_level = recursive_dict_diff(value, dict_old[key], dict_diff.setdefault(key, {}), diff_level_threshold) + 1
            min_diff_level = min(min_diff_level, this_diff_level)
            if this_diff_level > NO_DIFF and min_diff_level > diff_level_threshold:
                del dict_diff[key]
        elif key not in dict_old or value != dict_old[key]:
                dict_diff[key] = copy.deepcopy(value)
                min_diff_level = 0
    return min_diff_level


def _get_tech_bonus(upgrade_dict, part_name):
    try:
        upgrades = upgrade_dict[part_name]
    except KeyError:
        if part_name not in _raised_warnings:
            _raised_warnings.add(part_name)
            error(("WARNING: Encountered unknown part (%s): "
                   "The AI can play on but its damage estimates may be incorrect leading to worse decision-making. "
                   "Please update AIDependencies.py") % part_name, exc_info=True)
        return 0
    total_tech_bonus = 0
    for tech, bonus in upgrades:
        total_tech_bonus += bonus if tech_is_complete(tech) else 0
        # TODO: Error checking if tech is actually a valid tech (tech_is_complete simply returns false)
    return total_tech_bonus
