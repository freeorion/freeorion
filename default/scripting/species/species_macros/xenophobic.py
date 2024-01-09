from focs._effects import (
    Contains,
    EffectsGroup,
    EmpireHasAdoptedPolicy,
    GiveEmpireTech,
    Good,
    HasSpecies,
    HasTag,
    IsBuilding,
    IsSource,
    IsTarget,
    JumpsBetween,
    LocalCandidate,
    MaxOf,
    NamedIntegerLookup,
    NamedReal,
    NamedRealLookup,
    Number,
    OwnedBy,
    Planet,
    RootCandidate,
    SetTargetHappiness,
    SetTargetPopulation,
    Source,
    Statistic,
    StatisticCount,
    Sum,
    Target,
    Turn,
    Value,
    VisibleToEmpire,
    WithinStarlaneJumps,
)
from macros.priorities import TARGET_AFTER_SCALING_PRIORITY, TARGET_POPULATION_LAST_BEFORE_OVERRIDE_PRIORITY


def _condition_other_species_nearby():
    return (
        Planet()
        & HasSpecies()
        & ~HasSpecies(name=[Source.Species])
        & ~HasTag(name="LOW_BRAINPOWER")
        & ~Contains(IsBuilding(name=["BLD_CONC_CAMP"]))
        & VisibleToEmpire(empire=Source.Owner)
        & (~OwnedBy(empire=Source.Owner) | ~EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_RACIAL_PURITY"))
        & WithinStarlaneJumps(jumps=NamedIntegerLookup(name="XENOPHOBIC_MAX_JUMPS"), condition=IsSource)
    )


def _xenophobic_selfsustaining_qualifying_planet_count():
    return StatisticCount(
        int,
        condition=Planet()
        & HasSpecies()
        & ~(HasSpecies(name=[Source.Species]) | HasTag(name="LOW_BRAINPOWER"))
        & WithinStarlaneJumps(jumps=NamedIntegerLookup(name="XENOPHOBIC_MAX_JUMPS"), condition=IsSource),
    )


XENOPHOBIC_SELF = [
    # Give the concentration camp tech at game start
    EffectsGroup(
        scope=IsSource,
        activation=Turn(high=0),
        effects=GiveEmpireTech(name="CON_CONC_CAMP"),
    ),
    # Xenophobic Frenzy: stability malus for xenophobic species nearby other species
    EffectsGroup(
        scope=IsSource,
        activation=Number(low=1, condition=_condition_other_species_nearby()),
        stackinggroup="XENOPHOBIC_LABEL_SELF",
        accountinglabel="XENOPHOBIC_LABEL_SELF",
        priority=TARGET_AFTER_SCALING_PRIORITY,
        effects=(
            SetTargetHappiness(
                value=Value
                + NamedReal(name="XENOPHOBIC_TARGET_STABILITY_PERJUMP", value=-0.5)
                * Statistic(
                    float,
                    Sum,
                    value=1
                    + NamedIntegerLookup(name="XENOPHOBIC_MAX_JUMPS")
                    - JumpsBetween(Target.ID, LocalCandidate.ID),
                    condition=Planet()
                    & VisibleToEmpire(empire=Source.Owner)
                    & HasSpecies()
                    & ~HasSpecies(name=[Source.Species])
                    & ~HasTag(name="LOW_BRAINPOWER")
                    & ~Contains(IsBuilding(name=["BLD_CONC_CAMP"]))
                    & (
                        ~OwnedBy(empire=RootCandidate.Owner)
                        | ~EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_RACIAL_PURITY")
                    )
                    & WithinStarlaneJumps(jumps=NamedIntegerLookup(name="XENOPHOBIC_MAX_JUMPS"), condition=IsSource),
                )
            )
        ),
    ),
    # Pop malus for self-sustaining xenophobic species based on the number of nearby planets with other species
    EffectsGroup(
        scope=IsSource,
        activation=Planet(environment=[Good])
        & HasTag(name="SELF_SUSTAINING")
        & (0 < _xenophobic_selfsustaining_qualifying_planet_count()),
        stackinggroup="XENOPHOBIC_POP_SELF",
        accountinglabel="XENOPHOBIC_LABEL_SELF",
        priority=TARGET_POPULATION_LAST_BEFORE_OVERRIDE_PRIORITY,
        effects=SetTargetPopulation(
            value=Value
            + MaxOf(
                float,
                -3 * Target.HabitableSize,  # Cap malus at the self-sustaining bonus
                Target.HabitableSize
                * NamedReal(name="XENOPHOBIC_TARGET_POPULATION_PERJUMP", value=-0.2)
                * Statistic(
                    float,
                    Sum,
                    value=1
                    + NamedIntegerLookup(name="XENOPHOBIC_MAX_JUMPS")
                    - JumpsBetween(Target.ID, LocalCandidate.ID),
                    condition=Planet()
                    & HasSpecies()
                    & ~HasSpecies(name=[Source.Species])
                    & ~HasTag(name="LOW_BRAINPOWER")
                    & WithinStarlaneJumps(jumps=NamedIntegerLookup(name="XENOPHOBIC_MAX_JUMPS"), condition=IsSource),
                ),
            )
        ),
    ),
]


# single argument should be the name of the species capitalized
def XENOPHOBIC_OTHER(name):
    # Xenophobic Harassment: stability malus to non-exobot species nearby different, xenophobic species
    return EffectsGroup(
        scope=_condition_other_species_nearby(),
        activation=Planet(),
        stackinggroup=f"XENOPHOBIC_LABEL_{name}_OTHER",
        accountinglabel=f"XENOPHOBIC_LABEL_{name}_OTHER",
        priority=TARGET_AFTER_SCALING_PRIORITY,
        effects=SetTargetHappiness(
            value=Value
            + NamedRealLookup(name="XENOPHOBIC_TARGET_STABILITY_PERJUMP")
            * Statistic(
                float,
                Sum,
                value=1 + NamedIntegerLookup(name="XENOPHOBIC_MAX_JUMPS") - JumpsBetween(Target.ID, LocalCandidate.ID),
                condition=Planet()
                & HasSpecies(name=[Source.Species])
                & (
                    ~OwnedBy(empire=Target.Owner)
                    | ~EmpireHasAdoptedPolicy(empire=LocalCandidate.Owner, name="PLC_RACIAL_PURITY")
                )
                & WithinStarlaneJumps(jumps=NamedIntegerLookup(name="XENOPHOBIC_MAX_JUMPS"), condition=IsTarget),
            )
        ),
    )
