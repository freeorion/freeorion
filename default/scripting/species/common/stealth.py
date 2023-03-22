from common.stealth import LOW_STEALTH

GOOD_STEALTH = EffectsGroup(
    description="GOOD_STEALTH_DESC",
    scope=IsSource | (OnPlanet(id=Source.ID) & ~HasTag(name="ORBITAL")),
    activation=Planet(),
    stackinggroup="SPECIES_STEALTH_STACK",
    effects=SetStealth(value=Value + LOW_STEALTH),
)
