from common.misc import SHIP_WEAPON_DAMAGE_FACTOR
from techs.techs import (
    ARBITRARY_BIG_NUMBER_FOR_METER_TOPUP,
    EMPIRE_OWNED_SHIP_WITH_PART,
    SHIP_PART_UPGRADE_RESUPPLY_CHECK,
)


# @1@ part name
def WEAPON_BASE_EFFECTS(part_name: str):
    # these NamedReals need to be parsed to be registered in the pedia. XXX remove this when the pedia can look up ship meters/part capacity
    NamedReal(name=part_name + "_PART_CAPACITY", value=PartCapacity(name=part_name))
    NamedReal(name=part_name + "_PART_SECONDARY_STAT", value=PartSecondaryStat(name=part_name))

    # The following is really only needed on the first resupplied turn after an upgrade is researched, since the resupply currently
    # takes place in a portion of the turn before meters are updated, but currently there is no good way to restrict this to
    # only that first resupply (and it is simply mildly inefficient to repeat the topup later).
    topup_effects_group = EffectsGroup(
        scope=EMPIRE_OWNED_SHIP_WITH_PART(part_name) & Turn(high=LocalCandidate.LastTurnResupplied),
        accountinglabel=part_name,
        effects=SetCapacity(partname=part_name, value=Value + ARBITRARY_BIG_NUMBER_FOR_METER_TOPUP),
    )
    return [topup_effects_group]


# @1@ part name
# @2@ value added to max capacity
def WEAPON_UPGRADE_CAPACITY_EFFECTS(tech_name: str, part_name: str, value: int, upgraded_damage_override: int = -1):
    # the following recursive lookup works, but is not acceptable because of delays. as long as the parser is sequential, the parallel waiting feature is kind of a bug
    # previous_upgrade_effect = PartCapacity(name=part_name) if (tech_name[-1] == "2") else NamedRealLookup(name = tech_name[0:-1] + "2_UPGRADED_DAMAGE")  # + str(int(tech_name[-1]) - 1))
    upgraded_damage = upgraded_damage_override if upgraded_damage_override != -1 else value * (int(tech_name[-1]) - 1)
    return [
        EffectsGroup(
            scope=EMPIRE_OWNED_SHIP_WITH_PART(part_name) & SHIP_PART_UPGRADE_RESUPPLY_CHECK(CurrentContent),
            accountinglabel=part_name,
            effects=SetMaxCapacity(partname=part_name, value=Value + value * SHIP_WEAPON_DAMAGE_FACTOR),
        ),
        # Inform the researching empire that ships in supply will get upgraded before next combat
        EffectsGroup(
            scope=IsSource,
            activation=(CurrentTurn == TurnTechResearched(empire=Source.Owner, name=CurrentContent)),
            effects=GenerateSitRepMessage(
                message="SITREP_WEAPONS_UPGRADED",
                label="SITREP_WEAPONS_UPGRADED_LABEL",
                icon="icons/sitrep/weapon_upgrade.png",
                parameters={
                    "empire": Source.Owner,
                    "shippart": part_name,
                    "tech": CurrentContent,
                    # str(CurrentContent) -> <ValueRefString object at 0x...>
                    "dam": NamedReal(name=tech_name + "_UPGRADE_DAMAGE", value=value * SHIP_WEAPON_DAMAGE_FACTOR),
                    "sum": NamedReal(
                        name=tech_name + "_UPGRADED_DAMAGE",
                        value=PartCapacity(name=part_name) + (SHIP_WEAPON_DAMAGE_FACTOR * upgraded_damage),
                    ),
                },
                empire=Target.Owner,
            ),
        ),
    ]
