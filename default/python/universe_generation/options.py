import freeorion as fo

from planets import planet_sizes_real, planet_types_real


#############################
# STAR GROUP NAMING OPTIONS #
#############################

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


#################################
# HOME SYSTEM SELECTION OPTIONS #
#################################

# These options are needed in the home system selection/placement process. They determine the minimum number of
# systems and planets that a home system must have in its near vicinity, define the extend of this "near vicinity", etc.

# The following two options are used to determine the minimum number of systems and planets. This limit is
# HS_MIN_SYSTEMS_IN_VICINITY systems and HS_MIN_PLANETS_IN_VICINITY_PER_SYSTEM planets per system within the near
# vicinity of a home system, capped at HS_MIN_PLANETS_IN_VICINITY_TOTAL
HS_MIN_SYSTEMS_IN_VICINITY = 8
HS_MIN_PLANETS_IN_VICINITY_TOTAL = 10
HS_MIN_PLANETS_IN_VICINITY_PER_SYSTEM = 1

# This option defines the extend of what is considered the "near vicinity" of a home system. This are all systems that
# are within the number of jumps specified by HS_VICINITY_RANGE.
HS_VICINITY_RANGE = 3

# This options sets the maximum starting value for the minimum jump distance limit required between home systems.
# With large galaxies an excessive amount of time can be used in failed attempts to select home systems, so defining
# an upper limit for the home system selection process to use when calculating the starting value for the minimum
# jump distance limit is reasonable.
HS_MAX_JUMP_DISTANCE_LIMIT = 10

# This options defines the minimum jump distance limit between home systems that should be considered high priority.
# As long as the jump distance limit which home systems must at least be apart does not get reduced below this limit
# during the home system selection process, the minimum systems in home system vicinity requirement takes
# precedence over the jump distance limit. If the jump distance limit drops below this minimum jump distance limit,
# the process is restarted giving the jump distance limit precedence.
HS_MIN_DISTANCE_PRIORITY_LIMIT = 5

# These two options define which types of planets are counted when determining the number of planets in the near
# vicinity of a home system. HS_ACCEPTABLE_PLANET_SIZES is actually only needed for the process of adding planets
# to the near vicinity of a home system in case that's needed to meet the limit.
HS_ACCEPTABLE_PLANET_TYPES = planet_types_real + (fo.planetType.asteroids,)
HS_ACCEPTABLE_PLANET_SIZES = planet_sizes_real + (fo.planetSize.asteroids,)
