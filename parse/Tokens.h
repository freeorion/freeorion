// -*- C++ -*-
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
    (Alignment)                                 \
    (AlignmentEffects)                          \
    (All)                                       \
    (AllyOf)                                    \
    (And)                                       \
    (AnyEmpire)                                 \
    (Armed)                                     \
    (Armour)                                    \
    (Article)                                   \
    (Application)                               \
    (Asteroids)                                 \
    (Barren)                                    \
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
    (Distance)                                  \
    (DistanceFromOriginalType)                  \
    (Effects)                                   \
    (EffectsGroup)                              \
    (EffectsGroups)                             \
    (Empire)                                    \
    (EmpireMeter)                               \
    (EmpireShipsDestroyed)                      \
    (Endpoint)                                  \
    (EnemyOf)                                   \
    (Enqueued)                                  \
    (EnqueueLocation)                           \
    (Environment)                               \
    (Environments)                              \
    (ExploredByEmpire)                          \
    (External)

#define TOKEN_SEQ_4                             \
    (Field)                                     \
    (FieldType)                                 \
    (Fighters)                                  \
    (FinalDestinationID)                        \
    (Fleet)                                     \
    (FleetID)                                   \
    (FleetSupplyableByEmpire)                   \
    (Foci)                                      \
    (Focus)                                     \
    (FocusType)                                 \
    (Fuel)                                      \
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
    (Icon)                                      \
    (ID)                                        \
    (If)                                        \
    (Industry)                                  \
    (Inferno)                                   \
    (InSystem)                                  \
    (Interceptor)                               \
    (Internal)                                  \
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
    (LeastHappySpecies)                         \
    (LocalCandidate)                            \
    (Location)                                  \
    (Log)                                       \
    (Low)                                       \
    (LowestCostEnqueuedTech)                    \
    (LowestCostResearchableTech)                \
    (LowestCostTransferrableTech)               \
    (Max)                                       \
    (MaxDefense)                                \
    (MaxFuel)                                   \
    (MaximumNumberOf)                           \
    (MaxShield)                                 \
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
    (None)                                      \
    (NoStar)                                    \
    (NoStringtableLookup)                       \
    (Not)                                       \
    (Number)                                    \
    (NumberOf)                                  \
    (NumShips)                                  \

#define TOKEN_SEQ_8                             \
    (Object)                                    \
    (ObjectType)                                \
    (Ocean)                                     \
    (Opinion)                                   \
    (Or)                                        \
    (Orange)                                    \
    (Orbit)                                     \
    (OrderedBombardedBy)                        \
    (OriginalType)                              \
    (OutpostsOwned)                             \
    (OwnedBy)                                   \
    (Owner)                                     \
    (OwnerHasTech)                              \
    (OwnerTradeStockpile)                       \
    (Parameters)                                \
    (Part)                                      \
    (PartCapacity)                              \
    (PartDamage)                                \
    (PartClass)                                 \
    (PartName)

#define TOKEN_SEQ_9                             \
    (Parts)                                     \
    (PartClassInShipDesign)                     \
    (PartsInShipDesign)                         \
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
    (Probability)                               \
    (ProducedByEmpire)                          \
    (ProducedByEmpireID)                        \
    (Producible)                                \
    (Product)                                   \
    (ProductionCenter)                          \
    (ProductionLocation)                        \
    (Progress)                                  \
    (Property)

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
    (SetEmpireTradeStockpile)                   \
    (SetFuel)                                   \
    (SetHappiness)                              \
    (SetIndustry)                               \
    (SetMaxDefense)                             \
    (SetMaxFuel)                                \
    (SetMaxShield)                              \
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
    (SetShield)                                 \
    (SetSize)                                   \
    (SetSpecies)                                \
    (SetSpeciesOpinion)                         \
    (SetSpeed)                                  \
    (SetStarType)                               \
    (SetStealth)                                \
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
    (Shield)                                    \
    (ShipDesign)                                \
    (ShipDesignsDestroyed)                      \
    (ShipDesignsLost)                           \
    (ShipDesignsOwned)                          \
    (ShipDesignsProduced)                       \
    (ShipDesignsScrapped)                       \
    (Ship)                                      \
    (ShipHull)                                  \
    (ShipPart)                                  \
    (ShipPartMeter)                             \
    (Ships)                                     \
    (Short_Description)                         \
    (ShortestPath)                              \
    (ShortRange)                                \
    (Sin)                                       \
    (Size)                                      \
    (SizeAsDouble)

#define TOKEN_SEQ_13                            \
    (Slot)                                      \
    (Slots)                                     \
    (SlotType)                                  \
    (Small)                                     \
    (SortBy)                                    \
    (SortKey)                                   \
    (Source)                                    \
    (Spacebound)                                \
    (SpawnLimit)                                \
    (SpawnRate)                                 \
    (Special)                                   \
    (Species)                                   \
    (SpeciesOpinion)                            \
    (SpeciesPlanetsBombed)                      \
    (SpeciesColoniesOwned)                      \
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
    (Structure)                                 \
    (Sum)                                       \
    (Supply)                                    \
    (Swamp)                                     \
    (System)                                    \
    (SystemID)

#define TOKEN_SEQ_14                            \
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
    (TechType)                                  \
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
    (TopPriorityEnqueuedTech)                   \
    (TopPriorityResearchableTech)               \
    (TopPriorityTransferrableTech)              \
    (Toxic)                                     \
    (Trade)                                     \
    (TradeStockpile)                            \
    (Troops)                                    \
    (Tundra)                                    \
    (Turn)                                      \
    (TurnsSinceFocusChange)                     \
    (Type)                                      \
    (Uninhabitable)                             \
    (UniverseCentreX)                           \
    (UniverseCentreY)                           \
    (Unlock)                                    \
    (Unowned)                                   \
    (Unproducible)                              \
    (Unresearchable)                            \
    (Value)                                     \
    (ValueTest)                                 \
    (Victory)                                   \
    (VisibleToEmpire)                           \
    (White)                                     \
    (WithinDistance)                            \
    (WithinStarlaneJumps)                       \
    (X)                                         \
    (Y)                                         \
    (Yellow)

#define DECLARE_TOKEN(r, _, elem) extern const char* BOOST_PP_CAT(elem, _token);
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

#undef DECLARE_TOKEN


#endif
