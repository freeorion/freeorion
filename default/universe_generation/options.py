###############################
## STAR GROUP NAMING OPTIONS ##
###############################

# if star_groups_use_chars is true use entries from the stringtable entry STAR_GROUP_CHARS (single characters),
# otherwise use words like 'Alpha' from the stringtable entry STAR_GROUP_WORDS.
star_groups_use_chars = True

# postfix_stargroup_modifiers determines how star group system names are modified.
# if true, the greek letters (as chars or words, determined by star_group_use_chars,
# see above) are appended as postfix ("Centauri Alpha"), if false, they are prepended
# as prefix ("Alpha Centauri").
postfix_stargroup_modifiers = True

# the target proportion of star systems to be given individual names, dependent on size of galaxy
target_indiv_ratio_small = 0.6  # in small galaxies, 60% of the star systems get individual names
target_indiv_ratio_large = 0.3  # in large galaxies, 30% of the star systems get individual names
naming_large_galaxy_size = 200  # a galaxy with 200+ star systems is considered as large, below that value as small