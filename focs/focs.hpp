#ifndef _focs_hpp_
#define _focs_hpp_


namespace focs {

enum ComparisonType : int;
enum ContentType : int;
enum OpType : int;
enum ReferenceType : int;
enum SearchDomain : int;
enum SortingMethod : int;
enum StatisticType : int;
struct Aggressive;
struct All;
struct And;
struct Armed;
struct Building;
struct CanAddStarlaneConnection;
struct CanColonize;
struct CanProduceShips;
struct Capital;
struct Chance;
struct CombatTarget;
struct Condition;
struct ContainedBy;
struct Contains;
struct CreatedOnTurn;
struct Described;
struct DesignHasHull;
struct DesignHasPart;
struct DesignHasPartClass;
struct EmpireAffiliation;
struct EmpireHasAdoptedPolicy;
struct EmpireMeterValue;
struct EmpireStockpileValue;
struct Enqueued;
struct ExploredByEmpire;
struct FleetSupplyableByEmpire;
struct FocusType;
struct HasSpecial;
struct HasTag;
struct Homeworld;
struct InOrIsSystem;
struct Location;
struct MeterValue;
struct Monster;
struct NameLookup;
struct None;
struct Not;
struct Number;
struct NumberedShipDesign;
struct ObjectID;
struct OnPlanet;
struct Or;
struct OrderedAlternativesOf;
struct OrderedBombarded;
struct OwnerHasBuildingTypeAvailable;
struct OwnerHasShipDesignAvailable;
struct OwnerHasShipPartAvailable;
struct OwnerHasTech;
struct PlanetEnvironment;
struct PlanetSize;
struct PlanetType;
struct PredefinedShipDesign;
struct ProducedByEmpire;
struct ResourceSupplyConnectedByEmpire;
struct RootCandidate;
struct ShipPartMeterValue;
struct SortedNumberOf;
struct Source;
struct Species;
struct StarType;
struct Stationary;
struct Target;
struct Turn;
struct Type;
struct ValueRefBase;
struct ValueTest;
struct VisibleToEmpire;
struct WithinDistance;
struct WithinStarlaneJumps;
template <typename From> struct StringCast;
template <typename From> struct UserStringLookup;
template <typename From, typename To> struct StaticCast;
template <typename Type> struct ComplexVariable;
template <typename Type> struct Constant;
template <typename Type> struct Operation;
template <typename Type> struct Statistic;
template <typename Type> struct ValueRef;
template <typename Type> struct Variable;

}


#endif
