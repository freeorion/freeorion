from common.misc import SHIP_WEAPON_DAMAGE_FACTOR
from techs.techs import (
    ARBITRARY_BIG_NUMBER_FOR_METER_TOPUP,
    EMPIRE_OWNED_SHIP_WITH_PART,
    SHIP_PART_UPGRADE_RESUPPLY_CHECK,
)


# @1@ part name
def WEAPON_BASE_EFFECTS(part_name: str):
    return [
        EffectsGroup(
            scope=EMPIRE_OWNED_SHIP_WITH_PART(part_name),
            accountinglabel=part_name,
            effects=[
                SetMaxCapacity(partname=part_name, value=Value + PartCapacity(name=part_name)),
                SetMaxSecondaryStat(partname=part_name, value=Value + PartSecondaryStat(name=part_name)),
            ],
        ),
        # The following is really only needed on the first resupplied turn after an upgrade is researched, since the resupply currently
        # takes place in a portion of the turn before meters are updated, but currently there is no good way to restrict this to
        # only that first resupply (and it is simply mildly inefficient to repeat the topup later).
        EffectsGroup(
            scope=EMPIRE_OWNED_SHIP_WITH_PART(part_name) & Turn(high=LocalCandidate.LastTurnResupplied),
            accountinglabel=part_name,
            effects=SetCapacity(partname=part_name, value=Value + ARBITRARY_BIG_NUMBER_FOR_METER_TOPUP),
        ),
    ]


# @1@ part name
# @2@ value added to max capacity
def WEAPON_UPGRADE_CAPACITY_EFFECTS(part_name: str, value: int):
    return [
        EffectsGroup(
            scope=EMPIRE_OWNED_SHIP_WITH_PART(part_name) & SHIP_PART_UPGRADE_RESUPPLY_CHECK(CurrentContent),
            accountinglabel=part_name,
            effects=SetMaxCapacity(partname=part_name, value=Value + value * SHIP_WEAPON_DAMAGE_FACTOR),
        ),
        # Inform the researching empire that ships in supply will get upgraded before next combat
        EffectsGroup(
            scope=Source,
            activation=(CurrentTurn == TurnTechResearched(empire=Source.Owner, name=CurrentContent)),
            effects=GenerateSitRepMessage(
                message="SITREP_WEAPONS_UPGRADED",
                label="SITREP_WEAPONS_UPGRADED_LABEL",
                icon="icons/sitrep/weapon_upgrade.png",
                parameters={
                    "empire": Source.Owner,
                    "shippart": part_name,
                    "tech": CurrentContent,
                    "dam": value * SHIP_WEAPON_DAMAGE_FACTOR,
                },
                empire=Target.Owner,
            ),
        ),
    ]
