from focs._effects import (
    CurrentContent,
    CurrentTurn,
    EffectsGroup,
    GenerateSitRepMessage,
    IsSource,
    LocalCandidate,
    Max,
    MaxSecondaryStat,
    NamedReal,
    OwnedBy,
    PartCapacity,
    PartSecondaryStat,
    SetCapacity,
    SetMaxCapacity,
    SetMaxSecondaryStat,
    SetSecondaryStat,
    Ship,
    ShipPartMeter,
    Source,
    Statistic,
    Target,
    TurnTechResearched,
    Value,
)
from macros.misc import (
    FIGHTER_DAMAGE_FACTOR,
    SHIP_WEAPON_DAMAGE_FACTOR,
)
from macros.priorities import AFTER_ALL_TARGET_MAX_METERS_PRIORITY
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


# Tech Upgrade a direct weapon number of shots each turn if it was resupplied after the tech was researched.
# Be careful this does show in the ship damage/shot estimates in the ship designer UI if the tech is not researched.
# @1@ tech name
# @2@ part name
# @3@ extra_shots gets added to max secondary stat
def WEAPON_UPGRADE_SECONDARY_STAT_EFFECTS(tech_name: str, part_name: str, extra_shots: int):
    return [
        EffectsGroup(
            scope=EMPIRE_OWNED_SHIP_WITH_PART(part_name) & SHIP_PART_UPGRADE_RESUPPLY_CHECK(CurrentContent),
            accountinglabel=part_name,
            effects=SetMaxSecondaryStat(partname=part_name, value=Value + extra_shots),
        ),
        # Inform the researching empire that ships in supply will get upgraded before next combat
        # TODO use conditional If effect to print a different message using a different shotsum e.g. "?" when there are no ships built yet
        #      or do value_ref<string> ops (which does not exist yet AFAIK)
        #         ("?" * StatisticIf(float, ~( Ship & OwnedBy(empire = Source.Owner) & DesignHasPart(part_name) ) )) + Statistic(...
        EffectsGroup(
            scope=IsSource,
            activation=(CurrentTurn == TurnTechResearched(empire=Source.Owner, name=CurrentContent)),
            effects=GenerateSitRepMessage(
                message="SITREP_WEAPON_SHOTS_UPGRADED",
                label="SITREP_WEAPON_SHOTS_UPGRADED_LABEL",
                icon="icons/sitrep/weapon_upgrade.png",
                parameters={
                    "empire": Source.Owner,
                    "shippart": part_name,
                    "tech": CurrentContent,
                    "shots": NamedReal(
                        name=tech_name + "_UPGRADE_SHOTS",
                        value=extra_shots,
                    ),
                    # a part value is not changeable. In the backend we also do not know which techs apply to a part
                    # options. Also note we cant register this as a named value ref as the value changes.
                    # 0) count the bonus in an empire meter
                    # 1) register all relevant tech names before calling this; and check here if those are researched
                    #    a named value ref to a list of strings could help for lazy eval
                    # 2) trying to get the new value from the highest known one - note this only works if a ship with the part is built
                    # 3) forget about it
                    "shotsum": extra_shots
                    + Statistic(
                        float,
                        Max,
                        value=ShipPartMeter(part=part_name, meter=MaxSecondaryStat, ship=LocalCandidate.ID),
                        condition=Ship & OwnedBy(empire=Source.Owner),
                    ),
                },
                empire=Target.Owner,
            ),
        ),
    ]


# Tech Upgrade SetMaxSecondaryStat effect for hangar part damage
# @1@ tech name
# @2@ part name
# @3@ base_damage_bonus scaled by figher damage gets added to max secondary stat
def HANGAR_UPGRADE_SECONDARY_STAT_EFFECT(tech_name: str, part_name: str, base_damage_bonus: int):
    return SetMaxSecondaryStat(
        partname=part_name,
        value=Value
        + NamedReal(
            name=tech_name + "_" + part_name + "_DAMAGE_BONUS", value=base_damage_bonus * FIGHTER_DAMAGE_FACTOR
        ),
    )
