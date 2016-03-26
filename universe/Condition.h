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

enum ComparisonType {
    INVALID_COMPARISON = -1,
    EQUAL,
    GREATER_THAN,
    GREATER_THAN_OR_EQUAL,
    LESS_THAN,
    LESS_THAN_OR_EQUAL,
    NOT_EQUAL
};

enum ContentType {
    CONTENT_BUILDING,
    CONTENT_SPECIES,
    CONTENT_SHIP_HULL,
    CONTENT_SHIP_PART,
    CONTENT_SPECIAL,
    CONTENT_FOCUS
};

struct ConditionBase;

/** Same as ConditionDescription, but returns a string only with conditions that have not been met. */
FO_COMMON_API std::string ConditionFailedDescription(const std::vector<ConditionBase*>& conditions,
                                                     TemporaryPtr<const UniverseObject> candidate_object = TemporaryPtr<const UniverseObject>(),
                                                     TemporaryPtr<const UniverseObject> source_object = TemporaryPtr<const UniverseObject>());

/** Returns a single string which describes a vector of Conditions. If multiple
  * conditions are passed, they are treated as if they were contained by an And
  * condition. Subconditions within an And (or nested And) are listed as
  * lines in a list, with duplicates removed, titled something like "All of...".
  * Subconditions within an Or (or nested Ors) are similarly listed as lines in
  * a list, with duplicates removed, titled something like "One of...". If a
  * candidate object is provided, the returned string will indicate which
  * subconditions the candidate matches, and indicate if the overall combination
  * of conditions matches the object. */
FO_COMMON_API std::string ConditionDescription(const std::vector<ConditionBase*>& conditions,
                                               TemporaryPtr<const UniverseObject> candidate_object = TemporaryPtr<const UniverseObject>(),
                                               TemporaryPtr<const UniverseObject> source_object = TemporaryPtr<const UniverseObject>());

/** The base class for all Conditions. */
struct FO_COMMON_API ConditionBase {
    ConditionBase() :
        m_root_candidate_invariant(UNKNOWN_INVARIANCE),
        m_target_invariant(UNKNOWN_INVARIANCE),
        m_source_invariant(UNKNOWN_INVARIANCE)
    {}
    virtual ~ConditionBase();

    virtual bool        operator==(const ConditionBase& rhs) const;
    bool                operator!=(const ConditionBase& rhs) const { return !(*this == rhs); }

    virtual void        Eval(const ScriptingContext& parent_context,
                             ObjectSet& matches,
                             ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const;

    /** Matches with an empty ScriptingContext */
    void                Eval(ObjectSet& matches,
                             ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const;

    /** Tests all objects in universe as NON_MATCHES. */
    void                Eval(const ScriptingContext& parent_context,
                             ObjectSet& matches) const;

    /** Tests all objects in universe as NON_MATCHES with empty ScriptingContext. */
    void                Eval(ObjectSet& matches) const;

    /** Tests single candidate object, returning true iff it matches condition. */
    bool                Eval(const ScriptingContext& parent_context,
                             TemporaryPtr<const UniverseObject> candidate) const;

    /** Tests single candidate object, returning true iff it matches condition
      * with empty ScriptingContext. */
    bool                Eval(TemporaryPtr<const UniverseObject> candidate) const;

    virtual void        GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                          ObjectSet& condition_non_targets) const;

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

    virtual std::string Description(bool negated = false) const = 0;
    virtual std::string Dump() const = 0;

    virtual void        SetTopLevelContent(const std::string& content_name) = 0;

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
struct FO_COMMON_API Number : public ConditionBase {
    Number(ValueRef::ValueRefBase<int>* low, ValueRef::ValueRefBase<int>* high,
           ConditionBase* condition) :
        m_low(low),
        m_high(high),
        m_condition(condition)
    {}
    virtual ~Number();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<int>*  Low() const { return m_low; }
    const ValueRef::ValueRefBase<int>*  High() const { return m_high; }
    const ConditionBase*                GetCondition() const { return m_condition; }

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ValueRef::ValueRefBase<int>*    m_low;
    ValueRef::ValueRefBase<int>*    m_high;
    ConditionBase*                  m_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects if the current game turn is >= \a low and < \a high. */
struct FO_COMMON_API Turn : public ConditionBase {
    explicit Turn(ValueRef::ValueRefBase<int>* low, ValueRef::ValueRefBase<int>* high = 0) :
        m_low(low),
        m_high(high)
    {}
    virtual ~Turn();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<int>*  Low() const { return m_low; }
    const ValueRef::ValueRefBase<int>*  High() const { return m_high; }

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ValueRef::ValueRefBase<int>*    m_low;
    ValueRef::ValueRefBase<int>*    m_high;

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
struct FO_COMMON_API SortedNumberOf : public ConditionBase {
    /** Sorts randomly, without considering a sort key. */
    SortedNumberOf(ValueRef::ValueRefBase<int>* number,
                   ConditionBase* condition) :
        m_number(number),
        m_sort_key(0),
        m_sorting_method(SORT_RANDOM),
        m_condition(condition)
    {}

    /** Sorts according to the specified method, based on the key values
      * evaluated for each object. */
    SortedNumberOf(ValueRef::ValueRefBase<int>* number,
                   ValueRef::ValueRefBase<double>* sort_key_ref,
                   SortingMethod sorting_method,
                   ConditionBase* condition) :
        m_number(number),
        m_sort_key(sort_key_ref),
        m_sorting_method(sorting_method),
        m_condition(condition)
    {}
    virtual ~SortedNumberOf();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<int>*      Number() const { return m_number; }
    const ValueRef::ValueRefBase<double>*   SortKey() const { return m_sort_key; }
    SortingMethod                           GetSortingMethod() const { return m_sorting_method; }
    const ConditionBase*                    GetCondition() const { return m_condition; }
    virtual void                            GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                                              ObjectSet& condition_non_targets) const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    ValueRef::ValueRefBase<int>*    m_number;
    ValueRef::ValueRefBase<double>* m_sort_key;
    SortingMethod                   m_sorting_method;
    ConditionBase*                  m_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects. */
struct FO_COMMON_API All : public ConditionBase {
    All() : ConditionBase() {}
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    virtual bool        RootCandidateInvariant() const { return true; }
    virtual bool        TargetInvariant() const { return true; }
    virtual bool        SourceInvariant() const { return true; }

    virtual void        SetTopLevelContent(const std::string& content_name) {}

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches no objects. Currently only has an experimental use for efficient immediate rejection as the top-line condition.
 *  Essentially the entire point of this Condition is to provide the specialized GetDefaultInitialCandidateObjects() */
struct FO_COMMON_API None : public ConditionBase {
    None() : ConditionBase() {}
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    virtual bool        RootCandidateInvariant() const { return true; }
    virtual bool        TargetInvariant() const { return true; }
    virtual bool        SourceInvariant() const { return true; }

    virtual void        SetTopLevelContent(const std::string& content_name) {}
    virtual void        GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                          ObjectSet& condition_non_targets) const { }  // efficient rejection of everything


private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that are owned (if \a exclusive == false) or only owned
  * (if \a exclusive == true) by an empire that has affilitation type
  * \a affilitation with Empire \a empire_id. */
struct FO_COMMON_API EmpireAffiliation : public ConditionBase {
    explicit EmpireAffiliation(ValueRef::ValueRefBase<int>* empire_id, EmpireAffiliationType affiliation = AFFIL_SELF) :
        m_empire_id(empire_id),
        m_affiliation(affiliation)
    {}
    explicit EmpireAffiliation(EmpireAffiliationType affiliation) :
       m_empire_id(0),
       m_affiliation(affiliation)
    {}
    virtual ~EmpireAffiliation();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

    const ValueRef::ValueRefBase<int>*  EmpireID() const { return m_empire_id; }
    EmpireAffiliationType               GetAffiliation() const { return m_affiliation; }

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ValueRef::ValueRefBase<int>*    m_empire_id;
    EmpireAffiliationType           m_affiliation;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches the source object only. */
struct FO_COMMON_API Source : public ConditionBase {
    Source() : ConditionBase() {}
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual bool        RootCandidateInvariant() const { return true; }
    virtual bool        TargetInvariant() const { return true; }
    //virtual bool        SourceInvariant() const { return false; } // same as ConditionBase
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    virtual void        GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                          ObjectSet& condition_non_targets) const;

    virtual void        SetTopLevelContent(const std::string& content_name) {}

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
struct FO_COMMON_API RootCandidate : public ConditionBase {
    RootCandidate() : ConditionBase() {}
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual bool        RootCandidateInvariant() const { return false; }
    virtual bool        TargetInvariant() const { return true; }
    virtual bool        SourceInvariant() const { return true; }
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name) {}

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** There is no LocalCandidate condition.  To match any local candidate object,
  * use the All condition. */

/** Matches the target of an effect being executed. */
struct FO_COMMON_API Target : public ConditionBase {
    Target() : ConditionBase() {}
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual bool        RootCandidateInvariant() const { return true; }
    virtual bool        TargetInvariant() const { return false; }
    virtual bool        SourceInvariant() const { return true; }
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    virtual void        GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                          ObjectSet& condition_non_targets) const;

    virtual void        SetTopLevelContent(const std::string& content_name) {}

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches planets that are a homeworld for any of the species specified in
  * \a names.  If \a names is empty, matches any planet that is a homeworld for
  * any species in the current game Universe. */
struct FO_COMMON_API Homeworld : public ConditionBase {
    Homeworld() :
        ConditionBase(),
        m_names()
    {}
    explicit Homeworld(const std::vector<ValueRef::ValueRefBase<std::string>*>& names) :
        ConditionBase(),
        m_names(names)
    {}
    virtual ~Homeworld();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const std::vector<ValueRef::ValueRefBase<std::string>*>   Names() const { return m_names; }
    virtual void        GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                          ObjectSet& condition_non_targets) const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::vector<ValueRef::ValueRefBase<std::string>*> m_names;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches planets that are an empire's capital. */
struct FO_COMMON_API Capital : public ConditionBase {
    Capital() : ConditionBase() {}
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual bool        RootCandidateInvariant() const { return true; }
    virtual bool        TargetInvariant() const { return true; }
    virtual bool        SourceInvariant() const { return true; }
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    virtual void        GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                          ObjectSet& condition_non_targets) const;

    virtual void        SetTopLevelContent(const std::string& content_name) {}

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches space monsters. */
struct FO_COMMON_API Monster : public ConditionBase {
    Monster() : ConditionBase() {}
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual bool        RootCandidateInvariant() const { return true; }
    virtual bool        TargetInvariant() const { return true; }
    virtual bool        SourceInvariant() const { return true; }
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    virtual void        GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                          ObjectSet& condition_non_targets) const;

    virtual void        SetTopLevelContent(const std::string& content_name) {}

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches armed ships and monsters. */
struct FO_COMMON_API Armed : public ConditionBase {
    Armed() : ConditionBase() {}
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual bool        RootCandidateInvariant() const { return true; }
    virtual bool        TargetInvariant() const { return true; }
    virtual bool        SourceInvariant() const { return true; }
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name) {}

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that are of UniverseObjectType \a type. */
struct FO_COMMON_API Type : public ConditionBase {
    explicit Type(ValueRef::ValueRefBase<UniverseObjectType>* type) :
        ConditionBase(),
        m_type(type)
    {}
    virtual ~Type();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<UniverseObjectType>*   GetType() const { return m_type; }
    virtual void        GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                          ObjectSet& condition_non_targets) const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ValueRef::ValueRefBase<UniverseObjectType>* m_type;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all Building objects that are one of the building types specified
  * in \a names. */
struct FO_COMMON_API Building : public ConditionBase {
    explicit Building(const std::vector<ValueRef::ValueRefBase<std::string>*>& names) :
        ConditionBase(),
        m_names(names)
    {}
    virtual ~Building();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const std::vector<ValueRef::ValueRefBase<std::string>*>   Names() const { return m_names; }
    virtual void        GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                          ObjectSet& condition_non_targets) const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::vector<ValueRef::ValueRefBase<std::string>*> m_names;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that have an attached Special named \a name. */
struct FO_COMMON_API HasSpecial : public ConditionBase {
    explicit HasSpecial(const std::string& name);
    explicit HasSpecial(ValueRef::ValueRefBase<std::string>* name = 0) :
        ConditionBase(),
        m_name(name),
        m_capacity_low(0),
        m_capacity_high(0),
        m_since_turn_low(0),
        m_since_turn_high(0)
    {}
    HasSpecial(ValueRef::ValueRefBase<std::string>* name,
               ValueRef::ValueRefBase<int>* since_turn_low,
               ValueRef::ValueRefBase<int>* since_turn_high = 0) :
        ConditionBase(),
        m_name(name),
        m_capacity_low(0),
        m_capacity_high(0),
        m_since_turn_low(since_turn_low),
        m_since_turn_high(since_turn_high)
    {}
    HasSpecial(ValueRef::ValueRefBase<std::string>* name,
               ValueRef::ValueRefBase<double>* capacity_low,
               ValueRef::ValueRefBase<double>* capacity_high = 0) :
        ConditionBase(),
        m_name(name),
        m_capacity_low(capacity_low),
        m_capacity_high(capacity_high),
        m_since_turn_low(0),
        m_since_turn_high(0)
    {}
    virtual ~HasSpecial();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<std::string>*  Name() const { return m_name; }
    const ValueRef::ValueRefBase<double>*       CapacityLow() const { return m_capacity_low; }
    const ValueRef::ValueRefBase<double>*       CapacityHigh() const { return m_capacity_high; }
    const ValueRef::ValueRefBase<int>*          SinceTurnLow() const { return m_since_turn_low; }
    const ValueRef::ValueRefBase<int>*          SinceTurnHigh() const { return m_since_turn_high; }

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ValueRef::ValueRefBase<std::string>*    m_name;
    ValueRef::ValueRefBase<double>*         m_capacity_low;
    ValueRef::ValueRefBase<double>*         m_capacity_high;
    ValueRef::ValueRefBase<int>*            m_since_turn_low;
    ValueRef::ValueRefBase<int>*            m_since_turn_high;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that have the tag \a tag. */
struct FO_COMMON_API HasTag : public ConditionBase {
    explicit HasTag(const std::string& name);
    explicit HasTag(ValueRef::ValueRefBase<std::string>* name = 0) :
        ConditionBase(),
        m_name(name)
    {}
    virtual ~HasTag();

    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

    ValueRef::ValueRefBase<std::string>*    Name() const { return m_name; }

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ValueRef::ValueRefBase<std::string>*    m_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that were created on turns within the specified range. */
struct FO_COMMON_API CreatedOnTurn : public ConditionBase {
    CreatedOnTurn(ValueRef::ValueRefBase<int>* low, ValueRef::ValueRefBase<int>* high) :
        ConditionBase(),
        m_low(low),
        m_high(high)
    {}
    virtual ~CreatedOnTurn();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<int>*  Low() const { return m_low; }
    const ValueRef::ValueRefBase<int>*  High() const { return m_high; }

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ValueRef::ValueRefBase<int>*    m_low;
    ValueRef::ValueRefBase<int>*    m_high;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that contain an object that matches Condition
  * \a condition.  Container objects are Systems, Planets (which contain
  * Buildings), and Fleets (which contain Ships). */
struct FO_COMMON_API Contains : public ConditionBase {
    Contains(ConditionBase* condition) :
        ConditionBase(),
        m_condition(condition)
    {}
    virtual ~Contains();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ConditionBase*GetCondition() const { return m_condition; }
    virtual void        GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                          ObjectSet& condition_non_targets) const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ConditionBase*  m_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that are contained by an object that matches Condition
  * \a condition.  Container objects are Systems, Planets (which contain
  * Buildings), and Fleets (which contain Ships). */
struct FO_COMMON_API ContainedBy : public ConditionBase {
    ContainedBy(ConditionBase* condition) :
        ConditionBase(),
        m_condition(condition)
    {}
    virtual ~ContainedBy();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ConditionBase*GetCondition() const { return m_condition; }
    virtual void        GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                          ObjectSet& condition_non_targets) const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ConditionBase* m_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that are in the system with the indicated \a system_id */
struct FO_COMMON_API InSystem : public ConditionBase {
    InSystem(ValueRef::ValueRefBase<int>* system_id) :
        ConditionBase(),
        m_system_id(system_id)
    {}
    virtual ~InSystem();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<int>*  SystemId() const { return m_system_id; }
    virtual void        GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                          ObjectSet& condition_non_targets) const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ValueRef::ValueRefBase<int>* m_system_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches the object with the id \a object_id */
struct FO_COMMON_API ObjectID : public ConditionBase {
    ObjectID(ValueRef::ValueRefBase<int>* object_id) :
        ConditionBase(),
        m_object_id(object_id)
    {}
    virtual ~ObjectID();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<int>*  ObjectId() const { return m_object_id; }
    virtual void        GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                          ObjectSet& condition_non_targets) const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ValueRef::ValueRefBase<int>* m_object_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all Planet objects that have one of the PlanetTypes in \a types.
  * Note that all Building objects which are on matching planets are also
  * matched. */
struct FO_COMMON_API PlanetType : public ConditionBase {
    PlanetType(const std::vector<ValueRef::ValueRefBase< ::PlanetType>*>& types) :
        ConditionBase(),
        m_types(types)
    {}
    virtual ~PlanetType();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const std::vector<ValueRef::ValueRefBase< ::PlanetType>*>&    Types() const { return m_types; }
    void                GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                          ObjectSet& condition_non_targets) const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::vector<ValueRef::ValueRefBase< ::PlanetType>*> m_types;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all Planet objects that have one of the PlanetSizes in \a sizes.
  * Note that all Building objects which are on matching planets are also
  * matched. */
struct FO_COMMON_API PlanetSize : public ConditionBase {
    PlanetSize(const std::vector<ValueRef::ValueRefBase< ::PlanetSize>*>& sizes) :
        ConditionBase(),
        m_sizes(sizes)
    {}
    virtual ~PlanetSize();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const std::vector<ValueRef::ValueRefBase< ::PlanetSize>*>&    Sizes() const { return m_sizes; }
    void                GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                          ObjectSet& condition_non_targets) const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::vector<ValueRef::ValueRefBase< ::PlanetSize>*> m_sizes;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all Planet objects that have one of the PlanetEnvironments in
  * \a environments.  Note that all Building objects which are on matching
  * planets are also matched. */
struct FO_COMMON_API PlanetEnvironment : public ConditionBase {
    PlanetEnvironment(const std::vector<ValueRef::ValueRefBase< ::PlanetEnvironment>*>& environments,
                      ValueRef::ValueRefBase<std::string>* species_name_ref = 0) :
        ConditionBase(),
        m_environments(environments),
        m_species_name(species_name_ref)
    {}
    virtual ~PlanetEnvironment();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const std::vector<ValueRef::ValueRefBase< ::PlanetEnvironment>*>& Environments() const { return m_environments; }
    void                GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                          ObjectSet& condition_non_targets) const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::vector<ValueRef::ValueRefBase< ::PlanetEnvironment>*>  m_environments;
    ValueRef::ValueRefBase<std::string>*                        m_species_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all planets or ships that have one of the species in \a species.
  * Note that all Building object which are on matching planets are also
  * matched. */
struct FO_COMMON_API Species : public ConditionBase {
    Species(const std::vector<ValueRef::ValueRefBase<std::string>*>& names) :
        ConditionBase(),
        m_names(names)
    {}
    Species() :
        ConditionBase(),
        m_names()
    {}
    virtual ~Species();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const std::vector<ValueRef::ValueRefBase<std::string>*>&  Names() const { return m_names; }
    void                GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                          ObjectSet& condition_non_targets) const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::vector<ValueRef::ValueRefBase<std::string>*> m_names;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches planets where the indicated number of the indicated building type
  * or ship design are enqueued on the production queue. */
struct FO_COMMON_API Enqueued : public ConditionBase {
    Enqueued(BuildType build_type,
             ValueRef::ValueRefBase<std::string>* name,
             ValueRef::ValueRefBase<int>* empire_id = 0,
             ValueRef::ValueRefBase<int>* low = 0,
             ValueRef::ValueRefBase<int>* high = 0) :
        ConditionBase(),
        m_build_type(build_type),
        m_name(name),
        m_design_id(0),
        m_empire_id(empire_id),
        m_low(low),
        m_high(high)
    {}
    explicit Enqueued(ValueRef::ValueRefBase<int>* design_id,
             ValueRef::ValueRefBase<int>* empire_id = 0,
             ValueRef::ValueRefBase<int>* low = 0,
             ValueRef::ValueRefBase<int>* high = 0) :
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
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    BuildType           GetBuildType() const { return m_build_type; }
    const ValueRef::ValueRefBase<std::string>*  GetName() const { return m_name; }
    const ValueRef::ValueRefBase<int>*          DesignID() const { return m_design_id; }
    const ValueRef::ValueRefBase<int>*          EmpireID() const { return m_empire_id; }
    const ValueRef::ValueRefBase<int>*          Low() const { return m_low; }
    const ValueRef::ValueRefBase<int>*          High() const { return m_high; }
    void                GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                          ObjectSet& condition_non_targets) const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    BuildType                               m_build_type;
    ValueRef::ValueRefBase<std::string>*    m_name;
    ValueRef::ValueRefBase<int>*            m_design_id;
    ValueRef::ValueRefBase<int>*            m_empire_id;
    ValueRef::ValueRefBase<int>*            m_low;
    ValueRef::ValueRefBase<int>*            m_high;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all ProdCenter objects that have one of the FocusTypes in \a foci. */
struct FO_COMMON_API FocusType : public ConditionBase {
    FocusType(const std::vector<ValueRef::ValueRefBase<std::string>*>& names) :
        ConditionBase(),
        m_names(names)
    {}
    virtual ~FocusType();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const std::vector<ValueRef::ValueRefBase<std::string>*>&  Names() const { return m_names; }
    void                GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                          ObjectSet& condition_non_targets) const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::vector<ValueRef::ValueRefBase<std::string>*> m_names;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all System objects that have one of the StarTypes in \a types.  Note that all objects
    in matching Systems are also matched (Ships, Fleets, Buildings, Planets, etc.). */
struct FO_COMMON_API StarType : public ConditionBase {
    StarType(const std::vector<ValueRef::ValueRefBase< ::StarType>*>& types) :
        ConditionBase(),
        m_types(types)
    {}
    virtual ~StarType();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const std::vector<ValueRef::ValueRefBase< ::StarType>*>&  Types() const { return m_types; }

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    std::vector<ValueRef::ValueRefBase< ::StarType>*> m_types;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all ships whose ShipDesign has the hull specified by \a name. */
struct FO_COMMON_API DesignHasHull : public ConditionBase {
    explicit DesignHasHull(ValueRef::ValueRefBase<std::string>* name) :
        ConditionBase(),
        m_name(name)
    {}
    virtual ~DesignHasHull();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<std::string>*  Name() const { return m_name; }
    void                GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                          ObjectSet& condition_non_targets) const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ValueRef::ValueRefBase<std::string>*    m_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all ships whose ShipDesign has >= \a low and < \a high of the ship
  * part specified by \a name. */
struct FO_COMMON_API DesignHasPart : public ConditionBase {
    DesignHasPart(ValueRef::ValueRefBase<std::string>* name, ValueRef::ValueRefBase<int>* low = 0,
                  ValueRef::ValueRefBase<int>* high = 0) :
        ConditionBase(),
        m_low(low),
        m_high(high),
        m_name(name)
    {}
    virtual ~DesignHasPart();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<int>*          Low() const  { return m_low; }
    const ValueRef::ValueRefBase<int>*          High() const { return m_high; }
    const ValueRef::ValueRefBase<std::string>*  Name() const { return m_name; }
    void                GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                          ObjectSet& condition_non_targets) const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ValueRef::ValueRefBase<int>*            m_low;
    ValueRef::ValueRefBase<int>*            m_high;
    ValueRef::ValueRefBase<std::string>*    m_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches ships whose ShipDesign has >= \a low and < \a high of ship parts of
  * the specified \a part_class */
struct FO_COMMON_API DesignHasPartClass : public ConditionBase {
    DesignHasPartClass(ShipPartClass part_class, ValueRef::ValueRefBase<int>* low,
                       ValueRef::ValueRefBase<int>* high) :
        ConditionBase(),
        m_low(low),
        m_high(high),
        m_class(part_class)
    {}
    virtual ~DesignHasPartClass();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<int>*  Low() const { return m_low; }
    const ValueRef::ValueRefBase<int>*  High() const { return m_high; }
    ShipPartClass                       Class() const { return m_class; }
    void                GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                          ObjectSet& condition_non_targets) const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ValueRef::ValueRefBase<int>*    m_low;
    ValueRef::ValueRefBase<int>*    m_high;
    ShipPartClass                   m_class;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches ships who ShipDesign is a predefined shipdesign with the name
  * \a name */
struct FO_COMMON_API PredefinedShipDesign : public ConditionBase {
    explicit PredefinedShipDesign(ValueRef::ValueRefBase<std::string>* name) :
        ConditionBase(),
        m_name(name)
    {}
    virtual ~PredefinedShipDesign();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

    const ValueRef::ValueRefBase<std::string>*  Name() const { return m_name; }

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ValueRef::ValueRefBase<std::string>*    m_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches ships whose design id \a id. */
struct FO_COMMON_API NumberedShipDesign : public ConditionBase {
    NumberedShipDesign(ValueRef::ValueRefBase<int>* design_id) :
        ConditionBase(),
        m_design_id(design_id)
    {}
    virtual ~NumberedShipDesign();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<int>*  DesignID() const { return m_design_id; }

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ValueRef::ValueRefBase<int>* m_design_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches ships or buildings produced by the empire with id \a empire_id.*/
struct FO_COMMON_API ProducedByEmpire : public ConditionBase {
    ProducedByEmpire(ValueRef::ValueRefBase<int>* empire_id) :
        ConditionBase(),
        m_empire_id(empire_id)
    {}
    virtual ~ProducedByEmpire();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<int>*  EmpireID() const { return m_empire_id; }

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ValueRef::ValueRefBase<int>* m_empire_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches a given object with a linearly distributed probability of \a chance. */
struct FO_COMMON_API Chance : public ConditionBase {
    Chance(ValueRef::ValueRefBase<double>* chance) :
        ConditionBase(),
        m_chance(chance)
    {}
    virtual ~Chance();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<double>*   GetChance() const { return m_chance; }

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ValueRef::ValueRefBase<double>* m_chance;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that have a meter of type \a meter, and whose current
  * value is >= \a low and <= \a high. */
struct FO_COMMON_API MeterValue : public ConditionBase {
    MeterValue(MeterType meter, ValueRef::ValueRefBase<double>* low,
               ValueRef::ValueRefBase<double>* high) :
        ConditionBase(),
        m_meter(meter),
        m_low(low),
        m_high(high)
    {}
    virtual ~MeterValue();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<double>*   Low() const { return m_low; }
    const ValueRef::ValueRefBase<double>*   High() const { return m_high; }
    MeterType                               GetMeterType() const { return m_meter; }

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    MeterType                       m_meter;
    ValueRef::ValueRefBase<double>* m_low;
    ValueRef::ValueRefBase<double>* m_high;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches ships that have a ship part meter of type \a meter for part \a part
  * whose current value is >= low and <= high. */
struct FO_COMMON_API ShipPartMeterValue : public ConditionBase {
    ShipPartMeterValue(ValueRef::ValueRefBase<std::string>* ship_part_name,
                       MeterType meter,
                       ValueRef::ValueRefBase<double>* low,
                       ValueRef::ValueRefBase<double>* high) :
        ConditionBase(),
        m_part_name(ship_part_name),
        m_meter(meter),
        m_low(low),
        m_high(high)
    {}
    virtual ~ShipPartMeterValue();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<std::string>*  PartName() const { return m_part_name; }
    const ValueRef::ValueRefBase<double>*       Low() const { return m_low; }
    const ValueRef::ValueRefBase<double>*       High() const { return m_high; }
    MeterType                                   GetMeterType() const { return m_meter; }

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ValueRef::ValueRefBase<std::string>*    m_part_name;
    MeterType                               m_meter;
    ValueRef::ValueRefBase<double>*         m_low;
    ValueRef::ValueRefBase<double>*         m_high;
};

/** Matches all objects if the empire with id \a empire_id has an empire meter
  * \a meter whose current value is >= \a low and <= \a high. */
struct FO_COMMON_API EmpireMeterValue : public ConditionBase {
    EmpireMeterValue(const std::string& meter,
                     ValueRef::ValueRefBase<double>* low,
                     ValueRef::ValueRefBase<double>* high) :
        ConditionBase(),
        m_empire_id(0),
        m_meter(meter),
        m_low(low),
        m_high(high)
    {}
    EmpireMeterValue(ValueRef::ValueRefBase<int>* empire_id,
                     const std::string& meter,
                     ValueRef::ValueRefBase<double>* low,
                     ValueRef::ValueRefBase<double>* high) :
        ConditionBase(),
        m_empire_id(empire_id),
        m_meter(meter),
        m_low(low),
        m_high(high)
    {}
    virtual ~EmpireMeterValue();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
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

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ValueRef::ValueRefBase<int>*    m_empire_id;
    const std::string               m_meter;
    ValueRef::ValueRefBase<double>* m_low;
    ValueRef::ValueRefBase<double>* m_high;
};

/** Matches all objects whose owner's stockpile of \a stockpile is between
  * \a low and \a high, inclusive. */
struct FO_COMMON_API EmpireStockpileValue : public ConditionBase {
    EmpireStockpileValue(ResourceType stockpile, ValueRef::ValueRefBase<double>* low,
                         ValueRef::ValueRefBase<double>* high) :
        ConditionBase(),
        m_stockpile(stockpile),
        m_low(low),
        m_high(high)
    {}
    virtual ~EmpireStockpileValue();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<double>*   Low() const { return m_low; }
    const ValueRef::ValueRefBase<double>*   High() const { return m_high; }
    ResourceType                            Stockpile() const { return m_stockpile; }

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ResourceType                    m_stockpile;
    ValueRef::ValueRefBase<double>* m_low;
    ValueRef::ValueRefBase<double>* m_high;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects whose owner who has tech \a name. */
struct FO_COMMON_API OwnerHasTech : public ConditionBase {
    explicit OwnerHasTech(ValueRef::ValueRefBase<std::string>* name) :
        ConditionBase(),
        m_name(name)
    {}
    virtual ~OwnerHasTech();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

    const ValueRef::ValueRefBase<std::string>*  Tech() const { return m_name; }

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ValueRef::ValueRefBase<std::string>* m_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects whose owner who has the building type \a name available. */
struct FO_COMMON_API OwnerHasBuildingTypeAvailable : public ConditionBase {
    explicit OwnerHasBuildingTypeAvailable(const std::string& name);
    explicit OwnerHasBuildingTypeAvailable(ValueRef::ValueRefBase<std::string>* name) :
        ConditionBase(),
        m_name(name)
    {}
    virtual ~OwnerHasBuildingTypeAvailable();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

    const ValueRef::ValueRefBase<std::string>*  GetBuildingType() const { return m_name; }

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ValueRef::ValueRefBase<std::string>*    m_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects whose owner who has the ship design \a id available. */
struct FO_COMMON_API OwnerHasShipDesignAvailable : public ConditionBase {
    explicit OwnerHasShipDesignAvailable(int id);
    explicit OwnerHasShipDesignAvailable(ValueRef::ValueRefBase<int>* id) :
        ConditionBase(),
        m_id(id)
    {}
    virtual ~OwnerHasShipDesignAvailable();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

    const ValueRef::ValueRefBase<int>*  GetDesignID() const { return m_id; }

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ValueRef::ValueRefBase<int>*    m_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that are visible to at least one Empire in \a empire_ids. */
struct FO_COMMON_API VisibleToEmpire : public ConditionBase {
    explicit VisibleToEmpire(ValueRef::ValueRefBase<int>* empire_id) :
        ConditionBase(),
        m_empire_id(empire_id)
    {}
    virtual ~VisibleToEmpire();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<int>*  EmpireID() const { return m_empire_id; }

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ValueRef::ValueRefBase<int>* m_empire_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that are within \a distance units of at least one
  * object that meets \a condition.  Warning: this Condition can slow things
  * down considerably if overused.  It is best to use Conditions that yield
  * relatively few matches. */
struct FO_COMMON_API WithinDistance : public ConditionBase {
    WithinDistance(ValueRef::ValueRefBase<double>* distance, ConditionBase* condition) :
        ConditionBase(),
        m_distance(distance),
        m_condition(condition)
    {}
    virtual ~WithinDistance();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ValueRef::ValueRefBase<double>* m_distance;
    ConditionBase*                  m_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that are within \a jumps starlane jumps of at least one
  * object that meets \a condition.  Warning: this Condition can slow things
  * down considerably if overused.  It is best to use Conditions that yield
  * relatively few matches. */
struct FO_COMMON_API WithinStarlaneJumps : public ConditionBase {
    WithinStarlaneJumps(ValueRef::ValueRefBase<int>* jumps, ConditionBase* condition) :
        ConditionBase(),
        m_jumps(jumps),
        m_condition(condition)
    {}
    virtual ~WithinStarlaneJumps();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ValueRef::ValueRefBase<int>* m_jumps;
    ConditionBase*               m_condition;

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
struct FO_COMMON_API CanAddStarlaneConnection :  ConditionBase {
    explicit CanAddStarlaneConnection(ConditionBase* condition) :
        ConditionBase(),
        m_condition(condition)
    {}
    virtual ~CanAddStarlaneConnection();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ConditionBase*      m_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches systems that have been explored by at least one Empire
  * in \a empire_ids. */
struct FO_COMMON_API ExploredByEmpire : public ConditionBase {
    explicit ExploredByEmpire(ValueRef::ValueRefBase<int>* empire_id) :
        ConditionBase(),
        m_empire_id(empire_id)
    {}
    virtual ~ExploredByEmpire();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ValueRef::ValueRefBase<int>* m_empire_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches objects that are moving. ... What does that mean?  Departing this
  * turn, or were located somewhere else last turn...? */
struct FO_COMMON_API Stationary : public ConditionBase {
    explicit Stationary() : ConditionBase() {}
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual bool        RootCandidateInvariant() const { return true; }
    virtual bool        TargetInvariant() const { return true; }
    virtual bool        SourceInvariant() const { return true; }
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name) {}

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches objects that are in systems that can be fleet supplied by the
  * empire with id \a empire_id */
struct FO_COMMON_API FleetSupplyableByEmpire : public ConditionBase {
    explicit FleetSupplyableByEmpire(ValueRef::ValueRefBase<int>* empire_id) :
        ConditionBase(),
        m_empire_id(empire_id)
    {}
    virtual ~FleetSupplyableByEmpire();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ValueRef::ValueRefBase<int>*  m_empire_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches objects that are in systems that are connected by resource-sharing
  * to at least one object that meets \a condition using the resource-sharing
  * network of the empire with id \a empire_id */
struct FO_COMMON_API ResourceSupplyConnectedByEmpire : public ConditionBase {
    ResourceSupplyConnectedByEmpire(ValueRef::ValueRefBase<int>* empire_id, ConditionBase* condition) :
        ConditionBase(),
        m_empire_id(empire_id),
        m_condition(condition)
    {}
    virtual ~ResourceSupplyConnectedByEmpire();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ValueRef::ValueRefBase<int>*  m_empire_id;
    ConditionBase*                m_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches objects whose species has the ability to found new colonies. */
struct FO_COMMON_API CanColonize : public ConditionBase {
    explicit CanColonize() : ConditionBase() {}
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual bool        RootCandidateInvariant() const { return true; }
    virtual bool        TargetInvariant() const { return true; }
    virtual bool        SourceInvariant() const { return true; }
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name) {}

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches objects whose species has the ability to produce ships. */
struct FO_COMMON_API CanProduceShips : public ConditionBase {
    CanProduceShips() : ConditionBase() {}
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual bool        RootCandidateInvariant() const { return true; }
    virtual bool        TargetInvariant() const { return true; }
    virtual bool        SourceInvariant() const { return true; }
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name) {}

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches the objects that have been targeted for bombardment by at least one
  * object that matches \a m_by_object_condition. */
struct FO_COMMON_API OrderedBombarded : public ConditionBase {
    OrderedBombarded(ConditionBase* by_object_condition) :
        ConditionBase(),
        m_by_object_condition(by_object_condition)
    {}
    virtual ~OrderedBombarded();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ConditionBase*      m_by_object_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects if the comparisons between values of ValueRefs meet the
  * specified comparison types. */
struct FO_COMMON_API ValueTest : public ConditionBase {
    ValueTest(ValueRef::ValueRefBase<double>* value_ref1,
              ComparisonType comp1,
              ValueRef::ValueRefBase<double>* value_ref2,
              ComparisonType comp2 = INVALID_COMPARISON,
              ValueRef::ValueRefBase<double>* value_ref3 = 0) :
        ConditionBase(),
        m_value_ref1(value_ref1),
        m_value_ref2(value_ref2),
        m_value_ref3(value_ref3),
        m_compare_type1(comp1),
        m_compare_type2(comp2)
    {}
    virtual ~ValueTest();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<double>*   GetValueRef1() const { return m_value_ref1; }
    const ValueRef::ValueRefBase<double>*   GetValueRef2() const { return m_value_ref2; }
    const ValueRef::ValueRefBase<double>*   GetValueRef3() const { return m_value_ref3; }
    ComparisonType                          GetComparisonType1() const { return m_compare_type1; }
    ComparisonType                          GetComparisonType2() const { return m_compare_type2; }

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ValueRef::ValueRefBase<double>* m_value_ref1;
    ValueRef::ValueRefBase<double>* m_value_ref2;
    ValueRef::ValueRefBase<double>* m_value_ref3;
    ComparisonType                  m_compare_type1;
    ComparisonType                  m_compare_type2;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches objects that match the location condition of the specified
  * content.  */
struct FO_COMMON_API Location : public ConditionBase {
public:
    Location(ContentType content_type, ValueRef::ValueRefBase<std::string>* name1,
             ValueRef::ValueRefBase<std::string>* name2 = 0) :
        ConditionBase(),
        m_name1(name1),
        m_name2(name2),
        m_content_type(content_type)
    {}
    virtual ~Location();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ValueRef::ValueRefBase<std::string>*  GetName1() const { return m_name1; }
    const ValueRef::ValueRefBase<std::string>*  GetName2() const { return m_name2; }

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    virtual bool        Match(const ScriptingContext& local_context) const;

    ValueRef::ValueRefBase<std::string>*    m_name1;
    ValueRef::ValueRefBase<std::string>*    m_name2;
    ContentType                             m_content_type;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that match every Condition in \a operands. */
struct FO_COMMON_API And : public ConditionBase {
    And(const std::vector<ConditionBase*>& operands) :
        ConditionBase(),
        m_operands(operands)
    {}
    virtual ~And();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const std::vector<ConditionBase*>&
                        Operands() const { return m_operands; }
    virtual void        GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                          ObjectSet& condition_non_targets) const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    std::vector<ConditionBase*> m_operands;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that match at least one Condition in \a operands. */
struct FO_COMMON_API Or : public ConditionBase {
    Or(const std::vector<ConditionBase*>& operands) :
        ConditionBase(),
        m_operands(operands)
    {}
    virtual ~Or();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const std::vector<ConditionBase*>&
                        Operands() const { return m_operands; }

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    std::vector<ConditionBase*> m_operands;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that do not match the Condition \a operand. */
struct FO_COMMON_API Not : public ConditionBase {
    Not(ConditionBase* operand) :
        ConditionBase(),
        m_operand(operand)
    {}
    virtual ~Not();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
    const ConditionBase*Operand() const { return m_operand; }

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    ConditionBase* m_operand;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches whatever its subcondition matches, but has a customized description
  * string that is returned by Description() by looking up in the stringtable. */
struct FO_COMMON_API Described : public ConditionBase {
    Described(ConditionBase* condition, const std::string& desc_stringtable_key) :
        ConditionBase(),
        m_condition(condition),
        m_desc_stringtable_key(desc_stringtable_key)
    {}
    virtual ~Described();
    virtual bool        operator==(const ConditionBase& rhs) const;
    virtual void        Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const;
    void                Eval(ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain = NON_MATCHES) const { ConditionBase::Eval(matches, non_matches, search_domain); }
    virtual bool        RootCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const                    { return m_condition ? m_condition->Dump() : ""; }
    const ConditionBase*SubCondition() const            { return m_condition; }
    const std::string&  DescriptionStringKey() const    { return m_desc_stringtable_key; }

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    ConditionBase*  m_condition;
    std::string     m_desc_stringtable_key;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// template implementations
template <class Archive>
void ConditionBase::serialize(Archive& ar, const unsigned int version)
{}

template <class Archive>
void Number::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_low)
        & BOOST_SERIALIZATION_NVP(m_high)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <class Archive>
void Turn::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_low)
        & BOOST_SERIALIZATION_NVP(m_high);
}

template <class Archive>
void SortedNumberOf::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_number)
        & BOOST_SERIALIZATION_NVP(m_sort_key)
        & BOOST_SERIALIZATION_NVP(m_sorting_method)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <class Archive>
void All::serialize(Archive& ar, const unsigned int version)
{ ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase); }

template <class Archive>
void None::serialize(Archive& ar, const unsigned int version)
{ ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase); }

template <class Archive>
void EmpireAffiliation::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_affiliation);
}

template <class Archive>
void Source::serialize(Archive& ar, const unsigned int version)
{ ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase); }

template <class Archive>
void RootCandidate::serialize(Archive& ar, const unsigned int version)
{ ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase); }

template <class Archive>
void Target::serialize(Archive& ar, const unsigned int version)
{ ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase); }

template <class Archive>
void Homeworld::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_names);
}

template <class Archive>
void Capital::serialize(Archive& ar, const unsigned int version)
{ ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase); }

template <class Archive>
void Monster::serialize(Archive& ar, const unsigned int version)
{ ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase); }

template <class Archive>
void Armed::serialize(Archive& ar, const unsigned int version)
{ ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase); }

template <class Archive>
void Type::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_type);
}

template <class Archive>
void Building::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_names);
}

template <class Archive>
void HasSpecial::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_capacity_low)
        & BOOST_SERIALIZATION_NVP(m_capacity_high)
        & BOOST_SERIALIZATION_NVP(m_since_turn_low)
        & BOOST_SERIALIZATION_NVP(m_since_turn_high);
}

template <class Archive>
void HasTag::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <class Archive>
void CreatedOnTurn::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_low)
        & BOOST_SERIALIZATION_NVP(m_high);
}

template <class Archive>
void Contains::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <class Archive>
void ContainedBy::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <class Archive>
void InSystem::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_system_id);
}

template <class Archive>
void ObjectID::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_object_id);
}

template <class Archive>
void PlanetType::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_types);
}

template <class Archive>
void PlanetSize::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_sizes);
}

template <class Archive>
void PlanetEnvironment::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_environments)
        & BOOST_SERIALIZATION_NVP(m_species_name);
}

template <class Archive>
void Species::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_names);
}

template <class Archive>
void Enqueued::serialize(Archive& ar, const unsigned int version)
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
void FocusType::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_names);
}

template <class Archive>
void StarType::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_types);
}

template <class Archive>
void DesignHasHull::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <class Archive>
void DesignHasPart::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_low)
        & BOOST_SERIALIZATION_NVP(m_high)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <class Archive>
void DesignHasPartClass::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_low)
        & BOOST_SERIALIZATION_NVP(m_high)
        & BOOST_SERIALIZATION_NVP(m_class);
}

template <class Archive>
void PredefinedShipDesign::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <class Archive>
void NumberedShipDesign::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_design_id);
}

template <class Archive>
void ProducedByEmpire::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template <class Archive>
void Chance::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_chance);
}

template <class Archive>
void MeterValue::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_meter)
        & BOOST_SERIALIZATION_NVP(m_low)
        & BOOST_SERIALIZATION_NVP(m_high);
}

template <class Archive>
void EmpireStockpileValue::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_low)
        & BOOST_SERIALIZATION_NVP(m_high);
}

template <class Archive>
void OwnerHasTech::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <class Archive>
void OwnerHasBuildingTypeAvailable::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <class Archive>
void OwnerHasShipDesignAvailable::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_id);
}

template <class Archive>
void VisibleToEmpire::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template <class Archive>
void WithinDistance::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_distance)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <class Archive>
void WithinStarlaneJumps::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_jumps)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <class Archive>
void CanAddStarlaneConnection::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <class Archive>
void ExploredByEmpire::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template <class Archive>
void Stationary::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase);
}

template <class Archive>
void FleetSupplyableByEmpire::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template <class Archive>
void ResourceSupplyConnectedByEmpire::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <class Archive>
void CanColonize::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase);
}

template <class Archive>
void CanProduceShips::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase);
}

template <class Archive>
void OrderedBombarded::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_by_object_condition);
}

template <class Archive>
void ValueTest::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_value_ref1)
        & BOOST_SERIALIZATION_NVP(m_value_ref2)
        & BOOST_SERIALIZATION_NVP(m_value_ref3)
        & BOOST_SERIALIZATION_NVP(m_compare_type1)
        & BOOST_SERIALIZATION_NVP(m_compare_type2);
}

template <class Archive>
void Location::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_name1)
        & BOOST_SERIALIZATION_NVP(m_name2)
        & BOOST_SERIALIZATION_NVP(m_content_type);
}

template <class Archive>
void And::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_operands);
}

template <class Archive>
void Or::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_operands);
}

template <class Archive>
void Not::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_operand);
}

template <class Archive>
void Described::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_condition)
        & BOOST_SERIALIZATION_NVP(m_desc_stringtable_key);
}
} // namespace Condition

#endif // _Condition_h_
