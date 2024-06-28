# The SpeciesCensusOrdering.focs.txt file is used in
# default/scripting/species.  The format is a single line,

# `SpeciesCensusOrdering`

# followed by any number of lines like,

# `tag = "LITHIC"\n`

# The list of species tags provides the order that species are displayed
# and ordered in the empire Census pop-up.
try:
    from focs._species import *
except ModuleNotFoundError:
    pass

SpeciesCensusOrdering(["LITHIC", "ORGANIC", "PHOTOTROPHIC", "ROBOTIC", "SELF_SUSTAINING", "TELEPATHIC", "GASEOUS"])
