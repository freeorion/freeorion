def FIXED_OPINION_EFFECTS(name: str, value):
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
                opinion=Value + GameRule(type=float, name="RULE_BASELINE_PLANET_STABILITY"),
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
