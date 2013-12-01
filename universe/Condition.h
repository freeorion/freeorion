// -*- C++ -*-
#ifndef _Condition_h_
#define _Condition_h_

#include "Enums.h"
#include "ValueRefFwd.h"
#include "TemporaryPtr.h"

#include "../util/Export.h"

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include <string>
#include <vector>

class UniverseObject;
struct ScriptingContext;

/** this namespace holds ConditionBase and its subclasses; these classes
  * represent predicates about UniverseObjects used by, for instance, the
  * Effect system. */
namespace Condition {
    typedef std::vector<TemporaryPtr<const UniverseObject> > ObjectSet;

    enum Invariance {
        UNKNOWN_INVARIANCE, ///< This condition hasn't yet calculated this invariance type
        INVARIANT,          ///< This condition is invariant to a particular type of object change
        VARIANT             ///< This condition's result depends on the state of a particular object
    };

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
    struct RootCandidate;
    struct Target;
    struct Homeworld;
    struct Capital;
    struct Monster;
    struct Armed;
    struct Type;
    struct Building;
    struct HasSpecial;
    struct HasTag;
    struct Contains;
    struct PlanetSize;
    struct PlanetType;
    struct PlanetEnvironment;
    struct Species;
    struct Enqueued;
    struct FocusType;
    struct StarType;
    struct DesignHasHull;
    struct DesignHasPart;
    struct DesignHasPartClass;
    struct PredefinedShipDesign;
    struct NumberedShipDesign;
    struct ProducedByEmpire;
    struct Chance;
    struct MeterValue;
    struct ShipPartMeterValue;
    struct EmpireMeterValue;
    struct EmpireStockpileValue;
    struct OwnerHasTech;
    struct OwnerHasBuildingTypeAvailable;
    struct OwnerHasShipDesignAvailable;
    struct VisibleToEmpire;
    struct WithinDistance;
    struct WithinStarlaneJumps;
    struct CanAddStarlaneConnection;
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
    struct InSystem;
    struct ObjectID;
    struct CreatedOnTurn;
    struct CanColonize;
    struct CanProduceShips;
    struct OrderedBombarded;
    struct ValueTest;
}

/** Returns a single string which describes a vector of Conditions. If multiple
  * conditions are passed, they are treated as if they were contained by an And
  * condition. Subconditions within an And (or nested And) are listed as
  * lines in a list, with duplicates removed, titled something like "All of...".
  * Subconditions within an Or (or nested Ors) are similarly listed as lines in
  * a list, with duplicates removed, titled something like "One of...". If a
  * candidate object is provided, the returned string will indicate which 
  * subconditions the candidate matches, and indicate if the overall combination
  * of conditions matches the object. */
FO_COMMON_API std::string ConditionDescription(const std::vector<const Condition::ConditionBase*>& conditions,
                                 TemporaryPtr<const UniverseObject> candidate_object = TemporaryPtr<const UniverseObject>(),
                                 TemporaryPtr<const UniverseObject> source_object = TemporaryPtr<const UniverseObject>());

/** The base class for all Conditions. */
struct FO_COMMON_API Condition::ConditionBase {
    ConditionBase() :
        m_root_candidate_invariant(UNKNOWN_INVARIANCE),
        m_target_invariant(UNKNOWN_INVARIANCE),
        m_source_invariant(UNKNOWN_INVARIANCE)
    {}
    virtual ~ConditionBase();

    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    bool                operator!=(const Condition::ConditionBase& rhs) const { return !(*this == rhs); }

    virtual void        Eval(const ScriptingContext& parent_context,
                             Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const;

    /** Matches with an empty ScriptingContext */
    void                Eval(Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const;

    /** Tests all objects in universe as NON_MATCHES. */
    void                Eval(const ScriptingContext& parent_context,
                             Condition::ObjectSet& matches) const;

    /** Tests all objects in universe as NON_MATCHES with empty ScriptingContext. */
    void                Eval(Condition::ObjectSet& matches) const;

    /** Tests single candidate object, returning true iff it matches condition. */
    bool                Eval(const ScriptingContext& parent_context,
                             TemporaryPtr<const UniverseObject> candidate) const;

    /** Tests single candidate object, returning true iff it matches condition
      * with empty ScriptingContext. */
    bool                Eval(TemporaryPtr<const UniverseObject> candidate) const;
    
    virtual void        GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context, Condition::ObjectSet& condition_non_targets) const;

    /** Returns true iff this condition's evaluation does not reference
      * the RootCandidate objects.  This requirement ensures that if this
      * condition is a subcondition to another Condition or a ValueRef, this
      * condition may be evaluated once and its result used to match all local
      * candidates to that condition. */
    virtual bool        RootCandidateInvariant() const { return false; }

    /** (Almost) all conditions are varying with local candidates; this is the
      * point of evaluating a condition.  This funciton is provided for
      * consistency with ValueRef, which may not depend on the local candidiate
      * of an enclosing condition. */
    bool                LocalCandidateInvariant() const { return false; }

    /** Returns true iff this condition's evaluation does not reference the
      * target object.*/
    virtual bool        TargetInvariant() const { return false; }

    /** Returns true iff this condition's evaluation does not reference the
      * source object.*/
    virtual bool        SourceInvariant() const { return false; }

    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

protected:
    mutable Invariance  m_root_candidate_invariant;
    mutable Invariance  m_target_invariant;
    mutable Invariance  m_source_invariant;

private:
    struct MatchHelper;
    friend struct MatchHelper;

    virtual bool        Match(const ScriptingContext& local_context) const;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects if the number of objects that match Condition
  * \a condition is is >= \a low and < \a high.  Matched objects may
  * or may not themselves match the condition. */
struct FO_COMMON_API Condition::Number : public Condition::ConditionBase {
    Number(const ValueRef::ValueRefBase<int>* low, const ValueRef::ValueRefBase<int>* high,
           const ConditionBase* condition) :
        m_low(low),
        m_high(high),
        m_condition(condition)
    {}
    virtual ~Number();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<int>*  Low() const { return m_low; }
    const ValueRef::ValueRefBase<int>*  High() const { return m_high; }
    const ConditionBase*                GetCondition() const { return m_condition; }

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    const ValueRef::ValueRefBase<int>* m_low;
    const ValueRef::ValueRefBase<int>* m_high;
    const ConditionBase*               m_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects if the current game turn is >= \a low and < \a high. */
struct FO_COMMON_API Condition::Turn : public Condition::ConditionBase {
    Turn(const ValueRef::ValueRefBase<int>* low, const ValueRef::ValueRefBase<int>* high) :
        m_low(low),
        m_high(high)
    {}
    virtual ~Turn();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<int>*  Low() const { return m_low; }
    const ValueRef::ValueRefBase<int>*  High() const { return m_high; }

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
struct FO_COMMON_API Condition::SortedNumberOf : public Condition::ConditionBase {
    /** Sorts randomly, without considering a sort key. */
    SortedNumberOf(const ValueRef::ValueRefBase<int>* number,
                   const ConditionBase* condition) :
        m_number(number),
        m_sort_key(0),
        m_sorting_method(Condition::SORT_RANDOM),
        m_condition(condition)
    {}

    /** Sorts according to the specified method, based on the key values
      * evaluated for each object. */
    SortedNumberOf(const ValueRef::ValueRefBase<int>* number,
                   const ValueRef::ValueRefBase<double>* sort_key_ref,
                   SortingMethod sorting_method,
                   const ConditionBase* condition) :
        m_number(number),
        m_sort_key(sort_key_ref),
        m_sorting_method(sorting_method),
        m_condition(condition)
    {}
    virtual ~SortedNumberOf();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<int>*      Number() const { return m_number; }
    const ValueRef::ValueRefBase<double>*   SortKey() const { return m_sort_key; }
    SortingMethod                           GetSortingMethod() const { return m_sorting_method; }
    const ConditionBase*                    GetCondition() const { return m_condition; }
    virtual void        GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context, Condition::ObjectSet& condition_non_targets) const;

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
struct FO_COMMON_API Condition::All : public Condition::ConditionBase {
    All() : ConditionBase() {}
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    virtual bool        RootCandidateInvariant() const { return true; }
    virtual bool        TargetInvariant() const { return true; }
    virtual bool        SourceInvariant() const { return true; }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that are owned (if \a exclusive == false) or only owned
  * (if \a exclusive == true) by an empire that has affilitation type
  * \a affilitation with Empire \a empire_id. */
struct FO_COMMON_API Condition::EmpireAffiliation : public Condition::ConditionBase {
    EmpireAffiliation(const ValueRef::ValueRefBase<int>* empire_id, EmpireAffiliationType affiliation) :
        m_empire_id(empire_id),
        m_affiliation(affiliation)
    {}
    EmpireAffiliation(EmpireAffiliationType affiliation) :
       m_empire_id(0),
       m_affiliation(affiliation)
    {}
    virtual ~EmpireAffiliation();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<int>*  EmpireID() const { return m_empire_id; }
    EmpireAffiliationType               GetAffiliation() const { return m_affiliation; }

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    const ValueRef::ValueRefBase<int>* m_empire_id;
    EmpireAffiliationType              m_affiliation;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches the source object only. */
struct FO_COMMON_API Condition::Source : public Condition::ConditionBase {
    Source() : ConditionBase() {}
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual bool        RootCandidateInvariant() const { return true; }
    virtual bool        TargetInvariant() const { return true; }
    //virtual bool        SourceInvariant() const { return false; } // same as ConditionBase
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    virtual void        GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context, Condition::ObjectSet& condition_non_targets) const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches the root candidate object in a condition tree.  This is useful
  * within a subcondition to match the object actually being matched by the
  * whole compound condition, rather than an object just being matched in a
  * subcondition in order to evaluate the outer condition. */
struct FO_COMMON_API Condition::RootCandidate : public Condition::ConditionBase {
    RootCandidate() : ConditionBase() {}
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual bool        RootCandidateInvariant() const { return false; }
    virtual bool        TargetInvariant() const { return true; }
    virtual bool        SourceInvariant() const { return true; }
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** There is no LocalCandidate condition.  To match any local candidate object,
  * use the All condition. */

/** Matches the target of an effect being executed. */
struct FO_COMMON_API Condition::Target : public Condition::ConditionBase {
    Target() : ConditionBase() {}
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual bool        RootCandidateInvariant() const { return true; }
    virtual bool        TargetInvariant() const { return false; }
    virtual bool        SourceInvariant() const { return true; }
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
struct FO_COMMON_API Condition::Homeworld : public Condition::ConditionBase {
    Homeworld() :
        ConditionBase(),
        m_names()
    {}
    Homeworld(const std::vector<const ValueRef::ValueRefBase<std::string>*>& names) :
        m_names(names)
    {}
    virtual ~Homeworld();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const std::vector<const ValueRef::ValueRefBase<std::string>*>   Names() const { return m_names; }
    virtual void        GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context, Condition::ObjectSet& condition_non_targets) const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::vector<const ValueRef::ValueRefBase<std::string>*> m_names;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches planets that are an empire's capital. */
struct FO_COMMON_API Condition::Capital : public Condition::ConditionBase {
    Capital() : ConditionBase() {}
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual bool        RootCandidateInvariant() const { return true; }
    virtual bool        TargetInvariant() const { return true; }
    virtual bool        SourceInvariant() const { return true; }
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    virtual void        GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context, Condition::ObjectSet& condition_non_targets) const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches space monsters. */
struct FO_COMMON_API Condition::Monster : public Condition::ConditionBase {
    Monster() : ConditionBase() {}
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual bool        RootCandidateInvariant() const { return true; }
    virtual bool        TargetInvariant() const { return true; }
    virtual bool        SourceInvariant() const { return true; }
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches armed ships and monsters. */
struct FO_COMMON_API Condition::Armed : public Condition::ConditionBase {
    Armed() {};
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual bool        RootCandidateInvariant() const { return true; }
    virtual bool        TargetInvariant() const { return true; }
    virtual bool        SourceInvariant() const { return true; }
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that are of UniverseObjectType \a type. */
struct FO_COMMON_API Condition::Type : public Condition::ConditionBase {
    Type(const ValueRef::ValueRefBase<UniverseObjectType>* type) :
        m_type(type)
    {}
    virtual ~Type();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<UniverseObjectType>*   GetType() const { return m_type; }
    virtual void        GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context, Condition::ObjectSet& condition_non_targets) const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    const ValueRef::ValueRefBase<UniverseObjectType>* m_type;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all Building objects that are one of the building types specified
  * in \a names. */
struct FO_COMMON_API Condition::Building : public Condition::ConditionBase {
    Building(const std::vector<const ValueRef::ValueRefBase<std::string>*>& names) :
        m_names(names)
    {}
    virtual ~Building();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const std::vector<const ValueRef::ValueRefBase<std::string>*>   Names() const { return m_names; }

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::vector<const ValueRef::ValueRefBase<std::string>*> m_names;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that have an attached Special named \a name. */
struct FO_COMMON_API Condition::HasSpecial : public Condition::ConditionBase {
    HasSpecial(const std::string& name) :
        m_name(name),
        m_since_turn_low(0),
        m_since_turn_high(0)
    {}
    HasSpecial(const std::string& name,
               const ValueRef::ValueRefBase<int>* since_turn_low,
               const ValueRef::ValueRefBase<int>* since_turn_high) :
        m_name(name),
        m_since_turn_low(since_turn_low),
        m_since_turn_high(since_turn_high)
    {}
    virtual ~HasSpecial();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const std::string&                  Name() const { return m_name; }
    const ValueRef::ValueRefBase<int>*  Low() const { return m_since_turn_low; }
    const ValueRef::ValueRefBase<int>*  High() const { return m_since_turn_high; }

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::string                         m_name;
    const ValueRef::ValueRefBase<int>*  m_since_turn_low;
    const ValueRef::ValueRefBase<int>*  m_since_turn_high;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that have the tag \a tag. */
struct FO_COMMON_API Condition::HasTag : public Condition::ConditionBase {
    HasTag(const std::string& name) :
        m_name(name)
    {}
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual bool        RootCandidateInvariant() const { return true; }
    virtual bool        TargetInvariant() const { return true; }
    virtual bool        SourceInvariant() const { return true; }
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const std::string&  Name() const { return m_name; }

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::string                         m_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that were created on turns within the specified range. */
struct FO_COMMON_API Condition::CreatedOnTurn : public Condition::ConditionBase {
    CreatedOnTurn(const ValueRef::ValueRefBase<int>* low, const ValueRef::ValueRefBase<int>* high) :
        m_low(low),
        m_high(high)
    {}
    virtual ~CreatedOnTurn();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<int>*  Low() const { return m_low; }
    const ValueRef::ValueRefBase<int>*  High() const { return m_high; }

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    const ValueRef::ValueRefBase<int>*  m_low;
    const ValueRef::ValueRefBase<int>*  m_high;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that contain an object that matches Condition
  * \a condition.  Container objects are Systems, Planets (which contain
  * Buildings), and Fleets (which contain Ships). */
struct FO_COMMON_API Condition::Contains : public Condition::ConditionBase {
    Contains(const ConditionBase* condition) :
        m_condition(condition)
    {}
    virtual ~Contains();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ConditionBase*GetCondition() const { return m_condition; }

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
struct FO_COMMON_API Condition::ContainedBy : public Condition::ConditionBase {
    ContainedBy(const ConditionBase* condition) :
        m_condition(condition)
    {}
    virtual ~ContainedBy();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ConditionBase*GetCondition() const { return m_condition; }

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    const ConditionBase* m_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that are in the system with the indicated \a system_id */
struct FO_COMMON_API Condition::InSystem : public Condition::ConditionBase {
    InSystem(const ValueRef::ValueRefBase<int>* system_id) :
        m_system_id(system_id)
    {}
    virtual ~InSystem();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<int>*  SystemId() const { return m_system_id; }

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    const ValueRef::ValueRefBase<int>* m_system_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches the object with the id \a object_id */
struct FO_COMMON_API Condition::ObjectID : public Condition::ConditionBase {
    ObjectID(const ValueRef::ValueRefBase<int>* object_id) :
        m_object_id(object_id)
    {}
    virtual ~ObjectID();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<int>*  ObjectId() const { return m_object_id; }

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    const ValueRef::ValueRefBase<int>* m_object_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all Planet objects that have one of the PlanetTypes in \a types.
  * Note that all Building objects which are on matching planets are also
  * matched. */
struct FO_COMMON_API Condition::PlanetType : public Condition::ConditionBase {
    PlanetType(const std::vector<const ValueRef::ValueRefBase< ::PlanetType>*>& types) :
        m_types(types)
    {}
    virtual ~PlanetType();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const std::vector<const ValueRef::ValueRefBase< ::PlanetType>*>&    Types() const { return m_types; }
    void                GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context, Condition::ObjectSet& condition_non_targets) const;

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
struct FO_COMMON_API Condition::PlanetSize : public Condition::ConditionBase {
    PlanetSize(const std::vector<const ValueRef::ValueRefBase< ::PlanetSize>*>& sizes) :
        m_sizes(sizes)
    {}
    virtual ~PlanetSize();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const std::vector<const ValueRef::ValueRefBase< ::PlanetSize>*>&    Sizes() const { return m_sizes; }
    void                GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context, Condition::ObjectSet& condition_non_targets) const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::vector<const ValueRef::ValueRefBase< ::PlanetSize>*> m_sizes;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all Planet objects that have one of the PlanetEnvironments in
  * \a environments.  Note that all Building objects which are on matching
  * planets are also matched. */
struct FO_COMMON_API Condition::PlanetEnvironment : public Condition::ConditionBase {
    PlanetEnvironment(const std::vector<const ValueRef::ValueRefBase< ::PlanetEnvironment>*>& environments) :
        m_environments(environments)
    {}
    virtual ~PlanetEnvironment();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const std::vector<const ValueRef::ValueRefBase< ::PlanetEnvironment>*>& Environments() const { return m_environments; }
    void                GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context, Condition::ObjectSet& condition_non_targets) const;

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
struct FO_COMMON_API Condition::Species : public Condition::ConditionBase {
    Species(const std::vector<const ValueRef::ValueRefBase<std::string>*>& names) :
        ConditionBase(),
        m_names(names)
    {}
    Species() :
        ConditionBase(),
        m_names()
    {}
    virtual ~Species();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const std::vector<const ValueRef::ValueRefBase<std::string>*>&  Names() const { return m_names; }

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::vector<const ValueRef::ValueRefBase<std::string>*> m_names;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches planets where the indicated number of the indicated building type
  * or ship design are enqueued on the production queue. */
struct FO_COMMON_API Condition::Enqueued : public Condition::ConditionBase {
    Enqueued(BuildType build_type,
             const std::string& name = "",
             const ValueRef::ValueRefBase<int>* empire_id = 0,
             const ValueRef::ValueRefBase<int>* low = 0,
             const ValueRef::ValueRefBase<int>* high = 0) :
        ConditionBase(),
        m_build_type(build_type),
        m_name(name),
        m_design_id(0),
        m_empire_id(empire_id),
        m_low(low),
        m_high(high)
    {}
    Enqueued(const ValueRef::ValueRefBase<int>* design_id,
             const ValueRef::ValueRefBase<int>* empire_id = 0,
             const ValueRef::ValueRefBase<int>* low = 0,
             const ValueRef::ValueRefBase<int>* high = 0) :
        ConditionBase(),
        m_build_type(BT_SHIP),
        m_name(),
        m_design_id(design_id),
        m_empire_id(empire_id),
        m_low(low),
        m_high(high)
    {}
    Enqueued() :
        ConditionBase(),
        m_build_type(BT_NOT_BUILDING),
        m_name(),
        m_design_id(0),
        m_empire_id(0),
        m_low(0),
        m_high(0)
    {}
    virtual ~Enqueued();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    BuildType           GetBuildType() const { return m_build_type; }
    const std::string&  GetName() const { return m_name; }
    const ValueRef::ValueRefBase<int>*  DesignID() const { return m_design_id; }
    const ValueRef::ValueRefBase<int>*  EmpireID() const { return m_empire_id; }
    const ValueRef::ValueRefBase<int>*  Low() const { return m_low; }
    const ValueRef::ValueRefBase<int>*  High() const { return m_high; }

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    BuildType                           m_build_type;
    std::string                         m_name;
    const ValueRef::ValueRefBase<int>*  m_design_id;
    const ValueRef::ValueRefBase<int>*  m_empire_id;
    const ValueRef::ValueRefBase<int>*  m_low;
    const ValueRef::ValueRefBase<int>*  m_high;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all ProdCenter objects that have one of the FocusTypes in \a foci. */
struct FO_COMMON_API Condition::FocusType : public Condition::ConditionBase {
    FocusType(const std::vector<const ValueRef::ValueRefBase<std::string>*>& names) :
        m_names(names)
    {}
    virtual ~FocusType();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const std::vector<const ValueRef::ValueRefBase<std::string>*>&  Names() const { return m_names; }

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::vector<const ValueRef::ValueRefBase<std::string>*> m_names;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all System objects that have one of the StarTypes in \a types.  Note that all objects
    in matching Systems are also matched (Ships, Fleets, Buildings, Planets, etc.). */
struct FO_COMMON_API Condition::StarType : public Condition::ConditionBase {
    StarType(const std::vector<const ValueRef::ValueRefBase< ::StarType>*>& types) :
        m_types(types)
    {}
    virtual ~StarType();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const std::vector<const ValueRef::ValueRefBase< ::StarType>*>&  Types() const { return m_types; }

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::vector<const ValueRef::ValueRefBase< ::StarType>*> m_types;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all ships whose ShipDesign has the hull specified by \a name. */
struct FO_COMMON_API Condition::DesignHasHull : public Condition::ConditionBase {
    DesignHasHull(const std::string& name) :
        m_name(name)
    {}
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual bool        RootCandidateInvariant() const { return true; }
    virtual bool        TargetInvariant() const { return true; }
    virtual bool        SourceInvariant() const { return true; }
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const std::string&  Name() const { return m_name; }

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::string     m_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all ships whose ShipDesign has >= \a low and < \a high of the ship
  * part specified by \a name. */
struct FO_COMMON_API Condition::DesignHasPart : public Condition::ConditionBase {
    DesignHasPart(const ValueRef::ValueRefBase<int>* low, const ValueRef::ValueRefBase<int>* high,
                  const std::string& name) :
        m_low(low),
        m_high(high),
        m_name(name)
    {}
    virtual ~DesignHasPart();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<int>*  Low() const { return m_low; }
    const ValueRef::ValueRefBase<int>*  High() const { return m_high; }
    const std::string&                  Name() const { return m_name; }

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
struct FO_COMMON_API Condition::DesignHasPartClass : public Condition::ConditionBase {
    DesignHasPartClass(const ValueRef::ValueRefBase<int>* low, const ValueRef::ValueRefBase<int>* high,
                       ShipPartClass part_class) :
        m_low(low),
        m_high(high),
        m_class(part_class)
    {}
    ~DesignHasPartClass();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<int>*  Low() const { return m_low; }
    const ValueRef::ValueRefBase<int>*  High() const { return m_high; }
    ShipPartClass                       Class() const { return m_class; }

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    const ValueRef::ValueRefBase<int>*  m_low;
    const ValueRef::ValueRefBase<int>*  m_high;
    ShipPartClass                       m_class;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches ships who ShipDesign is a predefined shipdesign with the name
  * \a name */
struct FO_COMMON_API Condition::PredefinedShipDesign : public Condition::ConditionBase {
    PredefinedShipDesign(const std::string& name) :
        m_name(name)
    {}
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual bool        RootCandidateInvariant() const { return true; }
    virtual bool        TargetInvariant() const { return true; }
    virtual bool        SourceInvariant() const { return true; }
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const std::string&  Name() const { return m_name; }

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::string     m_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches ships whose design id \a id. */
struct FO_COMMON_API Condition::NumberedShipDesign : public Condition::ConditionBase {
    NumberedShipDesign(const ValueRef::ValueRefBase<int>* design_id) :
        m_design_id(design_id)
    {}
    virtual ~NumberedShipDesign();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<int>*  DesignID() const { return m_design_id; }

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    const ValueRef::ValueRefBase<int>* m_design_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches ships or buildings produced by the empire with id \a empire_id.*/
struct FO_COMMON_API Condition::ProducedByEmpire : public Condition::ConditionBase {
    ProducedByEmpire(const ValueRef::ValueRefBase<int>* empire_id) :
        m_empire_id(empire_id)
    {}
    virtual ~ProducedByEmpire();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<int>*  EmpireID() const { return m_empire_id; }

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    const ValueRef::ValueRefBase<int>* m_empire_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches a given object with a linearly distributed probability of \a chance. */
struct FO_COMMON_API Condition::Chance : public Condition::ConditionBase {
    Chance(const ValueRef::ValueRefBase<double>* chance) :
        m_chance(chance)
    {}
    virtual ~Chance();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<double>*   GetChance() const { return m_chance; }

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    const ValueRef::ValueRefBase<double>* m_chance;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that have a meter of type \a meter, and whose current
  * value is >= \a low and <= \a high. */
struct FO_COMMON_API Condition::MeterValue : public Condition::ConditionBase {
    MeterValue(MeterType meter, const ValueRef::ValueRefBase<double>* low,
               const ValueRef::ValueRefBase<double>* high) :
        m_meter(meter),
        m_low(low),
        m_high(high)
    {}
    virtual ~MeterValue();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<double>*   Low() const { return m_low; }
    const ValueRef::ValueRefBase<double>*   High() const { return m_high; }
    MeterType                               GetMeterType() const { return m_meter; }

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    MeterType                             m_meter;
    const ValueRef::ValueRefBase<double>* m_low;
    const ValueRef::ValueRefBase<double>* m_high;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches ships that have a ship part meter of type \a meter for part \a part
  * whose current value is >= low and <= high. */
struct FO_COMMON_API Condition::ShipPartMeterValue : public Condition::ConditionBase {
    ShipPartMeterValue(const std::string& ship_part_name,
                       MeterType meter,
                       const ValueRef::ValueRefBase<double>* low,
                       const ValueRef::ValueRefBase<double>* high) :
        m_part_name(ship_part_name),
        m_meter(meter),
        m_low(low),
        m_high(high)
    {}
    virtual ~ShipPartMeterValue();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const std::string&                      PartName() const { return m_part_name; }
    const ValueRef::ValueRefBase<double>*   Low() const { return m_low; }
    const ValueRef::ValueRefBase<double>*   High() const { return m_high; }
    MeterType                               GetMeterType() const { return m_meter; }

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::string                             m_part_name;
    MeterType                               m_meter;
    const ValueRef::ValueRefBase<double>*   m_low;
    const ValueRef::ValueRefBase<double>*   m_high;
};

/** Matches all objects if the empire with id \a empire_id has an empire meter
  * \a meter whose current value is >= \a low and <= \a high. */
struct FO_COMMON_API Condition::EmpireMeterValue : public Condition::ConditionBase {
    EmpireMeterValue(const std::string& meter,
                     const ValueRef::ValueRefBase<double>* low,
                     const ValueRef::ValueRefBase<double>* high) :
        m_empire_id(0),
        m_meter(meter),
        m_low(low),
        m_high(high)
    {}
    EmpireMeterValue(const ValueRef::ValueRefBase<int>* empire_id,
                     const std::string& meter,
                     const ValueRef::ValueRefBase<double>* low,
                     const ValueRef::ValueRefBase<double>* high) :
        m_empire_id(empire_id),
        m_meter(meter),
        m_low(low),
        m_high(high)
    {}
    virtual ~EmpireMeterValue();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const std::string&                      Meter() const { return m_meter; }
    const ValueRef::ValueRefBase<double>*   Low() const { return m_low; }
    const ValueRef::ValueRefBase<double>*   High() const { return m_high; }
    const ValueRef::ValueRefBase<int>*      EmpireID() const { return m_empire_id; }

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    const ValueRef::ValueRefBase<int>*      m_empire_id;
    const std::string                       m_meter;
    const ValueRef::ValueRefBase<double>*   m_low;
    const ValueRef::ValueRefBase<double>*   m_high;
};

/** Matches all objects whose owner's stockpile of \a stockpile is between
  * \a low and \a high, inclusive. */
struct FO_COMMON_API Condition::EmpireStockpileValue : public Condition::ConditionBase {
    EmpireStockpileValue(ResourceType stockpile, const ValueRef::ValueRefBase<double>* low,
                         const ValueRef::ValueRefBase<double>* high) :
        m_stockpile(stockpile),
        m_low(low),
        m_high(high)
    {}
    virtual ~EmpireStockpileValue();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<double>*   Low() const { return m_low; }
    const ValueRef::ValueRefBase<double>*   High() const { return m_high; }
    ResourceType                            Stockpile() const { return m_stockpile; }

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ResourceType                            m_stockpile;
    const ValueRef::ValueRefBase<double>*   m_low;
    const ValueRef::ValueRefBase<double>*   m_high;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects whose owner who has tech \a name. */
struct FO_COMMON_API Condition::OwnerHasTech : public Condition::ConditionBase {
    OwnerHasTech(const std::string& name) :
        m_name(name)
    {}
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual bool        RootCandidateInvariant() const { return true; }
    virtual bool        TargetInvariant() const { return true; }
    virtual bool        SourceInvariant() const { return true; }
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const std::string&  Tech() const { return m_name; }

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::string m_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects whose owner who has the building type \a name available. */
struct FO_COMMON_API Condition::OwnerHasBuildingTypeAvailable : public Condition::ConditionBase {
    OwnerHasBuildingTypeAvailable(const std::string& name) :
        m_name(name)
    {}
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual bool        RootCandidateInvariant() const { return true; }
    virtual bool        TargetInvariant() const { return true; }
    virtual bool        SourceInvariant() const { return true; }
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const std::string&  GetBuildingType() const { return m_name; }

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::string m_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects whose owner who has the ship design \a id available. */
struct FO_COMMON_API Condition::OwnerHasShipDesignAvailable : public Condition::ConditionBase {
    OwnerHasShipDesignAvailable(int id) :
        m_id(id)
    {}
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual bool        RootCandidateInvariant() const { return true; }
    virtual bool        TargetInvariant() const { return true; }
    virtual bool        SourceInvariant() const { return true; }
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    int                 GetDesignID() const { return m_id; }

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    int m_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that are visible to at least one Empire in \a empire_ids. */
struct FO_COMMON_API Condition::VisibleToEmpire : public Condition::ConditionBase {
    VisibleToEmpire(const ValueRef::ValueRefBase<int>* empire_id) :
        m_empire_id(empire_id)
    {}
    virtual ~VisibleToEmpire();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<int>*  EmpireID() const { return m_empire_id; }

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    const ValueRef::ValueRefBase<int>* m_empire_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that are within \a distance units of at least one
  * object that meets \a condition.  Warning: this Condition can slow things
  * down considerably if overused.  It is best to use Conditions that yield
  * relatively few matches. */
struct FO_COMMON_API Condition::WithinDistance : public Condition::ConditionBase {
    WithinDistance(const ValueRef::ValueRefBase<double>* distance, const ConditionBase* condition) :
        m_distance(distance),
        m_condition(condition)
    {}
    virtual ~WithinDistance();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
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
struct FO_COMMON_API Condition::WithinStarlaneJumps : public Condition::ConditionBase {
    WithinStarlaneJumps(const ValueRef::ValueRefBase<int>* jumps, const ConditionBase* condition) :
        m_jumps(jumps),
        m_condition(condition)
    {}
    virtual ~WithinStarlaneJumps();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
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
struct Condition::CanAddStarlaneConnection :  Condition::ConditionBase {
    CanAddStarlaneConnection(const ConditionBase* condition) :
        m_condition(condition)
    {}
    virtual ~CanAddStarlaneConnection();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
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
struct FO_COMMON_API Condition::ExploredByEmpire : public Condition::ConditionBase {
    ExploredByEmpire(const ValueRef::ValueRefBase<int>* empire_id) :
        m_empire_id(empire_id)
    {}
    virtual ~ExploredByEmpire();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    const ValueRef::ValueRefBase<int>* m_empire_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches objects that are moving. ... What does that mean?  Departing this
  * turn, or were located somewhere else last turn...? */
struct FO_COMMON_API Condition::Stationary : public Condition::ConditionBase {
    Stationary() : ConditionBase() {}
    virtual ~Stationary() {}
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual bool        RootCandidateInvariant() const { return true; }
    virtual bool        TargetInvariant() const { return true; }
    virtual bool        SourceInvariant() const { return true; }
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
struct FO_COMMON_API Condition::FleetSupplyableByEmpire : public Condition::ConditionBase {
    FleetSupplyableByEmpire(const ValueRef::ValueRefBase<int>* empire_id) :
        m_empire_id(empire_id)
    {}
    virtual ~FleetSupplyableByEmpire();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
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
struct FO_COMMON_API Condition::ResourceSupplyConnectedByEmpire : public Condition::ConditionBase {
    ResourceSupplyConnectedByEmpire(const ValueRef::ValueRefBase<int>* empire_id, const ConditionBase* condition) :
        m_empire_id(empire_id),
        m_condition(condition)
    {}
    virtual ~ResourceSupplyConnectedByEmpire();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
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

/** Matches objects whose species has the ability to found new colonies. */
struct FO_COMMON_API Condition::CanColonize : public Condition::ConditionBase {
    CanColonize() : ConditionBase() {}
    virtual ~CanColonize() {}
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual bool        RootCandidateInvariant() const { return true; }
    virtual bool        TargetInvariant() const { return true; }
    virtual bool        SourceInvariant() const { return true; }
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches objects whose species has the ability to produce ships. */
struct FO_COMMON_API Condition::CanProduceShips : public Condition::ConditionBase {
    CanProduceShips() : ConditionBase() {}
    virtual ~CanProduceShips() {}
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual bool        RootCandidateInvariant() const { return true; }
    virtual bool        TargetInvariant() const { return true; }
    virtual bool        SourceInvariant() const { return true; }
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches the objects that have been targeted for bombardment by at least one
  * object that matches \a m_by_object_condition. */
struct FO_COMMON_API Condition::OrderedBombarded : public Condition::ConditionBase {
    OrderedBombarded(const ConditionBase* by_object_condition) :
        m_by_object_condition(by_object_condition)
    {}
    virtual ~OrderedBombarded();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    const ConditionBase*    m_by_object_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects if the evaluation of \a value_ref returns a value
  * within the range between \a low and \a high. */
struct FO_COMMON_API Condition::ValueTest : public Condition::ConditionBase {
    ValueTest(const ValueRef::ValueRefBase<double>* value_ref,
              const ValueRef::ValueRefBase<double>* low,
              const ValueRef::ValueRefBase<double>* high) :
        m_value_ref(value_ref),
        m_low(low),
        m_high(high)
    {}
    virtual ~ValueTest();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<double>*  GetValueRef() const { return m_value_ref; }
    const ValueRef::ValueRefBase<double>*  Low() const { return m_low; }
    const ValueRef::ValueRefBase<double>*  High() const { return m_high; }

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    const ValueRef::ValueRefBase<double>*   m_value_ref;
    const ValueRef::ValueRefBase<double>*   m_low;
    const ValueRef::ValueRefBase<double>*   m_high;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that match every Condition in \a operands. */
struct FO_COMMON_API Condition::And : public Condition::ConditionBase {
    And(const std::vector<const ConditionBase*>& operands) :
        m_operands(operands)
    {}
    virtual ~And();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const std::vector<const ConditionBase*>&
                        Operands() const { return m_operands; }
    virtual void        GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context, Condition::ObjectSet& condition_non_targets) const;

private:
    std::vector<const ConditionBase*> m_operands;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that match at least one Condition in \a operands. */
struct FO_COMMON_API Condition::Or : public Condition::ConditionBase {
    Or(const std::vector<const ConditionBase*>& operands) :
        m_operands(operands)
    {}
    virtual ~Or();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const std::vector<const ConditionBase*>&
                        Operands() const { return m_operands; }

private:
    std::vector<const ConditionBase*> m_operands;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that do not match the Condition \a operand. */
struct FO_COMMON_API Condition::Not : public Condition::ConditionBase {
    Not(const ConditionBase* operand) :
        m_operand(operand)
    {}
    virtual ~Not();
    virtual bool        operator==(const Condition::ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                             Condition::ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ConditionBase*Operand() const { return m_operand; }

private:
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
{ ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase); }

template <class Archive>
void Condition::EmpireAffiliation::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_affiliation);
}

template <class Archive>
void Condition::Source::serialize(Archive& ar, const unsigned int version)
{ ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase); }

template <class Archive>
void Condition::RootCandidate::serialize(Archive& ar, const unsigned int version)
{ ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase); }

template <class Archive>
void Condition::Target::serialize(Archive& ar, const unsigned int version)
{ ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase); }

template <class Archive>
void Condition::Homeworld::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_names);
}

template <class Archive>
void Condition::Capital::serialize(Archive& ar, const unsigned int version)
{ ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase); }

template <class Archive>
void Condition::Monster::serialize(Archive& ar, const unsigned int version)
{ ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase); }

template <class Archive>
void Condition::Armed::serialize(Archive& ar, const unsigned int version)
{ ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase); }

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
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_since_turn_low)
        & BOOST_SERIALIZATION_NVP(m_since_turn_high);
}

template <class Archive>
void Condition::HasTag::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <class Archive>
void Condition::CreatedOnTurn::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_low)
        & BOOST_SERIALIZATION_NVP(m_high);
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
void Condition::InSystem::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_system_id);
}

template <class Archive>
void Condition::ObjectID::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_object_id);
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
void Condition::Enqueued::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_build_type)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_design_id)
        & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_low)
        & BOOST_SERIALIZATION_NVP(m_high);
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
void Condition::PredefinedShipDesign::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <class Archive>
void Condition::NumberedShipDesign::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_design_id);
}

template <class Archive>
void Condition::ProducedByEmpire::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
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
void Condition::OwnerHasBuildingTypeAvailable::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <class Archive>
void Condition::OwnerHasShipDesignAvailable::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_id);
}

template <class Archive>
void Condition::VisibleToEmpire::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
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
void Condition::CanAddStarlaneConnection::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <class Archive>
void Condition::ExploredByEmpire::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
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
void Condition::CanColonize::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase);
}

template <class Archive>
void Condition::CanProduceShips::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase);
}

template <class Archive>
void Condition::OrderedBombarded::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_by_object_condition);
}

template <class Archive>
void Condition::ValueTest::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_value_ref)
        & BOOST_SERIALIZATION_NVP(m_low)
        & BOOST_SERIALIZATION_NVP(m_high);
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
