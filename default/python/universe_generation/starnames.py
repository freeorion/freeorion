import random

import freeorion as fo

import names
import options


# for starname modifiers
stargroup_words = []
stargroup_chars = []
stargroup_modifiers = []
greek_letters = ["Alpha", "Beta", "Gamma", "Delta", "Epsilon", "Zeta", "Eta", "Theta", "Iota", "Kappa",
                 "Lambda", "Mu", "Nu", "Xi", "Omicron", "Pi", "Rho", "Sigma", "Tau", "Upsilon", "Phi",
                 "Chi", "Psi", "Omega"]


def random_star_name():
    """
    Returns a random star system name.
    """
    return "".join(random.sample(names.consonants + names.vowels, 3)).upper() + "-" + str(random.randint(1000, 9999))


# used in clustering
def recalc_centers(ctrs, points, assignments):
    tallies = [[[], []] for _ in ctrs]
    for index_p, index_ctr in enumerate(assignments):
        tallies[index_ctr][0].append(points[index_p][0])
        tallies[index_ctr][1].append(points[index_p][1])
    for index_ctr, tally in enumerate(tallies):
        num_here = len(tally[0])
        if num_here == 0:
            pass  # leave ctr unchanged if no points assigned to it last round
        else:
            ctrs[index_ctr] = [float(sum(tally[0])) / num_here, float(sum(tally[1])) / num_here]


# used in clustering
def assign_clusters(points, ctrs):
    assignments = []
    for point in points:
        best_dist_sqr = 1e20
        best_ctr = 0
        for index_ctr, ctr in enumerate(ctrs):
            this_dist_sqr = (ctr[0] - point[0])**2 + (ctr[1] - point[1])**2  # TODO use math.hypot, max
            if this_dist_sqr < best_dist_sqr:
                best_dist_sqr = this_dist_sqr
                best_ctr = index_ctr
        assignments.append(best_ctr)
    return assignments


def cluster_stars(positions, num_star_groups):
    """
    Returns a list, same size as positions argument, containing indices from 0 to num_star_groups.
    """

    if num_star_groups > len(positions):
        return [[pos] for pos in positions]
    centers = [[pos[0], pos[1]] for pos in random.sample(positions, num_star_groups)]
    all_coords = [(pos[0], pos[1]) for pos in positions]
    clusters = [[], []]
    clusters[0] = assign_clusters(all_coords, centers)  # assign clusters based on init centers
    old_c = 0
    num_convergence_loops = 1  # if full convergence is deemed important then use a higher humber here
    for loop in range(num_convergence_loops):  # main loop to try getting some convergence of center assignments
        recalc_centers(centers, all_coords, clusters[old_c])  # get new centers
        clusters[1 - old_c] = assign_clusters(all_coords, centers)  # assign clusters based on new centers
        if clusters[0] == clusters[1]:
            break  # stop iterating if no change in cluster assignments
        old_c = 1 - old_c
    else:
        if loop > 0:  # if here at loop 0, then didn't try for convergence
            print "falling through system clustering iteration loop without convergence"
    return clusters[1 - old_c]


def check_deep_space(group_list, star_type_assignments, planet_assignments):
    deep_space = []
    not_deep = []
    for systemxy in group_list:
        if star_type_assignments.get(systemxy, fo.starType.noStar) != fo.starType.noStar:
            not_deep.append(systemxy)
        else:
            if len(planet_assignments.get(systemxy, [])) > 0:
                not_deep.append(systemxy)
            else:
                deep_space.append(systemxy)
    return not_deep, deep_space


def name_group(group_list, group_name, star_type_assignments, planet_assignments):
    group_size = len(group_list)
    if group_size == 1:
        return [(group_list[0], group_name)]
    modifiers = list(stargroup_modifiers)  # copy the list so we can safely add to it if needed
    not_deep, deep_space = check_deep_space(group_list, star_type_assignments, planet_assignments)
    these_systems = not_deep + deep_space  # so that unnamed deep space will get the later star group modifiers
    while len(modifiers) < group_size:  # emergency fallback
        trial_mod = random.choice(stargroup_modifiers) + " " +\
            "".join(random.sample(names.consonants + names.vowels, 3)).upper()
        if trial_mod not in modifiers:
            modifiers.append(trial_mod)
    if options.POSTFIX_STARGROUP_MODIFIERS:
        name_list = [group_name + " " + modifier for modifier in modifiers]
    else:
        name_list = [modifier + " " + group_name for modifier in modifiers]
    return zip(these_systems, name_list)


def name_star_systems(system_list):
    # choose star types and planet sizes, before choosing names, so naming can have special handling of Deep Space
    star_type_assignments = {}
    planet_assignments = {}
    position_list = []
    for system in system_list:
        star_type = fo.sys_get_star_type(system)
        systemxy = fo.get_pos(system)
        star_type_assignments[systemxy] = star_type
        planet_assignments[systemxy] = fo.sys_get_planets(system)
        position_list.append(systemxy)

    # will name name a portion of stars on a group basis, where the stars of each group share the same base star name,
    # suffixed by different (default greek) letters or characters (options at top of file)
    star_name_map = {}
    star_names = names.get_name_list("STAR_NAMES")
    group_names = names.get_name_list("STAR_GROUP_NAMES")
    potential_group_names = []
    individual_names = []
    stargroup_words[:] = names.get_name_list("STAR_GROUP_WORDS")
    stargroup_chars[:] = names.get_name_list("STAR_GROUP_CHARS")
    stargroup_modifiers[:] = [stargroup_words, stargroup_chars][options.STAR_GROUPS_USE_CHARS]
    for starname in star_names:
        if len(starname) > 6:  # if starname is long, don't allow it for groups
            individual_names.append(starname)
            continue
        # any names that already have a greek letter in them can only be used for individual stars, not groups
        for namepart in starname.split():
            if namepart in greek_letters:
                individual_names.append(starname)
                break
        else:
            potential_group_names.append(starname)

    if not potential_group_names:
        potential_group_names.append("XYZZY")

    # ensure at least a portion of galaxy gets individual starnames
    num_systems = len(system_list)
    choice = num_systems >= options.NAMING_LARGE_GALAXY_SIZE
    target_indiv_ratio = [options.TARGET_INDIV_RATIO_SMALL, options.TARGET_INDIV_RATIO_LARGE][choice]
    # TODO improve the following calc to be more likely to hit target_indiv_ratio if more or less than
    # 50% potential_group_names used for groups
    num_individual_stars = int(max(min(num_systems * target_indiv_ratio,
                                       len(individual_names) + int(0.5 * len(potential_group_names))),
                                   num_systems - 0.8 * len(stargroup_modifiers) *
                                   (len(group_names) + int(0.5 * len(potential_group_names)))))
    star_group_size = 1 + int((num_systems - num_individual_stars) /
                              (max(1, len(group_names) + int(0.5 * len(potential_group_names)))))
    # make group size a bit bigger than min necessary, at least a trio
    star_group_size = max(3, star_group_size)
    num_star_groups = 1 + int(num_systems / star_group_size)  # initial value

    # first cluster all systems, then remove some to be individually named (otherwise groups can have too many
    # individually named systems in their middle).  First remove any that are too small (only 1 or 2 systems).
    # The clusters with the most systems are generally the most closely spaced, and though they might make good
    # logical candidates for groups, their names are then prone to overlapping on the galaxy map, so after removing
    # small groups, remove the groups with the most systems.
    random.shuffle(position_list)  # just to be sure it is randomized
    init_cluster_assgts = cluster_stars(position_list, num_star_groups)
    star_groups = {}
    for index_pos, index_group in enumerate(init_cluster_assgts):
        systemxy = position_list[index_pos]
        star_groups.setdefault(index_group, []).append(systemxy)
    indiv_systems = []

    # remove groups with only one non-deep-system
    for groupindex, group_list in star_groups.items():
        max_can_transfer = len(potential_group_names) - len(star_groups) + len(individual_names) - len(indiv_systems)
        if max_can_transfer <= 0:
            break
        elif max_can_transfer <= len(group_list):
            continue
        not_deep, deep_space = check_deep_space(group_list, star_type_assignments, planet_assignments)
        if len(not_deep) > 1:
            continue
        for systemxy in star_groups[groupindex]:
            indiv_systems.append(systemxy)
        del star_groups[groupindex]

    # remove tiny groups
    group_sizes = [(len(group), index) for index, group in star_groups.items()]
    group_sizes.sort()
    while len(indiv_systems) < num_individual_stars and len(group_sizes) > 0:
        groupsize, groupindex = group_sizes.pop()
        max_can_transfer = len(potential_group_names) - len(star_groups) + len(individual_names) - len(indiv_systems)
        if (max_can_transfer <= 0) or (groupsize > 2):
            break
        if max_can_transfer <= groupsize:
            continue
        for systemxy in star_groups[groupindex]:
            indiv_systems.append(systemxy)
        del star_groups[groupindex]

    # remove largest (likely most compact) groups
    while len(indiv_systems) < num_individual_stars and len(group_sizes) > 0:
        groupsize, groupindex = group_sizes.pop(-1)
        max_can_transfer = len(potential_group_names) - len(star_groups) + len(individual_names) - len(indiv_systems)
        if max_can_transfer <= 0:
            break
        if max_can_transfer <= groupsize:
            continue
        for systemxy in star_groups[groupindex]:
            indiv_systems.append(systemxy)
        del star_groups[groupindex]

    num_star_groups = len(star_groups)
    num_individual_stars = len(indiv_systems)
    random.shuffle(potential_group_names)
    random.shuffle(individual_names)
    random.shuffle(group_names)
    num_for_indiv = min(max(len(potential_group_names) / 2, num_individual_stars + 1 - len(individual_names)),
                        len(potential_group_names))
    individual_names.extend(potential_group_names[:num_for_indiv])
    group_names.extend(potential_group_names[num_for_indiv:])

    # print "sampling for %d indiv names from list of %d total indiv names" % (
    #     num_individual_stars, len(individual_names))
    indiv_name_sample = random.sample(individual_names, num_individual_stars)
    # indiv_name_assignments = zip([(pos.x, pos.y) for pos in position_list[:num_individual_stars]], indiv_name_sample)
    indiv_name_assignments = zip(indiv_systems, indiv_name_sample)
    star_name_map.update(indiv_name_assignments)
    # print "sampling for %d group names from list of %d total group names"%(num_star_groups, len(group_names))
    if len(group_names) < num_star_groups:
        group_names.extend([names.random_name(6) for _ in range(num_star_groups - len(group_names))])
    group_name_sample = random.sample(group_names, num_star_groups)
    for index_group, group_list in enumerate(sorted(star_groups.values())):
        star_name_map.update(name_group(group_list, group_name_sample[index_group], star_type_assignments,
                                        planet_assignments))

    # assign names from star_name_map to star systems
    for system in system_list:
        fo.set_name(system, star_name_map.get(fo.get_pos(system), "") or random_star_name())
