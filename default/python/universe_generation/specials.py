import freeorion as fo
import random
from collections import defaultdict

import universe_statistics
import universe_tables

# REPEAT_RATE along with calculate_number_of_specials_to_place determines if there are multiple
# specials in a single location.  There can only be at most 4 specials in a single location.
# The probabilites break down as follows:
# Count  Probability
# one    (1 - REPEAT_RATE[0])
# two    REPEAT_RATE[0] * (1 - REPEAT_RATE[1])
# three  REPEAT_RATE[0] * REPEAT_RATE[1] *(1 - REPEAT_RATE[2])
# four   REPEAT_RATE[0] * REPEAT_RATE[1] * REPEAT_RATE[2]
REPEAT_RATE = {1: 0.08, 2: 0.05, 3: 0.01, 4: 0.00}


def calculate_number_of_specials_to_place(objs):
    """Return a list of number of specials to be placed at each obj"""
    return [
        1
        if random.random() > REPEAT_RATE[1]
        else 2
        if random.random() > REPEAT_RATE[2]
        else 3
        if random.random() > REPEAT_RATE[3]
        else 4
        for _ in objs
    ]


def place_special(specials, obj):
    """
    Place at most a single special.
    Return the number of specials placed.
    """
    # Calculate the conditional probabilities that each special is
    # placed here given that a special will be placed here.
    probs = [fo.special_spawn_rate(sp) for sp in specials]
    total_prob = float(sum(probs))
    if total_prob == 0:
        # This shouldn't happen since special_spawn_rate > 0.0 is checked in distribute_specials()
        return 0
    thresholds = [x / total_prob for x in probs]

    chance = random.random()
    for threshold, special in zip(thresholds, specials):
        if chance > threshold:
            chance -= threshold
            continue

        fo.add_special(obj, special)
        print("Special", special, "added to", fo.get_name(obj))
        universe_statistics.specials_summary[special] += 1

        return 1
    return 0


# TODO Bug:  distribute_specials forward checks that a special can be
# placed, but it doesn't recursively check all previously placed
# specials against the new special.
def distribute_specials(specials_freq, universe_objects):  # noqa: C901
    """
    Adds start-of-game specials to universe objects.
    """
    # get basic chance for occurrence of specials from the universe tables
    base_chance = universe_tables.SPECIALS_FREQUENCY[specials_freq]
    if base_chance <= 0:
        return

    # get a list with all specials that have a spawn rate and limit both > 0 and a location condition defined
    # (no location condition means a special shouldn't get added at game start)
    specials = [
        sp
        for sp in fo.get_all_specials()
        if fo.special_spawn_rate(sp) > 0.0 and fo.special_spawn_limit(sp) > 0 and fo.special_has_location(sp)
    ]
    if not specials:
        return

    # dump a list of all specials meeting that conditions and their properties to the log
    print("Specials available for distribution at game start:")
    for special in specials:
        print(
            "... {:30}: spawn rate {:2.3f} / spawn limit {}".format(
                special, fo.special_spawn_rate(special), fo.special_spawn_limit(special)
            )
        )

    objects_needing_specials = [obj for obj in universe_objects if random.random() < base_chance]

    track_num_placed = {obj: 0 for obj in universe_objects}

    print(
        "Base chance for specials is {}. Placing specials on {} of {} ({:1.4f})objects".format(
            base_chance,
            len(objects_needing_specials),
            len(universe_objects),
            float(len(objects_needing_specials)) / len(universe_objects),
        )
    )

    obj_tuple_needing_specials = set(
        zip(
            objects_needing_specials,
            fo.objs_get_systems(objects_needing_specials),
            calculate_number_of_specials_to_place(objects_needing_specials),
        )
    )

    # Equal to the largest distance in WithinStarlaneJumps conditions
    # GALAXY_DECOUPLING_DISTANCE is used as follows.  For any two or more objects
    # at least GALAXY_DECOUPLING_DISTANCE appart you only need to check
    # fo.special_locations once and then you can place as many specials as possible,
    # subject to number restrictions.
    #
    # Organize the objects into sets where all objects are spaced GALAXY_DECOUPLING_DISTANCE
    # appart.  Place a special on each one.  Repeat until you run out of specials or objects.
    GALAXY_DECOUPLING_DISTANCE = 6

    while obj_tuple_needing_specials:
        systems_needing_specials = defaultdict(set)
        for obj, system, specials_count in obj_tuple_needing_specials:
            systems_needing_specials[system].add((obj, system, specials_count))

        print(f" Placing in {len(systems_needing_specials)} locations remaining.")

        # Find a list of candidates all spaced GALAXY_DECOUPLING_DISTANCE apart
        candidates = []
        while systems_needing_specials:
            random_sys = random.choice(list(systems_needing_specials.values()))
            member = random.choice(list(random_sys))
            obj, system, specials_count = member
            candidates.append(obj)
            obj_tuple_needing_specials.remove(member)
            if specials_count > 1:
                obj_tuple_needing_specials.add((obj, system, specials_count - 1))

            # remove all neighbors from the local pool
            for neighbor in fo.systems_within_jumps_unordered(GALAXY_DECOUPLING_DISTANCE, [system]):
                if neighbor in systems_needing_specials:
                    systems_needing_specials.pop(neighbor)

        print(
            "Caching specials_locations() at {} of {} remaining locations.".format(
                str(len(candidates)), str(len(obj_tuple_needing_specials) + len(candidates))
            )
        )
        # Get the locations at which each special can be placed
        locations_cache = {}
        for special in specials:
            # The fo.special_locations in the following line consumes most of the time in this
            # function.  Decreasing GALAXY_DECOUPLING_DISTANCE will speed up the whole
            # function by reducing the number of times this needs to be called.
            locations_cache[special] = set(fo.special_locations(special, candidates))

        # Attempt to apply a special to each candidate
        # by finding a special that can be applied to it and hasn't been added too many times
        for obj in candidates:
            # check if the spawn limit for this special has already been reached (that is, if this special
            # has already been added the maximal allowed number of times)
            specials = [s for s in specials if universe_statistics.specials_summary[s] < fo.special_spawn_limit(s)]
            if not specials:
                break

            # Find which specials can be placed at this one location
            local_specials = [sp for sp in specials if obj in locations_cache[sp]]
            if not local_specials:
                universe_statistics.specials_repeat_dist[0] += 1
                continue

            # All prerequisites and the test have been met, now add this special to this universe object.
            track_num_placed[obj] += place_special(local_specials, obj)

    for num_placed in track_num_placed.values():
        universe_statistics.specials_repeat_dist[num_placed] += 1
