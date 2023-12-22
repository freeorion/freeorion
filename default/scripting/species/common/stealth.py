from focs._effects import EffectsGroup, HasTag, IsSource, OnPlanet, Planet, SetStealth, Source, Value
from macros.stealth import HIGH_STEALTH, LOW_STEALTH, MEDIUM_STEALTH

# Linter get crazy because of common suffix for two different files common/stealth.py
assert isinstance(LOW_STEALTH, int)
assert isinstance(MEDIUM_STEALTH, int)
assert isinstance(HIGH_STEALTH, int)

BAD_STEALTH = [
    EffectsGroup(
        description="BAD_STEALTH_DESC",
        scope=IsSource | (OnPlanet(id=Source.ID) & ~HasTag(name="ORBITAL")),
        activation=Planet(),
        stackinggroup="SPECIES_STEALTH_STACK",
        effects=SetStealth(value=Value - LOW_STEALTH),
    )
]

GOOD_STEALTH = [
    EffectsGroup(
        description="GOOD_STEALTH_DESC",
        scope=IsSource | (OnPlanet(id=Source.ID) & ~HasTag(name="ORBITAL")),
        activation=Planet(),
        stackinggroup="SPECIES_STEALTH_STACK",
        effects=SetStealth(value=Value + LOW_STEALTH),
    )
]


GREAT_STEALTH = [
    EffectsGroup(
        description="GREAT_STEALTH_DESC",
        scope=IsSource | (OnPlanet(id=Source.ID) & ~HasTag(name="ORBITAL")),
        activation=Planet(),
        stackinggroup="SPECIES_STEALTH_STACK",
        effects=SetStealth(value=Value + MEDIUM_STEALTH),
    )
]


ULTIMATE_STEALTH = [
    EffectsGroup(
        description="ULTIMATE_STEALTH_DESC",
        scope=IsSource | (OnPlanet(id=Source.ID) & ~HasTag(name="ORBITAL")),
        activation=Planet(),
        stackinggroup="SPECIES_STEALTH_STACK",
        effects=SetStealth(value=Value + HIGH_STEALTH),
    )
]
