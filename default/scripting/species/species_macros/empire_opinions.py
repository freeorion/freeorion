from focs._effects import (
    Capital,
    EffectsGroup,
    GameRule,
    HasSpecies,
    LocalCandidate,
    MaxOf,
    MinOf,
    Planet,
    Population,
    SetSpeciesOpinion,
    SetSpeciesTargetOpinion,
    SpeciesEmpireTargetOpinion,
    SpeciesShipsDestroyed,
    SpeciesShipsLost,
    StatisticCount,
    Target,
    ThisSpecies,
    Value,
)


def FIXED_OPINION_EFFECTS(name: str, value: float):
    return EffectsGroup(
        scope=Capital,
        stackinggroup="%s_FIXED_OPINION_OF_EMPIRE" % name,
        effects=[
            SetSpeciesTargetOpinion(species=ThisSpecies, empire=Target.Owner, opinion=value),
            SetSpeciesOpinion(species=ThisSpecies, empire=Target.Owner, opinion=value),
        ],
    )


def COMMON_OPINION_EFFECTS(name: str):
    return EffectsGroup(
        scope=Capital,
        stackinggroup="%s_TOWARDS_TARGET_OPINION_OF_EMPIRE" % name,
        effects=[
            SetSpeciesTargetOpinion(  # baseline opinion
                species=ThisSpecies,
                empire=Target.Owner,
                opinion=Value
                + GameRule(type=float, name="RULE_BASELINE_SPECIES_EMPIRE_OPINION")
                + (
                    GameRule(type=float, name="RULE_INVASION_OPINION_PENALTY_SCALING")
                    * StatisticCount(
                        float,
                        condition=Planet()
                        & HasSpecies(name=[ThisSpecies])
                        & (LocalCandidate.LastInvadedByEmpire == Target.Owner),
                    )
                    ** 0.5
                )
                + (
                    GameRule(type=float, name="RULE_SHIPS_LOST_DESTROYED_PENALTY_SCALING")
                    * (
                        SpeciesShipsDestroyed(empire=Target.Owner, name=ThisSpecies)
                        + SpeciesShipsLost(empire=Target.Owner, name=ThisSpecies)
                    )
                    ** 0.5
                )
                + (
                    GameRule(type=float, name="RULE_COLONIES_FOUNDED_BONUS_SCALING")
                    * StatisticCount(
                        float,
                        condition=Planet()
                        & HasSpecies(name=[ThisSpecies])
                        & Population(low=0.001)
                        & (LocalCandidate.LastColonizedByEmpire == Target.Owner),
                    )
                ),
            ),
            SetSpeciesOpinion(
                species=ThisSpecies,
                empire=Target.Owner,
                opinion=Value
                + MinOf(
                    float,
                    1.0,  # increase by 1 per turn up towards target
                    MaxOf(
                        float,
                        -1.0,  # decrease by 1 per turn down towards target
                        SpeciesEmpireTargetOpinion(species=ThisSpecies, empire=Target.Owner) - Value,
                    ),
                ),
            ),
        ],
    )
