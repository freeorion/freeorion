import freeorion as fo
from planets import planet_types_real, planet_sizes_real


###############################
## STAR GROUP NAMING OPTIONS ##
###############################

# if star_groups_use_chars is true use entries from the stringtable entry STAR_GROUP_CHARS (single characters),
# otherwise use words like 'Alpha' from the stringtable entry STAR_GROUP_WORDS.
STAR_GROUPS_USE_CHARS = True

# postfix_stargroup_modifiers determines how star group system names are modified.
# if true, the greek letters (as chars or words, determined by star_group_use_chars,
# see above) are appended as postfix ("Centauri Alpha"), if false, they are prepended
# as prefix ("Alpha Centauri").
POSTFIX_STARGROUP_MODIFIERS = True

# the target proportion of star systems to be given individual names, dependent on size of galaxy
TARGET_INDIV_RATIO_SMALL = 0.6  # in small galaxies, 60% of the star systems get individual names
TARGET_INDIV_RATIO_LARGE = 0.3  # in large galaxies, 30% of the star systems get individual names
NAMING_LARGE_GALAXY_SIZE = 200  # a galaxy with 200+ star systems is considered as large, below that value as small


###################################
## HOME SYSTEM SELECTION OPTIONS ##
###################################

# These options are needed in the home system selection/placement process. They determines the minimum number of
# planets that a home system must have in its near vicinity, and define the extend of this "near vicinity".

# The following two options are used to determine the minimum number of planets. This limit is
# HS_MIN_PLANETS_IN_VICINITY_PER_SYSTEM planets per system within the near vicinity of a home system, capped at
# HS_MIN_PLANETS_IN_VICINITY_TOTAL
HS_MIN_PLANETS_IN_VICINITY_TOTAL = 10
HS_MIN_PLANETS_IN_VICINITY_PER_SYSTEM = 1

# This option defines the extend of what is considered the "near vicinity" of a home system. This are all systems that
# are within the number of jumps specified by HS_VICINITY_RANGE.
HS_VICINITY_RANGE = 3

# These two options define which types of planets are counted when determining the number of planets in the near
# vicinity of a home system. HS_ACCEPTABLE_PLANET_SIZES is actually only needed for the process of adding planets
# to the near vicinity of a home system in case that's needed to meet the limit.
HS_ACCEPTABLE_PLANET_TYPES = planet_types_real + (fo.planetType.asteroids,)
HS_ACCEPTABLE_PLANET_SIZES = planet_sizes_real + (fo.planetSize.asteroids,)
