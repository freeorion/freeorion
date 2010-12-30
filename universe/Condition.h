// -*- C++ -*-
#ifndef _Condition_h_
#define _Condition_h_

#include "Enums.h"
#include "ValueRefFwd.h"

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include <set>
#include <string>
#include <vector>

class UniverseObject;
struct ScriptingContext;

/** this namespace holds ConditionBase and its subclasses; these classes
  * represent predicates about UniverseObjects used by, for instance, the
  * Effect system. */
namespace Condition {
    typedef std::set<const UniverseObject*> ObjectSet;

    enum SearchDomain {
        NON_MATCHES,    ///< The Condition will only examine items in the non matches set; those that match the Condition will be inserted into the matches set.
        MATCHES         ///< The Condition will only examine items in the matches set; those that do not match the Condition will be inserted into the nonmatches set.
    };

    enum SortingMethod {
        SORT_MAX,       ///< Objects with the largest sort key will be selected
        SORT_MIN,       ///< Objects with the smallest sort key will be selected
        SORT_MODE,      ///< Objects with the most common sort key will be selected
        SORT_RANDOM     ///< Objects will be selected randomly, without consideration of property values
    };

    struct ConditionBase;
    struct All;
    struct EmpireAffiliation;
    struct Source;
    struct Target;
    struct Homeworld;
    struct Capitol;
    struct Type;
    struct Building;
    struct HasSpecial;
    struct Contains;
    struct PlanetSize;
    struct PlanetType;
    struct PlanetEnvironment;
    struct Species;
    struct FocusType;
    struct StarType;
    struct DesignHasHull;
    struct DesignHasPart;
    struct DesignHasPartClass;
    struct Chance;
    struct MeterValue;
    struct EmpireStockpileValue;
    struct OwnerHasTech;
    struct VisibleToEmpire;
    struct WithinDistance;
    struct WithinStarlaneJumps;
    struct CanHaveStarlaneConnection;
    struct ExploredByEmpire;
    struct Stationary;
    struct FleetSupplyableByEmpire;
    struct ResourceSupplyConnectedByEmpire;
    struct And;
    struct Or;
    struct Not;
    struct Turn;
    struct ContainedBy;
    struct Number;
    struct SortedNumberOf;
}

/** The base class for all Conditions. */
struct Condition::ConditionBase
{
    ConditionBase();
    virtual ~ConditionBase();
    virtual void        Eval(const ScriptingContext& parent_context,
                             Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const;

    /** Matches with an empty ScriptingContext */
    void                Eval(Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const;

    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects if the number of objects that match Condition
  * \a condition is is >= \a low and < \a high.  Matched objects may
  * or may not themselves match the condition. */
struct Condition::Number : Condition::ConditionBase
{
    Number(const ValueRef::ValueRefBase<int>* low, const ValueRef::ValueRefBase<int>* high, const ConditionBase* condition);
    virtual ~Number();
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches, Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    const ValueRef::ValueRefBase<int>* m_low;
    const ValueRef::ValueRefBase<int>* m_high;
    const ConditionBase*               m_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects if the current game turn is >= \a low and < \a high. */
struct Condition::Turn : Condition::ConditionBase
{
    Turn(const ValueRef::ValueRefBase<int>* low, const ValueRef::ValueRefBase<int>* high);
    virtual ~Turn();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    const ValueRef::ValueRefBase<int>* m_low;
    const ValueRef::ValueRefBase<int>* m_high;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches a specified \a number of objects that match Condition \a condition
  * or as many objects as match the condition if the number of objects is less
  * than the number requested.  If more objects match the condition than are
  * requested, the objects are sorted according to the value of the specified
  * \a property_name and objects are matched according to whether they have
  * the specified \a sorting_type of those property values.  For example,
  * objects with the largest, smallest or most common property value may be
  * selected preferentially. */
struct Condition::SortedNumberOf : public Condition::ConditionBase
{
    /** Sorts randomly, without considering a sort key. */
    SortedNumberOf(const ValueRef::ValueRefBase<int>* number,
                   const ConditionBase* condition);

    /** Sorts according to the specified method, based on the key values
      * evaluated for each object. */
    SortedNumberOf(const ValueRef::ValueRefBase<int>* number,
                   const ValueRef::ValueRefBase<double>* sort_key_ref,
                   SortingMethod sorting_method,
                   const ConditionBase* condition);

    virtual ~SortedNumberOf();
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches, Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    const ValueRef::ValueRefBase<int>*      m_number;
    const ValueRef::ValueRefBase<double>*   m_sort_key;
    SortingMethod                           m_sorting_method;
    const ConditionBase*                    m_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects. */
struct Condition::All : Condition::ConditionBase
{
    All();
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches, Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that are owned (if \a exclusive == false) or only owned
  * (if \a exclusive == true) by an empire that has affilitation type
  * \a affilitation with Empire \a empire_id. */
struct Condition::EmpireAffiliation : Condition::ConditionBase
{
    EmpireAffiliation(const ValueRef::ValueRefBase<int>* empire_id, EmpireAffiliationType affiliation, bool exclusive);
    virtual ~EmpireAffiliation();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    const ValueRef::ValueRefBase<int>* m_empire_id;
    EmpireAffiliationType              m_affiliation;
    bool                               m_exclusive;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches the source object only. */
struct Condition::Source : Condition::ConditionBase
{
    Source();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches the target of an effect being executed. */
struct Condition::Target : Condition::ConditionBase
{
    Target();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches planets that are a homeworld for any of the species specified in
  * \a names.  If \a names is empty, matches any planet that is a homeworld for
  * any species in the current game Universe. */
struct Condition::Homeworld : Condition::ConditionBase
{
    Homeworld(const std::vector<const ValueRef::ValueRefBase<std::string>*>& names);
    virtual ~Homeworld();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::vector<const ValueRef::ValueRefBase<std::string>*> m_names;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches planets that are an empire's capitol. */
struct Condition::Capitol : Condition::ConditionBase
{
    Capitol();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that are of UniverseObjectType \a type. */
struct Condition::Type : Condition::ConditionBase
{
    Type(const ValueRef::ValueRefBase<UniverseObjectType>* type);
    virtual ~Type();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    const ValueRef::ValueRefBase<UniverseObjectType>* m_type;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all Building objects that are one of the building types specified
  * in \a names. */
struct Condition::Building : Condition::ConditionBase
{
    Building(const std::vector<const ValueRef::ValueRefBase<std::string>*>& names);
    virtual ~Building();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::vector<const ValueRef::ValueRefBase<std::string>*> m_names;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that have an attached Special of the sort specified by
  * \a name.  Passing "All" for \a name will match all objects with attached
  * Specials. */
struct Condition::HasSpecial : Condition::ConditionBase
{
    HasSpecial(const std::string& name);
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::string m_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that contain an object that matches Condition
  * \a condition.  Container objects are Systems, Planets (which contain
  * Buildings), and Fleets (which contain Ships). */
struct Condition::Contains : Condition::ConditionBase
{
    Contains(const ConditionBase* condition);
    virtual ~Contains();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    const ConditionBase* m_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that are contained by an object that matches Condition
  * \a condition.  Container objects are Systems, Planets (which contain
  * Buildings), and Fleets (which contain Ships). */
struct Condition::ContainedBy : Condition::ConditionBase
{
    ContainedBy(const ConditionBase* condition);
    virtual ~ContainedBy();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    const ConditionBase* m_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all Planet objects that have one of the PlanetTypes in \a types.
  * Note that all Building objects which are on matching planets are also
  * matched. */
struct Condition::PlanetType : Condition::ConditionBase
{
    PlanetType(const std::vector<const ValueRef::ValueRefBase< ::PlanetType>*>& types);
    virtual ~PlanetType();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::vector<const ValueRef::ValueRefBase< ::PlanetType>*> m_types;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all Planet objects that have one of the PlanetSizes in \a sizes.
  * Note that all Building objects which are on matching planets are also
  * matched. */
struct Condition::PlanetSize : Condition::ConditionBase
{
    PlanetSize(const std::vector<const ValueRef::ValueRefBase< ::PlanetSize>*>& sizes);
    virtual ~PlanetSize();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::vector<const ValueRef::ValueRefBase< ::PlanetSize>*> m_sizes;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all Planet objects that have one of the PlanetEnvironments in \a environments.  Note that all
    Building objects which are on matching planets are also matched. */
struct Condition::PlanetEnvironment : Condition::ConditionBase
{
    PlanetEnvironment(const std::vector<const ValueRef::ValueRefBase< ::PlanetEnvironment>*>& environments);
    virtual ~PlanetEnvironment();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::vector<const ValueRef::ValueRefBase< ::PlanetEnvironment>*> m_environments;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all planets or ships that have one of the species in \a species.
  * Note that all Building object which are on matching planets are also
  * matched. */
struct Condition::Species : Condition::ConditionBase
{
    Species(const std::vector<const ValueRef::ValueRefBase<std::string>*>& names);
    virtual ~Species();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::vector<const ValueRef::ValueRefBase<std::string>*> m_names;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all ProdCenter objects that have one of the FocusTypes in \a foci. */
struct Condition::FocusType : Condition::ConditionBase
{
    FocusType(const std::vector<const ValueRef::ValueRefBase<std::string>*>& names);
    virtual ~FocusType();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::vector<const ValueRef::ValueRefBase<std::string>*> m_names;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all System objects that have one of the StarTypes in \a types.  Note that all objects
    in matching Systems are also matched (Ships, Fleets, Buildings, Planets, etc.). */
struct Condition::StarType : Condition::ConditionBase
{
    StarType(const std::vector<const ValueRef::ValueRefBase< ::StarType>*>& types);
    virtual ~StarType();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::vector<const ValueRef::ValueRefBase< ::StarType>*> m_types;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all ships whose ShipDesign has the hull specified by \a name. */
struct Condition::DesignHasHull : Condition::ConditionBase
{
    DesignHasHull(const std::string& name);
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::string     m_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all ships whose ShipDesign has >= \a low and < \a high of the ship
  * part specified by \a name. */
struct Condition::DesignHasPart : Condition::ConditionBase
{
    DesignHasPart(const ValueRef::ValueRefBase<int>* low, const ValueRef::ValueRefBase<int>* high, const std::string& name);
    virtual ~DesignHasPart();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    const ValueRef::ValueRefBase<int>*  m_low;
    const ValueRef::ValueRefBase<int>*  m_high;
    std::string                         m_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches ships whose ShipDesign has >= \a low and < \a high of ship parts of
  * the specified \a part_class */
struct Condition::DesignHasPartClass : Condition::ConditionBase
{
    DesignHasPartClass(const ValueRef::ValueRefBase<int>* low, const ValueRef::ValueRefBase<int>* high, ShipPartClass part_class);
    ~DesignHasPartClass();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    const ValueRef::ValueRefBase<int>*  m_low;
    const ValueRef::ValueRefBase<int>*  m_high;
    ShipPartClass                       m_class;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches a given object with a linearly distributed probability of \a chance. */
struct Condition::Chance : Condition::ConditionBase
{
    Chance(const ValueRef::ValueRefBase<double>* chance);
    virtual ~Chance();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    const ValueRef::ValueRefBase<double>* m_chance;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that have a meter of type \a meter, and whose current
  * value is >= \a low and < \a high. */
struct Condition::MeterValue : Condition::ConditionBase
{
    MeterValue(MeterType meter, const ValueRef::ValueRefBase<double>* low, const ValueRef::ValueRefBase<double>* high);
    virtual ~MeterValue();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    MeterType                             m_meter;
    const ValueRef::ValueRefBase<double>* m_low;
    const ValueRef::ValueRefBase<double>* m_high;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects with exactly one owner, whose owner's stockpile of \a stockpile is between \a low
    and \a high, inclusive. */
struct Condition::EmpireStockpileValue : Condition::ConditionBase
{
    EmpireStockpileValue(ResourceType stockpile, const ValueRef::ValueRefBase<double>* low, const ValueRef::ValueRefBase<double>* high);
    virtual ~EmpireStockpileValue();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ResourceType m_stockpile;
    const ValueRef::ValueRefBase<double>* m_low;
    const ValueRef::ValueRefBase<double>* m_high;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that have a single owner who has tech \a tech_name. */
struct Condition::OwnerHasTech : Condition::ConditionBase
{
    OwnerHasTech(const std::string& name);
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::string m_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that are visible to at least one Empire in \a empire_ids. */
struct Condition::VisibleToEmpire : Condition::ConditionBase
{
    VisibleToEmpire(const std::vector<const ValueRef::ValueRefBase<int>*>& empire_ids);
    virtual ~VisibleToEmpire();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::vector<const ValueRef::ValueRefBase<int>*> m_empire_ids;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that are within \a distance units of at least one
  * object that meets \a condition.  Warning: this Condition can slow things
  * down considerably if overused.  It is best to use Conditions that yield
  * relatively few matches. */
struct Condition::WithinDistance : Condition::ConditionBase
{
    WithinDistance(const ValueRef::ValueRefBase<double>* distance, const ConditionBase* condition);
    virtual ~WithinDistance();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    const ValueRef::ValueRefBase<double>* m_distance;
    const ConditionBase*                  m_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that are within \a jumps starlane jumps of at least one
  * object that meets \a condition.  Warning: this Condition can slow things
  * down considerably if overused.  It is best to use Conditions that yield
  * relatively few matches. */
struct Condition::WithinStarlaneJumps : Condition::ConditionBase
{
    WithinStarlaneJumps(const ValueRef::ValueRefBase<int>* jumps, const ConditionBase* condition);
    virtual ~WithinStarlaneJumps();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    const ValueRef::ValueRefBase<int>* m_jumps;
    const ConditionBase*               m_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches objects that are in systems that could have starlanes added between
  * them and all (not just one) of the systems containing (or that are) one of
  * the objects matched by \a condition.  "Could have starlanes added" means
  * that a lane would be geometrically acceptable, meaning it wouldn't cross
  * any other lanes, pass too close to another system, or be too close in angle
  * to an existing lane. */
struct Condition::CanHaveStarlaneConnection :  Condition::ConditionBase
{
    CanHaveStarlaneConnection(const ConditionBase* condition);
    virtual ~CanHaveStarlaneConnection();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    const ConditionBase*               m_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches systems that have been explored by at least one Empire
  * in \a empire_ids. */
struct Condition::ExploredByEmpire : Condition::ConditionBase
{
    ExploredByEmpire(const std::vector<const ValueRef::ValueRefBase<int>*>& empire_ids);
    virtual ~ExploredByEmpire();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::vector<const ValueRef::ValueRefBase<int>*> m_empire_ids;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches objects that are moving. ... What does that mean?  Departing this
  * turn, or were located somewhere else last turn...? */
struct Condition::Stationary : Condition::ConditionBase
{
    Stationary();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches objects that are in systems that can be fleet supplied by the
  * empire with id \a empire_id */
struct Condition::FleetSupplyableByEmpire : Condition::ConditionBase
{
    FleetSupplyableByEmpire(const ValueRef::ValueRefBase<int>* empire_id);
    virtual ~FleetSupplyableByEmpire();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    const ValueRef::ValueRefBase<int>*  m_empire_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches objects that are in systems that are connected by resource-sharing
  * to at least one object that meets \a condition using the resource-sharing
  * network of the empire with id \a empire_id */
struct Condition::ResourceSupplyConnectedByEmpire : Condition::ConditionBase
{
    ResourceSupplyConnectedByEmpire(const ValueRef::ValueRefBase<int>* empire_id, const ConditionBase* condition);
    virtual ~ResourceSupplyConnectedByEmpire();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    const ValueRef::ValueRefBase<int>*  m_empire_id;
    const ConditionBase*                m_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that match every Condition in \a operands. */
struct Condition::And : Condition::ConditionBase
{
    And(const std::vector<const ConditionBase*>& operands);
    virtual ~And();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::vector<const ConditionBase*> m_operands;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that match at least one Condition in \a operands. */
struct Condition::Or : Condition::ConditionBase
{
    Or(const std::vector<const ConditionBase*>& operands);
    virtual ~Or();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::vector<const ConditionBase*> m_operands;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that do not match the Condition \a operand. */
struct Condition::Not : Condition::ConditionBase
{
    Not(const ConditionBase* operand);
    virtual ~Not();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    const ConditionBase* m_operand;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// template implementations
template <class Archive>
void Condition::ConditionBase::serialize(Archive& ar, const unsigned int version)
{}

template <class Archive>
void Condition::Number::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_low)
        & BOOST_SERIALIZATION_NVP(m_high)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <class Archive>
void Condition::Turn::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_low)
        & BOOST_SERIALIZATION_NVP(m_high);
}

template <class Archive>
void Condition::SortedNumberOf::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_number)
        & BOOST_SERIALIZATION_NVP(m_sort_key)
        & BOOST_SERIALIZATION_NVP(m_sorting_method)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <class Archive>
void Condition::All::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase);
}

template <class Archive>
void Condition::EmpireAffiliation::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_affiliation)
        & BOOST_SERIALIZATION_NVP(m_exclusive);
}

template <class Archive>
void Condition::Source::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase);
}

template <class Archive>
void Condition::Target::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase);
}

template <class Archive>
void Condition::Homeworld::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_names);
}

template <class Archive>
void Condition::Capitol::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase);
}

template <class Archive>
void Condition::Type::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_type);
}

template <class Archive>
void Condition::Building::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_names);
}

template <class Archive>
void Condition::HasSpecial::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <class Archive>
void Condition::Contains::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <class Archive>
void Condition::ContainedBy::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <class Archive>
void Condition::PlanetType::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_types);
}

template <class Archive>
void Condition::PlanetSize::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_sizes);
}

template <class Archive>
void Condition::PlanetEnvironment::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_environments);
}

template <class Archive>
void Condition::Species::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_names);
}

template <class Archive>
void Condition::FocusType::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_names);
}

template <class Archive>
void Condition::StarType::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_types);
}

template <class Archive>
void Condition::DesignHasHull::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <class Archive>
void Condition::DesignHasPart::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_low)
        & BOOST_SERIALIZATION_NVP(m_high)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <class Archive>
void Condition::DesignHasPartClass::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_low)
        & BOOST_SERIALIZATION_NVP(m_high)
        & BOOST_SERIALIZATION_NVP(m_class);
}

template <class Archive>
void Condition::Chance::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_chance);
}

template <class Archive>
void Condition::MeterValue::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_meter)
        & BOOST_SERIALIZATION_NVP(m_low)
        & BOOST_SERIALIZATION_NVP(m_high);
}

template <class Archive>
void Condition::EmpireStockpileValue::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_low)
        & BOOST_SERIALIZATION_NVP(m_high);
}

template <class Archive>
void Condition::OwnerHasTech::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <class Archive>
void Condition::VisibleToEmpire::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_empire_ids);
}

template <class Archive>
void Condition::WithinDistance::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_distance)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <class Archive>
void Condition::WithinStarlaneJumps::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_jumps)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <class Archive>
void Condition::CanHaveStarlaneConnection::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <class Archive>
void Condition::ExploredByEmpire::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_empire_ids);
}

template <class Archive>
void Condition::Stationary::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase);
}

template <class Archive>
void Condition::FleetSupplyableByEmpire::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template <class Archive>
void Condition::ResourceSupplyConnectedByEmpire::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <class Archive>
void Condition::And::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_operands);
}

template <class Archive>
void Condition::Or::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_operands);
}

template <class Archive>
void Condition::Not::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_operand);
}

#endif // _Condition_h_
