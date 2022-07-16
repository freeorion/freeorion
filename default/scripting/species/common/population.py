GROWTH_RATE_FACTOR = (
    0.1
    * (
        1 - StatisticIf(float, condition=Target & EmpireHasAdoptedPolicy(name="PLC_NO_GROWTH"))
    )  # no growth with no-growth policy
    * (
        1 + 0.5 * StatisticIf(float, condition=Target & EmpireHasAdoptedPolicy(name="PLC_POPULATION"))
    )  # +50% growth with population policy
    * (
        1
        + StatisticIf(float, condition=Target & EmpireHasAdoptedPolicy(name="PLC_AUGMENTATION"))
        # slower growth with augmentation on low-infrastructure planets
        * MinOf(
            float,
            1.2,
            MaxOf(
                float,
                0.5,
                Target.Construction / NamedRealLookup(name="AUGMENTATION_FULL_GROWTH_INFRASTRUCTURE_REQUIREMENT"),
            ),
        )
    )
)

#  BASIC_POPULATION
# '''
#         [[HOMEWORLD_BONUS_POPULATION]]
#         [[ENVIRONMENT_MODIFIER]]
#         [[SELF_SUSTAINING_BONUS]]
#         [[PHOTOTROPHIC_BONUS]]
#         [[GASEOUS_BONUS]]
#         [[HOMEWORLD_GROWTH_FOCUS_BOOST]]


#         // population growth or decay towards to target population
#         EffectsGroup
#             scope = Source
#             activation = And [
#                 Planet
#                 (LocalCandidate.LastTurnConquered < CurrentTurn)
#             ]
#             priority = [[POPULATION_FIRST_PRIORITY]]
#             effects = SetPopulation value =
#                 // growth is GROWTH_RATE_FACTOR * population * (maximum + 1 - population) / maximum
#                 // which gives factor for population 1, best growth around half full and aproaching
#                 // factor again when almost full
#                 ((Value < Target.TargetPopulation) *
#                     min(Target.TargetPopulation, Value + Value*[[GROWTH_RATE_FACTOR]]*
#                                                                (1 + (1 - Value)/Target.TargetPopulation)))
#                 // shrink if overpopulated
#               + ((Value >= Target.TargetPopulation) *
#                     max(Target.TargetPopulation, Value - 0.1*(Value - Target.TargetPopulation)))

#         // Since all species have the same advanced focus effects and
#         // infrastructure, the macros are stashed here, where they don't
#         // need to be manually included in each species' macros.
#         [[FOCUS_CHANGE_PENALTY]]
#         [[ADVANCED_FOCUS_EFFECTS]]
#         [[STANDARD_CONSTRUCTION]]
#         [[STANDARD_METER_GROWTH]]
# '''

#  AVERAGE_POPULATION
# '''     [[BASIC_POPULATION]]
# '''

#  EXTREMELY_BAD_POPULATION
# '''     [[BASIC_POPULATION]]
#
#         EffectsGroup
#             description = "EXTREMELY_BAD_POPULATION_DESC"
#             scope = Source
#             activation = Planet
#             accountinglabel = "EXTREMELY_BAD_POPULATION_LABEL"
#             priority = [[TARGET_POPULATION_SCALING_PRIORITY]]
#             effects = SetTargetPopulation value = Value -0.75*abs(Value)
# '''

#  VERY_BAD_POPULATION
# '''     [[BASIC_POPULATION]]
#
#         EffectsGroup
#             description = "VERY_BAD_POPULATION_DESC"
#             scope = Source
#             activation = Planet
#             accountinglabel = "VERY_BAD_POPULATION_LABEL"
#             priority = [[TARGET_POPULATION_SCALING_PRIORITY]]
#             effects = SetTargetPopulation value = Value -0.5*abs(Value)
# '''

#  BAD_POPULATION
# '''     [[BASIC_POPULATION]]
#
#         EffectsGroup
#             description = "BAD_POPULATION_DESC"
#             scope = Source
#             activation = Planet
#             accountinglabel = "BAD_POPULATION_LABEL"
#             priority = [[TARGET_POPULATION_SCALING_PRIORITY]]
#             effects = SetTargetPopulation value = Value -0.25*abs(Value)
# '''

#  GOOD_POPULATION
# '''     [[BASIC_POPULATION]]

#         EffectsGroup
#             description = "GOOD_POPULATION_DESC"
#             scope = Source
#             activation = Planet
#             accountinglabel = "GOOD_POPULATION_LABEL"
#             priority = [[TARGET_POPULATION_SCALING_PRIORITY]]
#             effects = SetTargetPopulation value = Value +0.25*abs(Value)
# '''

#  // reduces max population in systems with bright stars
#  LIGHT_SENSITIVE
# '''
#         EffectsGroup
#             description = "LIGHT_SENSITIVE_DESC"
#             scope = Source
#             activation = And [
#                 Planet
#                 Star type = [Blue]
#             ]
#             accountinglabel = "VERY_BRIGHT_STAR"
#             priority = [[TARGET_POPULATION_AFTER_SCALING_PRIORITY]]
#             effects = SetTargetPopulation value = Value - 2 * Source.HabitableSize

#         EffectsGroup
#             scope = Source
#             activation = And [
#                 Planet
#                 Star type = [White]
#             ]
#             accountinglabel = "BRIGHT_STAR"
#             priority = [[TARGET_POPULATION_AFTER_SCALING_PRIORITY]]
#             effects = SetTargetPopulation value = Value - Source.HabitableSize
# '''

#  /*
#  * Implements environmental modifiers from Growth techs.
#  * Changes to the growth tree should take this into account.
#  */
#  ENVIRONMENT_MODIFIER
# '''     EffectsGroup
#             scope = Source
#             activation = And [
#                 Planet
#                 Planet environment = Uninhabitable
#             ]
#             accountinglabel = "UNINHABTIABLE_ENVIRONMENT_LABEL"
#             priority = [[TARGET_POPULATION_OVERRIDE_PRIORITY]]
#             effects = SetTargetPopulation value = -999

#         EffectsGroup
#             scope = Source
#             activation = And [
#                 Planet
#                 Planet environment = Hostile
#             ]
#             accountinglabel = "HOSTILE_ENVIRONMENT_LABEL"
#             priority = [[TARGET_POPULATION_BEFORE_SCALING_PRIORITY]]
#             effects = SetTargetPopulation value = Value - 4 * Source.HabitableSize

#         EffectsGroup
#             scope = Source
#             activation = And [
#                 Planet
#                 Planet environment = Poor
#             ]
#             accountinglabel = "POOR_ENVIRONMENT_LABEL"
#             priority = [[TARGET_POPULATION_BEFORE_SCALING_PRIORITY]]
#             effects = SetTargetPopulation value = Value - 2 * Source.HabitableSize

#  /*        EffectsGroup
#             scope = Source
#             activation = And [
#                 Planet
#                 Planet environment = Adequate
#             ]
#             accountinglabel = "ADEQUATE_ENVIRONMENT_LABEL"
#             priority = [[TARGET_POPULATION_BEFORE_SCALING_PRIORITY]]
#             effects = SetTargetPopulation value = Value + 0 * Source.HabitableSize
#  */

#         EffectsGroup
#             scope = Source
#             activation = Planet environment = Good
#             accountinglabel = "GOOD_ENVIRONMENT_LABEL"
#             priority = [[TARGET_POPULATION_BEFORE_SCALING_PRIORITY]]
#             effects = SetTargetPopulation value = Value + 3 * Target.HabitableSize
# '''

#  HOMEWORLD_BONUS_POPULATION
# '''     EffectsGroup
#             scope = And [
#                 Source
#                 Homeworld name = Source.Species
#             ]
#             activation = Planet
#             stackinggroup = "HOMEWORLD_STACK"
#             accountinglabel = "HOMEWORLD_BONUS"
#             priority = [[TARGET_POPULATION_AFTER_SCALING_PRIORITY]]
#             effects = SetTargetPopulation value = Value + 2 * Target.HabitableSize
# '''

#  HOMEWORLD_GROWTH_FOCUS_BOOST
# '''     EffectsGroup
#             scope = And [
#                 Planet
#                 OwnedBy empire = Source.Owner
#                 Species name = Source.Species
#                 Not Homeworld name = Source.Species
#                 ResourceSupplyConnected empire = Source.Owner condition = Source
#             ]
#             activation = And [
#                 Planet
#                 Focus type = "FOCUS_GROWTH"
#                 Homeworld
#             ]
#             stackinggroup = "HOMEWORLD_STACK"
#             accountinglabel = "HOMEWORLD_SUPPLY"
#             priority = [[TARGET_POPULATION_AFTER_SCALING_PRIORITY]]
#             effects = SetTargetPopulation value = Value + 1 * Target.HabitableSize
# '''

#  // This is dependent on current placement in population effects calc,
#  // just after Homeworld and Environment
#  SELF_SUSTAINING_BONUS
# '''     EffectsGroup
#             scope = Source
#             activation = And [
#                 Planet environment = Good
#                 HasTag name = "SELF_SUSTAINING"
#             ]
#             accountinglabel = "SELF_SUSTAINING_LABEL"
#             priority = [[TARGET_POPULATION_AFTER_SCALING_PRIORITY]]
#             effects = SetTargetPopulation value = Value + 3 * Target.HabitableSize  // Gets the same bonus as three growth specials
# '''

#  PHOTOTROPHIC_BONUS
# '''     EffectsGroup
#             scope = Contains Source
#             activation = And [
#                 Planet
#                 Or [
#                     OwnedBy affiliation = AnyEmpire
#                     Star type = [BlackHole NoStar] // Natives are allowed to have bright or dim stars
#                 ]
#                 HasTag name = "PHOTOTROPHIC"
#                 Turn high = 0
#                 Not Star type = [Orange]
#             ]
#             effects = SetStarType type = Yellow // start with a normal star to be balanced
#
#         EffectsGroup
#             scope = Source
#             activation = And [
#                 Planet
#                 HasTag name = "PHOTOTROPHIC"
#                 Star type = [Blue]
#                 TargetPopulation low = 0
#             ]
#             accountinglabel = "VERY_BRIGHT_STAR"
#             priority = [[TARGET_POPULATION_LAST_BEFORE_OVERRIDE_PRIORITY]]
#             effects = SetTargetPopulation value = Value + 3 * Source.HabitableSize

#         EffectsGroup
#             scope = Source
#             activation = And [
#                 Planet
#                 HasTag name = "PHOTOTROPHIC"
#                 Star type = [White]
#                 TargetPopulation low = 0
#             ]
#             accountinglabel = "BRIGHT_STAR"
#             priority = [[TARGET_POPULATION_LAST_BEFORE_OVERRIDE_PRIORITY]]
#             effects = SetTargetPopulation value = Value + 1.5 * Source.HabitableSize

#         EffectsGroup
#             scope = Source
#             activation = And [
#                 Planet
#                 HasTag name = "PHOTOTROPHIC"
#                 Star type = [Red Neutron]
#             ]
#             accountinglabel = "DIM_STAR"
#             priority = [[TARGET_POPULATION_LAST_BEFORE_OVERRIDE_PRIORITY]]
#             effects = SetTargetPopulation value = Value - 1 * Source.HabitableSize

#         EffectsGroup
#             scope = Source
#             activation = And [
#                 Planet
#                 HasTag name = "PHOTOTROPHIC"
#                 Star type = [BlackHole NoStar]
#             ]
#             accountinglabel = "NO_STAR"
#             priority = [[TARGET_POPULATION_LAST_BEFORE_OVERRIDE_PRIORITY]]
#             effects = SetTargetPopulation value = Value - 10 * Source.HabitableSize
# '''

#  GASEOUS_BONUS
# '''     EffectsGroup
#             scope = Source
#             activation = And [
#                 Planet type = GasGiant
#                 HasTag name = "GASEOUS"
#             ]
#             accountinglabel = "GASEOUS_LABEL"
#             priority = [[TARGET_POPULATION_SCALING_PRIORITY]]
#             effects = SetTargetPopulation value = Value - 0.5*abs(Value)
# '''

#  // @1@ Species key
#  LIFECYCLE_MANIP_POPULATION_EFFECTS
# '''
#         EffectsGroup
#             scope = And [
#                 Object id = Source.PlanetID
#                 Planet
#             ]
#             activation = And [
#                 Not OwnerHasTech name = "GRO_LIFECYCLE_MAN"
#                 Turn low = LocalCandidate.CreationTurn + 1 high = LocalCandidate.CreationTurn + 1
#             ]
#             priority = [[POPULATION_DEFAULT_PRIORITY]]
#             effects = [
#                 SetSpecies name = @1@
#                 SetPopulation value = 1
#             ]
#
#         EffectsGroup
#             scope = And [
#                 Object id = Source.PlanetID
#                 Planet
#             ]
#             activation = And [
#                 OwnerHasTech name = "GRO_LIFECYCLE_MAN"
#                 Turn low = LocalCandidate.CreationTurn + 1 high = LocalCandidate.CreationTurn + 1
#             ]
#             priority = [[POPULATION_DEFAULT_PRIORITY]]
#             effects = [
#                 SetSpecies name = @1@
#                 SetPopulation value = [[MIN_RECOLONIZING_SIZE]]
#             ]
# '''

# #include "/scripting/species/common/advanced_focus.macros"
# #include "/scripting/species/common/focus.macros"
# #include "/scripting/species/common/general.macros"
