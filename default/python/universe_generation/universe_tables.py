import freeorion as fo


#################################
# Galaxy Generation Data Tables #
#################################

# For non-coders:
# If you aren't adding a new table, you only need to change the numbers.
# The order of rows doesn't matter, but the order of columns does (they should
# always be the same order as in universe/Enums.h).

# How are these numbers used?
# For each star type (including no star / deep space), a random number between
# 0 and 100 is chosen and all applicable bonuses are applied. Whichever star
# type gets the highest random number + fixed bonus is chosen for that system.
#
# The same system also gets used for planets, once all star types have been chosen.

# A simple one-dimensional table; in every roll in every galaxy, the star type
# before the colon gets the specified bonus.
BASE_STAR_TYPE_DIST = {
    fo.starType.blue: 15,
    fo.starType.white: 20,
    fo.starType.yellow: 25,
    fo.starType.orange: 20,
    fo.starType.red: 15,
    fo.starType.neutron: 0,
    fo.starType.blackHole: -5,
    fo.starType.noStar: 10,
}

# More complicated: These bonuses depend not only on the star type, but also
# on the value chosen on the galaxy setup screen for Age. If you look at it as
# a table, the entry in the "Young" row and "blue" column is the bonus for blue
# stars if you chose a Young galaxy.
UNIVERSE_AGE_MOD_TO_STAR_TYPE_DIST = {
#                                blue  white  yellow  orange  red  neutron  blackHole  noStar
#                        Young:
    fo.galaxySetupOption.low:   ( 10,    20,      0,      0,   0,       0,         0,     60),
#                        Mature:
    fo.galaxySetupOption.medium:( -5,     0,     10,     20,   0,      10,        10,     40),
#                        Ancient:
    fo.galaxySetupOption.high:  (-20,   -10,      0,     10,  20,      20,        20,     20),
}

# These following tables are as above, but for planet sizes rather than star
# types. The star types are now the givens, and the planet sizes are being
# rolled for.
#
# Note that for these purposes, Asteroids and Gas Giants are a size of planet.
#
# This checks what you chose for galaxy planet density:
DENSITY_MOD_TO_PLANET_SIZE_DIST = {
#                                none  tiny  small  medium  large  huge  asteroids  gas giant
    fo.galaxySetupOption.low:   ( 85,    0,     0,      0,    -5,  -10,         0,         0),
    fo.galaxySetupOption.medium:( 70,    0,     0,      0,    -5,  -10,         0,         0),
    fo.galaxySetupOption.high:  ( 50,    0,     0,      0,    -5,  -10,         0,         0),
}

# Given the star type that was already chosen, what will the bonus be for this
# size of planet?
# Note: A positive value for none, will mean a chance of empty systems
# of that type in game.  All star types with a zero or less chance for
# none will be forced to have at least one satellite.
STAR_TYPE_MOD_TO_PLANET_SIZE_DIST = {
#                           none  tiny  small  medium  large  huge  asteroids  gas giant
    fo.starType.blue:      (  0,    0,     0,      0,    -5,  -10,         0,         0),
    fo.starType.white:     (  0,    0,     0,      0,    -5,  -10,         0,         0),
    fo.starType.yellow:    (  0,    0,     0,      0,    -5,  -10,         0,         0),
    fo.starType.orange:    (  0,    0,     0,      0,    -5,  -10,         0,         0),
    fo.starType.red:       (  0,    0,     0,      0,    -5,  -10,         0,         0),
    fo.starType.neutron:   (  5,    0,     0,      0,    -5,  -10,        20,         0),
    fo.starType.blackHole: ( 30,    0,     0,     -5,   -10,  -15,        10,        -5),
    fo.starType.noStar:    ( 80,    0,     0,      0,   -10,  -15,         0,        10),
}

# In this one, the given is the orbit number of the planet. So the first row
# corresponds to the planet closest to the star, the second is the next one,
# and so on.
ORBIT_MOD_TO_PLANET_SIZE_DIST = (
#    none  tiny  small  medium  large  huge  asteroids  gas giant
    ( 10,    0,    20,     10,     0,    0,         0,       -80),
    (  5,    0,    15,     20,     5,    0,         0,       -70),
    (  0,    0,    10,     30,    10,    5,         0,       -60),
    (  0,    0,    10,     30,    10,    5,        10,       -30),
    (  0,    0,    10,     20,    15,   10,        25,         0),
    (  0,    0,     5,      5,    20,   15,        10,        30),
    (  0,    0,     5,      5,    15,   10,         5,        30),
    (  0,    0,    10,      5,    10,    5,         5,        30),
    (  5,    0,    10,      5,     5,    5,         5,        30),
    ( 10,    0,    10,      0,     0,    0,         5,        10),
)

# Bonus to the planet size based on the shape of the galaxy.
GALAXY_SHAPE_MOD_TO_PLANET_SIZE_DIST = {
#                                none  tiny  small  medium  large  huge  asteroids  gas giant
    fo.galaxyShape.spiral2:    (   0,    0,     0,      0,     0,    0,         0,         0),
    fo.galaxyShape.spiral3:    (   0,    0,     0,      0,     0,    0,         0,         0),
    fo.galaxyShape.spiral4:    (   0,    0,     0,      0,     0,    0,         0,         0),
    fo.galaxyShape.cluster:    (   0,    0,     0,      0,     0,    0,         0,         0),
    fo.galaxyShape.elliptical: (   0,    0,     0,      0,     0,    0,         0,         0),
    fo.galaxyShape.disc:       (   0,    0,     0,      0,     0,    0,         0,         0),
    fo.galaxyShape.box:        (   0,    0,     0,      0,     0,    0,         0,         0),
    fo.galaxyShape.irregular:  (   0,    0,     0,      0,     0,    0,         0,         0),
    fo.galaxyShape.ring:       (   0,    0,     0,      0,     0,    0,         0,         0),
}

# Whenever any special might be placed, the following probability is the chance
# that it actually will be (after other factors are considered).
SPECIALS_FREQUENCY = {
    fo.galaxySetupOption.none:    0,
    fo.galaxySetupOption.low:    .1,
    fo.galaxySetupOption.medium: .3,
    fo.galaxySetupOption.high:   .8,
}

# Whenever a monster might be placed, the following probability is the chance
# that it actually will be (after other factors are considered).
MONSTER_FREQUENCY = {
    fo.galaxySetupOption.none:   0,
    fo.galaxySetupOption.low:    0.033,
    fo.galaxySetupOption.medium: 0.125,
    fo.galaxySetupOption.high:   0.333,
}

# Whenever natives might be placed on a planet, the following probability is
# the chance that they actually will be (after other factors are considered).
NATIVE_FREQUENCY = {
    fo.galaxySetupOption.none:   0,
    fo.galaxySetupOption.low:    0.083,
    fo.galaxySetupOption.medium: 0.143,
    fo.galaxySetupOption.high:   0.200,
}

# This is the maximum size of the shortest path via starlane that the generator
# can create between two "adjacent" systems. So when the number is smaller, the
# map will end up with more starlanes to create enough connections between adjacent
# systems.
MAX_JUMPS_BETWEEN_SYSTEMS = {
    fo.galaxySetupOption.low:    8,
    fo.galaxySetupOption.medium: 3,
    fo.galaxySetupOption.high:   1,
}

# The maximum length of any starlane the galaxy generator can create (in uu).
# The maximum length enforcment only works if there is at least 2*MIN_SYSTEM_SEPARATION
# between the nearest stars in two groups, in order to place extra star(s) in between.
# Consequently, MAX_STARLANE_LENGTH >= 2 * MIN_SYSTEM_SEPARATION
MAX_STARLANE_LENGTH = 120

# This is the minimum distance between two star systems (in uu).
MIN_SYSTEM_SEPARATION = 35.0
