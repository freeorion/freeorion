#ifndef _Tokens_h_
#define _Tokens_h_

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq/for_each.hpp>

#define TOKEN_SEQ_1                             \
    (Abs)                                       \
    (AccountingLabel)                           \
    (Activation)                                \
    (AddedBefore)                               \
    (AddedSince)                                \
    (AddSpecial)                                \
    (AddStarlanes)                              \
    (Adequate)                                  \
    (Affiliation)                               \
    (Age)                                       \
    (Aggressive)                                \
    (All)                                       \
    (Allowed)                                   \
    (AllyOf)                                    \
    (And)                                       \
    (AnyEmpire)                                 \
    (Armed)                                     \
    (Armour)                                    \
    (ArrivedOnTurn)                             \
    (Article)                                   \
    (Application)                               \
    (Asteroids)                                 \
    (Attack)                                    \
    (Barren)                                    \
    (Basic)                                     \
    (BlackHole)                                 \
    (Blue)                                      \
    (Bombard)                                   \
    (Bomber)                                    \
    (BuildCost)

#define TOKEN_SEQ_2                             \
    (Building)                                  \
    (BuildingTypesOwned)                        \
    (BuildingType)                              \
    (BuildingTypesProduced)                     \
    (BuildingTypesScrapped)                     \
    (BuildTime)                                 \
    (CanAddStarlanesTo)                         \
    (CanColonize)                               \
    (CanProduceShips)                           \
    (CanSee)                                    \
    (Capacity)                                  \
    (Capital)                                   \
    (Capture)                                   \
    (CaptureResult)                             \
    (Category)                                  \
    (Class)                                     \
    (ClockwiseNextPlanetType)                   \
    (Colony)                                    \
    (Colour)                                    \
    (Condition)                                 \
    (Construction)                              \
    (Consumption)                               \
    (ContainedBy)                               \
    (Contains)                                  \
    (Core)                                      \
    (Cos)                                       \
    (Count)                                     \
    (CountUnique)                               \
    (CounterClockwiseNextPlanetType)            \
    (CreateBuilding)                            \
    (CreatedOnTurn)                             \
    (CreateField)                               \
    (CreatePlanet)                              \
    (CreateShip)                                \
    (CreateSystem)                              \
    (CreationTurn)                              \
    (CurrentContent)                            \
    (CurrentTurn)

#define TOKEN_SEQ_3                             \
    (Damage)                                    \
    (Data)                                      \
    (Default)                                   \
    (Defense)                                   \
    (Described)                                 \
    (Description)                               \
    (Desert)                                    \
    (Design)                                    \
    (DesignHasHull)                             \
    (DesignHasPart)                             \
    (DesignHasPartClass)                        \
    (DesignID)                                  \
    (DesignName)                                \
    (Destination)                               \
    (Destroy)                                   \
    (Detection)                                 \
    (DirectDistanceBetween)                     \
    (Disabled)                                  \
    (Distance)                                  \
    (DistanceFromOriginalType)                  \
    (Effects)                                   \
    (EffectsGroup)                              \
    (EffectsGroups)                             \
    (Else)                                      \
    (Empire)                                    \
    (EmpireMeter)                               \
    (EmpireMeterValue)                          \
    (EmpireObjectVisibility)                    \
    (EmpireShipsDestroyed)                      \
    (Enabled)                                   \
    (Endpoint)                                  \
    (EnemyOf)                                   \
    (Enqueued)                                  \
    (EnqueueLocation)                           \
    (Environment)                               \
    (Environments)                              \
    (ETA)                                       \
    (ExploredByEmpire)                          \
    (Exclusions)                                \
    (External)

#define TOKEN_SEQ_4                             \
    (Field)                                     \
    (FieldType)                                 \
    (FighterBay)                                \
    (FighterHangar)                             \
    (FighterWeapon)                             \
    (FinalDestinationID)                        \
    (Fleet)                                     \
    (FleetID)                                   \
    (FleetSupplyableByEmpire)                   \
    (Foci)                                      \
    (Focus)                                     \
    (FocusType)                                 \
    (Fuel)                                      \
    (Full)                                      \
    (GalaxyAge)                                 \
    (GalaxyMaxAIAggression)                     \
    (GalaxyMonsterFrequency)                    \
    (GalaxyNativeFrequency)                     \
    (GalaxyPlanetDensity)                       \
    (GalaxySeed)                                \
    (GalaxyShape)                               \
    (GalaxySize)                                \
    (GalaxySpecialFrequency)                    \
    (GalaxyStarlaneFrequency)                   \
    (Gameplay_Description)                      \
    (GameRule)                                  \
    (GasGiant)                                  \
    (General)

#define TOKEN_SEQ_5                             \
    (GenerateSitrepMessage)                     \
    (GiveEmpireTech)                            \
    (Good)                                      \
    (Graphic)                                   \
    (Happiness)                                 \
    (HasSpecial)                                \
    (HasSpecialCapacity)                        \
    (HasSpecialSinceTurn)                       \
    (HasTag)                                    \
    (High)                                      \
    (HighestCostEnqueuedTech)                   \
    (HighestCostResearchableTech)               \
    (HighestCostTransferrableTech)              \
    (Homeworld)                                 \
    (Hostile)                                   \
    (Huge)                                      \
    (Hull)                                      \
    (HullFuel)                                  \
    (HullSpeed)                                 \
    (HullStealth)                               \
    (HullStructure)                             \
    (HullType)                                  \
    (Human)                                     \
    (Icon)                                      \
    (ID)                                        \
    (If)                                        \
    (Industry)                                  \
    (Inferno)                                   \
    (InSystem)                                  \
    (Interceptor)                               \
    (Integer)                                   \
    (Internal)                                  \
    (Invisible)                                 \
    (Item)                                      \
    (Jumps)                                     \
    (JumpsBetween)                              \
    (Keymap)                                    \
    (Keys)

#define TOKEN_SEQ_6                             \
    (Label)                                     \
    (Large)                                     \
    (LastTurnBattleHere)                        \
    (LastTurnActiveInBattle)                    \
    (LastTurnResupplied)                        \
    (LeastHappySpecies)                         \
    (LocalCandidate)                            \
    (Location)                                  \
    (Log)                                       \
    (Low)                                       \
    (LowestCostEnqueuedTech)                    \
    (LowestCostResearchableTech)                \
    (LowestCostTransferrableTech)               \
    (Max)                                       \
    (MaxCapacity)                               \
    (MaxDamage)                                 \
    (MaxDefense)                                \
    (MaxFuel)                                   \
    (MaximumNumberOf)                           \
    (MaxShield)                                 \
    (MaxStockpile)                              \
    (MaxStructure)                              \
    (MaxSupply)                                 \
    (MaxTroops)                                 \
    (Mean)                                      \
    (Medium)                                    \
    (Message)                                   \
    (Meter)                                     \
    (Min)                                       \
    (MinimumNumberOf)                           \
    (Missiles)

#define TOKEN_SEQ_7                             \
    (Mode)                                      \
    (Model)                                     \
    (ModeNumberOf)                              \
    (Monster)                                   \
    (MonsterFleet)                              \
    (MostHappySpecies)                          \
    (MostPopulousSpecies)                       \
    (MostSpentEnqueuedTech)                     \
    (MostSpentResearchableTech)                 \
    (MostSpentTransferrableTech)                \
    (MountableSlotTypes)                        \
    (MoveInOrbit)                               \
    (MoveTo)                                    \
    (MoveTowards)                               \
    (Name)                                      \
    (Native)                                    \
    (NearestSystemID)                           \
    (Neutron)                                   \
    (NextBetterPlanetType)                      \
    (NextCloserToOriginalPlanetType)            \
    (NextLargerPlanetSize)                      \
    (NextSmallerPlanetSize)                     \
    (NextOlderStarType)                         \
    (NextSystemID)                              \
    (NextTurnPopGrowth)                         \
    (NextYoungerStarType)                       \
    (NoDefaultCapacityEffect)                   \
    (None)                                      \
    (NoOp)                                      \
    (NoStar)                                    \
    (NoStringtableLookup)                       \
    (Not)                                       \
    (Number)                                    \
    (NumberOf)                                  \
    (NumShips)                                  \
    (NumStarlanes)                              \

#define TOKEN_SEQ_8                             \
    (Object)                                    \
    (ObjectType)                                \
    (Ocean)                                     \
    (Off)                                       \
    (On)                                        \
    (OneOf)                                     \
    (Opinion)                                   \
    (Or)                                        \
    (Orange)                                    \
    (Orbit)                                     \
    (OrderedBombardedBy)                        \
    (OriginalType)                              \
    (OutpostsOwned)                             \
    (OwnedBy)                                   \
    (Owner)                                     \
    (OwnerHasShipPartAvailable)                 \
    (OwnerHasTech)                              \
    (OwnerTradeStockpile)                       \
    (Parameters)                                \
    (Part)                                      \
    (PartCapacity)                              \
    (PartClass)                                 \
    (PartDamage)                                \
    (Partial)                                   \
    (PartName)                                  \
    (PartSecondaryStat)

#define TOKEN_SEQ_9                             \
    (Parts)                                     \
    (PartOfClassInShipDesign)                   \
    (PartsInShipDesign)                         \
    (PartType)                                  \
    (Passive)                                   \
    (Planet)                                    \
    (Planetbound)                               \
    (PlanetEnvironment)                         \
    (PlanetID)                                  \
    (PlanetSize)                                \
    (PlanetType)                                \
    (Playable)                                  \
    (PointDefense)                              \
    (Poor)                                      \
    (Population)                                \
    (PopulationCenter)                          \
    (Position)                                  \
    (PreferredFocus)                            \
    (Prerequisites)                             \
    (PreviousSystemID)                          \
    (Priority)                                  \
    (Probability)                               \
    (ProducedByEmpire)                          \
    (ProducedByEmpireID)                        \
    (Producible)                                \
    (Product)                                   \
    (ProductionCenter)                          \
    (ProductionLocation)                        \
    (PropagatedSupplyRange)                     \
    (Property)                                  \
    (Progress)

#define TOKEN_SEQ_10                            \
    (Radiated)                                  \
    (Radius)                                    \
    (Random)                                    \
    (RandomColonizableSpecies)                  \
    (RandomCompleteTech)                        \
    (RandomControlledSpecies)                   \
    (RandomEnqueuedTech)                        \
    (RandomResearchableTech)                    \
    (RandomTransferrableTech)                   \
    (RandomNumber)                              \
    (Range)                                     \
    (Real)                                      \
    (Reason)                                    \
    (RebelTroops)                               \
    (Red)                                       \
    (Refinement)                                \
    (RemoveSpecial)                             \
    (RemoveStarlanes)                           \
    (Research)                                  \
    (Researchable)                              \
    (ResearchCost)                              \
    (ResearchTurns)                             \
    (ResourceSupplyConnected)                   \
    (ResupplyableBy)                            \
    (Retain)                                    \
    (RMS)                                       \
    (RootCandidate)                             \
    (Scope)

#define TOKEN_SEQ_11                            \
    (SetAggressive)                             \
    (SetCapacity)                               \
    (SetConstruction)                           \
    (SetDamage)                                 \
    (SetDefense)                                \
    (SetDestination)                            \
    (SetDetection)                              \
    (SetEmpireCapital)                          \
    (SetEmpireMeter)                            \
    (SetEmpireTechProgress)                     \
    (SetEmpireStockpile)                        \
    (SetFuel)                                   \
    (SetHappiness)                              \
    (SetIndustry)                               \
    (SetMaxCapacity)                            \
    (SetMaxDamage)                              \
    (SetMaxDefense)                             \
    (SetMaxFuel)                                \
    (SetMaxSecondaryStat)                       \
    (SetMaxShield)                              \
    (SetMaxStockpile)                           \
    (SetMaxStructure)                           \
    (SetMaxSupply)                              \
    (SetMaxTroops)                              \
    (SetOverlayTexture)                         \
    (SetOwner)                                  \
    (SetPassive)                                \
    (SetPlanetSize)                             \
    (SetPlanetType)                             \
    (SetPopulation)

#define TOKEN_SEQ_12                            \
    (SetRange)                                  \
    (SetRebelTroops)                            \
    (SetResearch)                               \
    (SetSecondaryStat)                          \
    (SetShield)                                 \
    (SetSize)                                   \
    (SetSpecialCapacity)                        \
    (SetSpecies)                                \
    (SetSpeciesOpinion)                         \
    (SetSpeed)                                  \
    (SetStarType)                               \
    (SetStealth)                                \
    (SetStockpile)                              \
    (SetStructure)                              \
    (SetSupply)                                 \
    (SetTargetConstruction)                     \
    (SetTargetHappiness)                        \
    (SetTargetIndustry)                         \
    (SetTargetPopulation)                       \
    (SetTargetResearch)                         \
    (SetTargetTrade)                            \
    (SetTexture)                                \
    (SetTrade)                                  \
    (SetTroops)                                 \
    (SetVisibility)

#define TOKEN_SEQ_13                            \
    (Shield)                                    \
    (ShipDesign)                                \
    (ShipDesignOrdering)                        \
    (ShipDesignsDestroyed)                      \
    (ShipDesignsLost)                           \
    (ShipDesignsOwned)                          \
    (ShipDesignsProduced)                       \
    (ShipDesignsScrapped)                       \
    (Ship)                                      \
    (ShipHull)                                  \
    (ShipPart)                                  \
    (ShipPartMeter)                             \
    (ShipPartsOwned)                            \
    (Ships)                                     \
    (Short_Description)                         \
    (ShortestPath)                              \
    (ShortRange)                                \
    (Shots)                                     \
    (Sin)                                       \
    (Size)                                      \
    (SizeAsDouble)

#define TOKEN_SEQ_14                            \
    (Slot)                                      \
    (Slots)                                     \
    (SlotsInHull)                               \
    (SlotsInShipDesign)                         \
    (SlotType)                                  \
    (Small)                                     \
    (SortBy)                                    \
    (SortKey)                                   \
    (Source)                                    \
    (Spacebound)                                \
    (SpawnLimit)                                \
    (SpawnRate)                                 \
    (Special)                                   \
    (SpecialAddedOnTurn)                        \
    (SpecialCapacity)                           \
    (Species)                                   \
    (SpeciesID)                                 \
    (SpeciesOpinion)                            \
    (SpeciesPlanetsBombed)                      \
    (SpeciesColoniesOwned)

#define TOKEN_SEQ_15                            \
    (SpeciesPlanetsDepoped)                     \
    (SpeciesPlanetsInvaded)                     \
    (SpeciesShipsDestroyed)                     \
    (SpeciesShipsLost)                          \
    (SpeciesShipsOwned)                         \
    (SpeciesShipsProduced)                      \
    (SpeciesShipsScrapped)                      \
    (Speed)                                     \
    (Spread)                                    \
    (StackingGroup)                             \
    (Star)                                      \
    (StarType)                                  \
    (Stationary)                                \
    (Statistic)                                 \
    (StDev)                                     \
    (Stealth)                                   \
    (Stockpile)                                 \
    (String)                                    \
    (StringList)                                \
    (Structure)                                 \
    (Sum)                                       \
    (Supply)                                    \
    (SupplyingEmpire)                           \
    (Swamp)                                     \
    (System)                                    \
    (SystemID)

#define TOKEN_SEQ_16                            \
    (Tag)                                       \
    (Tags)                                      \
    (Target)                                    \
    (TargetConstruction)                        \
    (TargetHappiness)                           \
    (TargetIndustry)                            \
    (TargetPopulation)                          \
    (TargetResearch)                            \
    (TargetTrade)                               \
    (Tech)                                      \
    (Terran)                                    \
    (TestValue)                                 \
    (TheEmpire)                                 \
    (Theory)                                    \
    (ThisBuilding)                              \
    (ThisField)                                 \
    (ThisHull)                                  \
    (ThisPart)                                  \
    (ThisTech)                                  \
    (ThisSpecies)                               \
    (ThisSpecial)                               \
    (Tiny)                                      \
    (Toggle)                                    \
    (TopPriorityEnqueuedTech)                   \
    (TopPriorityResearchableTech)               \
    (TopPriorityTransferrableTech)              \
    (Toxic)                                     \
    (Trade)                                     \
    (TradeStockpile)

#define TOKEN_SEQ_17                            \
    (Troops)                                    \
    (Tundra)                                    \
    (Turn)                                      \
    (TurnsSinceFocusChange)                     \
    (TurnTechResearched)                        \
    (Type)                                      \
    (Uninhabitable)                             \
    (UniverseCentreX)                           \
    (UniverseCentreY)                           \
    (UniverseWidth)                             \
    (Unlock)                                    \
    (Unowned)                                   \
    (Unproducible)                              \
    (Unresearchable)                            \
    (UpgradeVisibility)                         \
    (UserString)                                \
    (UUID)                                      \
    (Value)                                     \
    (Victory)                                   \
    (VisibleToEmpire)                           \
    (Visibility)                                \
    (White)                                     \
    (WithinDistance)                            \
    (WithinStarlaneJumps)                       \
    (X)                                         \
    (Y)                                         \
    (Yellow)

#define DECLARE_TOKEN(r, _, elem) extern const char* const BOOST_PP_CAT(elem, _token);
BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_1)
BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_2)
BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_3)
BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_4)
BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_5)
BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_6)
BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_7)
BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_8)
BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_9)
BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_10)
BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_11)
BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_12)
BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_13)
BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_14)
BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_15)
BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_16)
BOOST_PP_SEQ_FOR_EACH(DECLARE_TOKEN, _, TOKEN_SEQ_17)

#undef DECLARE_TOKEN


#endif
