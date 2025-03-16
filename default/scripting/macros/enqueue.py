# Only check for own buildings. The client may have seen a building once, which has been
# destroyed while outside vision range. In this case the building remains forever in the
# client's context, but it should not stop a player from rebuilding the same type.
from focs._effects import BuildBuilding, Contains, CurrentContent, Enqueued, IsBuilding, OwnedBy, Source

ENQUEUE_BUILD_ONE_PER_PLANET = (
    # Only check for own buildings. The client may have seen a building once, which has been
    # destroyed while outside vision range. In this case the building remains forever in the
    # client's context, but it should not stop a player from rebuilding the same type.
    ~Contains(IsBuilding(name=[CurrentContent]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name=CurrentContent)
    & OwnedBy(empire=Source.Owner)
)

ENQUEUE_ARTIFICIAL_PLANET_EXCLUSION = (
    ~Contains(IsBuilding(name=["BLD_ART_PLANET"]))
    & ~Contains(IsBuilding(name=["BLD_ART_FACTORY_PLANET"]))
    & ~Contains(IsBuilding(name=["BLD_ART_PARADISE_PLANET"]))
    & ~Enqueued(type=BuildBuilding, name="BLD_ART_PLANET")
    & ~Enqueued(type=BuildBuilding, name="BLD_ART_FACTORY_PLANET")
    & ~Enqueued(type=BuildBuilding, name="BLD_ART_PARADISE_PLANET")
)


def DO_NOT_CONTAIN_FOR_ALL_TERRAFORM_PLANET_TYPES():
    planet_types = [
        "BARREN",
        "TUNDRA",
        "DESERT",
        "TERRAN",
        "OCEAN",
        "SWAMP",
        "TOXIC",
        "INFERNO",
        "RADIATED",
        "BARREN",
    ]

    expressions = []

    for planet_type in planet_types:
        expressions.append(~Contains(IsBuilding(name=[f"BLD_TERRAFORM_{planet_type}"])))

    # We add enqueue check in the second loop, because we want to preserve the same order of checks for better diff
    # Once conversion of building is done, we should refactor this code.
    for planet_type in planet_types:
        expressions.append(~Enqueued(type=BuildBuilding, name=f"BLD_TERRAFORM_{planet_type}"))

    return reduce(lambda x, y: x & y, expressions)


def reduce(function, expressions):
    if not expressions:
        return

    if len(expressions) == 1:
        return expressions

    result = expressions[0]
    for expr in expressions[1:]:
        result = function(result, expr)
    return result


def LOCATION_ALLOW_ENQUEUE_IF_PREREQ_ENQUEUED(building_name: str):
    """
    Allows the current building to be enqueued if the given prerequisite is built or enqueued.
    Takes the prerequisite name as parameter; Usage:
        LOCATION_ALLOW_ENQUEUE_IF_PREREQ_ENQUEUED("BLD_SHIPYARD_BASE")
    """
    return (
        Contains(IsBuilding(name=[building_name]) & OwnedBy(empire=Source.Owner))
        |
        # Allows enqueue if this is not enqueued but prerequisite @1@ is
        Enqueued(type=BuildBuilding, name=building_name) & ~Enqueued(type=BuildBuilding, name=CurrentContent)
    )
