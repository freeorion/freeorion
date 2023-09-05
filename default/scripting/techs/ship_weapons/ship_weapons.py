from common.misc import SHIP_WEAPON_DAMAGE_FACTOR
from common.priorities import AFTER_ALL_TARGET_MAX_METERS_PRIORITY
from focs._effects import (
    CurrentContent,
    CurrentTurn,
    EffectsGroup,
    GenerateSitRepMessage,
    IsSource,
    NamedReal,
    PartCapacity,
    PartSecondaryStat,
    SetCapacity,
    SetMaxCapacity,
    SetSecondaryStat,
    Source,
    Target,
    TurnTechResearched,
    Value,
)
from techs.techs import (
    ARBITRARY_BIG_NUMBER_FOR_METER_TOPUP,
    EMPIRE_OWNED_SHIP_WITH_PART,
    SHIP_PART_UPGRADE_RESUPPLY_CHECK,
)

# Tech based setting of damage and number of shots of a weapon. Registers named reals for use in the pedia.
# Be careful the effect does not show in the ship damage/shot estimates in the ship designer UI if the tech is not researched yet.
# @1@ part name


def WEAPON_BASE_EFFECTS(part_name: str):
    # these NamedReals need to be parsed to be registered in the pedia. XXX remove this when the pedia can look up ship meters/part capacity
    NamedReal(name=f"{part_name}_PART_CAPACITY", value=PartCapacity(name=part_name))
    NamedReal(name=f"{part_name}_PART_SECONDARY_STAT", value=PartSecondaryStat(name=part_name))

    # Set the damage and shot meters to the max values every turn.
    # The topup_effects_group is necessary..
    # 1) the first turn after an upgrade is researched, since the resupply currently takes place in a portion of the turn before meters are updated
    # 2) for any arbitrary "temporary" change of Max values (e.g. PLC_FLANKING); "permanent" changes could be modeled differently
    topup_effects_group = EffectsGroup(
        scope=EMPIRE_OWNED_SHIP_WITH_PART(part_name),
        accountinglabel=part_name,
        priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
        effects=[
            SetCapacity(partname=part_name, value=Value + ARBITRARY_BIG_NUMBER_FOR_METER_TOPUP),
            SetSecondaryStat(partname=part_name, value=Value + ARBITRARY_BIG_NUMBER_FOR_METER_TOPUP),
        ],
    )
    return [topup_effects_group]


# Tech Upgrade a direct weapon damage each turn if it was resupplied after the tech was researched.
# Be careful this does show in the ship damage/shot estimates in the ship designer UI if the tech is not researched.
# This can correctly estimate the total sum of all related weapon techs up to this tech if
# * following the tech naming scheme TECH_NAME_X with X being the level of the upgrade (and X being in 2..9)
# * and all tech level upgrade give the same amount of upgrade ( so the total sum is X * unscaled_upgrade)
# @1@ tech name
# @2@ part name
# @3@ unscaled_upgrade gets added to max capacity after scaling it with SHIP_WEAPON_DAMAGE_FACTOR
# @4@ upgraded_damage_override if given overrides the total sum over the researched techs shown in the upgrade info sitrep
def WEAPON_UPGRADE_CAPACITY_EFFECTS(
    tech_name: str, part_name: str, unscaled_upgrade: int, upgraded_damage_override: int = -1
):
    # the following recursive lookup works, but is not acceptable because of delays. as long as the parser is sequential, the parallel waiting feature is kind of a bug
    # previous_upgrade_effect = PartCapacity(name=part_name) if (tech_name[-1] == "2") else NamedRealLookup(name = tech_name[0:-1] + "2_UPGRADED_DAMAGE")  # + str(int(tech_name[-1]) - 1))
    upgraded_damage = (
        upgraded_damage_override if upgraded_damage_override != -1 else unscaled_upgrade * (int(tech_name[-1]) - 1)
    )
    return [
        EffectsGroup(
            scope=EMPIRE_OWNED_SHIP_WITH_PART(part_name) & SHIP_PART_UPGRADE_RESUPPLY_CHECK(CurrentContent),
            accountinglabel=part_name,
            effects=SetMaxCapacity(partname=part_name, value=Value + unscaled_upgrade * SHIP_WEAPON_DAMAGE_FACTOR),
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
                    "dam": NamedReal(
                        name=tech_name + "_UPGRADE_DAMAGE", value=unscaled_upgrade * SHIP_WEAPON_DAMAGE_FACTOR
                    ),
                    "sum": NamedReal(
                        name=tech_name + "_UPGRADED_DAMAGE",
                        value=PartCapacity(name=part_name) + (SHIP_WEAPON_DAMAGE_FACTOR * upgraded_damage),
                    ),
                },
                empire=Target.Owner,
            ),
        ),
    ]
