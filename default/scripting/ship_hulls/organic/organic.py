from focs._conditions import IsSource
from focs._effects import EffectsGroup, SetMaxStructure
from focs._sources import Source
from focs._value_refs import MinOf, NamedIntegerLookup, NamedRealLookup, Value

# macros used for organic ships


def LIVING_HULL_GROWTH_EFFECTS_GROUP(hull_name: str):
    return EffectsGroup(
        scope=IsSource,
        accountinglabel="ORGANIC_GROWTH",
        effects=SetMaxStructure(
            value=Value
            + MinOf(
                float,
                Source.Age * NamedRealLookup(name=f"{hull_name}_GROWTH_PER_TURN"),
                NamedIntegerLookup(name=f"{hull_name}_GROWTH_STRUCTURE"),
            )
        ),
    )
