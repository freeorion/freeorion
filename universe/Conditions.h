#ifndef _Conditions_h_
#define _Conditions_h_


#include "EnumsFwd.h"
#include "Condition.h"
#include "ConditionSource.h"
#include "ConditionAll.h"

#include "../util/Export.h"
#include "../util/CheckSums.h"

#include <boost/serialization/nvp.hpp>

#include <memory>
#include <string>
#include <vector>


namespace ValueRef {
    template <typename T>
    struct ValueRef;
}

/** this namespace holds Condition and its subclasses; these classes
  * represent predicates about UniverseObjects used by, for instance, the
  * Effect system. */
namespace Condition {

enum SortingMethod : int {
    SORT_MAX,       ///< Objects with the largest sort key will be selected
    SORT_MIN,       ///< Objects with the smallest sort key will be selected
    SORT_MODE,      ///< Objects with the most common sort key will be selected
    SORT_RANDOM     ///< Objects will be selected randomly, without consideration of property values
};

enum ComparisonType : int {
    INVALID_COMPARISON = -1,
    EQUAL,
    GREATER_THAN,
    GREATER_THAN_OR_EQUAL,
    LESS_THAN,
    LESS_THAN_OR_EQUAL,
    NOT_EQUAL
};

enum ContentType : int {
    CONTENT_BUILDING,
    CONTENT_SPECIES,
    CONTENT_SHIP_HULL,
    CONTENT_SHIP_PART,
    CONTENT_SPECIAL,
    CONTENT_FOCUS
};

/** Same as ConditionDescription, but returns a string only with conditions that have not been met. */
FO_COMMON_API std::string ConditionFailedDescription(const std::vector<Condition*>& conditions,
                                                     std::shared_ptr<const UniverseObject> candidate_object = nullptr,
                                                     std::shared_ptr<const UniverseObject> source_object = nullptr);

/** Returns a single string which describes a vector of Conditions. If multiple
  * conditions are passed, they are treated as if they were contained by an And
  * condition. Subconditions within an And (or nested And) are listed as
  * lines in a list, with duplicates removed, titled something like "All of...".
  * Subconditions within an Or (or nested Ors) are similarly listed as lines in
  * a list, with duplicates removed, titled something like "One of...". If a
  * candidate object is provided, the returned string will indicate which
  * subconditions the candidate matches, and indicate if the overall combination
  * of conditions matches the object. */
FO_COMMON_API std::string ConditionDescription(const std::vector<Condition*>& conditions,
                                               std::shared_ptr<const UniverseObject> candidate_object = nullptr,
                                               std::shared_ptr<const UniverseObject> source_object = nullptr);

/** Matches all objects if the number of objects that match Condition
  * \a condition is is >= \a low and < \a high.  Matched objects may
  * or may not themselves match the condition. */
struct FO_COMMON_API Number final : public Condition {
    Number(std::unique_ptr<ValueRef::ValueRef<int>>&& low,
           std::unique_ptr<ValueRef::ValueRef<int>>&& high,
           std::unique_ptr<Condition>&& condition);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    bool RootCandidateInvariant() const override;
    bool TargetInvariant() const override;
    bool SourceInvariant() const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_low;
    std::unique_ptr<ValueRef::ValueRef<int>> m_high;
    std::unique_ptr<Condition> m_condition;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects if the current game turn is >= \a low and < \a high. */
struct FO_COMMON_API Turn final : public Condition {
    explicit Turn(std::unique_ptr<ValueRef::ValueRef<int>>&& low,
                  std::unique_ptr<ValueRef::ValueRef<int>>&& high = nullptr);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    bool RootCandidateInvariant() const override;
    bool TargetInvariant() const override;
    bool SourceInvariant() const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_low;
    std::unique_ptr<ValueRef::ValueRef<int>> m_high;

    friend class boost::serialization::access;
    template <typename Archive>
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
struct FO_COMMON_API SortedNumberOf final : public Condition {
    /** Sorts randomly, without considering a sort key. */
    SortedNumberOf(std::unique_ptr<ValueRef::ValueRef<int>>&& number,
                   std::unique_ptr<Condition>&& condition);

    /** Sorts according to the specified method, based on the key values
      * evaluated for each object. */
    SortedNumberOf(std::unique_ptr<ValueRef::ValueRef<int>>&& number,
                   std::unique_ptr<ValueRef::ValueRef<double>>&& sort_key_ref,
                   SortingMethod sorting_method,
                   std::unique_ptr<Condition>&& condition);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    void GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                           ObjectSet& condition_non_targets) const override;
    bool RootCandidateInvariant() const override;
    bool TargetInvariant() const override;
    bool SourceInvariant() const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    std::unique_ptr<ValueRef::ValueRef<int>> m_number;
    std::unique_ptr<ValueRef::ValueRef<double>> m_sort_key;
    SortingMethod m_sorting_method;
    std::unique_ptr<Condition> m_condition;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches no objects. Currently only has an experimental use for efficient immediate rejection as the top-line condition.
 *  Essentially the entire point of this Condition is to provide the specialized GetDefaultInitialCandidateObjects() */
struct FO_COMMON_API None final : public Condition {
    None();
    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    void GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                           ObjectSet& condition_non_targets) const override
    { /* efficient rejection of everything. */ }
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override
    {}
    unsigned int GetCheckSum() const override;

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that are owned (if \a exclusive == false) or only owned
  * (if \a exclusive == true) by an empire that has affilitation type
  * \a affilitation with Empire \a empire_id. */
struct FO_COMMON_API EmpireAffiliation final : public Condition {
    EmpireAffiliation(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id, EmpireAffiliationType affiliation);
    explicit EmpireAffiliation(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id);
    explicit EmpireAffiliation(EmpireAffiliationType affiliation);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_empire_id;
    EmpireAffiliationType m_affiliation;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches the root candidate object in a condition tree.  This is useful
  * within a subcondition to match the object actually being matched by the
  * whole compound condition, rather than an object just being matched in a
  * subcondition in order to evaluate the outer condition. */
struct FO_COMMON_API RootCandidate final : public Condition {
    RootCandidate();
    bool operator==(const Condition& rhs) const override;
    void GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                           ObjectSet& condition_non_targets) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override
    {}
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** There is no LocalCandidate condition. To match any local candidate object,
  * use the All condition. */

/** Matches the target of an effect being executed. */
struct FO_COMMON_API Target final : public Condition {
    Target();
    bool operator==(const Condition& rhs) const override;
    void GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                           ObjectSet& condition_non_targets) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override
    {}
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches planets that are a homeworld for any of the species specified in
  * \a names.  If \a names is empty, matches any planet that is a homeworld for
  * any species in the current game Universe. */
struct FO_COMMON_API Homeworld final : public Condition {
    Homeworld();
    explicit Homeworld(std::vector<std::unique_ptr<ValueRef::ValueRef<std::string>>>&& names);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    void GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                           ObjectSet& condition_non_targets) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::vector<std::unique_ptr<ValueRef::ValueRef<std::string>>> m_names;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches planets that are an empire's capital. */
struct FO_COMMON_API Capital final : public Condition {
    Capital();
    bool operator==(const Condition& rhs) const override;
    void GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                           ObjectSet& condition_non_targets) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override
    {}
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches space monsters. */
struct FO_COMMON_API Monster final : public Condition {
    Monster();
    bool operator==(const Condition& rhs) const override;
    void GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                           ObjectSet& condition_non_targets) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override
    {}
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches armed ships and monsters. */
struct FO_COMMON_API Armed final : public Condition {
    Armed();
    bool operator==(const Condition& rhs) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override
    {}
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that are of UniverseObjectType \a type. */
struct FO_COMMON_API Type final : public Condition {
    explicit Type(std::unique_ptr<ValueRef::ValueRef<UniverseObjectType>>&& type);
    explicit Type(UniverseObjectType type);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    void GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                           ObjectSet& condition_non_targets) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<UniverseObjectType>> m_type;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all Building objects that are one of the building types specified
  * in \a names. */
struct FO_COMMON_API Building final : public Condition {
    explicit Building(std::vector<std::unique_ptr<ValueRef::ValueRef<std::string>>>&& names);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    void GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                           ObjectSet& condition_non_targets) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::vector<std::unique_ptr<ValueRef::ValueRef<std::string>>> m_names;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that have an attached Special named \a name. */
struct FO_COMMON_API HasSpecial final : public Condition {
    explicit HasSpecial();
    explicit HasSpecial(const std::string& name);
    explicit HasSpecial(std::unique_ptr<ValueRef::ValueRef<std::string>>&& name);
    explicit HasSpecial(ValueRef::ValueRef<std::string>* name);
    HasSpecial(std::unique_ptr<ValueRef::ValueRef<std::string>>&& name,
               std::unique_ptr<ValueRef::ValueRef<int>>&& since_turn_low,
               std::unique_ptr<ValueRef::ValueRef<int>>&& since_turn_high = nullptr);
    HasSpecial(std::unique_ptr<ValueRef::ValueRef<std::string>>&& name,
               std::unique_ptr<ValueRef::ValueRef<double>>&& capacity_low,
               std::unique_ptr<ValueRef::ValueRef<double>>&& capacity_high = nullptr);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<std::string>>    m_name;
    std::unique_ptr<ValueRef::ValueRef<double>>         m_capacity_low;
    std::unique_ptr<ValueRef::ValueRef<double>>         m_capacity_high;
    std::unique_ptr<ValueRef::ValueRef<int>>            m_since_turn_low;
    std::unique_ptr<ValueRef::ValueRef<int>>            m_since_turn_high;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that have the tag \a tag. */
struct FO_COMMON_API HasTag final : public Condition {
    HasTag();
    explicit HasTag(const std::string& name);
    explicit HasTag(std::unique_ptr<ValueRef::ValueRef<std::string>>&& name);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<std::string>> m_name;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that were created on turns within the specified range. */
struct FO_COMMON_API CreatedOnTurn final : public Condition {
    CreatedOnTurn(std::unique_ptr<ValueRef::ValueRef<int>>&& low,
                  std::unique_ptr<ValueRef::ValueRef<int>>&& high);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    bool RootCandidateInvariant() const override;
    bool TargetInvariant() const override;
    bool SourceInvariant() const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_low;
    std::unique_ptr<ValueRef::ValueRef<int>> m_high;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that contain an object that matches Condition
  * \a condition.  Container objects are Systems, Planets (which contain
  * Buildings), and Fleets (which contain Ships). */
struct FO_COMMON_API Contains final : public Condition {
    Contains(std::unique_ptr<Condition>&& condition);
    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    void GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                           ObjectSet& condition_non_targets) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<Condition> m_condition;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that are contained by an object that matches Condition
  * \a condition.  Container objects are Systems, Planets (which contain
  * Buildings), and Fleets (which contain Ships). */
struct FO_COMMON_API ContainedBy final : public Condition {
    ContainedBy(std::unique_ptr<Condition>&& condition);
    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    void GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                           ObjectSet& condition_non_targets) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<Condition> m_condition;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that are in the system with the indicated \a system_id */
struct FO_COMMON_API InSystem final : public Condition {
    InSystem(std::unique_ptr<ValueRef::ValueRef<int>>&& system_id);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    void GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                           ObjectSet& condition_non_targets) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_system_id;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches the object with the id \a object_id */
struct FO_COMMON_API ObjectID final : public Condition {
    ObjectID(std::unique_ptr<ValueRef::ValueRef<int>>&& object_id);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    void GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                           ObjectSet& condition_non_targets) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_object_id;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all Planet objects that have one of the PlanetTypes in \a types.
  * Note that all Building objects which are on matching planets are also
  * matched. */
struct FO_COMMON_API PlanetType final : public Condition {
    PlanetType(std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetType>>>&& types);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    void GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                           ObjectSet& condition_non_targets) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::vector<std::unique_ptr<ValueRef::ValueRef<::PlanetType>>> m_types;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all Planet objects that have one of the PlanetSizes in \a sizes.
  * Note that all Building objects which are on matching planets are also
  * matched. */
struct FO_COMMON_API PlanetSize final : public Condition {
    PlanetSize(std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetSize>>>&& sizes);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    void GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                           ObjectSet& condition_non_targets) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::vector<std::unique_ptr<ValueRef::ValueRef<::PlanetSize>>> m_sizes;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all Planet objects that have one of the PlanetEnvironments in
  * \a environments.  Note that all Building objects which are on matching
  * planets are also matched. */
struct FO_COMMON_API PlanetEnvironment final : public Condition {
    PlanetEnvironment(std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetEnvironment>>>&& environments,
                      std::unique_ptr<ValueRef::ValueRef<std::string>>&& species_name_ref = nullptr);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    void GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                           ObjectSet& condition_non_targets) const override;
    bool RootCandidateInvariant() const override;
    bool TargetInvariant() const override;
    bool SourceInvariant() const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::vector<std::unique_ptr<ValueRef::ValueRef<::PlanetEnvironment>>> m_environments;
    std::unique_ptr<ValueRef::ValueRef<std::string>> m_species_name;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all planets or ships that have one of the species in \a species.
  * Note that all Building object which are on matching planets are also
  * matched. */
struct FO_COMMON_API Species final : public Condition {
    explicit Species(std::vector<std::unique_ptr<ValueRef::ValueRef<std::string>>>&& names);
    Species();

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    void GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                           ObjectSet& condition_non_targets) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::vector<std::unique_ptr<ValueRef::ValueRef<std::string>>> m_names;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches planets where the indicated number of the indicated building type
  * or ship design are enqueued on the production queue. */
struct FO_COMMON_API Enqueued final : public Condition {
    Enqueued(BuildType build_type,
             std::unique_ptr<ValueRef::ValueRef<std::string>>&& name,
             std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id = nullptr,
             std::unique_ptr<ValueRef::ValueRef<int>>&& low = nullptr,
             std::unique_ptr<ValueRef::ValueRef<int>>&& high = nullptr);
    explicit Enqueued(std::unique_ptr<ValueRef::ValueRef<int>>&& design_id,
                      std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id = nullptr,
                      std::unique_ptr<ValueRef::ValueRef<int>>&& low = nullptr,
                      std::unique_ptr<ValueRef::ValueRef<int>>&& high = nullptr);
    Enqueued();

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    void GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                           ObjectSet& condition_non_targets) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    BuildType m_build_type;
    std::unique_ptr<ValueRef::ValueRef<std::string>>    m_name;
    std::unique_ptr<ValueRef::ValueRef<int>>            m_design_id;
    std::unique_ptr<ValueRef::ValueRef<int>>            m_empire_id;
    std::unique_ptr<ValueRef::ValueRef<int>>            m_low;
    std::unique_ptr<ValueRef::ValueRef<int>>            m_high;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all ProdCenter objects that have one of the FocusTypes in \a foci. */
struct FO_COMMON_API FocusType final : public Condition {
    FocusType(std::vector<std::unique_ptr<ValueRef::ValueRef<std::string>>>&& names);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                           ObjectSet& condition_non_targets) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::vector<std::unique_ptr<ValueRef::ValueRef<std::string>>> m_names;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all System objects that have one of the StarTypes in \a types.  Note that all objects
    in matching Systems are also matched (Ships, Fleets, Buildings, Planets, etc.). */
struct FO_COMMON_API StarType final : public Condition {
    StarType(std::vector<std::unique_ptr<ValueRef::ValueRef< ::StarType>>>&& types);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::vector<std::unique_ptr<ValueRef::ValueRef<::StarType>>> m_types;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all ships whose ShipDesign has the hull specified by \a name. */
struct FO_COMMON_API DesignHasHull final : public Condition {
    explicit DesignHasHull(std::unique_ptr<ValueRef::ValueRef<std::string>>&& name);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    void GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                           ObjectSet& condition_non_targets) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<std::string>> m_name;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all ships whose ShipDesign has >= \a low and < \a high of the ship
  * part specified by \a name. */
struct FO_COMMON_API DesignHasPart final : public Condition {
    DesignHasPart(std::unique_ptr<ValueRef::ValueRef<std::string>>&& name,
                  std::unique_ptr<ValueRef::ValueRef<int>>&& low = nullptr,
                  std::unique_ptr<ValueRef::ValueRef<int>>&& high = nullptr);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    void GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                           ObjectSet& condition_non_targets) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_low;
    std::unique_ptr<ValueRef::ValueRef<int>> m_high;
    std::unique_ptr<ValueRef::ValueRef<std::string>> m_name;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches ships whose ShipDesign has >= \a low and < \a high of ship parts of
  * the specified \a part_class */
struct FO_COMMON_API DesignHasPartClass final : public Condition {
    DesignHasPartClass(ShipPartClass part_class,
                       std::unique_ptr<ValueRef::ValueRef<int>>&& low,
                       std::unique_ptr<ValueRef::ValueRef<int>>&& high);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    void GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                           ObjectSet& condition_non_targets) const override;
    bool RootCandidateInvariant() const override;
    bool TargetInvariant() const override;
    bool SourceInvariant() const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_low;
    std::unique_ptr<ValueRef::ValueRef<int>> m_high;
    ShipPartClass m_class;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches ships who ShipDesign is a predefined shipdesign with the name
  * \a name */
struct FO_COMMON_API PredefinedShipDesign final : public Condition {
    explicit PredefinedShipDesign(std::unique_ptr<ValueRef::ValueRef<std::string>>&& name);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<std::string>> m_name;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches ships whose design id \a id. */
struct FO_COMMON_API NumberedShipDesign final : public Condition {
    NumberedShipDesign(std::unique_ptr<ValueRef::ValueRef<int>>&& design_id);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_design_id;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches ships or buildings produced by the empire with id \a empire_id.*/
struct FO_COMMON_API ProducedByEmpire final : public Condition {
    ProducedByEmpire(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_empire_id;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches a given object with a linearly distributed probability of \a chance. */
struct FO_COMMON_API Chance final : public Condition {
    Chance(std::unique_ptr<ValueRef::ValueRef<double>>&& chance);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<double>> m_chance;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that have a meter of type \a meter, and whose current
  * value is >= \a low and <= \a high. */
struct FO_COMMON_API MeterValue final : public Condition {
    MeterValue(MeterType meter,
               std::unique_ptr<ValueRef::ValueRef<double>>&& low,
               std::unique_ptr<ValueRef::ValueRef<double>>&& high);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    bool RootCandidateInvariant() const override;
    bool TargetInvariant() const override;
    bool SourceInvariant() const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    MeterType m_meter;
    std::unique_ptr<ValueRef::ValueRef<double>> m_low;
    std::unique_ptr<ValueRef::ValueRef<double>> m_high;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches ships that have a ship part meter of type \a meter for part \a part
  * whose current value is >= low and <= high. */
struct FO_COMMON_API ShipPartMeterValue final : public Condition {
    ShipPartMeterValue(std::unique_ptr<ValueRef::ValueRef<std::string>>&& ship_part_name,
                       MeterType meter,
                       std::unique_ptr<ValueRef::ValueRef<double>>&& low,
                       std::unique_ptr<ValueRef::ValueRef<double>>&& high);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    bool RootCandidateInvariant() const override;
    bool TargetInvariant() const override;
    bool SourceInvariant() const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<std::string>> m_part_name;
    MeterType m_meter;
    std::unique_ptr<ValueRef::ValueRef<double>> m_low;
    std::unique_ptr<ValueRef::ValueRef<double>> m_high;
};

/** Matches all objects if the empire with id \a empire_id has an empire meter
  * \a meter whose current value is >= \a low and <= \a high. */
struct FO_COMMON_API EmpireMeterValue final : public Condition {
    EmpireMeterValue(const std::string& meter,
                     std::unique_ptr<ValueRef::ValueRef<double>>&& low,
                     std::unique_ptr<ValueRef::ValueRef<double>>&& high);
    EmpireMeterValue(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id,
                     const std::string& meter,
                     std::unique_ptr<ValueRef::ValueRef<double>>&& low,
                     std::unique_ptr<ValueRef::ValueRef<double>>&& high);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    bool RootCandidateInvariant() const override;
    bool TargetInvariant() const override;
    bool SourceInvariant() const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_empire_id;
    const std::string m_meter;
    std::unique_ptr<ValueRef::ValueRef<double>> m_low;
    std::unique_ptr<ValueRef::ValueRef<double>> m_high;
};

/** Matches all objects whose owner's stockpile of \a stockpile is between
  * \a low and \a high, inclusive. */
struct FO_COMMON_API EmpireStockpileValue final : public Condition {
    EmpireStockpileValue(ResourceType stockpile,
                         std::unique_ptr<ValueRef::ValueRef<double>>&& low,
                         std::unique_ptr<ValueRef::ValueRef<double>>&& high);
    EmpireStockpileValue(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id,
                         ResourceType stockpile,
                         std::unique_ptr<ValueRef::ValueRef<double>>&& low,
                         std::unique_ptr<ValueRef::ValueRef<double>>&& high);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    bool RootCandidateInvariant() const override;
    bool TargetInvariant() const override;
    bool SourceInvariant() const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>>    m_empire_id;
    ResourceType                                m_stockpile;
    std::unique_ptr<ValueRef::ValueRef<double>> m_low;
    std::unique_ptr<ValueRef::ValueRef<double>> m_high;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects whose owner who has tech \a name. */
struct FO_COMMON_API OwnerHasTech final : public Condition {
    OwnerHasTech(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id,
                 std::unique_ptr<ValueRef::ValueRef<std::string>>&& name);
    explicit OwnerHasTech(std::unique_ptr<ValueRef::ValueRef<std::string>>&& name);

    bool            operator==(const Condition& rhs) const override;
    void            Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                         ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    bool            RootCandidateInvariant() const override;
    bool            TargetInvariant() const override;
    bool            SourceInvariant() const override;
    std::string     Description(bool negated = false) const override;
    std::string     Dump(unsigned short ntabs = 0) const override;
    void            SetTopLevelContent(const std::string& content_name) override;
    unsigned int    GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<std::string>> m_name;
    std::unique_ptr<ValueRef::ValueRef<int>>         m_empire_id;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects whose owner who has the building type \a name available. */
struct FO_COMMON_API OwnerHasBuildingTypeAvailable final : public Condition {
    OwnerHasBuildingTypeAvailable(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id,
                                  std::unique_ptr<ValueRef::ValueRef<std::string>>&& name);
    explicit OwnerHasBuildingTypeAvailable(const std::string& name);
    explicit OwnerHasBuildingTypeAvailable(std::unique_ptr<ValueRef::ValueRef<std::string>>&& name);

    bool            operator==(const Condition& rhs) const override;
    void            Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                         ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    bool            RootCandidateInvariant() const override;
    bool            TargetInvariant() const override;
    bool            SourceInvariant() const override;
    std::string     Description(bool negated = false) const override;
    std::string     Dump(unsigned short ntabs = 0) const override;
    void            SetTopLevelContent(const std::string& content_name) override;
    unsigned int    GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<std::string>> m_name;
    std::unique_ptr<ValueRef::ValueRef<int>>         m_empire_id;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects whose owner who has the ship design \a id available. */
struct FO_COMMON_API OwnerHasShipDesignAvailable final : public Condition {
    OwnerHasShipDesignAvailable(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id,
                                std::unique_ptr<ValueRef::ValueRef<int>>&& design_id);
    explicit OwnerHasShipDesignAvailable(int design_id);
    explicit OwnerHasShipDesignAvailable(std::unique_ptr<ValueRef::ValueRef<int>>&& design_id);

    bool            operator==(const Condition& rhs) const override;
    void            Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                         ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    bool            RootCandidateInvariant() const override;
    bool            TargetInvariant() const override;
    bool            SourceInvariant() const override;
    std::string     Description(bool negated = false) const override;
    std::string     Dump(unsigned short ntabs = 0) const override;
    void            SetTopLevelContent(const std::string& content_name) override;
    unsigned int    GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_id;
    std::unique_ptr<ValueRef::ValueRef<int>> m_empire_id;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects whose owner who has the ship part @a name available. */
struct FO_COMMON_API OwnerHasShipPartAvailable final : public Condition {
    OwnerHasShipPartAvailable(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id,
                              std::unique_ptr<ValueRef::ValueRef<std::string>>&& name);
    explicit OwnerHasShipPartAvailable(const std::string& name);
    explicit OwnerHasShipPartAvailable(std::unique_ptr<ValueRef::ValueRef<std::string>>&& name);

    bool            operator==(const Condition& rhs) const override;
    void            Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                         ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    bool            RootCandidateInvariant() const override;
    bool            TargetInvariant() const override;
    bool            SourceInvariant() const override;
    std::string     Description(bool negated = false) const override;
    std::string     Dump(unsigned short ntabs = 0) const override;
    void            SetTopLevelContent(const std::string& content_name) override;
    unsigned int    GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<std::string>> m_name;
    std::unique_ptr<ValueRef::ValueRef<int>>         m_empire_id;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that are visible to at least one Empire in \a empire_ids. */
struct FO_COMMON_API VisibleToEmpire final : public Condition {
    explicit VisibleToEmpire(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_empire_id;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that are within \a distance units of at least one
  * object that meets \a condition.  Warning: this Condition can slow things
  * down considerably if overused.  It is best to use Conditions that yield
  * relatively few matches. */
struct FO_COMMON_API WithinDistance final : public Condition {
    WithinDistance(std::unique_ptr<ValueRef::ValueRef<double>>&& distance,
                   std::unique_ptr<Condition>&& condition);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    bool RootCandidateInvariant() const override;
    bool TargetInvariant() const override;
    bool SourceInvariant() const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<double>> m_distance;
    std::unique_ptr<Condition> m_condition;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that are within \a jumps starlane jumps of at least one
  * object that meets \a condition.  Warning: this Condition can slow things
  * down considerably if overused.  It is best to use Conditions that yield
  * relatively few matches. */
struct FO_COMMON_API WithinStarlaneJumps final : public Condition {
    WithinStarlaneJumps(std::unique_ptr<ValueRef::ValueRef<int>>&& jumps,
                        std::unique_ptr<Condition>&& condition);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    bool RootCandidateInvariant() const override;
    bool TargetInvariant() const override;
    bool SourceInvariant() const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_jumps;
    std::unique_ptr<Condition> m_condition;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches objects that are in systems that could have starlanes added between
  * them and all (not just one) of the systems containing (or that are) one of
  * the objects matched by \a condition.  "Could have starlanes added" means
  * that a lane would be geometrically acceptable, meaning it wouldn't cross
  * any other lanes, pass too close to another system, or be too close in angle
  * to an existing lane. */
struct FO_COMMON_API CanAddStarlaneConnection : Condition {
    explicit CanAddStarlaneConnection(std::unique_ptr<Condition>&& condition);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<Condition> m_condition;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches systems that have been explored by at least one Empire
  * in \a empire_ids. */
struct FO_COMMON_API ExploredByEmpire final : public Condition {
    explicit ExploredByEmpire(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_empire_id;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches objects that are moving. ... What does that mean?  Departing this
  * turn, or were located somewhere else last turn...? */
struct FO_COMMON_API Stationary final : public Condition {
    explicit Stationary();

    bool operator==(const Condition& rhs) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override
    {}
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches objects that are aggressive fleets or are in aggressive fleets. */
struct FO_COMMON_API Aggressive final : public Condition {
    explicit Aggressive();
    explicit Aggressive(bool aggressive);

    bool operator==(const Condition& rhs) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override
    {}
    bool GetAggressive() const
    { return m_aggressive; }
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    bool m_aggressive;   // false to match passive ships/fleets

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches objects that are in systems that can be fleet supplied by the
  * empire with id \a empire_id */
struct FO_COMMON_API FleetSupplyableByEmpire final : public Condition {
    explicit FleetSupplyableByEmpire(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_empire_id;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches objects that are in systems that are connected by resource-sharing
  * to at least one object that meets \a condition using the resource-sharing
  * network of the empire with id \a empire_id */
struct FO_COMMON_API ResourceSupplyConnectedByEmpire final : public Condition {
    ResourceSupplyConnectedByEmpire(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id,
                                    std::unique_ptr<Condition>&& condition);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    bool RootCandidateInvariant() const override;
    bool TargetInvariant() const override;
    bool SourceInvariant() const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<int>> m_empire_id;
    std::unique_ptr<Condition> m_condition;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches objects whose species has the ability to found new colonies. */
struct FO_COMMON_API CanColonize final : public Condition {
    explicit CanColonize();

    bool operator==(const Condition& rhs) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override
    {}
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches objects whose species has the ability to produce ships. */
struct FO_COMMON_API CanProduceShips final : public Condition {
    CanProduceShips();

    bool operator==(const Condition& rhs) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override
    {}
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches the objects that have been targeted for bombardment by at least one
  * object that matches \a m_by_object_condition. */
struct FO_COMMON_API OrderedBombarded final : public Condition {
    explicit OrderedBombarded(std::unique_ptr<Condition>&& by_object_condition);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    virtual void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<Condition> m_by_object_condition;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects if the comparisons between values of ValueRefs meet the
  * specified comparison types. */
struct FO_COMMON_API ValueTest final : public Condition {
    ValueTest(std::unique_ptr<ValueRef::ValueRef<double>>&& value_ref1,
              ComparisonType comp1,
              std::unique_ptr<ValueRef::ValueRef<double>>&& value_ref2,
              ComparisonType comp2 = INVALID_COMPARISON,
              std::unique_ptr<ValueRef::ValueRef<double>>&& value_ref3 = nullptr);

    ValueTest(std::unique_ptr<ValueRef::ValueRef<std::string>>&& value_ref1,
              ComparisonType comp1,
              std::unique_ptr<ValueRef::ValueRef<std::string>>&& value_ref2,
              ComparisonType comp2 = INVALID_COMPARISON,
              std::unique_ptr<ValueRef::ValueRef<std::string>>&& value_ref3 = nullptr);

    ValueTest(std::unique_ptr<ValueRef::ValueRef<int>>&& value_ref1,
              ComparisonType comp1,
              std::unique_ptr<ValueRef::ValueRef<int>>&& value_ref2,
              ComparisonType comp2 = INVALID_COMPARISON,
              std::unique_ptr<ValueRef::ValueRef<int>>&& value_ref3 = nullptr);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<double>> m_value_ref1;
    std::unique_ptr<ValueRef::ValueRef<double>> m_value_ref2;
    std::unique_ptr<ValueRef::ValueRef<double>> m_value_ref3;
    std::unique_ptr<ValueRef::ValueRef<std::string>> m_string_value_ref1;
    std::unique_ptr<ValueRef::ValueRef<std::string>> m_string_value_ref2;
    std::unique_ptr<ValueRef::ValueRef<std::string>> m_string_value_ref3;
    std::unique_ptr<ValueRef::ValueRef<int>> m_int_value_ref1;
    std::unique_ptr<ValueRef::ValueRef<int>> m_int_value_ref2;
    std::unique_ptr<ValueRef::ValueRef<int>> m_int_value_ref3;

    ComparisonType m_compare_type1 = INVALID_COMPARISON;
    ComparisonType m_compare_type2 = INVALID_COMPARISON;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches objects that match the location condition of the specified
  * content.  */
struct FO_COMMON_API Location final : public Condition {
public:
    Location(ContentType content_type,
             std::unique_ptr<ValueRef::ValueRef<std::string>>&& name1,
             std::unique_ptr<ValueRef::ValueRef<std::string>>&& name2 = nullptr);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    bool RootCandidateInvariant() const override;
    bool TargetInvariant() const override;
    bool SourceInvariant() const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<std::string>> m_name1;
    std::unique_ptr<ValueRef::ValueRef<std::string>> m_name2;
    ContentType m_content_type;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches objects that match the combat targeting condition of the specified
  * content.  */
struct FO_COMMON_API CombatTarget final : public Condition {
public:
    CombatTarget(ContentType content_type,
                 std::unique_ptr<ValueRef::ValueRef<std::string>>&& name);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;

    std::unique_ptr<ValueRef::ValueRef<std::string>> m_name;
    ContentType m_content_type;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/** Matches all objects that match every Condition in \a operands. */
struct FO_COMMON_API And final : public Condition {
    explicit And(std::vector<std::unique_ptr<Condition>>&& operands);
    And(std::unique_ptr<Condition>&& operand1,
        std::unique_ptr<Condition>&& operand2,
        std::unique_ptr<Condition>&& operand3 = nullptr,
        std::unique_ptr<Condition>&& operand4 = nullptr);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    void GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                           ObjectSet& condition_non_targets) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    const std::vector<Condition*> Operands() const;
    unsigned int GetCheckSum() const override;

private:
    std::vector<std::unique_ptr<Condition>> m_operands;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that match at least one Condition in \a operands. */
struct FO_COMMON_API Or final : public Condition {
    explicit Or(std::vector<std::unique_ptr<Condition>>&& operands);
    Or(std::unique_ptr<Condition>&& operand1,
       std::unique_ptr<Condition>&& operand2,
       std::unique_ptr<Condition>&& operand3 = nullptr,
       std::unique_ptr<Condition>&& operand4 = nullptr);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    virtual void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    std::vector<std::unique_ptr<Condition>> m_operands;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that do not match the Condition \a operand. */
struct FO_COMMON_API Not final : public Condition {
    explicit Not(std::unique_ptr<Condition>&& operand);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    std::unique_ptr<Condition> m_operand;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Tests conditions in \a operands in order, to find the first condition that 
  * matches at least one candidate object. Matches all objects that match that
  * condaition, ignoring any conditions listed later. If no candidate matches
  * any of the conditions, it matches nothing. */
struct FO_COMMON_API OrderedAlternativesOf final : public Condition {
    explicit OrderedAlternativesOf(std::vector<std::unique_ptr<Condition>>&& operands);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    const std::vector<Condition*> Operands() const;
    unsigned int GetCheckSum() const override;

private:
    std::vector<std::unique_ptr<Condition>> m_operands;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches whatever its subcondition matches, but has a customized description
  * string that is returned by Description() by looking up in the stringtable. */
struct FO_COMMON_API Described final : public Condition {
    Described(std::unique_ptr<Condition>&& condition, const std::string& desc_stringtable_key);

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override
    { return m_condition ? m_condition->Dump(ntabs) : ""; }
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    std::unique_ptr<Condition> m_condition;
    std::string m_desc_stringtable_key;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// template implementations
template <typename Archive>
void Condition::serialize(Archive& ar, const unsigned int version)
{}

template <typename Archive>
void Number::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_low)
        & BOOST_SERIALIZATION_NVP(m_high)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <typename Archive>
void Turn::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_low)
        & BOOST_SERIALIZATION_NVP(m_high);
}

template <typename Archive>
void SortedNumberOf::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_number)
        & BOOST_SERIALIZATION_NVP(m_sort_key)
        & BOOST_SERIALIZATION_NVP(m_sorting_method)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <typename Archive>
void All::serialize(Archive& ar, const unsigned int version)
{ ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition); }

template <typename Archive>
void None::serialize(Archive& ar, const unsigned int version)
{ ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition); }

template <typename Archive>
void EmpireAffiliation::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_affiliation);
}

template <typename Archive>
void Source::serialize(Archive& ar, const unsigned int version)
{ ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition); }

template <typename Archive>
void RootCandidate::serialize(Archive& ar, const unsigned int version)
{ ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition); }

template <typename Archive>
void Target::serialize(Archive& ar, const unsigned int version)
{ ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition); }

template <typename Archive>
void Homeworld::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_names);
}

template <typename Archive>
void Capital::serialize(Archive& ar, const unsigned int version)
{ ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition); }

template <typename Archive>
void Monster::serialize(Archive& ar, const unsigned int version)
{ ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition); }

template <typename Archive>
void Armed::serialize(Archive& ar, const unsigned int version)
{ ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition); }

template <typename Archive>
void Type::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_type);
}

template <typename Archive>
void Building::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_names);
}

template <typename Archive>
void HasSpecial::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_capacity_low)
        & BOOST_SERIALIZATION_NVP(m_capacity_high)
        & BOOST_SERIALIZATION_NVP(m_since_turn_low)
        & BOOST_SERIALIZATION_NVP(m_since_turn_high);
}

template <typename Archive>
void HasTag::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <typename Archive>
void CreatedOnTurn::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_low)
        & BOOST_SERIALIZATION_NVP(m_high);
}

template <typename Archive>
void Contains::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <typename Archive>
void ContainedBy::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <typename Archive>
void InSystem::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_system_id);
}

template <typename Archive>
void ObjectID::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_object_id);
}

template <typename Archive>
void PlanetType::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_types);
}

template <typename Archive>
void PlanetSize::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_sizes);
}

template <typename Archive>
void PlanetEnvironment::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_environments)
        & BOOST_SERIALIZATION_NVP(m_species_name);
}

template <typename Archive>
void Species::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_names);
}

template <typename Archive>
void Enqueued::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_build_type)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_design_id)
        & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_low)
        & BOOST_SERIALIZATION_NVP(m_high);
}

template <typename Archive>
void FocusType::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_names);
}

template <typename Archive>
void StarType::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_types);
}

template <typename Archive>
void DesignHasHull::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <typename Archive>
void DesignHasPart::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_low)
        & BOOST_SERIALIZATION_NVP(m_high)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <typename Archive>
void DesignHasPartClass::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_low)
        & BOOST_SERIALIZATION_NVP(m_high)
        & BOOST_SERIALIZATION_NVP(m_class);
}

template <typename Archive>
void PredefinedShipDesign::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <typename Archive>
void NumberedShipDesign::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_design_id);
}

template <typename Archive>
void ProducedByEmpire::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template <typename Archive>
void Chance::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_chance);
}

template <typename Archive>
void MeterValue::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_meter)
        & BOOST_SERIALIZATION_NVP(m_low)
        & BOOST_SERIALIZATION_NVP(m_high);
}

template <typename Archive>
void EmpireStockpileValue::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_low)
        & BOOST_SERIALIZATION_NVP(m_high)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template <typename Archive>
void OwnerHasTech::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template <typename Archive>
void OwnerHasBuildingTypeAvailable::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template <typename Archive>
void OwnerHasShipDesignAvailable::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_id)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template <typename Archive>
void OwnerHasShipPartAvailable::serialize(Archive& ar,
                                          const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template <typename Archive>
void VisibleToEmpire::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template <typename Archive>
void WithinDistance::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_distance)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <typename Archive>
void WithinStarlaneJumps::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_jumps)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <typename Archive>
void CanAddStarlaneConnection::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <typename Archive>
void ExploredByEmpire::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template <typename Archive>
void Stationary::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition);
}

template <typename Archive>
void Aggressive::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_aggressive);
}

template <typename Archive>
void FleetSupplyableByEmpire::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template <typename Archive>
void ResourceSupplyConnectedByEmpire::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <typename Archive>
void CanColonize::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition);
}

template <typename Archive>
void CanProduceShips::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition);
}

template <typename Archive>
void OrderedBombarded::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_by_object_condition);
}

template <typename Archive>
void ValueTest::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_value_ref1)
        & BOOST_SERIALIZATION_NVP(m_value_ref2)
        & BOOST_SERIALIZATION_NVP(m_value_ref3)
        & BOOST_SERIALIZATION_NVP(m_string_value_ref1)
        & BOOST_SERIALIZATION_NVP(m_string_value_ref2)
        & BOOST_SERIALIZATION_NVP(m_string_value_ref3)
        & BOOST_SERIALIZATION_NVP(m_int_value_ref1)
        & BOOST_SERIALIZATION_NVP(m_int_value_ref2)
        & BOOST_SERIALIZATION_NVP(m_int_value_ref3)
        & BOOST_SERIALIZATION_NVP(m_compare_type1)
        & BOOST_SERIALIZATION_NVP(m_compare_type2);
}

template <typename Archive>
void Location::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_name1)
        & BOOST_SERIALIZATION_NVP(m_name2)
        & BOOST_SERIALIZATION_NVP(m_content_type);
}

template <typename Archive>
void CombatTarget::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_content_type);
}

template <typename Archive>
void And::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_operands);
}

template <typename Archive>
void Or::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_operands);
}

template <typename Archive>
void Not::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_operand);
}

template <typename Archive>
void OrderedAlternativesOf::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_operands);
}

template <typename Archive>
void Described::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Condition)
        & BOOST_SERIALIZATION_NVP(m_condition)
        & BOOST_SERIALIZATION_NVP(m_desc_stringtable_key);
}
} // namespace Condition

#endif // _Conditions_h_
