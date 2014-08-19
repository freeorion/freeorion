import random
import fo_universe_generator as fo
import statistics


def distribute_specials(specials_freq, universe_objects):
    """
    Adds start-of-game specials to universe objects.
    """

    # get basic chance for occurrence of specials from the universe tables
    # the values there are integers, so we have to divide them by 10,000 to get the actual basic probability value
    basic_chance = float(fo.specials_frequency(specials_freq)) / 10000.0
    if basic_chance <= 0:
        return

    # get a list with all specials that have a spawn rate and limit both > 0 and a location condition defined
    # (no location condition means a special shouldn't get added at game start)
    specials = [sp for sp in fo.get_all_specials() if fo.special_spawn_rate(sp) > 0.0 and
                fo.special_spawn_limit(sp) > 0 and fo.special_has_location(sp)]
    if not specials:
        return
    # dump a list of all specials meeting that conditions and their properties to the log
    print "Specials available for distribution at game start:"
    for special in specials:
        print "...", special, ": spawn rate", fo.special_spawn_rate(special),\
              "/ spawn limit", fo.special_spawn_limit(special)

    # attempt to apply a special to each universe object in the list that has been passed to this function
    # by finding a special that can be applied to it and hasn't been added too many times, and then attempt
    # to add that special by testing its spawn rate
    repeat_rate = {1 : 0.08, 2 : 0.05, 3 : 0.01, 4 : 0.00}
    for univ_obj in universe_objects:
        # for this universe object, find a suitable special
        # start by shuffling our specials list, so each time the specials are considered in a new random order
        random.shuffle(specials)
        num_here = 0

        # then, consider each special until one has been found or we run out of specials
        # (the latter case means that no special is added to this universe object)
        for special in specials:
            # check if the spawn limit for this special has already been reached (that is, if this special
            # has already been added the maximal allowed number of times)
            if statistics.specials_summary[special] >= fo.special_spawn_limit(special):
                # if yes, consider next special
                continue

            # check if this universe object matches the location condition for this special
            # (meaning, if this special can be added to this universe object at all)
            if not fo.special_location(special, univ_obj):
                # if not, consider next special
                continue

            # we have found a special that meets all prerequisites
            # now do the test if we want to add the selected special to this universe object by making a roll against
            # the basic probability multiplied by the spawn rate of the special
            if random.random() > basic_chance * fo.special_spawn_rate(special):
                # no, test failed, break out of the specials loop and continue with the next universe object
                statistics.specials_repeat_dist[num_here] += 1
                break
            num_here += 1

            # all prerequisites and the test have been met, now add this special to this universe object
            fo.add_special(univ_obj, special)
            # increase the statistic counter for this special, so we can keep track of how often it has already
            # been added (needed for the spawn limit test above, and to dump some statistics to the log later)
            statistics.specials_summary[special] += 1
            print "Special", special, "added to", fo.get_name(univ_obj)

            # stop attempting to add specials here?  give a small chance to try more than one special
            if random.random() > repeat_rate.get(num_here, 0.0):
                # sorry, no, break out of the specials loop and continue with the next universe object
                statistics.specials_repeat_dist[num_here] += 1
                break
        else:
                statistics.specials_repeat_dist[num_here] += 1
