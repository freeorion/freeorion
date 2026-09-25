from focs._conditions import ContainedBy, Contains, IsSource, Object, Planet
from focs._effects import EffectsGroup, SetStealth
from focs._enums import AsteroidsType
from focs._sources import Source
from focs._value_refs import Value

ASTEROID_FIELD_STEALTH_BONUS = EffectsGroup(
    scope=IsSource,
    activation=ContainedBy(Object(id=Source.SystemID) & Contains(Planet(type=[AsteroidsType]))),
    accountinglabel="ASTEROID_FIELD_STEALTH",
    effects=SetStealth(value=Value + 20),
)
