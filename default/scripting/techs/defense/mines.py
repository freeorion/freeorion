# EffectsGroups for System Defense Mines
# arg1 total damage
# arg2 priority
# arg3 scope of systems effected: SOURCE, EMPIRE
#
# This macro handles multiple entries, where only the earliest applicable effects a target.
#   arg2/priority: the value should be unique from any other uses of EG_SYSTEM_MINES
#   arg3/scope: applies to either the source system only, or to all systems with an empire owned planet.
# Initial priority order of entries (earliest first):
#   SYS_DEF_MINES_3
#   SYS_DEF_MINES_2
#   FORTRESS_SPECIAL
#   SYS_DEF_MINES_1

from focs._effects import (
    ContainedBy,
    Contains,
    Destroy,
    EffectsGroup,
    EnemyOf,
    Fleet,
    GenerateSitRepMessage,
    HasTag,
    InSystem,
    OwnedBy,
    Planet,
    SetStructure,
    Ship,
    Source,
    Structure,
    System,
    Target,
    Unowned,
    Value,
    VisibleToEmpire,
)


def EG_SYSTEM_MINES(total_damage, priority, scope: str):
    return [
        # Empire owned planet, damage enemy ship; sitreps issued below
        EffectsGroup(
            scope=Ship
            & SYSTEM_MINES_SCOPE(scope)
            & (OwnedBy(affiliation=EnemyOf, empire=Source.Owner) | Unowned)
            & Structure(low=total_damage + 0.0001),
            activation=~Unowned,
            stackinggroup="DEF_MINES_DAMAGE_STACK",
            priority=priority,
            effects=[SetStructure(value=Value - total_damage)],
        ),
        # Unowned planet, damage non-friendly ship; sitreps issued below
        EffectsGroup(
            scope=Ship
            & SYSTEM_MINES_SCOPE(scope)
            & (~Unowned | (Unowned & (~HasTag(name="UNOWNED_FRIENDLY"))))
            & Structure(low=total_damage + 0.0001),
            activation=Unowned,
            stackinggroup="DEF_MINES_DAMAGE_STACK",
            priority=priority,
            effects=[SetStructure(value=Value - total_damage)],
        ),
        # Empire owned planet, destroy enemy ship; sitreps issued below
        EffectsGroup(
            scope=Ship
            & SYSTEM_MINES_SCOPE(scope)
            & (OwnedBy(affiliation=EnemyOf, empire=Source.Owner) | Unowned)
            & Structure(high=total_damage),
            activation=~Unowned,
            stackinggroup="DEF_MINES_DAMAGE_STACK",
            priority=priority,
            effects=[Destroy],
        ),
        # Unowned planet, destroy non-friendly ship; sitreps issued below
        EffectsGroup(
            scope=Ship
            & SYSTEM_MINES_SCOPE(scope)
            & (~Unowned | (Unowned & (~HasTag(name="UNOWNED_FRIENDLY"))))
            & Structure(high=total_damage),
            activation=Unowned,
            stackinggroup="DEF_MINES_DAMAGE_STACK",
            priority=priority,
            effects=[Destroy],
        ),
        # Sitreps
        # Planet owner sitrep - visible enemy fleet damaged
        EffectsGroup(
            scope=Fleet
            & SYSTEM_MINES_SCOPE(scope)
            & OwnedBy(affiliation=EnemyOf, empire=Source.Owner)
            & VisibleToEmpire(empire=Source.Owner),
            activation=~Unowned,
            stackinggroup="DEF_MINES_SITREP_PLANET_VISIBLE_ENEMY_STACK",
            priority=priority,
            effects=[
                GenerateSitRepMessage(
                    message="EFFECT_MINES",
                    label="EFFECT_MINES_LABEL",
                    icon="icons/sitrep/combat_damage.png",
                    parameters={
                        "empire": Target.Owner,
                        "fleet": Target.ID,
                        "rawtext": total_damage,
                        "system": Target.SystemID,
                    },
                    empire=Source.Owner,
                )
            ],
        ),
        # Planet owner sitrep - visible unowned ship damaged
        EffectsGroup(
            scope=Ship
            & SYSTEM_MINES_SCOPE(scope)
            & Unowned
            & VisibleToEmpire(empire=Source.Owner)
            & Structure(low=total_damage + 0.0001),
            activation=~Unowned,
            stackinggroup="DEF_MINES_SITREP_PLANET_VISIBLE_UNOWNED_STACK",
            priority=priority,
            effects=[
                GenerateSitRepMessage(
                    message="EFFECT_MINES_SINGLE_SHIP",
                    label="EFFECT_MINES_LABEL",
                    icon="icons/sitrep/combat_damage.png",
                    parameters={
                        "ship": Target.ID,
                        "shipdesign": Target.DesignID,
                        "rawtext": total_damage,
                        "system": Target.SystemID,
                    },
                    empire=Source.Owner,
                )
            ],
        ),
        # Planet owner sitrep - visible unowned ship destroyed
        EffectsGroup(
            scope=Ship
            & SYSTEM_MINES_SCOPE(scope)
            & Unowned
            & VisibleToEmpire(empire=Source.Owner)
            & Structure(high=total_damage),
            activation=~Unowned,
            stackinggroup="DEF_MINES_SITREP_PLANET_VISIBLE_UNOWNED_STACK",
            priority=priority,
            effects=[
                GenerateSitRepMessage(
                    message="EFFECT_MINES_UNOWNED_DESTROYED",
                    label="EFFECT_MINES_SHIP_DESTROYED_LABEL",
                    icon="icons/sitrep/combat_damage.png",
                    parameters={"ship": Target.ID, "shipdesign": Target.DesignID, "system": Target.SystemID},
                    empire=Source.Owner,
                )
            ],
        ),
        # Planet owner sitrep - visible enemy ship destroyed
        EffectsGroup(
            scope=Ship
            & SYSTEM_MINES_SCOPE(scope)
            & OwnedBy(affiliation=EnemyOf, empire=Source.Owner)
            & Structure(high=total_damage)
            & VisibleToEmpire(empire=Source.Owner),
            activation=~Unowned,
            stackinggroup="DEF_MINES_SITREP_PLANET_VISIBLE_ENEMY_STACK",
            priority=priority,
            effects=[
                GenerateSitRepMessage(
                    message="EFFECT_MINES_SHIP_DESTROYED",
                    label="EFFECT_MINES_SHIP_DESTROYED_LABEL",
                    icon="icons/sitrep/combat_destroyed.png",
                    parameters={"empire": Target.Owner, "ship": Target.ID, "system": Target.SystemID},
                    empire=Source.Owner,
                )
            ],
        ),
        # Planet owner sitrep - non-visible ship damaged or destroyed
        EffectsGroup(
            scope=Ship
            & SYSTEM_MINES_SCOPE(scope)
            & (OwnedBy(affiliation=EnemyOf, empire=Source.Owner) | Unowned)
            & (~VisibleToEmpire(empire=Source.Owner)),
            activation=~Unowned,
            stackinggroup="DEF_MINES_SITREP_PLANET_INVISIBLE_STACK",
            priority=priority,
            effects=[
                GenerateSitRepMessage(
                    message="EFFECT_MINES_UNKNOWN",
                    label="EFFECT_MINES_UNKNOWN_LABEL",
                    icon="icons/sitrep/combat_damage.png",
                    parameters={"system": Target.SystemID},
                    empire=Source.Owner,
                )
            ],
        ),
        # Fleet owner sitrep - damaged fleet
        EffectsGroup(
            scope=Fleet & SYSTEM_MINES_SCOPE(scope) & (~Unowned) & OwnedBy(affiliation=EnemyOf, empire=Source.Owner),
            stackinggroup="DEF_MINES_SITREP_FLEET_STACK",
            priority=priority,
            effects=[
                GenerateSitRepMessage(
                    message="EFFECT_MINES",
                    label="EFFECT_MINES_LABEL",
                    icon="icons/sitrep/combat_damage.png",
                    parameters={
                        "empire": Target.Owner,
                        "fleet": Target.ID,
                        "rawtext": total_damage,
                        "system": Target.SystemID,
                    },
                    empire=Target.Owner,
                )
            ],
        ),
        # Ship owner sitrep - destroyed ship
        EffectsGroup(
            scope=Ship
            & SYSTEM_MINES_SCOPE(scope)
            & (~Unowned)
            & OwnedBy(affiliation=EnemyOf, empire=Source.Owner)
            & Structure(high=total_damage),
            stackinggroup="DEF_MINES_SITREP_SHIP_STACK",
            priority=priority,
            effects=[
                GenerateSitRepMessage(
                    message="EFFECT_MINES_SHIP_DESTROYED",
                    label="EFFECT_MINES_SHIP_DESTROYED_LABEL",
                    icon="icons/sitrep/combat_destroyed.png",
                    parameters={"empire": Target.Owner, "ship": Target.ID, "system": Target.SystemID},
                    empire=Target.Owner,
                )
            ],
        ),
    ]


def SYSTEM_MINES_SCOPE(scope: str):
    if scope == "SOURCE":
        return InSystem(id=Source.SystemID)
    elif scope == "EMPIRE":
        return ContainedBy(System & Contains(Planet() & OwnedBy(empire=Source.Owner)))
    else:
        raise Exception("Unknown mines scope %s" % scope)
