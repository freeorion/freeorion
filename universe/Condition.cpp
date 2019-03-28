#include "Condition.h"

#include "../util/Logger.h"
#include "../util/Random.h"
#include "../util/i18n.h"
#include "UniverseObject.h"
#include "Pathfinder.h"
#include "Universe.h"
#include "Building.h"
#include "Fighter.h"
#include "Fleet.h"
#include "Ship.h"
#include "ObjectMap.h"
#include "Planet.h"
#include "System.h"
#include "Species.h"
#include "Special.h"
#include "Meter.h"
#include "ValueRef.h"
#include "Enums.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Supply.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/bind.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/st_connected.hpp>

//TODO: replace with std::make_unique when transitioning to C++14
#include <boost/smart_ptr/make_unique.hpp>

using boost::io::str;

FO_COMMON_API extern const int INVALID_DESIGN_ID;

bool UserStringExists(const std::string& str);

namespace {
    const std::string EMPTY_STRING;
}

namespace {
    void AddAllObjectsSet(Condition::ObjectSet& condition_non_targets) {
        condition_non_targets.reserve(condition_non_targets.size() + Objects().ExistingObjects().size());
        std::transform(Objects().ExistingObjects().begin(), Objects().ExistingObjects().end(),
                       std::back_inserter(condition_non_targets),
                       boost::bind(&std::map<int, std::shared_ptr<UniverseObject>>::value_type::second, _1));
    }

    void AddBuildingSet(Condition::ObjectSet& condition_non_targets) {
        condition_non_targets.reserve(condition_non_targets.size() + Objects().ExistingBuildings().size());
        std::transform(Objects().ExistingBuildings().begin(), Objects().ExistingBuildings().end(),
                       std::back_inserter(condition_non_targets),
                       boost::bind(&std::map<int, std::shared_ptr<UniverseObject>>::value_type::second, _1));
    }

    void AddFieldSet(Condition::ObjectSet& condition_non_targets) {
        condition_non_targets.reserve(condition_non_targets.size() + Objects().ExistingFields().size());
        std::transform(Objects().ExistingFields().begin(), Objects().ExistingFields().end(),
                       std::back_inserter(condition_non_targets),
                       boost::bind(&std::map<int, std::shared_ptr<UniverseObject>>::value_type::second, _1));
    }

    void AddFleetSet(Condition::ObjectSet& condition_non_targets) {
        condition_non_targets.reserve(condition_non_targets.size() + Objects().ExistingFleets().size());
        std::transform(Objects().ExistingFleets().begin(), Objects().ExistingFleets().end(),
                       std::back_inserter(condition_non_targets),
                       boost::bind(&std::map<int, std::shared_ptr<UniverseObject>>::value_type::second,_1));
    }

    void AddPlanetSet(Condition::ObjectSet& condition_non_targets) {
        condition_non_targets.reserve(condition_non_targets.size() + Objects().ExistingPlanets().size());
        std::transform(Objects().ExistingPlanets().begin(), Objects().ExistingPlanets().end(),
                       std::back_inserter(condition_non_targets),
                       boost::bind(&std::map<int, std::shared_ptr<UniverseObject>>::value_type::second,_1));
    }

    void AddPopCenterSet(Condition::ObjectSet& condition_non_targets) {
        condition_non_targets.reserve(condition_non_targets.size() + Objects().ExistingPopCenters().size());
        std::transform(Objects().ExistingPopCenters().begin(), Objects().ExistingPopCenters().end(),
                       std::back_inserter(condition_non_targets),
                       boost::bind(&std::map<int, std::shared_ptr<UniverseObject>>::value_type::second,_1));
    }

    void AddResCenterSet(Condition::ObjectSet& condition_non_targets) {
        condition_non_targets.reserve(condition_non_targets.size() + Objects().ExistingResourceCenters().size());
        std::transform(Objects().ExistingResourceCenters().begin(), Objects().ExistingResourceCenters().end(),
                       std::back_inserter(condition_non_targets),
                       boost::bind(&std::map<int, std::shared_ptr<UniverseObject>>::value_type::second,_1));
    }

    void AddShipSet(Condition::ObjectSet& condition_non_targets) {
        condition_non_targets.reserve(condition_non_targets.size() + Objects().ExistingShips().size());
        std::transform(Objects().ExistingShips().begin(), Objects().ExistingShips().end(),
                       std::back_inserter(condition_non_targets),
                       boost::bind(&std::map<int, std::shared_ptr<UniverseObject>>::value_type::second,_1));
    }

    void AddSystemSet(Condition::ObjectSet& condition_non_targets) {
        condition_non_targets.reserve(condition_non_targets.size() + Objects().ExistingSystems().size());
        std::transform(Objects().ExistingSystems().begin(), Objects().ExistingSystems().end(),
                       std::back_inserter(condition_non_targets),
                       boost::bind(&std::map<int, std::shared_ptr<UniverseObject>>::value_type::second,_1));
    }

    /** Used by 4-parameter ConditionBase::Eval function, and some of its
      * overrides, to scan through \a matches or \a non_matches set and apply
      * \a pred to each object, to test if it should remain in its current set
      * or be transferred from the \a search_domain specified set into the
      * other. */
    template <class Pred>
    void EvalImpl(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                  Condition::SearchDomain search_domain, const Pred& pred)
    {
        auto& from_set = search_domain == Condition::MATCHES ? matches : non_matches;
        auto& to_set = search_domain == Condition::MATCHES ? non_matches : matches;
        for (auto it = from_set.begin(); it != from_set.end(); ) {
            bool match = pred(*it);
            if ((search_domain == Condition::MATCHES && !match) ||
                (search_domain == Condition::NON_MATCHES && match))
            {
                to_set.push_back(*it);
                *it = from_set.back();
                from_set.pop_back();
            } else {
                ++it;
            }
        }
    }

    std::vector<Condition::ConditionBase*> FlattenAndNestedConditions(
        const std::vector<Condition::ConditionBase*>& input_conditions)
    {
        std::vector<Condition::ConditionBase*> retval;
        for (Condition::ConditionBase* condition : input_conditions) {
            if (Condition::And* and_condition = dynamic_cast<Condition::And*>(condition)) {
                std::vector<Condition::ConditionBase*> flattened_operands =
                    FlattenAndNestedConditions(and_condition->Operands());
                std::copy(flattened_operands.begin(), flattened_operands.end(), std::back_inserter(retval));
            } else {
                if (condition)
                    retval.push_back(condition);
            }
        }
        return retval;
    }

    std::map<std::string, bool> ConditionDescriptionAndTest(
        const std::vector<Condition::ConditionBase*>& conditions,
        const ScriptingContext& parent_context,
        std::shared_ptr<const UniverseObject> candidate_object/* = nullptr*/)
    {
        std::map<std::string, bool> retval;

        std::vector<Condition::ConditionBase*> flattened_conditions;
        if (conditions.empty())
            return retval;
        else if (conditions.size() > 1 || dynamic_cast<Condition::And*>(*conditions.begin()))
            flattened_conditions = FlattenAndNestedConditions(conditions);
        //else if (dynamic_cast<const Condition::Or*>(*conditions.begin()))
        //    flattened_conditions = FlattenOrNestedConditions(conditions);
        else
            flattened_conditions = conditions;

        for (Condition::ConditionBase* condition : flattened_conditions) {
            retval[condition->Description()] = condition->Eval(parent_context, candidate_object);
        }
        return retval;
    }
}

namespace Condition {
std::string ConditionFailedDescription(const std::vector<ConditionBase*>& conditions,
                                       std::shared_ptr<const UniverseObject> candidate_object/* = nullptr*/,
                                       std::shared_ptr<const UniverseObject> source_object/* = nullptr*/)
{
    if (conditions.empty())
        return UserString("NONE");

    ScriptingContext parent_context(source_object);
    std::string retval;

    // test candidate against all input conditions, and store descriptions of each
    for (const auto& result : ConditionDescriptionAndTest(conditions, parent_context, candidate_object)) {
            if (!result.second)
                 retval += UserString("FAILED") + " <rgba 255 0 0 255>" + result.first +"</rgba>\n";
    }

    // remove empty line from the end of the string
    retval = retval.substr(0, retval.length() - 1);

    return retval;
}

std::string ConditionDescription(const std::vector<ConditionBase*>& conditions,
                                 std::shared_ptr<const UniverseObject> candidate_object/* = nullptr*/,
                                 std::shared_ptr<const UniverseObject> source_object/* = nullptr*/)
{
    if (conditions.empty())
        return UserString("NONE");

    ScriptingContext parent_context(source_object);
    // test candidate against all input conditions, and store descriptions of each
    auto condition_description_and_test_results =
        ConditionDescriptionAndTest(conditions, parent_context, candidate_object);
    bool all_conditions_match_candidate = true, at_least_one_condition_matches_candidate = false;
    for (const auto& result : condition_description_and_test_results) {
        all_conditions_match_candidate = all_conditions_match_candidate && result.second;
        at_least_one_condition_matches_candidate = at_least_one_condition_matches_candidate || result.second;
    }

    // concatenate (non-duplicated) single-description results
    std::string retval;
    if (conditions.size() > 1 || dynamic_cast<And*>(*conditions.begin())) {
        retval += UserString("ALL_OF") + " ";
        retval += (all_conditions_match_candidate ? UserString("PASSED") : UserString("FAILED")) + "\n";
    } else if (dynamic_cast<Or*>(*conditions.begin())) {
        retval += UserString("ANY_OF") + " ";
        retval += (at_least_one_condition_matches_candidate ? UserString("PASSED") : UserString("FAILED")) + "\n";
    }
    // else just output single condition description and PASS/FAIL text

    for (const auto& result : condition_description_and_test_results) {
        retval += (result.second ? UserString("PASSED") : UserString("FAILED"));
        retval += " " + result.first + "\n";
    }
    return retval;
}

#define CHECK_COND_VREF_MEMBER(m_ptr) { if (m_ptr == rhs_.m_ptr) {              \
                                            /* check next member */             \
                                        } else if (!m_ptr || !rhs_.m_ptr) {     \
                                            return false;                       \
                                        } else {                                \
                                            if (*m_ptr != *(rhs_.m_ptr))        \
                                                return false;                   \
                                        }   }

///////////////////////////////////////////////////////////
// ConditionBase                                         //
///////////////////////////////////////////////////////////
struct ConditionBase::MatchHelper {
    MatchHelper(const ConditionBase* this_, const ScriptingContext& parent_context) :
        m_this(this_),
        m_parent_context(parent_context)
    {}

    bool operator()(std::shared_ptr<const UniverseObject> candidate) const
    { return m_this->Match(ScriptingContext(m_parent_context, candidate)); }

    const ConditionBase* m_this;
    const ScriptingContext& m_parent_context;
};

ConditionBase::~ConditionBase() = default;

bool ConditionBase::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;

    if (typeid(*this) != typeid(rhs))
        return false;

    return true;
}

void ConditionBase::Eval(const ScriptingContext& parent_context,
                         ObjectSet& matches, ObjectSet& non_matches,
                         SearchDomain search_domain/* = NON_MATCHES*/) const
{ EvalImpl(matches, non_matches, search_domain, MatchHelper(this, parent_context)); }

void ConditionBase::Eval(const ScriptingContext& parent_context,
                         ObjectSet& matches) const
{
    matches.clear();
    ObjectSet condition_initial_candidates;

    // evaluate condition only on objects that could potentially be matched by the condition
    GetDefaultInitialCandidateObjects(parent_context, condition_initial_candidates);

    matches.reserve(condition_initial_candidates.size());
    Eval(parent_context, matches, condition_initial_candidates);
}

bool ConditionBase::Eval(const ScriptingContext& parent_context,
                         std::shared_ptr<const UniverseObject> candidate) const
{
    if (!candidate)
        return false;
    ObjectSet non_matches, matches;
    non_matches.push_back(candidate);
    Eval(parent_context, matches, non_matches);
    return non_matches.empty(); // if candidate has been matched, non_matches will now be empty
}

bool ConditionBase::Eval(std::shared_ptr<const UniverseObject> candidate) const {
    if (!candidate)
        return false;
    ObjectSet non_matches, matches;
    non_matches.push_back(candidate);
    Eval(ScriptingContext(), matches, non_matches);
    return non_matches.empty(); // if candidate has been matched, non_matches will now be empty
}

void ConditionBase::GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                      ObjectSet& condition_non_targets) const
{ AddAllObjectsSet(condition_non_targets); }

std::string ConditionBase::Description(bool negated/* = false*/) const
{ return ""; }

std::string ConditionBase::Dump(unsigned short ntabs) const
{ return ""; }

bool ConditionBase::Match(const ScriptingContext& local_context) const
{ return false; }

///////////////////////////////////////////////////////////
// Number                                                //
///////////////////////////////////////////////////////////
Number::Number(std::unique_ptr<ValueRef::ValueRefBase<int>>&& low,
               std::unique_ptr<ValueRef::ValueRefBase<int>>&& high,
               std::unique_ptr<ConditionBase>&& condition) :
    m_low(std::move(low)),
    m_high(std::move(high)),
    m_condition(std::move(condition))
{}

bool Number::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const Number& rhs_ = static_cast<const Number&>(rhs);

    CHECK_COND_VREF_MEMBER(m_low)
    CHECK_COND_VREF_MEMBER(m_high)
    CHECK_COND_VREF_MEMBER(m_condition)

    return true;
}

std::string Number::Description(bool negated/* = false*/) const {
    std::string low_str = (m_low ? (m_low->ConstantExpr() ?
                                    std::to_string(m_low->Eval()) :
                                    m_low->Description())
                                 : "0");
    std::string high_str = (m_high ? (m_high->ConstantExpr() ?
                                      std::to_string(m_high->Eval()) :
                                      m_high->Description())
                                   : std::to_string(INT_MAX));

    const std::string& description_str = (!negated)
        ? UserString("DESC_NUMBER")
        : UserString("DESC_NUMBER_NOT");
    return str(FlexibleFormat(description_str)
               % low_str
               % high_str
               % m_condition->Description());
}

std::string Number::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "Number";
    if (m_low)
        retval += " low = " + m_low->Dump(ntabs);
    if (m_high)
        retval += " high = " + m_high->Dump(ntabs);
    retval += " condition =\n";
    retval += m_condition->Dump(ntabs+1);
    return retval;
}

void Number::Eval(const ScriptingContext& parent_context,
                  ObjectSet& matches, ObjectSet& non_matches,
                  SearchDomain search_domain/* = NON_MATCHES*/) const
{
    // Number does not have a single valid local candidate to be matched, as it
    // will match anything if the proper number of objects match the
    // subcondition.  So, the local context that is passed to the subcondition
    // needs to have a null local candidate.
    std::shared_ptr<const UniverseObject> no_object;
    ScriptingContext local_context(parent_context, no_object);

    if (!(
                (!m_low  || m_low->LocalCandidateInvariant())
             && (!m_high || m_high->LocalCandidateInvariant())
         )
       )
    {
        ErrorLogger() << "Condition::Number::Eval has local candidate-dependent ValueRefs, but no valid local candidate!";
    } else if (
                !local_context.condition_root_candidate
                && !(
                        (!m_low  || m_low->RootCandidateInvariant())
                     && (!m_high || m_high->RootCandidateInvariant())
                    )
              )
    {
        ErrorLogger() << "Condition::Number::Eval has root candidate-dependent ValueRefs, but expects local candidate to be the root candidate, and has no valid local candidate!";
    }

    if (!local_context.condition_root_candidate && !this->RootCandidateInvariant()) {
        // no externally-defined root candidate, so each object matched must
        // separately act as a root candidate, and sub-condition must be re-
        // evaluated for each tested object and the number of objects matched
        // checked for each object being tested
        ConditionBase::Eval(local_context, matches, non_matches, search_domain);

    } else {
        // parameters for number of subcondition objects that needs to be matched
        int low = (m_low ? m_low->Eval(local_context) : 0);
        int high = (m_high ? m_high->Eval(local_context) : INT_MAX);

        // get set of all UniverseObjects that satisfy m_condition
        ObjectSet condition_matches;
        // can evaluate subcondition once for all objects being tested by this condition
        m_condition->Eval(local_context, condition_matches);
        // compare number of objects that satisfy m_condition to the acceptable range of such objects
        int matched = condition_matches.size();
        bool in_range = (low <= matched && matched <= high);

        // transfer objects to or from candidate set, according to whether number of matches was within
        // the requested range.
        if (search_domain == MATCHES && !in_range) {
            non_matches.insert(non_matches.end(), matches.begin(), matches.end());
            matches.clear();
        }
        if (search_domain == NON_MATCHES && in_range) {
            matches.insert(matches.end(), non_matches.begin(), non_matches.end());
            non_matches.clear();
        }
    }
}

bool Number::RootCandidateInvariant() const {
    return (!m_low || m_low->RootCandidateInvariant()) &&
           (!m_high || m_high->RootCandidateInvariant()) &&
           m_condition->RootCandidateInvariant();
}

bool Number::TargetInvariant() const {
    return (!m_low || m_low->TargetInvariant()) &&
           (!m_high || m_high->TargetInvariant()) &&
           m_condition->TargetInvariant();
}

bool Number::SourceInvariant() const {
    return (!m_low || m_low->SourceInvariant()) &&
           (!m_high || m_high->SourceInvariant()) &&
           m_condition->SourceInvariant();
}

bool Number::Match(const ScriptingContext& local_context) const {
    // get acceptable range of subcondition matches for candidate
    int low = (m_low ? std::max(0, m_low->Eval(local_context)) : 0);
    int high = (m_high ? std::min(m_high->Eval(local_context), INT_MAX) : INT_MAX);

    // get set of all UniverseObjects that satisfy m_condition
    ObjectSet condition_matches;
    m_condition->Eval(local_context, condition_matches);

    // compare number of objects that satisfy m_condition to the acceptable range of such objects
    int matched = condition_matches.size();
    bool in_range = (low <= matched && matched <= high);
    return in_range;
}

void Number::SetTopLevelContent(const std::string& content_name) {
    if (m_low)
        m_low->SetTopLevelContent(content_name);
    if (m_high)
        m_high->SetTopLevelContent(content_name);
    if (m_condition)
        m_condition->SetTopLevelContent(content_name);
}

unsigned int Number::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::Number");
    CheckSums::CheckSumCombine(retval, m_low);
    CheckSums::CheckSumCombine(retval, m_high);
    CheckSums::CheckSumCombine(retval, m_condition);

    TraceLogger() << "GetCheckSum(Number): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// Turn                                                  //
///////////////////////////////////////////////////////////
Turn::Turn(std::unique_ptr<ValueRef::ValueRefBase<int>>&& low,
           std::unique_ptr<ValueRef::ValueRefBase<int>>&& high) :
    m_low(std::move(low)),
    m_high(std::move(high))
{}

bool Turn::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const Turn& rhs_ = static_cast<const Turn&>(rhs);

    CHECK_COND_VREF_MEMBER(m_low)
    CHECK_COND_VREF_MEMBER(m_high)

    return true;
}

void Turn::Eval(const ScriptingContext& parent_context,
                ObjectSet& matches, ObjectSet& non_matches,
                SearchDomain search_domain/* = NON_MATCHES*/) const
{
    // if ValueRef for low or high range limits depend on local candidate, then
    // they must be evaluated per-candidate.
    // if there already is a root candidate, then this condition's parameters
    // can be evaluated assuming it will not change.
    // if there is no root candidate in the parent context, then this
    // condition's candidates will be the root candidates, and this condition's
    // parameters must be root candidate invariant or else must be evaluated
    // per-candidate
    bool simple_eval_safe = ((!m_low || m_low->LocalCandidateInvariant()) &&
                             (!m_high || m_high->LocalCandidateInvariant()) &&
                             (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate turn limits once, check range, and use result to match or
        // reject all the search domain, since the current turn doesn't change
        // from object to object, and neither do the range limits.
        std::shared_ptr<const UniverseObject> no_object;
        ScriptingContext local_context(parent_context, no_object);
        int low =  (m_low ? std::max(BEFORE_FIRST_TURN, m_low->Eval(local_context)) : BEFORE_FIRST_TURN);
        int high = (m_high ? std::min(m_high->Eval(local_context), IMPOSSIBLY_LARGE_TURN) : IMPOSSIBLY_LARGE_TURN);
        int turn = CurrentTurn();
        bool match = (low <= turn && turn <= high);

        if (match && search_domain == NON_MATCHES) {
            // move all objects from non_matches to matches
            matches.insert(matches.end(), non_matches.begin(), non_matches.end());
            non_matches.clear();
        } else if (!match && search_domain == MATCHES) {
            // move all objects from matches to non_matches
            non_matches.insert(non_matches.end(), matches.begin(), matches.end());
            matches.clear();
        }
    } else {
        // re-evaluate allowed turn range for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Turn::RootCandidateInvariant() const
{ return (!m_low || m_low->RootCandidateInvariant()) && (!m_high || m_high->RootCandidateInvariant()); }

bool Turn::TargetInvariant() const
{ return (!m_low || m_low->TargetInvariant()) && (!m_high || m_high->TargetInvariant()); }

bool Turn::SourceInvariant() const
{ return (!m_low || m_low->SourceInvariant()) && (!m_high || m_high->SourceInvariant()); }

std::string Turn::Description(bool negated/* = false*/) const {
    std::string low_str;
    if (m_low)
        low_str = (m_low->ConstantExpr() ?
                   std::to_string(m_low->Eval()) :
                   m_low->Description());
    std::string high_str;
    if (m_high)
        high_str = (m_high->ConstantExpr() ?
                    std::to_string(m_high->Eval()) :
                    m_high->Description());
    std::string description_str;

    if (m_low && m_high) {
        description_str = (!negated)
            ? UserString("DESC_TURN")
            : UserString("DESC_TURN_NOT");
        return str(FlexibleFormat(description_str)
                   % low_str
                   % high_str);

    } else if (m_low) {
        description_str = (!negated)
            ? UserString("DESC_TURN_MIN_ONLY")
            : UserString("DESC_TURN_MIN_ONLY_NOT");
        return str(FlexibleFormat(description_str)
                   % low_str);

    } else if (m_high) {
        description_str = (!negated)
            ? UserString("DESC_TURN_MAX_ONLY")
            : UserString("DESC_TURN_MAX_ONLY_NOT");
        return str(FlexibleFormat(description_str)
                   % high_str);

    } else {
        return (!negated)
            ? UserString("DESC_TURN_ANY")
            : UserString("DESC_TURN_ANY_NOT");
    }
}

std::string Turn::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "Turn";
    if (m_low)
        retval += " low = " + m_low->Dump(ntabs);
    if (m_high)
        retval += " high = " + m_high->Dump(ntabs);
    retval += "\n";
    return retval;
}

bool Turn::Match(const ScriptingContext& local_context) const {
    int low = (m_low ? std::max(0, m_low->Eval(local_context)) : 0);
    int high = (m_high ? std::min(m_high->Eval(local_context), IMPOSSIBLY_LARGE_TURN) : IMPOSSIBLY_LARGE_TURN);
    int turn = CurrentTurn();
    return (low <= turn && turn <= high);
}

void Turn::SetTopLevelContent(const std::string& content_name) {
    if (m_low)
        m_low->SetTopLevelContent(content_name);
    if (m_high)
        m_high->SetTopLevelContent(content_name);
}

unsigned int Turn::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::Turn");
    CheckSums::CheckSumCombine(retval, m_low);
    CheckSums::CheckSumCombine(retval, m_high);

    TraceLogger() << "GetCheckSum(Turn): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// SortedNumberOf                                        //
///////////////////////////////////////////////////////////
SortedNumberOf::SortedNumberOf(std::unique_ptr<ValueRef::ValueRefBase<int>>&& number,
                               std::unique_ptr<ConditionBase>&& condition) :
    m_number(std::move(number)),
    m_condition(std::move(condition))
{}

SortedNumberOf::SortedNumberOf(std::unique_ptr<ValueRef::ValueRefBase<int>>&& number,
                               std::unique_ptr<ValueRef::ValueRefBase<double>>&& sort_key_ref,
                               SortingMethod sorting_method,
                               std::unique_ptr<ConditionBase>&& condition) :
    m_number(std::move(number)),
    m_sort_key(std::move(sort_key_ref)),
    m_sorting_method(sorting_method),
    m_condition(std::move(condition))
{}

bool SortedNumberOf::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const SortedNumberOf& rhs_ = static_cast<const SortedNumberOf&>(rhs);

    if (m_sorting_method != rhs_.m_sorting_method)
        return false;

    CHECK_COND_VREF_MEMBER(m_number)
    CHECK_COND_VREF_MEMBER(m_sort_key)
    CHECK_COND_VREF_MEMBER(m_condition)

    return true;
}

namespace {
    /** Random number genrator function to use with random_shuffle */
    int CustomRandInt(int max_plus_one)
    { return RandSmallInt(0, max_plus_one - 1); }
    int (*CRI)(int) = CustomRandInt;

    /** Transfers the indicated \a number of objects, randomly selected from from_set to to_set */
    void TransferRandomObjects(unsigned int number, ObjectSet& from_set, ObjectSet& to_set) {
        // ensure number of objects to be moved is within reasonable range
        number = std::min<unsigned int>(number, from_set.size());
        if (number == 0)
            return;

        // create list of bool flags to indicate whether each item in from_set
        // with corresponding place in iteration order should be transfered
        std::vector<bool> transfer_flags(from_set.size(), false);   // initialized to all false

        // set first  number  flags to true
        std::fill_n(transfer_flags.begin(), number, true);

        // shuffle flags to randomize which flags are set
        std::random_shuffle(transfer_flags.begin(), transfer_flags.end(), CRI);

        // transfer objects that have been flagged
        int i = 0;
        for (auto it = from_set.begin(); it != from_set.end(); ++i) {
            if (transfer_flags[i]) {
                to_set.push_back(*it);
                *it = from_set.back();
                from_set.pop_back();
            } else {
                ++it;
            }
        }
    }

    /** Transfers the indicated \a number of objects, selected from \a from_set
      * into \a to_set.  The objects transferred are selected based on the value
      * of \a sort_key evaluated on them, with the largest / smallest / most
      * common sort keys chosen, or a random selection chosen, depending on the
      * specified \a sorting_method */
    void TransferSortedObjects(unsigned int number, ValueRef::ValueRefBase<double>* sort_key,
                               const ScriptingContext& context, SortingMethod sorting_method,
                               ObjectSet& from_set, ObjectSet& to_set)
    {
        // handle random case, which doesn't need sorting key
        if (sorting_method == SORT_RANDOM) {
            TransferRandomObjects(number, from_set, to_set);
            return;
        }

        // for other SoringMethods, need sort key values
        if (!sort_key) {
            ErrorLogger() << "TransferSortedObjects given null sort_key";
            return;
        }

        // get sort key values for all objects in from_set, and sort by inserting into map
        std::multimap<float, std::shared_ptr<const UniverseObject>> sort_key_objects;
        for (auto& from : from_set) {
            float sort_value = sort_key->Eval(ScriptingContext(context, from));
            sort_key_objects.insert({sort_value, from});
        }

        // how many objects to select?
        number = std::min<unsigned int>(number, sort_key_objects.size());
        if (number == 0)
            return;
        unsigned int number_transferred(0);

        // pick max / min / most common values
        if (sorting_method == SORT_MIN) {
            // move (number) objects with smallest sort key (at start of map)
            // from the from_set into the to_set.
            for (const auto& entry : sort_key_objects) {
                auto object_to_transfer = entry.second;
                auto from_it = std::find(from_set.begin(), from_set.end(), object_to_transfer);
                if (from_it != from_set.end()) {
                    *from_it = from_set.back();
                    from_set.pop_back();
                    to_set.push_back(object_to_transfer);
                    number_transferred++;
                    if (number_transferred >= number)
                        return;
                }
            }

        } else if (sorting_method == SORT_MAX) {
            // move (number) objects with largest sort key (at end of map)
            // from the from_set into the to_set.
            for (auto sorted_it = sort_key_objects.rbegin();  // would use const_reverse_iterator but this causes a compile error in some compilers
                 sorted_it != sort_key_objects.rend(); ++sorted_it)
            {
                auto object_to_transfer = sorted_it->second;
                auto from_it = std::find(from_set.begin(), from_set.end(), object_to_transfer);
                if (from_it != from_set.end()) {
                    *from_it = from_set.back();
                    from_set.pop_back();
                    to_set.push_back(object_to_transfer);
                    number_transferred++;
                    if (number_transferred >= number)
                        return;
                }
            }

        } else if (sorting_method == SORT_MODE) {
            // compile histogram of of number of times each sort key occurs
            std::map<float, unsigned int> histogram;
            for (const auto& entry : sort_key_objects) {
                histogram[entry.first]++;
            }

            // invert histogram to index by number of occurances
            std::multimap<unsigned int, float> inv_histogram;
            for (const auto& entry : histogram)
                inv_histogram.insert({entry.second, entry.first});

            // reverse-loop through inverted histogram to find which sort keys
            // occurred most frequently, and transfer objects with those sort
            // keys from from_set to to_set.
            for (auto inv_hist_it = inv_histogram.rbegin();  // would use const_reverse_iterator but this causes a compile error in some compilers
                 inv_hist_it != inv_histogram.rend(); ++inv_hist_it)
            {
                float cur_sort_key = inv_hist_it->second;

                // get range of objects with the current sort key
                auto key_range = sort_key_objects.equal_range(cur_sort_key);

                // loop over range, selecting objects to transfer from from_set to to_set
                for (auto sorted_it = key_range.first;
                     sorted_it != key_range.second; ++sorted_it)
                {
                    auto object_to_transfer = sorted_it->second;
                    auto from_it = std::find(from_set.begin(), from_set.end(), object_to_transfer);
                    if (from_it != from_set.end()) {
                        *from_it = from_set.back();
                        from_set.pop_back();
                        to_set.push_back(object_to_transfer);
                        number_transferred++;
                        if (number_transferred >= number)
                            return;
                    }
                }
            }

        } else {
             ErrorLogger() << "TransferSortedObjects given unknown sort method";
        }
    }
}

void SortedNumberOf::Eval(const ScriptingContext& parent_context,
                          ObjectSet& matches, ObjectSet& non_matches,
                          SearchDomain search_domain/* = NON_MATCHES*/) const
{
    // Most conditions match objects independently of the other objects being
    // tested, but the number parameter for NumberOf conditions makes things
    // more complicated.  In order to match some number of the potential
    // matches property, both the matches and non_matches need to be checked
    // against the subcondition, and the total number of subcondition matches
    // counted.
    // Then, when searching NON_MATCHES, non_matches may be moved into matches
    // so that the number of subcondition matches in matches is equal to the
    // requested number.  There may also be subcondition non-matches in
    // matches, but these are not counted or affected by this condition.
    // Or, when searching MATCHES, matches may be moved into non_matches so
    // that the number of subcondition matches in matches is equal to the
    // requested number.  There again may be subcondition non-matches in
    // matches, but these are also not counted or affected by this condition.

    // SortedNumberOf does not have a valid local candidate to be matched
    // before the subcondition is evaluated, so the local context that is
    // passed to the subcondition needs to have a null local candidate.
    std::shared_ptr<const UniverseObject> no_object;
    ScriptingContext local_context(parent_context, no_object);

    // which input matches match the subcondition?
    ObjectSet subcondition_matching_matches;
    subcondition_matching_matches.reserve(matches.size());
    m_condition->Eval(local_context, subcondition_matching_matches, matches, NON_MATCHES);

    // remaining input matches don't match the subcondition...
    ObjectSet subcondition_non_matching_matches = matches;
    matches.clear();    // to be refilled later

    // which input non_matches match the subcondition?
    ObjectSet subcondition_matching_non_matches;
    subcondition_matching_non_matches.reserve(non_matches.size());
    m_condition->Eval(local_context, subcondition_matching_non_matches, non_matches, NON_MATCHES);

    // remaining input non_matches don't match the subcondition...
    ObjectSet subcondition_non_matching_non_matches = non_matches;
    non_matches.clear();    // to be refilled later

    // assemble single set of subcondition matching objects
    ObjectSet all_subcondition_matches;
    all_subcondition_matches.reserve(subcondition_matching_matches.size() + subcondition_matching_non_matches.size());
    all_subcondition_matches.insert(all_subcondition_matches.end(), subcondition_matching_matches.begin(), subcondition_matching_matches.end());
    all_subcondition_matches.insert(all_subcondition_matches.end(), subcondition_matching_non_matches.begin(), subcondition_matching_non_matches.end());

    // how many subcondition matches to select as matches to this condition
    int number = m_number->Eval(local_context);

    // compile single set of all objects that are matched by this condition.
    // these are the objects that should be transferred from non_matches into
    // matches, or those left in matches while the rest are moved into non_matches
    ObjectSet matched_objects;
    matched_objects.reserve(number);
    TransferSortedObjects(number, m_sort_key.get(), local_context, m_sorting_method, all_subcondition_matches, matched_objects);

    // put objects back into matches and non_target sets as output...

    if (search_domain == NON_MATCHES) {
        // put matched objects that are in subcondition_matching_non_matches into matches
        for (auto& matched_object : matched_objects) {
            // is this matched object in subcondition_matching_non_matches?
            auto smnt_it = std::find(subcondition_matching_non_matches.begin(),
                                     subcondition_matching_non_matches.end(), matched_object);
            if (smnt_it != subcondition_matching_non_matches.end()) {
                // yes; move object to matches
                *smnt_it = subcondition_matching_non_matches.back();
                subcondition_matching_non_matches.pop_back();
                matches.push_back(matched_object);
            }
        }

        // put remaining (non-matched) objects in subcondition_matching_non_matches back into non_matches
        non_matches.insert( non_matches.end(), subcondition_matching_non_matches.begin(),      subcondition_matching_non_matches.end());
        // put objects in subcondition_non_matching_non_matches back into non_matches
        non_matches.insert( non_matches.end(), subcondition_non_matching_non_matches.begin(),  subcondition_non_matching_non_matches.end());
        // put objects in subcondition_matching_matches and subcondition_non_matching_matches back into matches
        matches.insert(     matches.end(),     subcondition_matching_matches.begin(),          subcondition_matching_matches.end());
        matches.insert(     matches.end(),     subcondition_non_matching_matches.begin(),      subcondition_non_matching_matches.end());
        // this leaves the original contents of matches unchanged, other than
        // possibly having transferred some objects into matches from non_matches

    } else { /*(search_domain == MATCHES)*/
        // put matched objecs that are in subcondition_matching_matches back into matches
        for (auto& matched_object : matched_objects) {
            // is this matched object in subcondition_matching_matches?
            auto smt_it = std::find(subcondition_matching_matches.begin(),
                                    subcondition_matching_matches.end(), matched_object);
            if (smt_it != subcondition_matching_matches.end()) {
                // yes; move back into matches
                *smt_it = subcondition_matching_matches.back();
                subcondition_matching_matches.pop_back();
                matches.push_back(matched_object);
            }
        }

        // put remaining (non-matched) objects in subcondition_matching_matches) into non_matches
        non_matches.insert( non_matches.end(), subcondition_matching_matches.begin(),          subcondition_matching_matches.end());
        // put objects in subcondition_non_matching_matches into non_matches
        non_matches.insert( non_matches.end(), subcondition_non_matching_matches.begin(),      subcondition_non_matching_matches.end());
        // put objects in subcondition_matching_non_matches and subcondition_non_matching_non_matches back into non_matches
        non_matches.insert( non_matches.end(), subcondition_matching_non_matches.begin(),      subcondition_matching_non_matches.end());
        non_matches.insert( non_matches.end(), subcondition_non_matching_non_matches.begin(),  subcondition_non_matching_non_matches.end());
        // this leaves the original contents of non_matches unchanged, other than
        // possibly having transferred some objects into non_matches from matches
    }
}

bool SortedNumberOf::RootCandidateInvariant() const
{ return ((!m_number || m_number->SourceInvariant()) &&
          (!m_sort_key || m_sort_key->SourceInvariant()) &&
          (!m_condition || m_condition->SourceInvariant())); }

bool SortedNumberOf::TargetInvariant() const
{ return ((!m_number || m_number->SourceInvariant()) &&
          (!m_sort_key || m_sort_key->SourceInvariant()) &&
          (!m_condition || m_condition->SourceInvariant())); }

bool SortedNumberOf::SourceInvariant() const
{ return ((!m_number || m_number->SourceInvariant()) &&
          (!m_sort_key || m_sort_key->SourceInvariant()) &&
          (!m_condition || m_condition->SourceInvariant())); }

std::string SortedNumberOf::Description(bool negated/* = false*/) const {
    std::string number_str = m_number->ConstantExpr() ? m_number->Dump() : m_number->Description();

    if (m_sorting_method == SORT_RANDOM) {
        return str(FlexibleFormat((!negated)
                                  ? UserString("DESC_NUMBER_OF")
                                  : UserString("DESC_NUMBER_OF_NOT")
                                 )
                   % number_str
                   % m_condition->Description());
    } else {
        std::string sort_key_str = m_sort_key->ConstantExpr() ? m_sort_key->Dump() : m_sort_key->Description();

        std::string description_str;
        switch (m_sorting_method) {
        case SORT_MAX:
            description_str = (!negated)
                ? UserString("DESC_MAX_NUMBER_OF")
                : UserString("DESC_MAX_NUMBER_OF_NOT");
            break;

        case SORT_MIN:
            description_str = (!negated)
                ? UserString("DESC_MIN_NUMBER_OF")
                : UserString("DESC_MIN_NUMBER_OF_NOT");
            break;

        case SORT_MODE:
            description_str = (!negated)
                ? UserString("DESC_MODE_NUMBER_OF")
                : UserString("DESC_MODE_NUMBER_OF_NOT");
            break;
        default:
            break;
        }

        return str(FlexibleFormat(description_str)
                   % number_str
                   % sort_key_str
                   % m_condition->Description());
    }
}

std::string SortedNumberOf::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs);
    switch (m_sorting_method) {
    case SORT_RANDOM:
        retval += "NumberOf";   break;
    case SORT_MAX:
        retval += "MaximumNumberOf";  break;
    case SORT_MIN:
        retval += "MinimumNumberOf"; break;
    case SORT_MODE:
        retval += "ModeNumberOf"; break;
    default:
        retval += "??NumberOf??"; break;
    }

    retval += " number = " + m_number->Dump(ntabs);

    if (m_sort_key)
         retval += " sortby = " + m_sort_key->Dump(ntabs);

    retval += " condition =\n";
    retval += m_condition->Dump(ntabs+1);

    return retval;
}

void SortedNumberOf::GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                       ObjectSet& condition_non_targets) const
{
    if (m_condition) {
        m_condition->GetDefaultInitialCandidateObjects(parent_context, condition_non_targets);
    } else {
        ConditionBase::GetDefaultInitialCandidateObjects(parent_context, condition_non_targets);
    }
}

void SortedNumberOf::SetTopLevelContent(const std::string& content_name) {
    if (m_number)
        m_number->SetTopLevelContent(content_name);
    if (m_sort_key)
        m_sort_key->SetTopLevelContent(content_name);
    if (m_condition)
        m_condition->SetTopLevelContent(content_name);
}

unsigned int SortedNumberOf::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::SortedNumberOf");
    CheckSums::CheckSumCombine(retval, m_number);
    CheckSums::CheckSumCombine(retval, m_sort_key);
    CheckSums::CheckSumCombine(retval, m_sorting_method);
    CheckSums::CheckSumCombine(retval, m_condition);

    TraceLogger() << "GetCheckSum(SortedNumberOf): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// All                                                   //
///////////////////////////////////////////////////////////
void All::Eval(const ScriptingContext& parent_context,
                          ObjectSet& matches, ObjectSet& non_matches,
                          SearchDomain search_domain/* = NON_MATCHES*/) const
{
    if (search_domain == NON_MATCHES) {
        // move all objects from non_matches to matches
        matches.insert(matches.end(), non_matches.begin(), non_matches.end());
        non_matches.clear();
    }
    // if search_comain is MATCHES, do nothing: all objects in matches set
    // match this condition, so should remain in matches set
}

bool All::operator==(const ConditionBase& rhs) const
{ return ConditionBase::operator==(rhs); }

std::string All::Description(bool negated/* = false*/) const {
    return (!negated)
        ? UserString("DESC_ALL")
        : UserString("DESC_ALL_NOT");
}

std::string All::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "All\n"; }

unsigned int All::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::All");

    TraceLogger() << "GetCheckSum(All): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// None                                                  //
///////////////////////////////////////////////////////////
void None::Eval(const ScriptingContext& parent_context,
                ObjectSet& matches, ObjectSet& non_matches,
                SearchDomain search_domain/* = NON_MATCHES*/) const
{
    if (search_domain == MATCHES) {
        // move all objects from matches to non_matches
        non_matches.insert(non_matches.end(), matches.begin(), matches.end());
        matches.clear();
    }
    // if search domain is non_matches, no need to do anything since none of them match None.
}

bool None::operator==(const ConditionBase& rhs) const
{ return ConditionBase::operator==(rhs); }

std::string None::Description(bool negated/* = false*/) const {
    return (!negated)
        ? UserString("DESC_NONE")
        : UserString("DESC_NONE_NOT");
}

std::string None::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "None\n"; }

unsigned int None::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::None");

    TraceLogger() << "GetCheckSum(None): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// EmpireAffiliation                                     //
///////////////////////////////////////////////////////////
EmpireAffiliation::EmpireAffiliation(std::unique_ptr<ValueRef::ValueRefBase<int>>&& empire_id,
                                     EmpireAffiliationType affiliation) :
    m_empire_id(std::move(empire_id)),
    m_affiliation(affiliation)
{}

EmpireAffiliation::EmpireAffiliation(std::unique_ptr<ValueRef::ValueRefBase<int>>&& empire_id) :
    m_empire_id(std::move(empire_id)),
    m_affiliation(AFFIL_SELF)
{}

EmpireAffiliation::EmpireAffiliation(EmpireAffiliationType affiliation) :
    m_empire_id(nullptr),
    m_affiliation(affiliation)
{}

bool EmpireAffiliation::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const EmpireAffiliation& rhs_ = static_cast<const EmpireAffiliation&>(rhs);

    if (m_affiliation != rhs_.m_affiliation)
        return false;

    CHECK_COND_VREF_MEMBER(m_empire_id)

    return true;
}

namespace {
    struct EmpireAffiliationSimpleMatch {
        EmpireAffiliationSimpleMatch(int empire_id, EmpireAffiliationType affiliation) :
            m_empire_id(empire_id),
            m_affiliation(affiliation)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;

            switch (m_affiliation) {
            case AFFIL_SELF:
                return m_empire_id != ALL_EMPIRES && candidate->OwnedBy(m_empire_id);
                break;

            case AFFIL_ENEMY: {
                if (m_empire_id == ALL_EMPIRES)
                    return true;
                if (m_empire_id == candidate->Owner())
                    return false;
                DiplomaticStatus status = Empires().GetDiplomaticStatus(m_empire_id, candidate->Owner());
                return (status == DIPLO_WAR);
                break;
            }

            case AFFIL_ALLY: {
                if (m_empire_id == ALL_EMPIRES)
                    return false;
                if (m_empire_id == candidate->Owner())
                    return false;
                DiplomaticStatus status = Empires().GetDiplomaticStatus(m_empire_id, candidate->Owner());
                return (status == DIPLO_PEACE);
                break;
            }

            case AFFIL_ANY:
                return !candidate->Unowned();
                break;

            case AFFIL_NONE:
                return candidate->Unowned();
                break;

            case AFFIL_CAN_SEE: {
                // todo...
                return false;
                break;
            }

            case AFFIL_HUMAN: {
                if (candidate->Unowned())
                    return false;
                if (GetEmpireClientType(candidate->Owner()) == Networking::CLIENT_TYPE_HUMAN_PLAYER)
                    return true;
                return false;
                break;
            }

            default:
                return false;
                break;
            }
        }

        int m_empire_id;
        EmpireAffiliationType m_affiliation;
    };
}

void EmpireAffiliation::Eval(const ScriptingContext& parent_context,
                             ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = (!m_empire_id || m_empire_id->ConstantExpr()) ||
                            ((!m_empire_id || m_empire_id->LocalCandidateInvariant()) &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate empire id once, and use to check all candidate objects
        std::shared_ptr<const UniverseObject> no_object;
        int empire_id = m_empire_id ? m_empire_id->Eval(ScriptingContext(parent_context, no_object)) : ALL_EMPIRES;
        EvalImpl(matches, non_matches, search_domain, EmpireAffiliationSimpleMatch(empire_id, m_affiliation));
    } else {
        // re-evaluate empire id for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool EmpireAffiliation::RootCandidateInvariant() const
{ return m_empire_id ? m_empire_id->RootCandidateInvariant() : true; }

bool EmpireAffiliation::TargetInvariant() const
{ return m_empire_id ? m_empire_id->TargetInvariant() : true; }

bool EmpireAffiliation::SourceInvariant() const
{ return m_empire_id ? m_empire_id->SourceInvariant() : true; }

std::string EmpireAffiliation::Description(bool negated/* = false*/) const {
    std::string empire_str;
    if (m_empire_id) {
        int empire_id = ALL_EMPIRES;
        if (m_empire_id->ConstantExpr())
            empire_id = m_empire_id->Eval();
        if (const Empire* empire = GetEmpire(empire_id))
            empire_str = empire->Name();
        else
            empire_str = m_empire_id->Description();
    }

    if (m_affiliation == AFFIL_SELF) {
        return str(FlexibleFormat((!negated)
            ? UserString("DESC_EMPIRE_AFFILIATION_SELF")
            : UserString("DESC_EMPIRE_AFFILIATION_SELF_NOT")) % empire_str);
    } else if (m_affiliation == AFFIL_ANY) {
        return (!negated)
            ? UserString("DESC_EMPIRE_AFFILIATION_ANY")
            : UserString("DESC_EMPIRE_AFFILIATION_ANY_NOT");
    } else if (m_affiliation == AFFIL_NONE) {
        return (!negated)
            ? UserString("DESC_EMPIRE_AFFILIATION_ANY_NOT")
            : UserString("DESC_EMPIRE_AFFILIATION_ANY");
    } else {
        return str(FlexibleFormat((!negated)
            ? UserString("DESC_EMPIRE_AFFILIATION")
            : UserString("DESC_EMPIRE_AFFILIATION_NOT"))
                   % UserString(boost::lexical_cast<std::string>(m_affiliation))
                   % empire_str);
    }
}

std::string EmpireAffiliation::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs);
    if (m_affiliation == AFFIL_SELF) {
        retval += "OwnedBy";
        if (m_empire_id)
            retval += " empire = " + m_empire_id->Dump(ntabs);

    } else if (m_affiliation == AFFIL_ANY) {
        retval += "OwnedBy affiliation = AnyEmpire";

    } else if (m_affiliation == AFFIL_NONE) {
        retval += "Unowned";

    } else if (m_affiliation == AFFIL_ENEMY) {
        retval += "OwnedBy affiliation = EnemyOf";
        if (m_empire_id)
            retval += " empire = " + m_empire_id->Dump(ntabs);

    } else if (m_affiliation == AFFIL_ALLY) {
        retval += "OwnedBy affiliation = AllyOf";
        if (m_empire_id)
            retval += " empire = " + m_empire_id->Dump(ntabs);

    } else if (m_affiliation == AFFIL_CAN_SEE) {
        retval += "OwnedBy affiliation = CanSee";

    } else if (m_affiliation == AFFIL_HUMAN) {
        retval += "OwnedBy affiliation = Human";

    } else {
        retval += "OwnedBy ??";
    }

    retval += "\n";
    return retval;
}

bool EmpireAffiliation::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "EmpireAffiliation::Match passed no candidate object";
        return false;
    }

    int empire_id = m_empire_id ? m_empire_id->Eval(local_context) : ALL_EMPIRES;

    return EmpireAffiliationSimpleMatch(empire_id, m_affiliation)(candidate);
}

void EmpireAffiliation::SetTopLevelContent(const std::string& content_name) {
    if (m_empire_id)
        m_empire_id->SetTopLevelContent(content_name);
}

unsigned int EmpireAffiliation::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::EmpireAffiliation");
    CheckSums::CheckSumCombine(retval, m_empire_id);
    CheckSums::CheckSumCombine(retval, m_affiliation);

    TraceLogger() << "GetCheckSum(EmpireAffiliation): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// Source                                                //
///////////////////////////////////////////////////////////
bool Source::operator==(const ConditionBase& rhs) const
{ return ConditionBase::operator==(rhs); }

std::string Source::Description(bool negated/* = false*/) const {
    return (!negated)
        ? UserString("DESC_SOURCE")
        : UserString("DESC_SOURCE_NOT");
}

std::string Source::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "Source\n"; }

bool Source::Match(const ScriptingContext& local_context) const {
    if (!local_context.source)
        return false;
    return local_context.source == local_context.condition_local_candidate;
}

void Source::GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                               ObjectSet& condition_non_targets) const
{
    if (parent_context.source)
        condition_non_targets.push_back(parent_context.source);
}

unsigned int Source::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::Source");

    TraceLogger() << "GetCheckSum(Source): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// RootCandidate                                         //
///////////////////////////////////////////////////////////
bool RootCandidate::operator==(const ConditionBase& rhs) const
{ return ConditionBase::operator==(rhs); }

std::string RootCandidate::Description(bool negated/* = false*/) const {
    return (!negated)
        ? UserString("DESC_ROOT_CANDIDATE")
        : UserString("DESC_ROOT_CANDIDATE_NOT");
}

std::string RootCandidate::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "RootCandidate\n"; }

bool RootCandidate::Match(const ScriptingContext& local_context) const {
    if (!local_context.condition_root_candidate)
        return false;
    return local_context.condition_root_candidate == local_context.condition_local_candidate;
}

void RootCandidate::GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                      ObjectSet& condition_non_targets) const
{
    if (parent_context.condition_root_candidate)
        condition_non_targets.push_back(parent_context.condition_root_candidate);
}


unsigned int RootCandidate::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::RootCandidate");

    TraceLogger() << "GetCheckSum(RootCandidate): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// Target                                                //
///////////////////////////////////////////////////////////
bool Target::operator==(const ConditionBase& rhs) const
{ return ConditionBase::operator==(rhs); }

std::string Target::Description(bool negated/* = false*/) const {
    return (!negated)
        ? UserString("DESC_TARGET")
        : UserString("DESC_TARGET_NOT");
}

std::string Target::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "Target\n"; }

bool Target::Match(const ScriptingContext& local_context) const {
    if (!local_context.effect_target)
        return false;
    return local_context.effect_target == local_context.condition_local_candidate;
}

void Target::GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                               ObjectSet& condition_non_targets) const
{
    if (parent_context.effect_target)
        condition_non_targets.push_back(parent_context.effect_target);
}

unsigned int Target::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::Target");

    TraceLogger() << "GetCheckSum(Target): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// Homeworld                                             //
///////////////////////////////////////////////////////////
Homeworld::Homeworld() :
    ConditionBase(),
    m_names()
{}

Homeworld::Homeworld(std::vector<std::unique_ptr<ValueRef::ValueRefBase<std::string>>>&& names) :
    ConditionBase(),
    m_names(std::move(names))
{}

bool Homeworld::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const Homeworld& rhs_ = static_cast<const Homeworld&>(rhs);

    if (m_names.size() != rhs_.m_names.size())
        return false;
    for (unsigned int i = 0; i < m_names.size(); ++i) {
        CHECK_COND_VREF_MEMBER(m_names.at(i))
    }

    return true;
}

namespace {
    struct HomeworldSimpleMatch {
        HomeworldSimpleMatch(const std::vector<std::string>& names) :
            m_names(names)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;

            // is it a planet or a building on a planet?
            auto planet = std::dynamic_pointer_cast<const Planet>(candidate);
            std::shared_ptr<const ::Building> building;
            if (!planet && (building = std::dynamic_pointer_cast<const ::Building>(candidate))) {
                planet = GetPlanet(building->PlanetID());
            }
            if (!planet)
                return false;

            int planet_id = planet->ID();
            const SpeciesManager& manager = GetSpeciesManager();

            if (m_names.empty()) {
                // match homeworlds for any species
                for (auto species_it = manager.begin(); species_it != manager.end(); ++species_it) {
                    if (const auto& species = species_it->second) {
                        const auto& homeworld_ids = species->Homeworlds();
                        if (homeworld_ids.count(planet_id))
                            return true;
                    }
                }

            } else {
                // match any of the species specified
                for (const std::string& name : m_names) {
                    if (auto species = GetSpecies(name)) {
                        const auto& homeworld_ids = species->Homeworlds();
                        if (homeworld_ids.count(planet_id))
                            return true;
                    }
                }
            }

            return false;
        }

        const std::vector<std::string>& m_names;
    };
}

void Homeworld::Eval(const ScriptingContext& parent_context,
                     ObjectSet& matches, ObjectSet& non_matches,
                     SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = parent_context.condition_root_candidate || RootCandidateInvariant();
    if (simple_eval_safe) {
        // check each valueref for invariance to local candidate
        for (auto& name : m_names) {
            if (!name->LocalCandidateInvariant()) {
                simple_eval_safe = false;
                break;
            }
        }
    }
    if (simple_eval_safe) {
        // evaluate names once, and use to check all candidate objects
        std::vector<std::string> names;
        // get all names from valuerefs
        for (auto& name : m_names) {
            names.push_back(name->Eval(parent_context));
        }
        EvalImpl(matches, non_matches, search_domain, HomeworldSimpleMatch(names));
    } else {
        // re-evaluate allowed names for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Homeworld::RootCandidateInvariant() const {
    for (auto& name : m_names) {
        if (!name->RootCandidateInvariant())
            return false;
    }
    return true;
}

bool Homeworld::TargetInvariant() const {
    for (auto& name : m_names) {
        if (!name->TargetInvariant())
            return false;
    }
    return true;
}

bool Homeworld::SourceInvariant() const {
    for (auto& name : m_names) {
        if (!name->SourceInvariant())
            return false;
    }
    return true;
}

std::string Homeworld::Description(bool negated/* = false*/) const {
    std::string values_str;
    for (unsigned int i = 0; i < m_names.size(); ++i) {
        values_str += m_names[i]->ConstantExpr() ?
                        UserString(m_names[i]->Eval()) :
                        m_names[i]->Description();
        if (2 <= m_names.size() && i < m_names.size() - 2) {
            values_str += ", ";
        } else if (i == m_names.size() - 2) {
            values_str += m_names.size() < 3 ? " " : ", ";
            values_str += UserString("OR");
            values_str += " ";
        }
    }
    return str(FlexibleFormat((!negated)
        ? UserString("DESC_HOMEWORLD")
        : UserString("DESC_HOMEWORLD_NOT"))
        % values_str);
}

std::string Homeworld::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "HomeWorld";
    if (m_names.size() == 1) {
        retval += " name = " + m_names[0]->Dump(ntabs);
    } else if (!m_names.empty()) {
        retval += " name = [ ";
        for (auto& name : m_names) {
            retval += name->Dump(ntabs) + " ";
        }
        retval += "]";
    }
    return retval;
}

bool Homeworld::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "Homeworld::Match passed no candidate object";
        return false;
    }

    // is it a planet or a building on a planet?
    auto planet = std::dynamic_pointer_cast<const Planet>(candidate);
    std::shared_ptr<const ::Building> building;
    if (!planet && (building = std::dynamic_pointer_cast<const ::Building>(candidate))) {
        planet = GetPlanet(building->PlanetID());
    }
    if (!planet)
        return false;

    int planet_id = planet->ID();
    const SpeciesManager& manager = GetSpeciesManager();

    if (m_names.empty()) {
        // match homeworlds for any species
        for (const auto& entry : manager) {
            if (const auto& species = entry.second) {
                const auto& homeworld_ids = species->Homeworlds();
                if (homeworld_ids.count(planet_id))
                    return true;
            }
        }

    } else {
        // match any of the species specified
        for (auto& name : m_names) {
            const auto& species_name = name->Eval(local_context);
            if (const auto species = manager.GetSpecies(species_name)) {
                const auto& homeworld_ids = species->Homeworlds();
                if (homeworld_ids.count(planet_id))
                    return true;
            }
        }
    }

    return false;
}

void Homeworld::GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                  ObjectSet& condition_non_targets) const
{ AddPlanetSet(condition_non_targets); }

void Homeworld::SetTopLevelContent(const std::string& content_name) {
    for (auto& name : m_names) {
        if (name)
            name->SetTopLevelContent(content_name);
    }
}

unsigned int Homeworld::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::Homeworld");
    CheckSums::CheckSumCombine(retval, m_names);

    TraceLogger() << "GetCheckSum(Homeworld): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// Capital                                               //
///////////////////////////////////////////////////////////
bool Capital::operator==(const ConditionBase& rhs) const
{ return ConditionBase::operator==(rhs); }

std::string Capital::Description(bool negated/* = false*/) const {
    return (!negated)
        ? UserString("DESC_CAPITAL")
        : UserString("DESC_CAPITAL_NOT");
}

std::string Capital::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "Capital\n"; }

bool Capital::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "Capital::Match passed no candidate object";
        return false;
    }
    int candidate_id = candidate->ID();

    // check if any empire's capital's ID is that candidate object's id.
    // if it is, the candidate object is a capital.
    for (const auto& entry : Empires())
        if (entry.second->CapitalID() == candidate_id)
            return true;
    return false;
}

void Capital::GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                ObjectSet& condition_non_targets) const
{ AddPlanetSet(condition_non_targets); }

unsigned int Capital::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::Capital");

    TraceLogger() << "GetCheckSum(Capital): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// Monster                                               //
///////////////////////////////////////////////////////////
bool Monster::operator==(const ConditionBase& rhs) const
{ return ConditionBase::operator==(rhs); }

std::string Monster::Description(bool negated/* = false*/) const {
    return (!negated)
        ? UserString("DESC_MONSTER")
        : UserString("DESC_MONSTER_NOT");
}

std::string Monster::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "Monster\n"; }

bool Monster::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "Monster::Match passed no candidate object";
        return false;
    }

    if (auto ship = std::dynamic_pointer_cast<const Ship>(candidate))
        if (ship->IsMonster())
            return true;

    return false;
}

void Monster::GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                ObjectSet& condition_non_targets) const
{ AddShipSet(condition_non_targets); }

unsigned int Monster::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::Monster");

    TraceLogger() << "GetCheckSum(Monster): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// Armed                                                 //
///////////////////////////////////////////////////////////
bool Armed::operator==(const ConditionBase& rhs) const
{ return ConditionBase::operator==(rhs); }

std::string Armed::Description(bool negated/* = false*/) const {
    return (!negated)
        ? UserString("DESC_ARMED")
        : UserString("DESC_ARMED_NOT");
}

std::string Armed::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "Armed\n"; }

bool Armed::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "Armed::Match passed no candidate object";
        return false;
    }

    if (auto ship = std::dynamic_pointer_cast<const Ship>(candidate))
        if (ship->IsArmed() || ship->HasFighters())
            return true;

    return false;
}

unsigned int Armed::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::Armed");

    TraceLogger() << "GetCheckSum(Armed): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// Type                                                  //
///////////////////////////////////////////////////////////
Type::Type(std::unique_ptr<ValueRef::ValueRefBase<UniverseObjectType>>&& type) :
    ConditionBase(),
    m_type(std::move(type))
{}

Type::Type(UniverseObjectType type) :
    ConditionBase(),
    m_type(boost::make_unique<ValueRef::Constant<UniverseObjectType>>(type))
{}

bool Type::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const Type& rhs_ = static_cast<const Type&>(rhs);

    CHECK_COND_VREF_MEMBER(m_type)

    return true;
}

namespace {
    struct TypeSimpleMatch {
        TypeSimpleMatch(UniverseObjectType type) :
            m_type(type)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;

            switch (m_type) {
            case OBJ_BUILDING:
            case OBJ_SHIP:
            case OBJ_FLEET:
            case OBJ_PLANET:
            case OBJ_SYSTEM:
            case OBJ_FIELD:
            case OBJ_FIGHTER:
                return candidate->ObjectType() == m_type;
                break;
            case OBJ_POP_CENTER:
                return (bool)std::dynamic_pointer_cast<const PopCenter>(candidate);
                break;
            case OBJ_PROD_CENTER:
                return (bool)std::dynamic_pointer_cast<const ResourceCenter>(candidate);
                break;
            default:
                break;
            }
            return false;
        }

        UniverseObjectType m_type;
    };
}

void Type::Eval(const ScriptingContext& parent_context,
                ObjectSet& matches, ObjectSet& non_matches,
                SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = m_type->ConstantExpr() ||
                            (m_type->LocalCandidateInvariant() &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        UniverseObjectType type = m_type->Eval(parent_context);
        EvalImpl(matches, non_matches, search_domain, TypeSimpleMatch(type));
    } else {
        // re-evaluate allowed turn range for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Type::RootCandidateInvariant() const
{ return m_type->RootCandidateInvariant(); }

bool Type::TargetInvariant() const
{ return m_type->TargetInvariant(); }

bool Type::SourceInvariant() const
{ return m_type->SourceInvariant(); }

std::string Type::Description(bool negated/* = false*/) const {
    std::string value_str = m_type->ConstantExpr() ?
                                UserString(boost::lexical_cast<std::string>(m_type->Eval())) :
                                m_type->Description();
    return str(FlexibleFormat((!negated)
           ? UserString("DESC_TYPE")
           : UserString("DESC_TYPE_NOT"))
           % value_str);
}

std::string Type::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs);
    if (dynamic_cast<ValueRef::Constant<UniverseObjectType>*>(m_type.get())) {
        switch (m_type->Eval()) {
        case OBJ_BUILDING:    retval += "Building\n"; break;
        case OBJ_SHIP:        retval += "Ship\n"; break;
        case OBJ_FLEET:       retval += "Fleet\n"; break;
        case OBJ_PLANET:      retval += "Planet\n"; break;
        case OBJ_POP_CENTER:  retval += "PopulationCenter\n"; break;
        case OBJ_PROD_CENTER: retval += "ProductionCenter\n"; break;
        case OBJ_SYSTEM:      retval += "System\n"; break;
        case OBJ_FIELD:       retval += "Field\n"; break;
        case OBJ_FIGHTER:     retval += "Fighter\n"; break;
        default: retval += "?\n"; break;
        }
    } else {
        retval += "ObjectType type = " + m_type->Dump(ntabs) + "\n";
    }
    return retval;
}

bool Type::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "Type::Match passed no candidate object";
        return false;
    }

    return TypeSimpleMatch(m_type->Eval(local_context))(candidate);
}

void Type::GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                             ObjectSet& condition_non_targets) const
{
    // Ships, Fleets and default checks for current objects only
    bool found_type = false;
    if (m_type) {
        found_type = true;
        //std::vector<std::shared_ptr<const T>> this_base;
        switch (m_type->Eval()) {
            case OBJ_BUILDING:
                AddBuildingSet(condition_non_targets);
                break;
            case OBJ_FIELD:
                AddFieldSet(condition_non_targets);
                break;
            case OBJ_FLEET:
                AddFleetSet(condition_non_targets);
                break;
            case OBJ_PLANET:
                AddPlanetSet(condition_non_targets);
                break;
            case OBJ_POP_CENTER:
                AddPopCenterSet(condition_non_targets);
                break;
            case OBJ_PROD_CENTER:
                AddResCenterSet(condition_non_targets);
                break;
            case OBJ_SHIP:
                AddShipSet(condition_non_targets);
                break;
            case OBJ_SYSTEM:
                AddSystemSet(condition_non_targets);
                break;
            case OBJ_FIGHTER:   // shouldn't exist outside of combat as a separate object
            default: 
                found_type = false;
                break;
        }
    }
    if (found_type) {
        //if (int(condition_non_targets.size()) < Objects().NumObjects()) {
        //    DebugLogger() << "Type::GetBaseNonMatches will provide " << condition_non_targets.size()
        //                  << " objects of type " << GetType()->Eval() << " rather than "
        //                  << Objects().NumObjects() << " total objects";
        //}
    } else {
        ConditionBase::GetDefaultInitialCandidateObjects(parent_context, condition_non_targets);
    }
}

void Type::SetTopLevelContent(const std::string& content_name) {
    if (m_type)
        m_type->SetTopLevelContent(content_name);
}

unsigned int Type::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::Type");
    CheckSums::CheckSumCombine(retval, m_type);

    TraceLogger() << "GetCheckSum(Type): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// Building                                              //
///////////////////////////////////////////////////////////
Building::Building(std::vector<std::unique_ptr<ValueRef::ValueRefBase<std::string>>>&& names) :
    ConditionBase(),
    m_names(std::move(names))
{}

bool Building::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const Building& rhs_ = static_cast<const Building&>(rhs);

    if (m_names.size() != rhs_.m_names.size())
        return false;
    for (unsigned int i = 0; i < m_names.size(); ++i) {
        CHECK_COND_VREF_MEMBER(m_names.at(i))
    }

    return true;
}

namespace {
    struct BuildingSimpleMatch {
        BuildingSimpleMatch(const std::vector<std::string>& names) :
            m_names(names)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;

            // is it a building?
            auto building = std::dynamic_pointer_cast<const ::Building>(candidate);
            if (!building)
                return false;

            // if no name supplied, match any building
            if (m_names.empty())
                return true;

            // is it one of the specified building types?
            return std::count(m_names.begin(), m_names.end(), building->BuildingTypeName());
        }

        const std::vector<std::string>& m_names;
    };
}

void Building::Eval(const ScriptingContext& parent_context,
                    ObjectSet& matches, ObjectSet& non_matches,
                    SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = parent_context.condition_root_candidate || RootCandidateInvariant();
    if (simple_eval_safe) {
        // check each valueref for invariance to local candidate
        for (auto& name : m_names) {
            if (!name->LocalCandidateInvariant()) {
                simple_eval_safe = false;
                break;
            }
        }
    }
    if (simple_eval_safe) {
        // evaluate names once, and use to check all candidate objects
        std::vector<std::string> names;
        // get all names from valuerefs
        for (auto& name : m_names) {
            names.push_back(name->Eval(parent_context));
        }
        EvalImpl(matches, non_matches, search_domain, BuildingSimpleMatch(names));
    } else {
        // re-evaluate allowed building types range for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Building::RootCandidateInvariant() const {
    for (auto& name : m_names) {
        if (!name->RootCandidateInvariant())
            return false;
    }
    return true;
}

bool Building::TargetInvariant() const {
    for (auto& name : m_names) {
        if (!name->TargetInvariant())
            return false;
    }
    return true;
}

bool Building::SourceInvariant() const {
    for (auto& name : m_names) {
        if (!name->SourceInvariant())
            return false;
    }
    return true;
}

std::string Building::Description(bool negated/* = false*/) const {
    std::string values_str;
    for (unsigned int i = 0; i < m_names.size(); ++i) {
        values_str += m_names[i]->ConstantExpr() ?
                        UserString(m_names[i]->Eval()) :
                        m_names[i]->Description();
        if (2 <= m_names.size() && i < m_names.size() - 2) {
            values_str += ", ";
        } else if (i == m_names.size() - 2) {
            values_str += m_names.size() < 3 ? " " : ", ";
            values_str += UserString("OR");
            values_str += " ";
        }
    }
    return str(FlexibleFormat((!negated)
           ? UserString("DESC_BUILDING")
           : UserString("DESC_BUILDING_NOT"))
           % values_str);
}

std::string Building::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "Building name = ";
    if (m_names.size() == 1) {
        retval += m_names[0]->Dump(ntabs) + "\n";
    } else {
        retval += "[ ";
        for (auto& name : m_names) {
            retval += name->Dump(ntabs) + " ";
        }
        retval += "]\n";
    }
    return retval;
}

void Building::GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                 ObjectSet& condition_non_targets) const
{ AddBuildingSet(condition_non_targets); }

bool Building::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "Building::Match passed no candidate object";
        return false;
    }

    // is it a building?
    auto building = std::dynamic_pointer_cast<const ::Building>(candidate);
    if (building) {
        // match any building type?
        if (m_names.empty())
            return true;

        // match one of the specified building types
        for (auto& name : m_names) {
            if (name->Eval(local_context) == building->BuildingTypeName())
                return true;
        }
    }

    return false;
}

void Building::SetTopLevelContent(const std::string& content_name) {
    for (auto& name : m_names) {
        if (name)
            name->SetTopLevelContent(content_name);
    }
}

unsigned int Building::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::Building");
    CheckSums::CheckSumCombine(retval, m_names);

    TraceLogger() << "GetCheckSum(Building): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// HasSpecial                                            //
///////////////////////////////////////////////////////////
HasSpecial::HasSpecial() :
    ConditionBase(),
    m_name(nullptr),
    m_capacity_low(nullptr),
    m_capacity_high(nullptr),
    m_since_turn_low(),
    m_since_turn_high()
{}

HasSpecial::HasSpecial(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& name) :
    ConditionBase(),
    m_name(std::move(name)),
    m_capacity_low(nullptr),
    m_capacity_high(nullptr),
    m_since_turn_low(),
    m_since_turn_high()
{}

HasSpecial::HasSpecial(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& name,
                       std::unique_ptr<ValueRef::ValueRefBase<int>>&& since_turn_low,
                       std::unique_ptr<ValueRef::ValueRefBase<int>>&& since_turn_high) :
    ConditionBase(),
    m_name(std::move(name)),
    m_capacity_low(nullptr),
    m_capacity_high(nullptr),
    m_since_turn_low(std::move(since_turn_low)),
    m_since_turn_high(std::move(since_turn_high))
{}

HasSpecial::HasSpecial(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& name,
                       std::unique_ptr<ValueRef::ValueRefBase<double>>&& capacity_low,
                       std::unique_ptr<ValueRef::ValueRefBase<double>>&& capacity_high) :
    ConditionBase(),
    m_name(std::move(name)),
    m_capacity_low(std::move(capacity_low)),
    m_capacity_high(std::move(capacity_high)),
    m_since_turn_low(nullptr),
    m_since_turn_high(nullptr)
{}

HasSpecial::HasSpecial(const std::string& name) :
    ConditionBase(),
    // TODO: Use std::make_unique when adopting C++14
    m_name(new ValueRef::Constant<std::string>(name)),
    m_capacity_low(nullptr),
    m_capacity_high(nullptr),
    m_since_turn_low(nullptr),
    m_since_turn_high(nullptr)
{}

bool HasSpecial::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const HasSpecial& rhs_ = static_cast<const HasSpecial&>(rhs);

    CHECK_COND_VREF_MEMBER(m_name)
    CHECK_COND_VREF_MEMBER(m_capacity_low)
    CHECK_COND_VREF_MEMBER(m_capacity_high)
    CHECK_COND_VREF_MEMBER(m_since_turn_low)
    CHECK_COND_VREF_MEMBER(m_since_turn_high)

    return true;
}

namespace {
    struct HasSpecialSimpleMatch {
        HasSpecialSimpleMatch(const std::string& name, float low_cap, float high_cap, int low_turn, int high_turn) :
            m_name(name),
            m_low_cap(low_cap),
            m_high_cap(high_cap),
            m_low_turn(low_turn),
            m_high_turn(high_turn)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;

            if (m_name.empty())
                return !candidate->Specials().empty();

            auto it = candidate->Specials().find(m_name);
            if (it == candidate->Specials().end())
                return false;

            int special_since_turn = it->second.first;
            float special_capacity = it->second.second;
            return m_low_turn <= special_since_turn
                && special_since_turn <= m_high_turn
                && m_low_cap <= special_capacity
                && special_capacity <= m_high_cap;
        }

        const std::string&  m_name;
        float               m_low_cap;
        float               m_high_cap;
        int                 m_low_turn;
        int                 m_high_turn;
    };
}

void HasSpecial::Eval(const ScriptingContext& parent_context,
                      ObjectSet& matches, ObjectSet& non_matches,
                      SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = ((!m_name || m_name->LocalCandidateInvariant()) &&
                             (!m_capacity_low || m_capacity_low->LocalCandidateInvariant()) &&
                             (!m_capacity_high || m_capacity_high->LocalCandidateInvariant()) &&
                             (!m_since_turn_low || m_since_turn_low->LocalCandidateInvariant()) &&
                             (!m_since_turn_high || m_since_turn_high->LocalCandidateInvariant()) &&
                             (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate turn limits once, pass to simple match for all candidates
        std::shared_ptr<const UniverseObject> no_object;
        ScriptingContext local_context(parent_context, no_object);
        std::string name = (m_name ? m_name->Eval(local_context) : "");
        float low_cap = (m_capacity_low ? m_capacity_low->Eval(local_context) : -FLT_MAX);
        float high_cap = (m_capacity_high ? m_capacity_high->Eval(local_context) : FLT_MAX);
        int low_turn = (m_since_turn_low ? m_since_turn_low->Eval(local_context) : BEFORE_FIRST_TURN);
        int high_turn = (m_since_turn_high ? m_since_turn_high->Eval(local_context) : IMPOSSIBLY_LARGE_TURN);
        EvalImpl(matches, non_matches, search_domain, HasSpecialSimpleMatch(name, low_cap, high_cap, low_turn, high_turn));
    } else {
        // re-evaluate allowed turn range for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool HasSpecial::RootCandidateInvariant() const
{ return ((!m_name || m_name->RootCandidateInvariant()) &&
          (!m_capacity_low || m_capacity_low->RootCandidateInvariant()) &&
          (!m_capacity_high || m_capacity_high->RootCandidateInvariant()) &&
          (!m_since_turn_low || m_since_turn_low->RootCandidateInvariant()) &&
          (!m_since_turn_high || m_since_turn_high->RootCandidateInvariant())); }

bool HasSpecial::TargetInvariant() const
{ return ((!m_name || m_name->TargetInvariant()) &&
          (!m_capacity_low || m_capacity_low->TargetInvariant()) &&
          (!m_capacity_high || m_capacity_high->TargetInvariant()) &&
          (!m_since_turn_low || m_since_turn_low->TargetInvariant()) &&
          (!m_since_turn_high || m_since_turn_high->TargetInvariant())); }

bool HasSpecial::SourceInvariant() const
{ return ((!m_name || m_name->SourceInvariant()) &&
          (!m_capacity_low || m_capacity_low->SourceInvariant()) &&
          (!m_capacity_high || m_capacity_high->SourceInvariant()) &&
          (!m_since_turn_low || m_since_turn_low->SourceInvariant()) &&
          (!m_since_turn_high || m_since_turn_high->SourceInvariant())); }

std::string HasSpecial::Description(bool negated/* = false*/) const {
    std::string name_str;
    if (m_name) {
        name_str = m_name->Description();
        if (m_name->ConstantExpr() && UserStringExists(name_str))
            name_str = UserString(name_str);
    }

    if (m_since_turn_low || m_since_turn_high) {
        // turn range has been specified; must indicate in description
        std::string low_str = std::to_string(BEFORE_FIRST_TURN);
        if (m_since_turn_low)
            low_str = m_since_turn_low->Description();

        std::string high_str = std::to_string(IMPOSSIBLY_LARGE_TURN);
        if (m_since_turn_high)
            high_str = m_since_turn_high->Description();

        return str(FlexibleFormat((!negated)
            ? UserString("DESC_SPECIAL_TURN_RANGE")
            : UserString("DESC_SPECIAL_TURN_RANGE_NOT"))
                % name_str
                % low_str
                % high_str);
    }

    if (m_capacity_low || m_capacity_high) {
        // capacity range has been specified; must indicate in description
        std::string low_str = std::to_string(-FLT_MAX);
        if (m_capacity_low)
            low_str = m_capacity_low->Description();

        std::string high_str = std::to_string(FLT_MAX);
        if (m_capacity_high)
            high_str = m_capacity_high->Description();

        return str(FlexibleFormat((!negated)
            ? UserString("DESC_SPECIAL_CAPACITY_RANGE")
            : UserString("DESC_SPECIAL_CAPACITY_RANGE_NOT"))
                % name_str
                % low_str
                % high_str);
    }

    return str(FlexibleFormat((!negated)
        ? UserString("DESC_SPECIAL")
        : UserString("DESC_SPECIAL_NOT"))
                % name_str);
}

std::string HasSpecial::Dump(unsigned short ntabs) const {
    std::string name_str = (m_name ? m_name->Dump(ntabs) : "");

    if (m_since_turn_low || m_since_turn_high) {
        std::string low_dump = (m_since_turn_low ? m_since_turn_low->Dump(ntabs) : std::to_string(BEFORE_FIRST_TURN));
        std::string high_dump = (m_since_turn_high ? m_since_turn_high->Dump(ntabs) : std::to_string(IMPOSSIBLY_LARGE_TURN));
        return DumpIndent(ntabs) + "HasSpecialSinceTurn name = \"" + name_str + "\" low = " + low_dump + " high = " + high_dump;
    }

    if (m_capacity_low || m_capacity_high) {
        std::string low_dump = (m_capacity_low ? m_capacity_low->Dump(ntabs) : std::to_string(-FLT_MAX));
        std::string high_dump = (m_capacity_high ? m_capacity_high->Dump(ntabs) : std::to_string(FLT_MAX));
        return DumpIndent(ntabs) + "HasSpecialCapacity name = \"" + name_str + "\" low = " + low_dump + " high = " + high_dump;
    }

    return DumpIndent(ntabs) + "HasSpecial name = \"" + name_str + "\"\n";
}

bool HasSpecial::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "HasSpecial::Match passed no candidate object";
        return false;
    }
    std::string name = (m_name ? m_name->Eval(local_context) : "");
    float low_cap = (m_capacity_low ? m_capacity_low->Eval(local_context) : -FLT_MAX);
    float high_cap = (m_capacity_high ? m_capacity_high->Eval(local_context) : FLT_MAX);
    int low_turn = (m_since_turn_low ? m_since_turn_low->Eval(local_context) : BEFORE_FIRST_TURN);
    int high_turn = (m_since_turn_high ? m_since_turn_high->Eval(local_context) : IMPOSSIBLY_LARGE_TURN);

    return HasSpecialSimpleMatch(name, low_cap, high_cap, low_turn, high_turn)(candidate);
}

void HasSpecial::SetTopLevelContent(const std::string& content_name) {
    if (m_name)
        m_name->SetTopLevelContent(content_name);
    if (m_capacity_low)
        m_capacity_low->SetTopLevelContent(content_name);
    if (m_capacity_high)
        m_capacity_high->SetTopLevelContent(content_name);
    if (m_since_turn_low)
        m_since_turn_low->SetTopLevelContent(content_name);
    if (m_since_turn_high)
        m_since_turn_high->SetTopLevelContent(content_name);
}

unsigned int HasSpecial::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::HasSpecial");
    CheckSums::CheckSumCombine(retval, m_name);
    CheckSums::CheckSumCombine(retval, m_capacity_low);
    CheckSums::CheckSumCombine(retval, m_capacity_high);
    CheckSums::CheckSumCombine(retval, m_since_turn_low);
    CheckSums::CheckSumCombine(retval, m_since_turn_high);

    TraceLogger() << "GetCheckSum(HasSpecial): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// HasTag                                                //
///////////////////////////////////////////////////////////
HasTag::HasTag() :
    ConditionBase(),
    m_name()
{}

HasTag::HasTag(const std::string& name) :
    ConditionBase(),
    // TODO: Use std::make_unique when adopting C++14
    m_name(new ValueRef::Constant<std::string>(name))
{}

HasTag::HasTag(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& name) :
    ConditionBase(),
    m_name(std::move(name))
{}

bool HasTag::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const HasTag& rhs_ = static_cast<const HasTag&>(rhs);

    CHECK_COND_VREF_MEMBER(m_name)

    return true;
}

namespace {
    struct HasTagSimpleMatch {
        HasTagSimpleMatch() :
            m_any_tag_ok(true),
            m_name()
        {}

        HasTagSimpleMatch(const std::string& name) :
            m_any_tag_ok(false),
            m_name(name)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;

            if (m_any_tag_ok && !candidate->Tags().empty())
                return true;

            return candidate->HasTag(m_name);
        }

        bool        m_any_tag_ok;
        std::string m_name;
    };
}

void HasTag::Eval(const ScriptingContext& parent_context,
                  ObjectSet& matches, ObjectSet& non_matches,
                  SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = (!m_name || m_name->LocalCandidateInvariant()) &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant());
    if (simple_eval_safe) {
        // evaluate number limits once, use to match all candidates
        std::shared_ptr<const UniverseObject> no_object;
        ScriptingContext local_context(parent_context, no_object);
        if (!m_name) {
            EvalImpl(matches, non_matches, search_domain, HasTagSimpleMatch());
        } else {
            std::string name = boost::to_upper_copy<std::string>(m_name->Eval(local_context));
            EvalImpl(matches, non_matches, search_domain, HasTagSimpleMatch(name));
        }
    } else {
        // re-evaluate allowed turn range for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool HasTag::RootCandidateInvariant() const
{ return !m_name || m_name->RootCandidateInvariant(); }

bool HasTag::TargetInvariant() const
{ return !m_name || m_name->TargetInvariant(); }

bool HasTag::SourceInvariant() const
{ return !m_name || m_name->SourceInvariant(); }

std::string HasTag::Description(bool negated/* = false*/) const {
    std::string name_str;
    if (m_name) {
        name_str = m_name->Description();
        if (m_name->ConstantExpr() && UserStringExists(name_str))
            name_str = UserString(name_str);
    }
    return str(FlexibleFormat((!negated)
        ? UserString("DESC_HAS_TAG")
        : UserString("DESC_HAS_TAG_NOT"))
        % name_str);
}

std::string HasTag::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "HasTag";
    if (m_name)
        retval += " name = " + m_name->Dump(ntabs);
    retval += "\n";
    return retval;
}

bool HasTag::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "HasTag::Match passed no candidate object";
        return false;
    }

    if (!m_name)
        return HasTagSimpleMatch()(candidate);

    std::string name = boost::to_upper_copy<std::string>(m_name->Eval(local_context));
    return HasTagSimpleMatch(name)(candidate);
}

void HasTag::SetTopLevelContent(const std::string& content_name) {
    if (m_name)
        m_name->SetTopLevelContent(content_name);
}

unsigned int HasTag::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::HasTag");
    CheckSums::CheckSumCombine(retval, m_name);

    TraceLogger() << "GetCheckSum(HasTag): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// CreatedOnTurn                                         //
///////////////////////////////////////////////////////////
CreatedOnTurn::CreatedOnTurn(std::unique_ptr<ValueRef::ValueRefBase<int>>&& low,
                             std::unique_ptr<ValueRef::ValueRefBase<int>>&& high) :
    ConditionBase(),
    m_low(std::move(low)),
    m_high(std::move(high))
{}

bool CreatedOnTurn::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const CreatedOnTurn& rhs_ = static_cast<const CreatedOnTurn&>(rhs);

    CHECK_COND_VREF_MEMBER(m_low)
    CHECK_COND_VREF_MEMBER(m_high)

    return true;
}

namespace {
    struct CreatedOnTurnSimpleMatch {
        CreatedOnTurnSimpleMatch(int low, int high) :
            m_low(low),
            m_high(high)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;
            int turn = candidate->CreationTurn();
            return m_low <= turn && turn <= m_high;
        }

        int m_low;
        int m_high;
    };
}

void CreatedOnTurn::Eval(const ScriptingContext& parent_context,
                         ObjectSet& matches, ObjectSet& non_matches,
                         SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = ((!m_low || m_low->LocalCandidateInvariant()) &&
                             (!m_high || m_high->LocalCandidateInvariant()) &&
                             (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        std::shared_ptr<const UniverseObject> no_object;
        ScriptingContext local_context(parent_context, no_object);
        int low = (m_low ? m_low->Eval(local_context) : BEFORE_FIRST_TURN);
        int high = (m_high ? m_high->Eval(local_context) : IMPOSSIBLY_LARGE_TURN);
        EvalImpl(matches, non_matches, search_domain, CreatedOnTurnSimpleMatch(low, high));
    } else {
        // re-evaluate allowed turn range for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool CreatedOnTurn::RootCandidateInvariant() const
{ return ((!m_low || m_low->RootCandidateInvariant()) && (!m_high || m_high->RootCandidateInvariant())); }

bool CreatedOnTurn::TargetInvariant() const
{ return ((!m_low || m_low->TargetInvariant()) && (!m_high || m_high->TargetInvariant())); }

bool CreatedOnTurn::SourceInvariant() const
{ return ((!m_low || m_low->SourceInvariant()) && (!m_high || m_high->SourceInvariant())); }

std::string CreatedOnTurn::Description(bool negated/* = false*/) const {
    std::string low_str = (m_low ? (m_low->ConstantExpr() ?
                                    std::to_string(m_low->Eval()) :
                                    m_low->Description())
                                 : std::to_string(BEFORE_FIRST_TURN));
    std::string high_str = (m_high ? (m_high->ConstantExpr() ?
                                      std::to_string(m_high->Eval()) :
                                      m_high->Description())
                                   : std::to_string(IMPOSSIBLY_LARGE_TURN));
    return str(FlexibleFormat((!negated)
            ? UserString("DESC_CREATED_ON_TURN")
            : UserString("DESC_CREATED_ON_TURN_NOT"))
               % low_str
               % high_str);
}

std::string CreatedOnTurn::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "CreatedOnTurn";
    if (m_low)
        retval += " low = " + m_low->Dump(ntabs);
    if (m_high)
        retval += " high = " + m_high->Dump(ntabs);
    retval += "\n";
    return retval;
}

bool CreatedOnTurn::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "CreatedOnTurn::Match passed no candidate object";
        return false;
    }
    int low = (m_low ? std::max(0, m_low->Eval(local_context)) : BEFORE_FIRST_TURN);
    int high = (m_high ? std::min(m_high->Eval(local_context), IMPOSSIBLY_LARGE_TURN) : IMPOSSIBLY_LARGE_TURN);
    return CreatedOnTurnSimpleMatch(low, high)(candidate);
}

void CreatedOnTurn::SetTopLevelContent(const std::string& content_name) {
    if (m_low)
        m_low->SetTopLevelContent(content_name);
    if (m_high)
        m_high->SetTopLevelContent(content_name);
}

unsigned int CreatedOnTurn::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::CreatedOnTurn");
    CheckSums::CheckSumCombine(retval, m_low);
    CheckSums::CheckSumCombine(retval, m_high);

    TraceLogger() << "GetCheckSum(CreatedOnTurn): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// Contains                                              //
///////////////////////////////////////////////////////////
bool Contains::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const Contains& rhs_ = static_cast<const Contains&>(rhs);

    CHECK_COND_VREF_MEMBER(m_condition)

    return true;
}

namespace {
    struct ContainsSimpleMatch {
        ContainsSimpleMatch(const ObjectSet& subcondition_matches) :
            m_subcondition_matches_ids()
        {
            // We need a sorted container for efficiently intersecting 
            // subcondition_matches with the set of objects contained in some
            // candidate object.
            // We only need ids, not objects, so we can do that conversion
            // here as well, simplifying later code.
            // Note that this constructor is called only once per 
            // Contains::Eval(), its work cannot help performance when executed 
            // for each candidate.
            m_subcondition_matches_ids.reserve(subcondition_matches.size());
            // gather the ids
            for (auto& obj : subcondition_matches) {
                if (obj)
                    m_subcondition_matches_ids.push_back(obj->ID());
            }
            // sort them
            std::sort(m_subcondition_matches_ids.begin(), m_subcondition_matches_ids.end());
        }

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;

            bool match = false;
            const auto& candidate_elements = candidate->ContainedObjectIDs(); // guaranteed O(1)

            // We need to test whether candidate_elements and m_subcondition_matches_ids have a common element.
            // We choose the strategy that is more efficient by comparing the sizes of both sets.
            if (candidate_elements.size() < m_subcondition_matches_ids.size()) {
                // candidate_elements is smaller, so we iterate it and look up each candidate element in m_subcondition_matches_ids
                for (int id : candidate_elements) {
                    // std::lower_bound requires m_subcondition_matches_ids to be sorted
                    auto matching_it = std::lower_bound(m_subcondition_matches_ids.begin(), m_subcondition_matches_ids.end(), id);

                    if (matching_it != m_subcondition_matches_ids.end() && *matching_it == id) {
                        match = true;
                        break;
                    }
                }
            } else {
                // m_subcondition_matches_ids is smaller, so we iterate it and look up each subcondition match in the set of candidate's elements
                for (int id : m_subcondition_matches_ids) {
                    // candidate->Contains() may have a faster implementation than candidate_elements->find()
                    if (candidate->Contains(id)) {
                        match = true;
                        break;
                    }
                }
            }

            return match;
        }

        std::vector<int> m_subcondition_matches_ids;
    };
}

void Contains::Eval(const ScriptingContext& parent_context,
                    ObjectSet& matches, ObjectSet& non_matches,
                    SearchDomain search_domain/* = NON_MATCHES*/) const
{
    unsigned int search_domain_size = (search_domain == MATCHES ? matches.size() : non_matches.size());
    bool simple_eval_safe = parent_context.condition_root_candidate ||
                            RootCandidateInvariant() ||
                            search_domain_size < 2;
    if (!simple_eval_safe) {
        // re-evaluate contained objects for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
        return;
    }

    // how complicated is this containment test?
    if (((search_domain == MATCHES) && matches.empty()) ||
        ((search_domain == NON_MATCHES) && non_matches.empty()))
    {
        // don't need to evaluate anything...

    } else if (search_domain_size == 1) {
        // evaluate subcondition on objects contained by the candidate
        ScriptingContext local_context(parent_context, search_domain == MATCHES ? *matches.begin() : *non_matches.begin());

        // initialize subcondition candidates from local candidate's contents
        const ObjectMap& objects = Objects();
        ObjectSet subcondition_matches = objects.FindObjects(local_context.condition_local_candidate->ContainedObjectIDs());

        // apply subcondition to candidates
        if (!subcondition_matches.empty()) {
            ObjectSet dummy;
            m_condition->Eval(local_context, subcondition_matches, dummy, Condition::MATCHES);
        }

        // move single local candidate as appropriate...
        if (search_domain == MATCHES && subcondition_matches.empty()) {
            // move to non_matches
            matches.clear();
            non_matches.push_back(local_context.condition_local_candidate);
        } else if (search_domain == NON_MATCHES && !subcondition_matches.empty()) {
            // move to matches
            non_matches.clear();
            matches.push_back(local_context.condition_local_candidate);
        }

    } else {
        // evaluate contained objects once using default initial candidates
        // of subcondition to find all subcondition matches in the Universe
        std::shared_ptr<const UniverseObject> no_object;
        ScriptingContext local_context(parent_context, no_object);
        ObjectSet subcondition_matches;
        m_condition->Eval(local_context, subcondition_matches);

        // check all candidates to see if they contain any subcondition matches
        EvalImpl(matches, non_matches, search_domain, ContainsSimpleMatch(subcondition_matches));
    }
}

bool Contains::RootCandidateInvariant() const
{ return m_condition->RootCandidateInvariant(); }

bool Contains::TargetInvariant() const
{ return m_condition->TargetInvariant(); }

bool Contains::SourceInvariant() const
{ return m_condition->SourceInvariant(); }

std::string Contains::Description(bool negated/* = false*/) const {
    return str(FlexibleFormat((!negated)
        ? UserString("DESC_CONTAINS")
        : UserString("DESC_CONTAINS_NOT"))
        % m_condition->Description());
}

std::string Contains::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "Contains condition =\n";
    retval += m_condition->Dump(ntabs+1);
    return retval;
}

void Contains::GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                 ObjectSet& condition_non_targets) const
{
    // objects that can contain other objects: systems, fleets, planets
    AddSystemSet(condition_non_targets);
    AddFleetSet(condition_non_targets);
    AddPlanetSet(condition_non_targets);
}

bool Contains::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "Contains::Match passed no candidate object";
        return false;
    }

    // get subcondition matches
    ObjectSet subcondition_matches;
    m_condition->Eval(local_context, subcondition_matches);

    // does candidate object contain any subcondition matches?
    for (auto& obj : subcondition_matches)
        if (candidate->Contains(obj->ID()))
            return true;

    return false;
}

void Contains::SetTopLevelContent(const std::string& content_name) {
    if (m_condition)
        m_condition->SetTopLevelContent(content_name);
}

unsigned int Contains::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::Contains");
    CheckSums::CheckSumCombine(retval, m_condition);

    TraceLogger() << "GetCheckSum(Contains): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// ContainedBy                                           //
///////////////////////////////////////////////////////////
bool ContainedBy::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const ContainedBy& rhs_ = static_cast<const ContainedBy&>(rhs);

    CHECK_COND_VREF_MEMBER(m_condition)

    return true;
}

namespace {
    struct ContainedBySimpleMatch {
        ContainedBySimpleMatch(const ObjectSet& subcondition_matches) :
            m_subcondition_matches_ids()
        {
            // We need a sorted container for efficiently intersecting 
            // subcondition_matches with the set of objects containing some
            // candidate object.
            // We only need ids, not objects, so we can do that conversion
            // here as well, simplifying later code.
            // Note that this constructor is called only once per 
            // ContainedBy::Eval(), its work cannot help performance when
            // executed for each candidate.
            m_subcondition_matches_ids.reserve(subcondition_matches.size());
            // gather the ids
            for (auto& obj : subcondition_matches) {
                if (obj)
                { m_subcondition_matches_ids.push_back(obj->ID()); }
            }
            // sort them
            std::sort(m_subcondition_matches_ids.begin(), m_subcondition_matches_ids.end());
        }

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;

            bool match = false;
            // gather the objects containing candidate
            std::vector<int> candidate_containers;
            const int candidate_id = candidate->ID();
            const int    system_id = candidate->SystemID();
            const int container_id = candidate->ContainerObjectID();
            if (   system_id != INVALID_OBJECT_ID &&    system_id != candidate_id) candidate_containers.push_back(   system_id);
            if (container_id != INVALID_OBJECT_ID && container_id !=    system_id) candidate_containers.push_back(container_id);
            // FIXME: currently, direct container and system will do. In the future, we might need a way to retrieve containers of containers

            // We need to test whether candidate_containers and m_subcondition_matches_ids have a common element.
            // We choose the strategy that is more efficient by comparing the sizes of both sets.
            if (candidate_containers.size() < m_subcondition_matches_ids.size()) {
                // candidate_containers is smaller, so we iterate it and look up each candidate container in m_subcondition_matches_ids
                for (int id : candidate_containers) {
                    // std::lower_bound requires m_subcondition_matches_ids to be sorted
                    auto matching_it = std::lower_bound(m_subcondition_matches_ids.begin(), m_subcondition_matches_ids.end(), id);

                    if (matching_it != m_subcondition_matches_ids.end() && *matching_it == id) {
                        match = true;
                        break;
                    }
                }
            } else {
                // m_subcondition_matches_ids is smaller, so we iterate it and look up each subcondition match in the set of candidate's containers
                for (int id : m_subcondition_matches_ids) {
                    // candidate->ContainedBy() may have a faster implementation than candidate_containers->find()
                    if (candidate->ContainedBy(id)) {
                        match = true;
                        break;
                    }
                }
            }

            return match;
        }

        std::vector<int> m_subcondition_matches_ids;
    };
}

void ContainedBy::Eval(const ScriptingContext& parent_context,
                       ObjectSet& matches, ObjectSet& non_matches,
                       SearchDomain search_domain/* = NON_MATCHES*/) const
{
    unsigned int search_domain_size = (search_domain == MATCHES ? matches.size() : non_matches.size());
    bool simple_eval_safe = parent_context.condition_root_candidate ||
                            RootCandidateInvariant() ||
                            search_domain_size < 2;

    if (!simple_eval_safe) {
        // re-evaluate container objects for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
        return;
    }

    // how complicated is this containment test?
    if (((search_domain == MATCHES) && matches.empty()) ||
        ((search_domain == NON_MATCHES) && non_matches.empty()))
    {
        // don't need to evaluate anything...

    } else if (search_domain_size == 1) {
        // evaluate subcondition on objects that contain the candidate
        ScriptingContext local_context(parent_context, search_domain == MATCHES ? *matches.begin() : *non_matches.begin());

        // initialize subcondition candidates from local candidate's containers
        const ObjectMap& objects = Objects();
        std::set<int> container_object_ids;
        if (local_context.condition_local_candidate->ContainerObjectID() != INVALID_OBJECT_ID)
            container_object_ids.insert(local_context.condition_local_candidate->ContainerObjectID());
        if (local_context.condition_local_candidate->SystemID() != INVALID_OBJECT_ID)
            container_object_ids.insert(local_context.condition_local_candidate->SystemID());

        ObjectSet subcondition_matches = objects.FindObjects(container_object_ids);

        // apply subcondition to candidates
        if (!subcondition_matches.empty()) {
            ObjectSet dummy;
            m_condition->Eval(local_context, subcondition_matches, dummy, Condition::MATCHES);
        }

        // move single local candidate as appropriate...
        if (search_domain == MATCHES && subcondition_matches.empty()) {
            // move to non_matches
            matches.clear();
            non_matches.push_back(local_context.condition_local_candidate);
        } else if (search_domain == NON_MATCHES && !subcondition_matches.empty()) {
            // move to matches
            non_matches.clear();
            matches.push_back(local_context.condition_local_candidate);
        }

    } else {
        // evaluate container objects once using default initial candidates
        // of subcondition to find all subcondition matches in the Universe
        std::shared_ptr<const UniverseObject> no_object;
        ScriptingContext local_context(parent_context, no_object);
        ObjectSet subcondition_matches;
        m_condition->Eval(local_context, subcondition_matches);

        // check all candidates to see if they contain any subcondition matches
        EvalImpl(matches, non_matches, search_domain, ContainedBySimpleMatch(subcondition_matches));
    }
}

bool ContainedBy::RootCandidateInvariant() const
{ return m_condition->RootCandidateInvariant(); }

bool ContainedBy::TargetInvariant() const
{ return m_condition->TargetInvariant(); }

bool ContainedBy::SourceInvariant() const
{ return m_condition->SourceInvariant(); }

std::string ContainedBy::Description(bool negated/* = false*/) const {
    return str(FlexibleFormat((!negated)
        ? UserString("DESC_CONTAINED_BY")
        : UserString("DESC_CONTAINED_BY_NOT"))
        % m_condition->Description());
}

std::string ContainedBy::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "ContainedBy condition =\n";
    retval += m_condition->Dump(ntabs+1);
    return retval;
}

void ContainedBy::GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                    ObjectSet& condition_non_targets) const
{
    // objects that can be contained by other objects: fleets, planets, ships, buildings
    AddFleetSet(condition_non_targets);
    AddPlanetSet(condition_non_targets);
    AddShipSet(condition_non_targets);
    AddBuildingSet(condition_non_targets);
}

bool ContainedBy::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "ContainedBy::Match passed no candidate object";
        return false;
    }

    // get containing objects
    std::set<int> containers;
    if (candidate->SystemID() != INVALID_OBJECT_ID)
        containers.insert(candidate->SystemID());
    if (candidate->ContainerObjectID() != INVALID_OBJECT_ID && candidate->ContainerObjectID() != candidate->SystemID())
        containers.insert(candidate->ContainerObjectID());

    ObjectSet container_objects = Objects().FindObjects<const UniverseObject>(containers);
    if (container_objects.empty())
        return false;

    m_condition->Eval(local_context, container_objects);

    return container_objects.empty();
}

void ContainedBy::SetTopLevelContent(const std::string& content_name) {
    if (m_condition)
        m_condition->SetTopLevelContent(content_name);
}

unsigned int ContainedBy::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::ContainedBy");
    CheckSums::CheckSumCombine(retval, m_condition);

    TraceLogger() << "GetCheckSum(ContainedBy): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// InSystem                                              //
///////////////////////////////////////////////////////////
InSystem::InSystem(std::unique_ptr<ValueRef::ValueRefBase<int>>&& system_id) :
    ConditionBase(),
    m_system_id(std::move(system_id))
{}

bool InSystem::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const InSystem& rhs_ = static_cast<const InSystem&>(rhs);

    CHECK_COND_VREF_MEMBER(m_system_id)

    return true;
}

namespace {
    struct InSystemSimpleMatch {
        InSystemSimpleMatch(int system_id) :
            m_system_id(system_id)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;
            if (m_system_id == INVALID_OBJECT_ID)
                return candidate->SystemID() != INVALID_OBJECT_ID;  // match objects in any system
            else
                return candidate->SystemID() == m_system_id;        // match objects in specified system
        }

        int m_system_id;
    };
}

void InSystem::Eval(const ScriptingContext& parent_context,
                    ObjectSet& matches, ObjectSet& non_matches,
                    SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = !m_system_id || m_system_id->ConstantExpr() ||
                            (m_system_id->LocalCandidateInvariant() &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate empire id once, and use to check all candidate objects
        std::shared_ptr<const UniverseObject> no_object;
        int system_id = (m_system_id ? m_system_id->Eval(ScriptingContext(parent_context, no_object)) : INVALID_OBJECT_ID);
        EvalImpl(matches, non_matches, search_domain, InSystemSimpleMatch(system_id));
    } else {
        // re-evaluate empire id for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool InSystem::RootCandidateInvariant() const
{ return !m_system_id || m_system_id->RootCandidateInvariant(); }

bool InSystem::TargetInvariant() const
{ return !m_system_id || m_system_id->TargetInvariant(); }

bool InSystem::SourceInvariant() const
{ return !m_system_id || m_system_id->SourceInvariant(); }

std::string InSystem::Description(bool negated/* = false*/) const {
    std::string system_str;
    int system_id = INVALID_OBJECT_ID;
    if (m_system_id && m_system_id->ConstantExpr())
        system_id = m_system_id->Eval();
    if (auto system = GetSystem(system_id))
        system_str = system->Name();
    else if (m_system_id)
        system_str = m_system_id->Description();

    std::string description_str;
    if (!system_str.empty())
        description_str = (!negated)
            ? UserString("DESC_IN_SYSTEM")
            : UserString("DESC_IN_SYSTEM_NOT");
    else
        description_str = (!negated)
            ? UserString("DESC_IN_SYSTEM_SIMPLE")
            : UserString("DESC_IN_SYSTEM_SIMPLE_NOT");

    return str(FlexibleFormat(description_str) % system_str);
}

std::string InSystem::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "InSystem";
    if (m_system_id)
        retval += " id = " + m_system_id->Dump(ntabs);
    retval += "\n";
    return retval;
}

void InSystem::GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                 ObjectSet& condition_non_targets) const
{
    if (!m_system_id) {
        // can match objects in any system, or any system
        AddAllObjectsSet(condition_non_targets);
        return;
    }

    bool simple_eval_safe = m_system_id->ConstantExpr() ||
                            (m_system_id->LocalCandidateInvariant() &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant()));

    if (!simple_eval_safe) {
        // almost anything can be in a system, and can also match the system itself
        AddAllObjectsSet(condition_non_targets);
        return;
    }

    // simple case of a single specified system id; can add just objects in that system
    int system_id = m_system_id->Eval(parent_context);
    auto system = GetSystem(system_id);
    if (!system)
        return;

    const ObjectMap& obj_map = Objects();
    const std::set<int>& system_object_ids = system->ObjectIDs();
    auto sys_objs = obj_map.FindObjects(system_object_ids);

    // insert all objects that have the specified system id
    condition_non_targets.reserve(sys_objs.size() + 1);
    std::copy(sys_objs.begin(), sys_objs.end(), std::back_inserter(condition_non_targets));
    // also insert system itself
    condition_non_targets.push_back(system);
}

bool InSystem::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "InSystem::Match passed no candidate object";
        return false;
    }
    int system_id = (m_system_id ? m_system_id->Eval(local_context) : INVALID_OBJECT_ID);
    return InSystemSimpleMatch(system_id)(candidate);
}

void InSystem::SetTopLevelContent(const std::string& content_name) {
    if (m_system_id)
        m_system_id->SetTopLevelContent(content_name);
}

unsigned int InSystem::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::InSystem");
    CheckSums::CheckSumCombine(retval, m_system_id);

    TraceLogger() << "GetCheckSum(InSystem): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// ObjectID                                              //
///////////////////////////////////////////////////////////
ObjectID::ObjectID(std::unique_ptr<ValueRef::ValueRefBase<int>>&& object_id) :
    ConditionBase(),
    m_object_id(std::move(object_id))
{}

bool ObjectID::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const ObjectID& rhs_ = static_cast<const ObjectID&>(rhs);

    CHECK_COND_VREF_MEMBER(m_object_id)

    return true;
}

namespace {
    struct ObjectIDSimpleMatch {
        ObjectIDSimpleMatch(int object_id) :
            m_object_id(object_id)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            return candidate &&
                m_object_id != INVALID_OBJECT_ID &&
                candidate->ID() == m_object_id;
        }

        int m_object_id;
    };
}

void ObjectID::Eval(const ScriptingContext& parent_context,
                    ObjectSet& matches, ObjectSet& non_matches,
                    SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = !m_object_id || m_object_id->ConstantExpr() ||
                            (m_object_id->LocalCandidateInvariant() &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate empire id once, and use to check all candidate objects
        std::shared_ptr<const UniverseObject> no_object;
        int object_id = (m_object_id ? m_object_id->Eval(ScriptingContext(parent_context, no_object)) : INVALID_OBJECT_ID);
        EvalImpl(matches, non_matches, search_domain, ObjectIDSimpleMatch(object_id));
    } else {
        // re-evaluate empire id for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool ObjectID::RootCandidateInvariant() const
{ return !m_object_id || m_object_id->RootCandidateInvariant(); }

bool ObjectID::TargetInvariant() const
{ return !m_object_id || m_object_id->TargetInvariant(); }

bool ObjectID::SourceInvariant() const
{ return !m_object_id || m_object_id->SourceInvariant(); }

std::string ObjectID::Description(bool negated/* = false*/) const {
    std::string object_str;
    int object_id = INVALID_OBJECT_ID;
    if (m_object_id && m_object_id->ConstantExpr())
        object_id = m_object_id->Eval();
    if (auto system = GetSystem(object_id))
        object_str = system->Name();
    else if (m_object_id)
        object_str = m_object_id->Description();
    else
        object_str = UserString("ERROR");   // should always have a valid ID for this condition

    return str(FlexibleFormat((!negated)
        ? UserString("DESC_OBJECT_ID")
        : UserString("DESC_OBJECT_ID_NOT"))
               % object_str);
}

std::string ObjectID::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "Object id = " + m_object_id->Dump(ntabs) + "\n"; }

void ObjectID::GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                 ObjectSet& condition_non_targets) const
{
    if (!m_object_id)
        return;

    bool simple_eval_safe = m_object_id->ConstantExpr() ||
                            (m_object_id->LocalCandidateInvariant() &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant()));

    if (!simple_eval_safe) {
        AddAllObjectsSet(condition_non_targets);
        return;
    }

    // simple case of a single specified id; can add just that object
    std::shared_ptr<const UniverseObject> no_object;
    int object_id = m_object_id->Eval(ScriptingContext(parent_context, no_object));
    if (object_id == INVALID_OBJECT_ID)
        return;

    auto obj = Objects().ExistingObject(object_id);
    if (obj)
        condition_non_targets.push_back(obj);
}

bool ObjectID::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "ObjectID::Match passed no candidate object";
        return false;
    }

    return ObjectIDSimpleMatch(m_object_id->Eval(local_context))(candidate);
}

void ObjectID::SetTopLevelContent(const std::string& content_name) {
    if (m_object_id)
        m_object_id->SetTopLevelContent(content_name);
}

unsigned int ObjectID::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::ObjectID");
    CheckSums::CheckSumCombine(retval, m_object_id);

    TraceLogger() << "GetCheckSum(ObjectID): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// PlanetType                                            //
///////////////////////////////////////////////////////////
PlanetType::PlanetType(std::vector<std::unique_ptr<ValueRef::ValueRefBase< ::PlanetType>>>&& types) :
    ConditionBase(),
    m_types(std::move(types))
{}

bool PlanetType::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const PlanetType& rhs_ = static_cast<const PlanetType&>(rhs);

    if (m_types.size() != rhs_.m_types.size())
        return false;
    for (unsigned int i = 0; i < m_types.size(); ++i) {
        CHECK_COND_VREF_MEMBER(m_types.at(i))
    }

    return true;
}

namespace {
    struct PlanetTypeSimpleMatch {
        PlanetTypeSimpleMatch(const std::vector< ::PlanetType>& types) :
            m_types(types)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;

            // is it a planet or on a planet?
            auto planet = std::dynamic_pointer_cast<const Planet>(candidate);
            std::shared_ptr<const ::Building> building;
            if (!planet && (building = std::dynamic_pointer_cast<const ::Building>(candidate))) {
                planet = GetPlanet(building->PlanetID());
            }
            if (planet) {
                // is it one of the specified building types?
                return std::count(m_types.begin(), m_types.end(), planet->Type());
            }

            return false;
        }

        const std::vector< ::PlanetType>& m_types;
    };
}

void PlanetType::Eval(const ScriptingContext& parent_context,
                                 ObjectSet& matches, ObjectSet& non_matches,
                                 SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = parent_context.condition_root_candidate || RootCandidateInvariant();
    if (simple_eval_safe) {
        // check each valueref for invariance to local candidate
        for (auto& type : m_types) {
            if (!type->LocalCandidateInvariant()) {
                simple_eval_safe = false;
                break;
            }
        }
    }
    if (simple_eval_safe) {
        // evaluate types once, and use to check all candidate objects
        std::vector< ::PlanetType> types;
        // get all types from valuerefs
        for (auto& type : m_types) {
            types.push_back(type->Eval(parent_context));
        }
        EvalImpl(matches, non_matches, search_domain, PlanetTypeSimpleMatch(types));
    } else {
        // re-evaluate contained objects for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool PlanetType::RootCandidateInvariant() const {
    for (auto& type : m_types) {
        if (!type->RootCandidateInvariant())
            return false;
    }
    return true;
}

bool PlanetType::TargetInvariant() const {
    for (auto& type : m_types) {
        if (!type->TargetInvariant())
            return false;
    }
    return true;
}

bool PlanetType::SourceInvariant() const {
    for (auto& type : m_types) {
        if (!type->SourceInvariant())
            return false;
    }
    return true;
}

std::string PlanetType::Description(bool negated/* = false*/) const {
    std::string values_str;
    for (unsigned int i = 0; i < m_types.size(); ++i) {
        values_str += m_types[i]->ConstantExpr() ?
                        UserString(boost::lexical_cast<std::string>(m_types[i]->Eval())) :
                        m_types[i]->Description();
        if (2 <= m_types.size() && i < m_types.size() - 2) {
            values_str += ", ";
        } else if (i == m_types.size() - 2) {
            values_str += m_types.size() < 3 ? " " : ", ";
            values_str += UserString("OR");
            values_str += " ";
        }
    }
    return str(FlexibleFormat((!negated)
        ? UserString("DESC_PLANET_TYPE")
        : UserString("DESC_PLANET_TYPE_NOT"))
        % values_str);
}

std::string PlanetType::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "Planet type = ";
    if (m_types.size() == 1) {
        retval += m_types[0]->Dump(ntabs) + "\n";
    } else {
        retval += "[ ";
        for (auto& type : m_types) {
            retval += type->Dump(ntabs) + " ";
        }
        retval += "]\n";
    }
    return retval;
}

void PlanetType::GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                   ObjectSet& condition_non_targets) const
{
    AddPlanetSet(condition_non_targets);
    AddBuildingSet(condition_non_targets);
}

bool PlanetType::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "PlanetType::Match passed no candidate object";
        return false;
    }

    auto planet = std::dynamic_pointer_cast<const Planet>(candidate);
    std::shared_ptr<const ::Building> building;
    if (!planet && (building = std::dynamic_pointer_cast<const ::Building>(candidate))) {
        planet = GetPlanet(building->PlanetID());
    }
    if (planet) {
        for (auto& type : m_types) {
            if (type->Eval(ScriptingContext(local_context)) == planet->Type())
                return true;
        }
    }
    return false;
}

void PlanetType::SetTopLevelContent(const std::string& content_name) {
    for (auto& type : m_types) {
        if (type)
            type->SetTopLevelContent(content_name);
    }
}

unsigned int PlanetType::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::PlanetType");
    CheckSums::CheckSumCombine(retval, m_types);

    TraceLogger() << "GetCheckSum(PlanetType): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// PlanetSize                                            //
///////////////////////////////////////////////////////////
PlanetSize::PlanetSize(std::vector<std::unique_ptr<ValueRef::ValueRefBase< ::PlanetSize>>>&& sizes) :
    ConditionBase(),
    m_sizes(std::move(sizes))
{}

bool PlanetSize::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const PlanetSize& rhs_ = static_cast<const PlanetSize&>(rhs);

    if (m_sizes.size() != rhs_.m_sizes.size())
        return false;
    for (unsigned int i = 0; i < m_sizes.size(); ++i) {
        CHECK_COND_VREF_MEMBER(m_sizes.at(i))
    }

    return true;
}

namespace {
    struct PlanetSizeSimpleMatch {
        PlanetSizeSimpleMatch(const std::vector< ::PlanetSize>& sizes) :
            m_sizes(sizes)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;

            // is it a planet or on a planet? TODO: This concept should be generalized and factored out.
            auto planet = std::dynamic_pointer_cast<const Planet>(candidate);
            std::shared_ptr<const ::Building> building;
            if (!planet && (building = std::dynamic_pointer_cast<const ::Building>(candidate))) {
                planet = GetPlanet(building->PlanetID());
            }
            if (planet) {
                // is it one of the specified building types?
                for (auto size : m_sizes) {
                    if (planet->Size() == size)
                        return true;
                }
            }

            return false;
        }

        const std::vector< ::PlanetSize>& m_sizes;
    };
}

void PlanetSize::Eval(const ScriptingContext& parent_context,
                                 ObjectSet& matches, ObjectSet& non_matches,
                                 SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = parent_context.condition_root_candidate || RootCandidateInvariant();
    if (simple_eval_safe) {
        // check each valueref for invariance to local candidate
        for (auto& size : m_sizes) {
            if (!size->LocalCandidateInvariant()) {
                simple_eval_safe = false;
                break;
            }
        }
    }
    if (simple_eval_safe) {
        // evaluate types once, and use to check all candidate objects
        std::vector< ::PlanetSize> sizes;
        // get all types from valuerefs
        for (auto& size : m_sizes) {
            sizes.push_back(size->Eval(parent_context));
        }
        EvalImpl(matches, non_matches, search_domain, PlanetSizeSimpleMatch(sizes));
    } else {
        // re-evaluate contained objects for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool PlanetSize::RootCandidateInvariant() const {
    for (auto& size : m_sizes) {
        if (!size->RootCandidateInvariant())
            return false;
    }
    return true;
}

bool PlanetSize::TargetInvariant() const {
    for (auto& size : m_sizes) {
        if (!size->TargetInvariant())
            return false;
    }
    return true;
}

bool PlanetSize::SourceInvariant() const {
    for (auto& size : m_sizes) {
        if (!size->SourceInvariant())
            return false;
    }
    return true;
}

std::string PlanetSize::Description(bool negated/* = false*/) const {
    std::string values_str;
    for (unsigned int i = 0; i < m_sizes.size(); ++i) {
        values_str += m_sizes[i]->ConstantExpr() ?
                        UserString(boost::lexical_cast<std::string>(m_sizes[i]->Eval())) :
                        m_sizes[i]->Description();
        if (2 <= m_sizes.size() && i < m_sizes.size() - 2) {
            values_str += ", ";
        } else if (i == m_sizes.size() - 2) {
            values_str += m_sizes.size() < 3 ? " " : ", ";
            values_str += UserString("OR");
            values_str += " ";
        }
    }
    return str(FlexibleFormat((!negated)
        ? UserString("DESC_PLANET_SIZE")
        : UserString("DESC_PLANET_SIZE_NOT"))
        % values_str);
}

std::string PlanetSize::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "Planet size = ";
    if (m_sizes.size() == 1) {
        retval += m_sizes[0]->Dump(ntabs) + "\n";
    } else {
        retval += "[ ";
        for (auto& size : m_sizes) {
            retval += size->Dump(ntabs) + " ";
        }
        retval += "]\n";
    }
    return retval;
}

void PlanetSize::GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                   ObjectSet& condition_non_targets) const
{
    AddPlanetSet(condition_non_targets);
    AddBuildingSet(condition_non_targets);
}

bool PlanetSize::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "PlanetSize::Match passed no candidate object";
        return false;
    }

    auto planet = std::dynamic_pointer_cast<const Planet>(candidate);
    std::shared_ptr<const ::Building> building;
    if (!planet && (building = std::dynamic_pointer_cast<const ::Building>(candidate))) {
        planet = GetPlanet(building->PlanetID());
    }
    if (planet) {
        for (auto& size : m_sizes) {
            if (size->Eval(local_context) == planet->Size())
                return true;
        }
    }
    return false;
}

void PlanetSize::SetTopLevelContent(const std::string& content_name) {
    for (auto& size : m_sizes) {
        if (size)
            size->SetTopLevelContent(content_name);
    }
}

unsigned int PlanetSize::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::PlanetSize");
    CheckSums::CheckSumCombine(retval, m_sizes);

    TraceLogger() << "GetCheckSum(PlanetSize): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// PlanetEnvironment                                     //
///////////////////////////////////////////////////////////
PlanetEnvironment::PlanetEnvironment(std::vector<std::unique_ptr<ValueRef::ValueRefBase< ::PlanetEnvironment>>>&& environments,
                                     std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& species_name_ref) :
    ConditionBase(),
    m_environments(std::move(environments)),
    m_species_name(std::move(species_name_ref))
{}

bool PlanetEnvironment::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const PlanetEnvironment& rhs_ = static_cast<const PlanetEnvironment&>(rhs);

    CHECK_COND_VREF_MEMBER(m_species_name)

    if (m_environments.size() != rhs_.m_environments.size())
        return false;
    for (unsigned int i = 0; i < m_environments.size(); ++i) {
        CHECK_COND_VREF_MEMBER(m_environments.at(i))
    }

    return true;
}

namespace {
    struct PlanetEnvironmentSimpleMatch {
        PlanetEnvironmentSimpleMatch(const std::vector< ::PlanetEnvironment>& environments,
                                     const std::string& species = "") :
            m_environments(environments),
            m_species(species)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;

            // is it a planet or on a planet? TODO: factor out
            auto planet = std::dynamic_pointer_cast<const Planet>(candidate);
            std::shared_ptr<const ::Building> building;
            if (!planet && (building = std::dynamic_pointer_cast<const ::Building>(candidate))) {
                planet = GetPlanet(building->PlanetID());
            }
            if (planet) {
                // is it one of the specified building types?
                for (auto environment : m_environments) {
                    if (planet->EnvironmentForSpecies(m_species) == environment)
                        return true;
                }
            }

            return false;
        }

        const std::vector< ::PlanetEnvironment>&    m_environments;
        const std::string&                          m_species;
    };
}

void PlanetEnvironment::Eval(const ScriptingContext& parent_context,
                             ObjectSet& matches, ObjectSet& non_matches,
                             SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = ((!m_species_name || m_species_name->LocalCandidateInvariant()) &&
                             (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // check each valueref for invariance to local candidate
        for (auto& environment : m_environments) {
            if (!environment->LocalCandidateInvariant()) {
                simple_eval_safe = false;
                break;
            }
        }
    }
    if (simple_eval_safe) {
        // evaluate types once, and use to check all candidate objects
        std::vector< ::PlanetEnvironment> environments;
        // get all types from valuerefs
        for (auto& environment : m_environments) {
            environments.push_back(environment->Eval(parent_context));
        }
        std::string species_name;
        if (m_species_name)
            species_name = m_species_name->Eval(parent_context);
        EvalImpl(matches, non_matches, search_domain, PlanetEnvironmentSimpleMatch(environments, species_name));
    } else {
        // re-evaluate contained objects for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool PlanetEnvironment::RootCandidateInvariant() const {
    if (m_species_name && !m_species_name->RootCandidateInvariant())
        return false;
    for (auto& environment : m_environments) {
        if (!environment->RootCandidateInvariant())
            return false;
    }
    return true;
}

bool PlanetEnvironment::TargetInvariant() const {
    if (m_species_name && !m_species_name->TargetInvariant())
        return false;
    for (auto& environment : m_environments) {
        if (!environment->TargetInvariant())
            return false;
    }
    return true;
}

bool PlanetEnvironment::SourceInvariant() const {
    if (m_species_name && !m_species_name->SourceInvariant())
        return false;
    for (auto& environment : m_environments) {
        if (!environment->SourceInvariant())
            return false;
    }
    return true;
}

std::string PlanetEnvironment::Description(bool negated/* = false*/) const {
    std::string values_str;
    for (unsigned int i = 0; i < m_environments.size(); ++i) {
        values_str += m_environments[i]->ConstantExpr() ?
                        UserString(boost::lexical_cast<std::string>(m_environments[i]->Eval())) :
                        m_environments[i]->Description();
        if (2 <= m_environments.size() && i < m_environments.size() - 2) {
            values_str += ", ";
        } else if (i == m_environments.size() - 2) {
            values_str += m_environments.size() < 3 ? " " : ", ";
            values_str += UserString("OR");
            values_str += " ";
        }
    }
    std::string species_str;
    if (m_species_name) {
        species_str = m_species_name->Description();
        if (m_species_name->ConstantExpr() && UserStringExists(species_str))
            species_str = UserString(species_str);
    }
    if (species_str.empty())
        species_str = UserString("DESC_PLANET_ENVIRONMENT_CUR_SPECIES");
    return str(FlexibleFormat((!negated)
        ? UserString("DESC_PLANET_ENVIRONMENT")
        : UserString("DESC_PLANET_ENVIRONMENT_NOT"))
        % values_str
        % species_str);
}

std::string PlanetEnvironment::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "Planet environment = ";
    if (m_environments.size() == 1) {
        retval += m_environments[0]->Dump(ntabs);
    } else {
        retval += "[ ";
        for (auto& environment : m_environments) {
            retval += environment->Dump(ntabs) + " ";
        }
        retval += "]";
    }
    if (m_species_name)
        retval += " species = " + m_species_name->Dump(ntabs);
    retval += "\n";
    return retval;
}

void PlanetEnvironment::GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                          ObjectSet& condition_non_targets) const
{
    AddPlanetSet(condition_non_targets);
    AddBuildingSet(condition_non_targets);
}

bool PlanetEnvironment::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "PlanetEnvironment::Match passed no candidate object";
        return false;
    }

    // is it a planet or on a planet? TODO: factor out
    auto planet = std::dynamic_pointer_cast<const Planet>(candidate);
    std::shared_ptr<const ::Building> building;
    if (!planet && (building = std::dynamic_pointer_cast<const ::Building>(candidate))) {
        planet = GetPlanet(building->PlanetID());
    }
    if (!planet)
        return false;

    std::string species_name;
    if (m_species_name)
        species_name = m_species_name->Eval(local_context);

    auto env_for_planets_species = planet->EnvironmentForSpecies(species_name);
    for (auto& environment : m_environments) {
        if (environment->Eval(local_context) == env_for_planets_species)
            return true;
    }
    return false;
}

void PlanetEnvironment::SetTopLevelContent(const std::string& content_name) {
    if (m_species_name)
        m_species_name->SetTopLevelContent(content_name);
    for (auto& environment : m_environments) {
        if (environment)
            environment->SetTopLevelContent(content_name);
    }
}

unsigned int PlanetEnvironment::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::PlanetEnvironment");
    CheckSums::CheckSumCombine(retval, m_environments);
    CheckSums::CheckSumCombine(retval, m_species_name);

    TraceLogger() << "GetCheckSum(PlanetEnvironment): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// Species                                               //
///////////////////////////////////////////////////////////
Species::Species(std::vector<std::unique_ptr<ValueRef::ValueRefBase<std::string>>>&& names) :
    ConditionBase(),
    m_names(std::move(names))
{}

Species::Species() :
    ConditionBase(),
    m_names()
{}

bool Species::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const Species& rhs_ = static_cast<const Species&>(rhs);

    if (m_names.size() != rhs_.m_names.size())
        return false;
    for (unsigned int i = 0; i < m_names.size(); ++i) {
        CHECK_COND_VREF_MEMBER(m_names.at(i))
    }

    return true;
}

namespace {
    struct SpeciesSimpleMatch {
        SpeciesSimpleMatch(const std::vector<std::string>& names) :
            m_names(names)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;

            // is it a population centre?
            if (auto pop = std::dynamic_pointer_cast<const ::PopCenter>(candidate)) {
                const std::string& species_name = pop->SpeciesName();
                // if the popcenter has a species and that species is one of those specified...
                return !species_name.empty() && (m_names.empty() || std::count(m_names.begin(), m_names.end(), species_name));
            }
            // is it a ship?
            if (auto ship = std::dynamic_pointer_cast<const Ship>(candidate)) {
                // if the ship has a species and that species is one of those specified...
                const std::string& species_name = ship->SpeciesName();
                return !species_name.empty() && (m_names.empty() || std::count(m_names.begin(), m_names.end(), species_name));
            }
            // is it a building on a planet?
            if (auto building = std::dynamic_pointer_cast<const ::Building>(candidate)) {
                auto planet = GetPlanet(building->PlanetID());
                const std::string& species_name = planet->SpeciesName();
                // if the planet (which IS a popcenter) has a species and that species is one of those specified...
                return !species_name.empty() && (m_names.empty() || std::count(m_names.begin(), m_names.end(), species_name));
            }

            return false;
        }

        const std::vector<std::string>& m_names;
    };
}

void Species::Eval(const ScriptingContext& parent_context,
                   ObjectSet& matches, ObjectSet& non_matches,
                   SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = parent_context.condition_root_candidate || RootCandidateInvariant();
    if (simple_eval_safe) {
        // check each valueref for invariance to local candidate
        for (auto& name : m_names) {
            if (!name->LocalCandidateInvariant()) {
                simple_eval_safe = false;
                break;
            }
        }
    }
    if (simple_eval_safe) {
        // evaluate names once, and use to check all candidate objects
        std::vector<std::string> names;
        // get all names from valuerefs
        for (auto& name : m_names) {
            names.push_back(name->Eval(parent_context));
        }
        EvalImpl(matches, non_matches, search_domain, SpeciesSimpleMatch(names));
    } else {
        // re-evaluate allowed building types range for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Species::RootCandidateInvariant() const {
    for (auto& name : m_names) {
        if (!name->RootCandidateInvariant())
            return false;
    }
    return true;
}

bool Species::TargetInvariant() const {
    for (auto& name : m_names) {
        if (!name->TargetInvariant())
            return false;
    }
    return true;
}

bool Species::SourceInvariant() const {
    for (auto& name : m_names) {
        if (!name->SourceInvariant())
            return false;
    }
    return true;
}

std::string Species::Description(bool negated/* = false*/) const {
    std::string values_str;
    if (m_names.empty())
        values_str = "(" + UserString("CONDITION_ANY") +")";
    for (unsigned int i = 0; i < m_names.size(); ++i) {
        values_str += m_names[i]->ConstantExpr() ?
                        UserString(m_names[i]->Eval()) :
                        m_names[i]->Description();
        if (2 <= m_names.size() && i < m_names.size() - 2) {
            values_str += ", ";
        } else if (i == m_names.size() - 2) {
            values_str += m_names.size() < 3 ? " " : ", ";
            values_str += UserString("OR");
            values_str += " ";
        }
    }
    return str(FlexibleFormat((!negated)
        ? UserString("DESC_SPECIES")
        : UserString("DESC_SPECIES_NOT"))
        % values_str);
}

std::string Species::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "Species";
    if (m_names.empty()) {
        // do nothing else
    } else if (m_names.size() == 1) {
        retval += " name = " + m_names[0]->Dump(ntabs) + "\n";
    } else {
        retval += " name = [ ";
        for (auto& name : m_names) {
            retval += name->Dump(ntabs) + " ";
        }
        retval += "]\n";
    }
    return retval;
}

void Species::GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                ObjectSet& condition_non_targets) const
{
    AddPlanetSet(condition_non_targets);
    AddBuildingSet(condition_non_targets);
    AddShipSet(condition_non_targets);
}

bool Species::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "Species::Match passed no candidate object";
        return false;
    }

    // is it a planet or a building on a planet? TODO: factor out
    auto planet = std::dynamic_pointer_cast<const Planet>(candidate);
    std::shared_ptr<const ::Building> building;
    if (!planet && (building = std::dynamic_pointer_cast<const ::Building>(candidate))) {
        planet = GetPlanet(building->PlanetID());
    }
    if (planet) {
        if (m_names.empty()) {
            return !planet->SpeciesName().empty();  // match any species name
        } else {
            // match only specified species names
            for (auto& name : m_names) {
                if (name->Eval(local_context) == planet->SpeciesName())
                    return true;
            }
        }
    }
    // is it a ship?
    auto ship = std::dynamic_pointer_cast<const Ship>(candidate);
    if (ship) {
        if (m_names.empty()) {
            return !ship->SpeciesName().empty();    // match any species name
        } else {
            // match only specified species names
            for (auto& name : m_names) {
                if (name->Eval(local_context) == ship->SpeciesName())
                    return true;
            }
        }
    }
    return false;
}

void Species::SetTopLevelContent(const std::string& content_name) {
    for (auto& name : m_names) {
        if (name)
            name->SetTopLevelContent(content_name);
    }
}

unsigned int Species::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::Species");
    CheckSums::CheckSumCombine(retval, m_names);

    TraceLogger() << "GetCheckSum(Species): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// Enqueued                                              //
///////////////////////////////////////////////////////////
Enqueued::Enqueued(std::unique_ptr<ValueRef::ValueRefBase<int>>&& design_id,
                   std::unique_ptr<ValueRef::ValueRefBase<int>>&& empire_id,
                   std::unique_ptr<ValueRef::ValueRefBase<int>>&& low,
                   std::unique_ptr<ValueRef::ValueRefBase<int>>&& high) :
    ConditionBase(),
    m_build_type(BT_SHIP),
    m_name(),
    m_design_id(std::move(design_id)),
    m_empire_id(std::move(empire_id)),
    m_low(std::move(low)),
    m_high(std::move(high))
{}

Enqueued::Enqueued() :
    ConditionBase(),
    m_build_type(BT_NOT_BUILDING),
    m_name(),
    m_design_id(nullptr),
    m_empire_id(nullptr),
    m_low(nullptr),
    m_high(nullptr)
{}

Enqueued::Enqueued(BuildType build_type,
                   std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& name,
                   std::unique_ptr<ValueRef::ValueRefBase<int>>&& empire_id,
                   std::unique_ptr<ValueRef::ValueRefBase<int>>&& low,
                   std::unique_ptr<ValueRef::ValueRefBase<int>>&& high) :
    ConditionBase(),
    m_build_type(build_type),
    m_name(std::move(name)),
    m_design_id(nullptr),
    m_empire_id(std::move(empire_id)),
    m_low(std::move(low)),
    m_high(std::move(high))
{}

bool Enqueued::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const Enqueued& rhs_ = static_cast<const Enqueued&>(rhs);

    if (m_build_type != rhs_.m_build_type)
        return false;

    CHECK_COND_VREF_MEMBER(m_name)
    CHECK_COND_VREF_MEMBER(m_design_id)
    CHECK_COND_VREF_MEMBER(m_empire_id)
    CHECK_COND_VREF_MEMBER(m_low)
    CHECK_COND_VREF_MEMBER(m_high)

    return true;
}

namespace {
    int NumberOnQueue(const ProductionQueue& queue, BuildType build_type, const int location_id,
                      const std::string& name = "", int design_id = INVALID_DESIGN_ID)
    {
        int retval = 0;
        for (const auto& element : queue) {
            if (!(build_type == INVALID_BUILD_TYPE || build_type == element.item.build_type))
                continue;
            if (location_id != element.location)
                continue;
            if (build_type == BT_BUILDING) {
                // if looking for buildings, accept specifically named building
                // or any building if no name specified
                if (!name.empty() && element.item.name != name)
                    continue;
            } else if (build_type == BT_SHIP) {
                if (design_id != INVALID_DESIGN_ID) {
                    // if looking for ships, accept design by id number...
                    if (design_id != element.item.design_id)
                        continue;
                } else if (!name.empty()) {
                    // ... or accept design by predefined name
                    const ShipDesign* design = GetShipDesign(element.item.design_id);
                    if (!design || name != design->Name(false))
                        continue;
                }
            } // else: looking for any production item

            retval += element.blocksize;
        }
        return retval;
    }

    struct EnqueuedSimpleMatch {
        EnqueuedSimpleMatch(BuildType build_type, const std::string& name, int design_id,
                            int empire_id, int low, int high) :
            m_build_type(build_type),
            m_name(name),
            m_design_id(design_id),
            m_empire_id(empire_id),
            m_low(low),
            m_high(high)
        {}
        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;

            int count = 0;

            if (m_empire_id == ALL_EMPIRES) {
                for (auto& item : Empires()) {
                    const Empire* empire = item.second;
                    count += NumberOnQueue(empire->GetProductionQueue(), m_build_type,
                                           candidate->ID(), m_name, m_design_id);
                }

            } else {
                const Empire* empire = GetEmpire(m_empire_id);
                if (!empire) return false;
                count = NumberOnQueue(empire->GetProductionQueue(), m_build_type,
                                      candidate->ID(), m_name, m_design_id);
            }

            return (m_low <= count && count <= m_high);
        }

        BuildType   m_build_type;
        std::string m_name;
        int         m_design_id;
        int         m_empire_id;
        int         m_low;
        int         m_high;
    };
}

void Enqueued::Eval(const ScriptingContext& parent_context,
                    ObjectSet& matches, ObjectSet& non_matches,
                    SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = parent_context.condition_root_candidate || RootCandidateInvariant();
    if (simple_eval_safe) {
        // check each valueref for invariance to local candidate
        if ((m_name &&      !m_name->LocalCandidateInvariant()) ||
            (m_design_id && !m_design_id->LocalCandidateInvariant()) ||
            (m_empire_id && !m_empire_id->LocalCandidateInvariant()) ||
            (m_low &&       !m_low->LocalCandidateInvariant()) ||
            (m_high &&      !m_high->LocalCandidateInvariant()))
        { simple_eval_safe = false; }
    }
    if (simple_eval_safe) {
        // evaluate valuerefs once, and use to check all candidate objects
        std::string name =  (m_name ?       m_name->Eval(parent_context) :      "");
        int design_id =     (m_design_id ?  m_design_id->Eval(parent_context) : INVALID_DESIGN_ID);
        int empire_id =     (m_empire_id ?  m_empire_id->Eval(parent_context) : ALL_EMPIRES);
        int low =           (m_low ?        m_low->Eval(parent_context) :       0);
        int high =          (m_high ?       m_high->Eval(parent_context) :      INT_MAX);
        // special case: if neither low nor high is specified, default to a
        // minimum of 1, so that just matching "Enqueued (type) (name/id)" will
        // match places where at least one of the specified item is enqueued.
        // if a max or other minimum are specified, then default to 0 low, so
        // that just specifying a max will include anything below that max,
        // including 0.
        if (!m_low && !m_high)
            low = 1;

        EvalImpl(matches, non_matches, search_domain, EnqueuedSimpleMatch(m_build_type, name, design_id, 
                                                                          empire_id, low, high));
    } else {
        // re-evaluate allowed building types range for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Enqueued::RootCandidateInvariant() const {
    if ((m_name &&      !m_name->RootCandidateInvariant()) ||
        (m_design_id && !m_design_id->RootCandidateInvariant()) ||
        (m_empire_id && !m_empire_id->RootCandidateInvariant()) ||
        (m_low &&       !m_low->RootCandidateInvariant()) ||
        (m_high &&      !m_high->RootCandidateInvariant()))
    { return false; }
    return true;
}

bool Enqueued::TargetInvariant() const {
    if ((m_name &&      !m_name->TargetInvariant()) ||
        (m_design_id && !m_design_id->TargetInvariant()) ||
        (m_empire_id && !m_empire_id->TargetInvariant()) ||
        (m_low &&       !m_low->TargetInvariant()) ||
        (m_high &&      !m_high->TargetInvariant()))
    { return false; }
    return true;
}

bool Enqueued::SourceInvariant() const {
    if ((m_name &&      !m_name->SourceInvariant()) ||
        (m_design_id && !m_design_id->SourceInvariant()) ||
        (m_empire_id && !m_empire_id->SourceInvariant()) ||
        (m_low &&       !m_low->SourceInvariant()) ||
        (m_high &&      !m_high->SourceInvariant()))
    { return false; }
    return true;
}

std::string Enqueued::Description(bool negated/* = false*/) const {
    std::string empire_str;
    if (m_empire_id) {
        int empire_id = ALL_EMPIRES;
        if (m_empire_id->ConstantExpr())
            empire_id = m_empire_id->Eval();
        if (const Empire* empire = GetEmpire(empire_id))
            empire_str = empire->Name();
        else
            empire_str = m_empire_id->Description();
    }
    std::string low_str = "1";
    if (m_low) {
        low_str = m_low->ConstantExpr() ?
                    std::to_string(m_low->Eval()) :
                    m_low->Description();
    }
    std::string high_str = std::to_string(INT_MAX);
    if (m_high) {
        high_str = m_high->ConstantExpr() ?
                    std::to_string(m_high->Eval()) :
                    m_high->Description();
    }
    std::string what_str;
    if (m_name) {
        what_str = m_name->Description();
        if (m_name->ConstantExpr() && UserStringExists(what_str))
            what_str = UserString(what_str);
    } else if (m_design_id) {
        what_str = m_design_id->ConstantExpr() ?
                    std::to_string(m_design_id->Eval()) :
                    m_design_id->Description();
    }
    std::string description_str;
    switch (m_build_type) {
    case BT_BUILDING:   description_str = (!negated)
                            ? UserString("DESC_ENQUEUED_BUILDING")
                            : UserString("DESC_ENQUEUED_BUILDING_NOT");
    break;
    case BT_SHIP:       description_str = (!negated)
                            ? UserString("DESC_ENQUEUED_DESIGN")
                            : UserString("DESC_ENQUEUED_DESIGN_NOT");
    break;
    default:            description_str = (!negated)
                            ? UserString("DESC_ENQUEUED")
                            : UserString("DESC_ENQUEUED_NOT");
    break;
    }
    return str(FlexibleFormat(description_str)
               % empire_str
               % low_str
               % high_str
               % what_str);
}

std::string Enqueued::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "Enqueued";

    if (m_build_type == BT_BUILDING) {
        retval += " type = Building";
        if (m_name)
            retval += " name = " + m_name->Dump(ntabs);
    } else if (m_build_type == BT_SHIP) {
        retval += " type = Ship";
        if (m_name)
            retval += " design = " + m_name->Dump(ntabs);
        else if (m_design_id)
            retval += " design = " + m_design_id->Dump(ntabs);
    }
    if (m_empire_id)
        retval += " empire = " + m_empire_id->Dump(ntabs);
    if (m_low)
        retval += " low = " + m_low->Dump(ntabs);
    if (m_high)
        retval += " high = " + m_high->Dump(ntabs);
    retval += "\n";
    return retval;
}

bool Enqueued::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "Enqueued::Match passed no candidate object";
        return false;
    }
    std::string name =  (m_name ?       m_name->Eval(local_context) :       "");
    int empire_id =     (m_empire_id ?  m_empire_id->Eval(local_context) :  ALL_EMPIRES);
    int design_id =     (m_design_id ?  m_design_id->Eval(local_context) :  INVALID_DESIGN_ID);
    int low =           (m_low ?        m_low->Eval(local_context) :        0);
    int high =          (m_high ?       m_high->Eval(local_context) :       INT_MAX);
    return EnqueuedSimpleMatch(m_build_type, name, design_id, empire_id, low, high)(candidate);
}

void Enqueued::GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                 ObjectSet& condition_non_targets) const
{
    AddPlanetSet(condition_non_targets);
}

void Enqueued::SetTopLevelContent(const std::string& content_name) {
    if (m_name)
        m_name->SetTopLevelContent(content_name);
    if (m_design_id)
        m_design_id->SetTopLevelContent(content_name);
    if (m_empire_id)
        m_empire_id->SetTopLevelContent(content_name);
    if (m_low)
        m_low->SetTopLevelContent(content_name);
    if (m_high)
        m_high->SetTopLevelContent(content_name);
}

unsigned int Enqueued::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::Enqueued");
    CheckSums::CheckSumCombine(retval, m_name);
    CheckSums::CheckSumCombine(retval, m_design_id);
    CheckSums::CheckSumCombine(retval, m_empire_id);
    CheckSums::CheckSumCombine(retval, m_low);
    CheckSums::CheckSumCombine(retval, m_high);

    TraceLogger() << "GetCheckSum(Enqueued): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// FocusType                                             //
///////////////////////////////////////////////////////////
FocusType::FocusType(std::vector<std::unique_ptr<ValueRef::ValueRefBase<std::string>>>&& names) :
    ConditionBase(),
    m_names(std::move(names))
{}

bool FocusType::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const FocusType& rhs_ = static_cast<const FocusType&>(rhs);

    if (m_names.size() != rhs_.m_names.size())
        return false;
    for (unsigned int i = 0; i < m_names.size(); ++i) {
        CHECK_COND_VREF_MEMBER(m_names.at(i))
    }

    return true;
}

namespace {
    struct FocusTypeSimpleMatch {
        FocusTypeSimpleMatch(const std::vector<std::string>& names) :
            m_names(names)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;

            // is it a ResourceCenter or a Building on a Planet (that is a ResourceCenter)
            auto res_center = std::dynamic_pointer_cast<const ResourceCenter>(candidate);
            std::shared_ptr<const ::Building> building;
            if (!res_center && (building = std::dynamic_pointer_cast<const ::Building>(candidate))) {
                if (auto planet = GetPlanet(building->PlanetID()))
                    res_center = std::dynamic_pointer_cast<const ResourceCenter>(planet);
            }
            if (res_center) {
                return !res_center->Focus().empty() &&
                    std::count(m_names.begin(), m_names.end(), res_center->Focus());
            }

            return false;
        }

        const std::vector<std::string>& m_names;
    };
}

void FocusType::Eval(const ScriptingContext& parent_context,
                     ObjectSet& matches, ObjectSet& non_matches,
                     SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = parent_context.condition_root_candidate || RootCandidateInvariant();
    if (simple_eval_safe) {
        // check each valueref for invariance to local candidate
        for (auto& name : m_names) {
            if (!name->LocalCandidateInvariant()) {
                simple_eval_safe = false;
                break;
            }
        }
    }
    if (simple_eval_safe) {
        // evaluate names once, and use to check all candidate objects
        std::vector<std::string> names;
        // get all names from valuerefs
        for (auto& name : m_names) {
            names.push_back(name->Eval(parent_context));
        }
        EvalImpl(matches, non_matches, search_domain, FocusTypeSimpleMatch(names));
    } else {
        // re-evaluate allowed building types range for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool FocusType::RootCandidateInvariant() const {
    for (auto& name : m_names) {
        if (!name->RootCandidateInvariant())
            return false;
    }
    return true;
}

bool FocusType::TargetInvariant() const {
    for (auto& name : m_names) {
        if (!name->TargetInvariant())
            return false;
    }
    return true;
}

bool FocusType::SourceInvariant() const {
    for (auto& name : m_names) {
        if (!name->SourceInvariant())
            return false;
    }
    return true;
}

std::string FocusType::Description(bool negated/* = false*/) const {
    std::string values_str;
    for (unsigned int i = 0; i < m_names.size(); ++i) {
        values_str += m_names[i]->ConstantExpr() ?
            UserString(m_names[i]->Eval()) :
            m_names[i]->Description();
        if (2 <= m_names.size() && i < m_names.size() - 2) {
            values_str += ", ";
        } else if (i == m_names.size() - 2) {
            values_str += m_names.size() < 3 ? " " : ", ";
            values_str += UserString("OR");
            values_str += " ";
        }
    }
    return str(FlexibleFormat((!negated)
        ? UserString("DESC_FOCUS_TYPE")
        : UserString("DESC_FOCUS_TYPE_NOT"))
        % values_str);
}

std::string FocusType::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "Focus name = ";
    if (m_names.size() == 1) {
        retval += m_names[0]->Dump(ntabs) + "\n";
    } else {
        retval += "[ ";
        for (auto& name : m_names) {
            retval += name->Dump(ntabs) + " ";
        }
        retval += "]\n";
    }
    return retval;
}

bool FocusType::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "FocusType::Match passed no candidate object";
        return false;
    }

    // is it a ResourceCenter or a Building on a Planet (that is a ResourceCenter)
    auto res_center = std::dynamic_pointer_cast<const ResourceCenter>(candidate);
    std::shared_ptr<const ::Building> building;
    if (!res_center && (building = std::dynamic_pointer_cast<const ::Building>(candidate))) {
        if (auto planet = GetPlanet(building->PlanetID()))
            res_center = std::dynamic_pointer_cast<const ResourceCenter>(planet);
    }
    if (res_center) {
        for (auto& name : m_names) {
            if (name->Eval(local_context) == res_center->Focus())
                return true;
        }
    }
    return false;
}

void FocusType::GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                  ObjectSet& condition_non_targets) const
{
    AddPlanetSet(condition_non_targets);
    AddBuildingSet(condition_non_targets);
}

void FocusType::SetTopLevelContent(const std::string& content_name) {
    for (auto& name : m_names) {
        if (name)
            name->SetTopLevelContent(content_name);
    }
}

unsigned int FocusType::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::FocusType");
    CheckSums::CheckSumCombine(retval, m_names);

    TraceLogger() << "GetCheckSum(FocusType): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// StarType                                              //
///////////////////////////////////////////////////////////
StarType::StarType(std::vector<std::unique_ptr<ValueRef::ValueRefBase< ::StarType>>>&& types) :
    ConditionBase(),
    m_types(std::move(types))
{}

bool StarType::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const StarType& rhs_ = static_cast<const StarType&>(rhs);

    if (m_types.size() != rhs_.m_types.size())
        return false;
    for (unsigned int i = 0; i < m_types.size(); ++i) {
        CHECK_COND_VREF_MEMBER(m_types.at(i))
    }

    return true;
}

namespace {
    struct StarTypeSimpleMatch {
        StarTypeSimpleMatch(const std::vector< ::StarType>& types) :
            m_types(types)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;

            std::shared_ptr<const System> system = GetSystem(candidate->SystemID());
            if (system || (system = std::dynamic_pointer_cast<const System>(candidate)))
                return !m_types.empty() && std::count(m_types.begin(), m_types.end(), system->GetStarType());

            return false;
        }

        const std::vector< ::StarType>& m_types;
    };
}

void StarType::Eval(const ScriptingContext& parent_context,
                    ObjectSet& matches, ObjectSet& non_matches,
                    SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = parent_context.condition_root_candidate || RootCandidateInvariant();
    if (simple_eval_safe) {
        // check each valueref for invariance to local candidate
        for (auto& type : m_types) {
            if (!type->LocalCandidateInvariant()) {
                simple_eval_safe = false;
                break;
            }
        }
    }
    if (simple_eval_safe) {
        // evaluate types once, and use to check all candidate objects
        std::vector< ::StarType> types;
        // get all types from valuerefs
        for (auto& type : m_types) {
            types.push_back(type->Eval(parent_context));
        }
        EvalImpl(matches, non_matches, search_domain, StarTypeSimpleMatch(types));
    } else {
        // re-evaluate contained objects for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool StarType::RootCandidateInvariant() const {
    for (auto& type : m_types) {
        if (!type->RootCandidateInvariant())
            return false;
    }
    return true;
}

bool StarType::TargetInvariant() const {
    for (auto& type : m_types) {
        if (!type->TargetInvariant())
            return false;
    }
    return true;
}

bool StarType::SourceInvariant() const {
    for (auto& type : m_types) {
        if (!type->SourceInvariant())
            return false;
    }
    return true;
}

std::string StarType::Description(bool negated/* = false*/) const {
    std::string values_str;
    for (unsigned int i = 0; i < m_types.size(); ++i) {
        values_str += m_types[i]->ConstantExpr() ?
                        UserString(boost::lexical_cast<std::string>(m_types[i]->Eval())) :
                        m_types[i]->Description();
        if (2 <= m_types.size() && i < m_types.size() - 2) {
            values_str += ", ";
        } else if (i == m_types.size() - 2) {
            values_str += m_types.size() < 3 ? " " : ", ";
            values_str += UserString("OR");
            values_str += " ";
        }
    }
    return str(FlexibleFormat((!negated)
        ? UserString("DESC_STAR_TYPE")
        : UserString("DESC_STAR_TYPE_NOT"))
        % values_str);
}

std::string StarType::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "Star type = ";
    if (m_types.size() == 1) {
        retval += m_types[0]->Dump(ntabs) + "\n";
    } else {
        retval += "[ ";
        for (auto& type : m_types) {
            retval += type->Dump(ntabs) + " ";
        }
        retval += "]\n";
    }
    return retval;
}

bool StarType::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "StarType::Match passed no candidate object";
        return false;
    }

    std::shared_ptr<const System> system = GetSystem(candidate->SystemID());
    if (system || (system = std::dynamic_pointer_cast<const System>(candidate))) {
        for (auto& type : m_types) {
            if (type->Eval(local_context) == system->GetStarType())
                return true;
        }
    }
    return false;
}

void StarType::SetTopLevelContent(const std::string& content_name) {
    for (auto& type : m_types) {
        if (type)
            (type)->SetTopLevelContent(content_name);
    }
}

unsigned int StarType::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::StarType");
    CheckSums::CheckSumCombine(retval, m_types);

    TraceLogger() << "GetCheckSum(StarType): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// DesignHasHull                                         //
///////////////////////////////////////////////////////////
DesignHasHull::DesignHasHull(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& name) :
    ConditionBase(),
    m_name(std::move(name))
{}

bool DesignHasHull::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const DesignHasHull& rhs_ = static_cast<const DesignHasHull&>(rhs);

    CHECK_COND_VREF_MEMBER(m_name);

    return true;
}

namespace {
    struct DesignHasHullSimpleMatch {
        explicit DesignHasHullSimpleMatch(const std::string& name) :
            m_name(name)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;

            // is it a ship?
            auto ship = std::dynamic_pointer_cast<const ::Ship>(candidate);
            if (!ship)
                return false;
            // with a valid design?
            const ShipDesign* design = ship->Design();
            if (!design)
                return false;

            return design->Hull() == m_name;
        }

        const std::string&  m_name;
    };
}

void DesignHasHull::Eval(const ScriptingContext& parent_context,
                         ObjectSet& matches, ObjectSet& non_matches,
                         SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = (!m_name || m_name->LocalCandidateInvariant()) &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant());
    if (simple_eval_safe) {
        // evaluate number limits once, use to match all candidates
        std::shared_ptr<const UniverseObject> no_object;
        ScriptingContext local_context(parent_context, no_object);
        std::string name = (m_name ? m_name->Eval(local_context) : "");
        EvalImpl(matches, non_matches, search_domain, DesignHasHullSimpleMatch(name));
    } else {
        // re-evaluate allowed turn range for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool DesignHasHull::RootCandidateInvariant() const
{ return (!m_name || m_name->RootCandidateInvariant()); }

bool DesignHasHull::TargetInvariant() const
{ return (!m_name || m_name->TargetInvariant()); }

bool DesignHasHull::SourceInvariant() const
{ return (!m_name || m_name->SourceInvariant()); }

std::string DesignHasHull::Description(bool negated/* = false*/) const {
    std::string name_str;
    if (m_name) {
        name_str = m_name->Description();
        if (m_name->ConstantExpr() && UserStringExists(name_str))
            name_str = UserString(name_str);
    }
    return str(FlexibleFormat((!negated)
        ? UserString("DESC_DESIGN_HAS_HULL")
        : UserString("DESC_DESIGN_HAS_HULL_NOT"))
        % name_str);
}

std::string DesignHasHull::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "DesignHasHull";
    if (m_name)
        retval += " name = " + m_name->Dump(ntabs);
    retval += "\n";
    return retval;
}

bool DesignHasHull::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "DesignHasHull::Match passed no candidate object";
        return false;
    }

    std::string name = (m_name ? m_name->Eval(local_context) : "");

    return DesignHasHullSimpleMatch(name)(candidate);
}

void DesignHasHull::GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                      ObjectSet& condition_non_targets) const
{
    AddShipSet(condition_non_targets);
}

void DesignHasHull::SetTopLevelContent(const std::string& content_name) {
    if (m_name)
        m_name->SetTopLevelContent(content_name);
}

unsigned int DesignHasHull::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::DesignHasHull");
    CheckSums::CheckSumCombine(retval, m_name);

    TraceLogger() << "GetCheckSum(DesignHasHull): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// DesignHasPart                                         //
///////////////////////////////////////////////////////////
DesignHasPart::DesignHasPart(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& name,
                             std::unique_ptr<ValueRef::ValueRefBase<int>>&& low,
                             std::unique_ptr<ValueRef::ValueRefBase<int>>&& high) :
    ConditionBase(),
    m_low(std::move(low)),
    m_high(std::move(high)),
    m_name(std::move(name))
{}

bool DesignHasPart::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const DesignHasPart& rhs_ = static_cast<const DesignHasPart&>(rhs);

    CHECK_COND_VREF_MEMBER(m_name);
    CHECK_COND_VREF_MEMBER(m_low)
    CHECK_COND_VREF_MEMBER(m_high)

    return true;
}

namespace {
    struct DesignHasPartSimpleMatch {
        DesignHasPartSimpleMatch(int low, int high, const std::string& name) :
            m_low(low),
            m_high(high),
            m_name(name)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;

            std::shared_ptr<const Ship> ship = nullptr;
            if (auto fighter = std::dynamic_pointer_cast<const ::Fighter>(candidate)) {
                // it is a fighter
                ship = Objects().Object<Ship>(fighter->LaunchedFrom());
            } else {
                ship = std::dynamic_pointer_cast<const ::Ship>(candidate);
            }

            // is it a ship
            if (!ship)
                return false;

            // with a valid design?
            const ShipDesign* design = ship->Design();
            if (!design)
                return false;

            int count = 0;
            for (const std::string& name : design->Parts()) {
                if (name == m_name || (m_name.empty() && !name.empty()))
                    // number of copies of specified part,
                    // or total number of parts if no part name specified
                    ++count;
            }
            return (m_low <= count && count <= m_high);
        }

        int                 m_low;
        int                 m_high;
        const std::string&  m_name;
    };
}

void DesignHasPart::Eval(const ScriptingContext& parent_context,
                         ObjectSet& matches, ObjectSet& non_matches,
                         SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = (!m_low || m_low->LocalCandidateInvariant()) &&
                            (!m_high || m_high->LocalCandidateInvariant()) &&
                            (!m_name || m_name->LocalCandidateInvariant()) &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant());
    if (simple_eval_safe) {
        // evaluate number limits once, use to match all candidates
        std::shared_ptr<const UniverseObject> no_object;
        ScriptingContext local_context(parent_context, no_object);
        std::string name = (m_name ? m_name->Eval(local_context) : "");
        int low =          (m_low ? std::max(0, m_low->Eval(local_context)) : 1);
        int high =         (m_high ? std::min(m_high->Eval(local_context), INT_MAX) : INT_MAX);
        EvalImpl(matches, non_matches, search_domain, DesignHasPartSimpleMatch(low, high, name));
    } else {
        // re-evaluate allowed turn range for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool DesignHasPart::RootCandidateInvariant() const
{   return (!m_low || m_low->RootCandidateInvariant()) &&
           (!m_high || m_high->RootCandidateInvariant()) &&
           (!m_name || m_name->RootCandidateInvariant()); }

bool DesignHasPart::TargetInvariant() const
{   return (!m_low || m_low->TargetInvariant()) &&
           (!m_high || m_high->TargetInvariant()) &&
           (!m_name || m_name->TargetInvariant()); }

bool DesignHasPart::SourceInvariant() const
{   return (!m_low || m_low->SourceInvariant()) &&
           (!m_high || m_high->SourceInvariant()) &&
           (!m_name || m_name->SourceInvariant()); }

std::string DesignHasPart::Description(bool negated/* = false*/) const {
    std::string low_str = "1";
    if (m_low) {
        low_str = m_low->ConstantExpr() ?
                    std::to_string(m_low->Eval()) :
                    m_low->Description();
    }
    std::string high_str = std::to_string(INT_MAX);
    if (m_high) {
        high_str = m_high->ConstantExpr() ?
                    std::to_string(m_high->Eval()) :
                    m_high->Description();
    };
    std::string name_str;
    if (m_name) {
        name_str = m_name->Description();
        if (m_name->ConstantExpr() && UserStringExists(name_str))
            name_str = UserString(name_str);
    }
    return str(FlexibleFormat((!negated)
        ? UserString("DESC_DESIGN_HAS_PART")
        : UserString("DESC_DESIGN_HAS_PART_NOT"))
        % low_str
        % high_str
        % name_str);
}

std::string DesignHasPart::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "DesignHasPart";
    if (m_low)
        retval += "low = " + m_low->Dump(ntabs);
    if (m_high)
        retval += " high = " + m_high->Dump(ntabs);
    if (m_name)
        retval += " name = " + m_name->Dump(ntabs);
    retval += "\n";
    return retval;
}

bool DesignHasPart::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "DesignHasPart::Match passed no candidate object";
        return false;
    }

    int low =  (m_low ? std::max(0, m_low->Eval(local_context)) : 0);
    int high = (m_high ? std::min(m_high->Eval(local_context), IMPOSSIBLY_LARGE_TURN) : IMPOSSIBLY_LARGE_TURN);
    std::string name = (m_name ? m_name->Eval(local_context) : "");

    return DesignHasPartSimpleMatch(low, high, name)(candidate);
}

void DesignHasPart::GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                      ObjectSet& condition_non_targets) const
{
    AddShipSet(condition_non_targets);
}

void DesignHasPart::SetTopLevelContent(const std::string& content_name) {
    if (m_low)
        m_low->SetTopLevelContent(content_name);
    if (m_high)
        m_high->SetTopLevelContent(content_name);
    if (m_name)
        m_name->SetTopLevelContent(content_name);
}

unsigned int DesignHasPart::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::DesignHasPart");
    CheckSums::CheckSumCombine(retval, m_low);
    CheckSums::CheckSumCombine(retval, m_high);
    CheckSums::CheckSumCombine(retval, m_name);

    TraceLogger() << "GetCheckSum(DesignHasPart): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// DesignHasPartClass                                    //
///////////////////////////////////////////////////////////
DesignHasPartClass::DesignHasPartClass(ShipPartClass part_class,
                                       std::unique_ptr<ValueRef::ValueRefBase<int>>&& low,
                                       std::unique_ptr<ValueRef::ValueRefBase<int>>&& high) :
    ConditionBase(),
    m_low(std::move(low)),
    m_high(std::move(high)),
    m_class(std::move(part_class))
{}

bool DesignHasPartClass::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const DesignHasPartClass& rhs_ = static_cast<const DesignHasPartClass&>(rhs);

    if (m_class != rhs_.m_class)
        return false;

    CHECK_COND_VREF_MEMBER(m_low)
    CHECK_COND_VREF_MEMBER(m_high)

    return true;
}

namespace {
    struct DesignHasPartClassSimpleMatch {
        DesignHasPartClassSimpleMatch(int low, int high, ShipPartClass part_class) :
            m_low(low),
            m_high(high),
            m_part_class(part_class)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;

            // is it a ship?
            auto ship = std::dynamic_pointer_cast<const ::Ship>(candidate);
            if (!ship)
                return false;
            // with a valid design?
            const ShipDesign* design = ship->Design();
            if (!design)
                return false;


            int count = 0;
            for (const std::string& name : design->Parts()) {
                if (const PartType* part_type = GetPartType(name)) {
                    if (part_type->Class() == m_part_class)
                        ++count;
                }
            }
            return (m_low <= count && count <= m_high);
        }

        int m_low;
        int m_high;
        ShipPartClass m_part_class;
    };
}

void DesignHasPartClass::Eval(const ScriptingContext& parent_context,
                              ObjectSet& matches, ObjectSet& non_matches,
                              SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = (!m_low || m_low->LocalCandidateInvariant()) &&
                            (!m_high || m_high->LocalCandidateInvariant()) &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant());
    if (simple_eval_safe) {
        // evaluate number limits once, use to match all candidates
        std::shared_ptr<const UniverseObject> no_object;
        ScriptingContext local_context(parent_context, no_object);
        int low =          (m_low ? std::max(0, m_low->Eval(local_context)) : 1);
        int high =         (m_high ? std::min(m_high->Eval(local_context), INT_MAX) : INT_MAX);
        EvalImpl(matches, non_matches, search_domain, DesignHasPartClassSimpleMatch(low, high, m_class));
    } else {
        // re-evaluate allowed turn range for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool DesignHasPartClass::RootCandidateInvariant() const
{ return (!m_low || m_low->RootCandidateInvariant()) && (!m_high || m_high->RootCandidateInvariant()); }

bool DesignHasPartClass::TargetInvariant() const
{ return (!m_low || m_low->TargetInvariant()) && (!m_high || m_high->TargetInvariant()); }

bool DesignHasPartClass::SourceInvariant() const
{ return (!m_low || m_low->SourceInvariant()) && (!m_high || m_high->SourceInvariant()); }

std::string DesignHasPartClass::Description(bool negated/* = false*/) const {
    std::string low_str = "1";
    if (m_low) {
        low_str = m_low->ConstantExpr() ?
                    std::to_string(m_low->Eval()) :
                    m_low->Description();
    }
    std::string high_str = std::to_string(INT_MAX);
    if (m_high) {
        high_str = m_high->ConstantExpr() ?
                    std::to_string(m_high->Eval()) :
                    m_high->Description();
    }
    return str(FlexibleFormat((!negated)
        ? UserString("DESC_DESIGN_HAS_PART_CLASS")
        : UserString("DESC_DESIGN_HAS_PART_CLASS_NOT"))
               % low_str
               % high_str
               % UserString(boost::lexical_cast<std::string>(m_class)));
}

std::string DesignHasPartClass::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "DesignHasPartClass";
    if (m_low)
        retval += " low = " + m_low->Dump(ntabs);
    if (m_high)
        retval += " high = " + m_high->Dump(ntabs);
    retval += " class = " + UserString(boost::lexical_cast<std::string>(m_class));
    retval += "\n";
    return retval;
}

bool DesignHasPartClass::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "DesignHasPartClass::Match passed no candidate object";
        return false;
    }

    int low =  (m_low ? m_low->Eval(local_context) : 0);
    int high = (m_high ? m_high->Eval(local_context) : INT_MAX);

    return DesignHasPartClassSimpleMatch(low, high, m_class)(candidate);
}

void DesignHasPartClass::GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                           ObjectSet& condition_non_targets) const
{
    AddShipSet(condition_non_targets);
}

void DesignHasPartClass::SetTopLevelContent(const std::string& content_name) {
    if (m_low)
        m_low->SetTopLevelContent(content_name);
    if (m_high)
        m_high->SetTopLevelContent(content_name);
}

unsigned int DesignHasPartClass::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::DesignHasPartClass");
    CheckSums::CheckSumCombine(retval, m_low);
    CheckSums::CheckSumCombine(retval, m_high);
    CheckSums::CheckSumCombine(retval, m_class);

    TraceLogger() << "GetCheckSum(DesignHasPartClass): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// PredefinedShipDesign                                  //
///////////////////////////////////////////////////////////
PredefinedShipDesign::PredefinedShipDesign(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& name) :
    ConditionBase(),
    m_name(std::move(name))
{}

PredefinedShipDesign::PredefinedShipDesign(ValueRef::ValueRefBase<std::string>* name) :
    ConditionBase(),
    m_name(name)
{}

bool PredefinedShipDesign::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const PredefinedShipDesign& rhs_ = static_cast<const PredefinedShipDesign&>(rhs);

    CHECK_COND_VREF_MEMBER(m_name)

    return true;
}

namespace {
    struct PredefinedShipDesignSimpleMatch {
        PredefinedShipDesignSimpleMatch() :
            m_any_predef_design_ok(true),
            m_name()
        {}

        PredefinedShipDesignSimpleMatch(const std::string& name) :
            m_any_predef_design_ok(false),
            m_name(name)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            auto ship = std::dynamic_pointer_cast<const Ship>(candidate);
            if (!ship)
                return false;
            const ShipDesign* candidate_design = ship->Design();
            if (!candidate_design)
                return false;

            // ship has a valid design.  see if it is / could be a predefined ship design...

            // all predefined named designs are hard-coded in parsing to have a designed on turn 0 (before first turn)
            if (candidate_design->DesignedOnTurn() != 0)
                return false;

            if (m_any_predef_design_ok)
                return true;    // any predefined design is OK; don't need to check name.

            return (m_name == candidate_design->Name(false)); // don't look up in stringtable; predefined designs are stored by stringtable entry key
        }

        bool        m_any_predef_design_ok;
        std::string m_name;
    };
}

void PredefinedShipDesign::Eval(const ScriptingContext& parent_context,
                                ObjectSet& matches, ObjectSet& non_matches,
                                SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = (!m_name || m_name->LocalCandidateInvariant()) &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant());
    if (simple_eval_safe) {
        // evaluate number limits once, use to match all candidates
        if (!m_name) {
            EvalImpl(matches, non_matches, search_domain, PredefinedShipDesignSimpleMatch());
        } else {
            std::shared_ptr<const UniverseObject> no_object;
            ScriptingContext local_context(parent_context, no_object);
            std::string name = m_name->Eval(local_context);
            EvalImpl(matches, non_matches, search_domain, PredefinedShipDesignSimpleMatch(name));
        }
    } else {
        // re-evaluate allowed turn range for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool PredefinedShipDesign::RootCandidateInvariant() const
{ return !m_name || m_name->RootCandidateInvariant(); }

bool PredefinedShipDesign::TargetInvariant() const
{ return !m_name || m_name->TargetInvariant(); }

bool PredefinedShipDesign::SourceInvariant() const
{ return !m_name || m_name->SourceInvariant(); }

std::string PredefinedShipDesign::Description(bool negated/* = false*/) const {
    std::string name_str;
    if (m_name) {
        name_str = m_name->Description();
        if (m_name->ConstantExpr() && UserStringExists(name_str))
            name_str = UserString(name_str);
    }
    return str(FlexibleFormat((!negated)
        ? UserString("DESC_PREDEFINED_SHIP_DESIGN")
        : UserString("DESC_PREDEFINED_SHIP_DESIGN_NOT"))
        % name_str);
}

std::string PredefinedShipDesign::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "PredefinedShipDesign";
    if (m_name)
        retval += " name = " + m_name->Dump(ntabs);
    retval += "\n";
    return retval;
}

bool PredefinedShipDesign::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "PredefinedShipDesign::Match passed no candidate object";
        return false;
    }

    if (!m_name)
        return PredefinedShipDesignSimpleMatch()(candidate);

    std::string name = m_name->Eval(local_context);
    return PredefinedShipDesignSimpleMatch(name)(candidate);
}

void PredefinedShipDesign::SetTopLevelContent(const std::string& content_name) {
    if (m_name)
        m_name->SetTopLevelContent(content_name);
}

unsigned int PredefinedShipDesign::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::PredefinedShipDesign");
    CheckSums::CheckSumCombine(retval, m_name);

    TraceLogger() << "GetCheckSum(PredefinedShipDesign): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// NumberedShipDesign                                    //
///////////////////////////////////////////////////////////
NumberedShipDesign::NumberedShipDesign(std::unique_ptr<ValueRef::ValueRefBase<int>>&& design_id) :
    ConditionBase(),
    m_design_id(std::move(design_id))
{}

bool NumberedShipDesign::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const NumberedShipDesign& rhs_ = static_cast<const NumberedShipDesign&>(rhs);

    CHECK_COND_VREF_MEMBER(m_design_id)

    return true;
}

namespace {
    struct NumberedShipDesignSimpleMatch {
        NumberedShipDesignSimpleMatch(int design_id) :
            m_design_id(design_id)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;
            if (m_design_id == INVALID_DESIGN_ID)
                return false;
            if (auto ship = std::dynamic_pointer_cast<const Ship>(candidate))
                if (ship->DesignID() == m_design_id)
                    return true;
            return false;
        }

        int m_design_id;
    };
}

void NumberedShipDesign::Eval(const ScriptingContext& parent_context,
                              ObjectSet& matches, ObjectSet& non_matches,
                              SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = m_design_id->ConstantExpr() ||
                            (m_design_id->LocalCandidateInvariant() &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate empire id once, and use to check all candidate objects
        std::shared_ptr<const UniverseObject> no_object;
        int design_id = m_design_id->Eval(ScriptingContext(parent_context, no_object));
        EvalImpl(matches, non_matches, search_domain, NumberedShipDesignSimpleMatch(design_id));
    } else {
        // re-evaluate design id for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool NumberedShipDesign::RootCandidateInvariant() const
{ return !m_design_id || m_design_id->RootCandidateInvariant(); }

bool NumberedShipDesign::TargetInvariant() const
{ return !m_design_id || m_design_id->TargetInvariant(); }

bool NumberedShipDesign::SourceInvariant() const
{ return !m_design_id || m_design_id->SourceInvariant(); }

std::string NumberedShipDesign::Description(bool negated/* = false*/) const {
    std::string id_str = m_design_id->ConstantExpr() ?
                            std::to_string(m_design_id->Eval()) :
                            m_design_id->Description();

    return str(FlexibleFormat((!negated)
        ? UserString("DESC_NUMBERED_SHIP_DESIGN")
        : UserString("DESC_NUMBERED_SHIP_DESIGN_NOT"))
               % id_str);
}

std::string NumberedShipDesign::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "NumberedShipDesign design_id = " + m_design_id->Dump(ntabs); }

bool NumberedShipDesign::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "NumberedShipDesign::Match passed no candidate object";
        return false;
    }
    return NumberedShipDesignSimpleMatch(m_design_id->Eval(local_context))(candidate);
}

void NumberedShipDesign::SetTopLevelContent(const std::string& content_name) {
    if (m_design_id)
        m_design_id->SetTopLevelContent(content_name);
}

unsigned int NumberedShipDesign::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::NumberedShipDesign");
    CheckSums::CheckSumCombine(retval, m_design_id);

    TraceLogger() << "GetCheckSum(NumberedShipDesign): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// ProducedByEmpire                                      //
///////////////////////////////////////////////////////////
ProducedByEmpire::ProducedByEmpire(std::unique_ptr<ValueRef::ValueRefBase<int>>&& empire_id) :
    ConditionBase(),
    m_empire_id(std::move(empire_id))
{}

bool ProducedByEmpire::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const ProducedByEmpire& rhs_ = static_cast<const ProducedByEmpire&>(rhs);

    CHECK_COND_VREF_MEMBER(m_empire_id)

    return true;
}

namespace {
    struct ProducedByEmpireSimpleMatch {
        ProducedByEmpireSimpleMatch(int empire_id) :
            m_empire_id(empire_id)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;

            if (auto ship = std::dynamic_pointer_cast<const ::Ship>(candidate))
                return ship->ProducedByEmpireID() == m_empire_id;
            else if (auto building = std::dynamic_pointer_cast<const ::Building>(candidate))
                return building->ProducedByEmpireID() == m_empire_id;
            return false;
        }

        int m_empire_id;
    };
}

void ProducedByEmpire::Eval(const ScriptingContext& parent_context,
                            ObjectSet& matches, ObjectSet& non_matches,
                            SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = m_empire_id->ConstantExpr() ||
                            (m_empire_id->LocalCandidateInvariant() &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate empire id once, and use to check all candidate objects
        std::shared_ptr<const UniverseObject> no_object;
        int empire_id = m_empire_id->Eval(ScriptingContext(parent_context, no_object));
        EvalImpl(matches, non_matches, search_domain, ProducedByEmpireSimpleMatch(empire_id));
    } else {
        // re-evaluate empire id for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool ProducedByEmpire::RootCandidateInvariant() const
{ return !m_empire_id || m_empire_id->RootCandidateInvariant(); }

bool ProducedByEmpire::TargetInvariant() const
{ return !m_empire_id || m_empire_id->TargetInvariant(); }

bool ProducedByEmpire::SourceInvariant() const
{ return !m_empire_id || m_empire_id->SourceInvariant(); }

std::string ProducedByEmpire::Description(bool negated/* = false*/) const {
    std::string empire_str;
    if (m_empire_id) {
        int empire_id = ALL_EMPIRES;
        if (m_empire_id->ConstantExpr())
            empire_id = m_empire_id->Eval();
        if (const Empire* empire = GetEmpire(empire_id))
            empire_str = empire->Name();
        else
            empire_str = m_empire_id->Description();
    }

    return str(FlexibleFormat((!negated)
        ? UserString("DESC_PRODUCED_BY_EMPIRE")
        : UserString("DESC_PRODUCED_BY_EMPIRE_NOT"))
               % empire_str);
}

std::string ProducedByEmpire::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "ProducedByEmpire empire_id = " + m_empire_id->Dump(ntabs); }

bool ProducedByEmpire::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "ProducedByEmpire::Match passed no candidate object";
        return false;
    }

    return ProducedByEmpireSimpleMatch(m_empire_id->Eval(local_context))(candidate);
}

void ProducedByEmpire::SetTopLevelContent(const std::string& content_name) {
    if (m_empire_id)
        m_empire_id->SetTopLevelContent(content_name);
}

unsigned int ProducedByEmpire::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::ProducedByEmpire");
    CheckSums::CheckSumCombine(retval, m_empire_id);

    TraceLogger() << "GetCheckSum(ProducedByEmpire): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// Chance                                                //
///////////////////////////////////////////////////////////
Chance::Chance(std::unique_ptr<ValueRef::ValueRefBase<double>>&& chance) :
    ConditionBase(),
    m_chance(std::move(chance))
{}

bool Chance::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const Chance& rhs_ = static_cast<const Chance&>(rhs);

    CHECK_COND_VREF_MEMBER(m_chance)

    return true;
}

namespace {
    struct ChanceSimpleMatch {
        ChanceSimpleMatch(float chance) :
            m_chance(chance)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const
        { return RandZeroToOne() <= m_chance; }

        float m_chance;
    };
}

void Chance::Eval(const ScriptingContext& parent_context,
                  ObjectSet& matches, ObjectSet& non_matches,
                  SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = m_chance->ConstantExpr() ||
                            (m_chance->LocalCandidateInvariant() &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate empire id once, and use to check all candidate objects
        std::shared_ptr<const UniverseObject> no_object;
        float chance = std::max(0.0, std::min(1.0, m_chance->Eval(ScriptingContext(parent_context, no_object))));
        EvalImpl(matches, non_matches, search_domain, ChanceSimpleMatch(chance));
    } else {
        // re-evaluate empire id for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Chance::RootCandidateInvariant() const
{ return !m_chance || m_chance->RootCandidateInvariant(); }

bool Chance::TargetInvariant() const
{ return !m_chance || m_chance->TargetInvariant(); }

bool Chance::SourceInvariant() const
{ return !m_chance || m_chance->SourceInvariant(); }

std::string Chance::Description(bool negated/* = false*/) const {
    if (m_chance->ConstantExpr()) {
        return str(FlexibleFormat((!negated)
            ? UserString("DESC_CHANCE_PERCENTAGE")
            : UserString("DESC_CHANCE_PERCENTAGE_NOT"))
                % std::to_string(std::max(0.0, std::min(m_chance->Eval(), 1.0)) * 100));
    } else {
        return str(FlexibleFormat((!negated)
            ? UserString("DESC_CHANCE")
            : UserString("DESC_CHANCE_NOT"))
            % m_chance->Description());
    }
}

std::string Chance::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "Random probability = " + m_chance->Dump(ntabs) + "\n"; }

bool Chance::Match(const ScriptingContext& local_context) const {
    float chance = std::max(0.0, std::min(m_chance->Eval(local_context), 1.0));
    return RandZeroToOne() <= chance;
}

void Chance::SetTopLevelContent(const std::string& content_name) {
    if (m_chance)
        m_chance->SetTopLevelContent(content_name);
}

unsigned int Chance::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::Chance");
    CheckSums::CheckSumCombine(retval, m_chance);

    TraceLogger() << "GetCheckSum(Chance): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// MeterValue                                            //
///////////////////////////////////////////////////////////
MeterValue::MeterValue(MeterType meter,
                       std::unique_ptr<ValueRef::ValueRefBase<double>>&& low,
                       std::unique_ptr<ValueRef::ValueRefBase<double>>&& high) :
    ConditionBase(),
    m_meter(meter),
    m_low(std::move(low)),
    m_high(std::move(high))
{}

bool MeterValue::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const MeterValue& rhs_ = static_cast<const MeterValue&>(rhs);

    if (m_meter != rhs_.m_meter)
        return false;

    CHECK_COND_VREF_MEMBER(m_low)
    CHECK_COND_VREF_MEMBER(m_high)

    return true;
}

namespace {
    struct MeterValueSimpleMatch {
        MeterValueSimpleMatch(float low, float high, MeterType meter_type) :
            m_low(low),
            m_high(high),
            m_meter_type(meter_type)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;

            if (const Meter* meter = candidate->GetMeter(m_meter_type)) {
                float value = meter->Initial();    // match Initial rather than Current to make results reproducible in a given turn, until back propagation happens
                return m_low <= value && value <= m_high;
            }

            return false;
        }

        float m_low;
        float m_high;
        MeterType m_meter_type;
    };

    std::string MeterTypeDumpString(MeterType meter) {
        switch (meter) {
        case INVALID_METER_TYPE:        return "INVALID_METER_TYPE"; break;
        case METER_TARGET_POPULATION:   return "TargetPopulation";   break;
        case METER_TARGET_INDUSTRY:     return "TargetIndustry";     break;
        case METER_TARGET_RESEARCH:     return "TargetResearch";     break;
        case METER_TARGET_TRADE:        return "TargetTrade";        break;
        case METER_TARGET_CONSTRUCTION: return "TargetConstruction"; break;
        case METER_MAX_FUEL:            return "MaxFuel";            break;
        case METER_MAX_SHIELD:          return "MaxShield";          break;
        case METER_MAX_STRUCTURE:       return "MaxStructure";       break;
        case METER_MAX_DEFENSE:         return "MaxDefense";         break;
        case METER_MAX_SUPPLY:          return "MaxSupply";          break;
        case METER_MAX_STOCKPILE:       return "MaxStockpile";       break;
        case METER_MAX_TROOPS:          return "MaxTroops";          break;
        case METER_POPULATION:          return "Population";         break;
        case METER_INDUSTRY:            return "Industry";           break;
        case METER_RESEARCH:            return "Research";           break;
        case METER_TRADE:               return "Trade";              break;
        case METER_CONSTRUCTION:        return "Construction";       break;
        case METER_FUEL:                return "Fuel";               break;
        case METER_SHIELD:              return "Shield";             break;
        case METER_STRUCTURE:           return "Structure";          break;
        case METER_DEFENSE:             return "Defense";            break;
        case METER_SUPPLY:              return "Supply";             break;
        case METER_STOCKPILE:           return "Stockpile";          break;
        case METER_STEALTH:             return "Stealth";            break;
        case METER_DETECTION:           return "Detection";          break;
        case METER_SPEED:               return "Speed";              break;
        case METER_CAPACITY:            return "Capacity";           break;
        default:                        return "?Meter?";            break;
        }
    }
}

void MeterValue::Eval(const ScriptingContext& parent_context,
                      ObjectSet& matches, ObjectSet& non_matches,
                      SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = ((!m_low || m_low->LocalCandidateInvariant()) &&
                             (!m_high || m_high->LocalCandidateInvariant()) &&
                             (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate number limits once, use to match all candidates
        std::shared_ptr<const UniverseObject> no_object;
        ScriptingContext local_context(parent_context, no_object);
        float low = (m_low ? m_low->Eval(local_context) : -Meter::LARGE_VALUE);
        float high = (m_high ? m_high->Eval(local_context) : Meter::LARGE_VALUE);
        EvalImpl(matches, non_matches, search_domain, MeterValueSimpleMatch(low, high, m_meter));
    } else {
        // re-evaluate allowed turn range for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool MeterValue::RootCandidateInvariant() const
{ return (!m_low || m_low->RootCandidateInvariant()) && (!m_high || m_high->RootCandidateInvariant()); }

bool MeterValue::TargetInvariant() const
{ return (!m_low || m_low->TargetInvariant()) && (!m_high || m_high->TargetInvariant()); }

bool MeterValue::SourceInvariant() const
{ return (!m_low || m_low->SourceInvariant()) && (!m_high || m_high->SourceInvariant()); }

std::string MeterValue::Description(bool negated/* = false*/) const {
    std::string low_str = (m_low ? (m_low->ConstantExpr() ?
                                    std::to_string(m_low->Eval()) :
                                    m_low->Description())
                                 : std::to_string(-Meter::LARGE_VALUE));
    std::string high_str = (m_high ? (m_high->ConstantExpr() ?
                                      std::to_string(m_high->Eval()) :
                                      m_high->Description())
                                   : std::to_string(Meter::LARGE_VALUE));

    if (m_low && !m_high) {
        return str(FlexibleFormat((!negated) ?
                                    UserString("DESC_METER_VALUE_CURRENT_MIN") :
                                    UserString("DESC_METER_VALUE_CURRENT_MIN_NOT"))
            % UserString(boost::lexical_cast<std::string>(m_meter))
            % low_str);
    } else if (m_high && !m_low) {
        return str(FlexibleFormat((!negated) ?
                                    UserString("DESC_METER_VALUE_CURRENT_MAX") :
                                    UserString("DESC_METER_VALUE_CURRENT_MAX_NOT"))
            % UserString(boost::lexical_cast<std::string>(m_meter))
            % high_str);
    } else {
        return str(FlexibleFormat((!negated) ?
                                    UserString("DESC_METER_VALUE_CURRENT") :
                                    UserString("DESC_METER_VALUE_CURRENT_NOT"))
            % UserString(boost::lexical_cast<std::string>(m_meter))
            % low_str
            % high_str);
    }
}

std::string MeterValue::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs);
    retval += MeterTypeDumpString(m_meter);
    if (m_low)
        retval += " low = " + m_low->Dump(ntabs);
    if (m_high)
        retval += " high = " + m_high->Dump(ntabs);
    retval += "\n";
    return retval;
}

bool MeterValue::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "MeterValue::Match passed no candidate object";
        return false;
    }
    float low = (m_low ? m_low->Eval(local_context) : -Meter::LARGE_VALUE);
    float high = (m_high ? m_high->Eval(local_context) : Meter::LARGE_VALUE);
    return MeterValueSimpleMatch(low, high, m_meter)(candidate);
}

void MeterValue::SetTopLevelContent(const std::string& content_name) {
    if (m_low)
        m_low->SetTopLevelContent(content_name);
    if (m_high)
        m_high->SetTopLevelContent(content_name);
}

unsigned int MeterValue::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::MeterValue");
    CheckSums::CheckSumCombine(retval, m_meter);
    CheckSums::CheckSumCombine(retval, m_low);
    CheckSums::CheckSumCombine(retval, m_high);

    TraceLogger() << "GetCheckSum(MeterValue): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// ShipPartMeterValue                                    //
///////////////////////////////////////////////////////////
ShipPartMeterValue::ShipPartMeterValue(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& ship_part_name,
                                       MeterType meter,
                                       std::unique_ptr<ValueRef::ValueRefBase<double>>&& low,
                                       std::unique_ptr<ValueRef::ValueRefBase<double>>&& high) :
    ConditionBase(),
    m_part_name(std::move(ship_part_name)),
    m_meter(meter),
    m_low(std::move(low)),
    m_high(std::move(high))
{}

bool ShipPartMeterValue::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const ShipPartMeterValue& rhs_ = static_cast<const ShipPartMeterValue&>(rhs);

    if (m_meter != rhs_.m_meter)
        return false;

    CHECK_COND_VREF_MEMBER(m_part_name)
    CHECK_COND_VREF_MEMBER(m_low)
    CHECK_COND_VREF_MEMBER(m_high)

    return true;
}

namespace {
    struct ShipPartMeterValueSimpleMatch {
        ShipPartMeterValueSimpleMatch(const std::string& ship_part_name,
                                      MeterType meter, float low, float high) :
            m_part_name(ship_part_name),
            m_low(low),
            m_high(high),
            m_meter(meter)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;
            auto ship = std::dynamic_pointer_cast<const Ship>(candidate);
            if (!ship)
                return false;
            const Meter* meter = ship->GetPartMeter(m_meter, m_part_name);
            if (!meter)
                return false;
            float meter_current = meter->Current();
            return (m_low <= meter_current && meter_current <= m_high);
        }

        std::string m_part_name;
        float       m_low;
        float       m_high;
        MeterType   m_meter;
    };
}

void ShipPartMeterValue::Eval(const ScriptingContext& parent_context,
                              ObjectSet& matches, ObjectSet& non_matches,
                              SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = ((!m_part_name || m_part_name->LocalCandidateInvariant()) &&
                             (!m_low || m_low->LocalCandidateInvariant()) &&
                             (!m_high || m_high->LocalCandidateInvariant()) &&
                             (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate number limits once, use to match all candidates
        std::shared_ptr<const UniverseObject> no_object;
        ScriptingContext local_context(parent_context, no_object);
        float low = (m_low ? m_low->Eval(local_context) : -Meter::LARGE_VALUE);
        float high = (m_high ? m_high->Eval(local_context) : Meter::LARGE_VALUE);
        std::string part_name = (m_part_name ? m_part_name->Eval(local_context) : "");
        EvalImpl(matches, non_matches, search_domain, ShipPartMeterValueSimpleMatch(part_name, m_meter, low, high));
    } else {
        // re-evaluate allowed turn range for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool ShipPartMeterValue::RootCandidateInvariant() const {
    return ((!m_part_name || m_part_name->RootCandidateInvariant()) &&
            (!m_low || m_low->RootCandidateInvariant()) &&
            (!m_high || m_high->RootCandidateInvariant()));
}

bool ShipPartMeterValue::TargetInvariant() const {
    return ((!m_part_name || m_part_name->TargetInvariant()) &&
            (!m_low || m_low->TargetInvariant()) &&
            (!m_high || m_high->TargetInvariant()));
}

bool ShipPartMeterValue::SourceInvariant() const {
    return ((!m_part_name || m_part_name->SourceInvariant()) &&
            (!m_low || m_low->SourceInvariant()) &&
            (!m_high || m_high->SourceInvariant()));
}

std::string ShipPartMeterValue::Description(bool negated/* = false*/) const {
    std::string low_str;
    if (m_low)
        low_str = m_low->Description();
    else
        low_str = std::to_string(-Meter::LARGE_VALUE);

    std::string high_str;
    if (m_high)
        high_str = m_high->Description();
    else
        high_str = std::to_string(Meter::LARGE_VALUE);

    std::string part_str;
    if (m_part_name) {
        part_str = m_part_name->Description();
        if (m_part_name->ConstantExpr() && UserStringExists(part_str))
            part_str = UserString(part_str);
    }

    return str(FlexibleFormat((!negated)
        ? UserString("DESC_SHIP_PART_METER_VALUE_CURRENT")
        : UserString("DESC_SHIP_PART_METER_VALUE_CURRENT_NOT"))
               % UserString(boost::lexical_cast<std::string>(m_meter))
               % part_str
               % low_str
               % high_str);
}

std::string ShipPartMeterValue::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs);
    retval += MeterTypeDumpString(m_meter);
    if (m_part_name)
        retval += " part = " + m_part_name->Dump(ntabs);
    if (m_low)
        retval += " low = " + m_low->Dump(ntabs);
    if (m_high)
        retval += " high = " + m_high->Dump(ntabs);
    retval += "\n";
    return retval;
}

bool ShipPartMeterValue::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "ShipPartMeterValue::Match passed no candidate object";
        return false;
    }
    float low = (m_low ? m_low->Eval(local_context) : -Meter::LARGE_VALUE);
    float high = (m_high ? m_high->Eval(local_context) : Meter::LARGE_VALUE);
    std::string part_name = (m_part_name ? m_part_name->Eval(local_context) : "");
    return ShipPartMeterValueSimpleMatch(part_name, m_meter, low, high)(candidate);
}

void ShipPartMeterValue::SetTopLevelContent(const std::string& content_name) {
    if (m_part_name)
        m_part_name->SetTopLevelContent(content_name);
    if (m_low)
        m_low->SetTopLevelContent(content_name);
    if (m_high)
        m_high->SetTopLevelContent(content_name);
}

unsigned int ShipPartMeterValue::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::ShipPartMeterValue");
    CheckSums::CheckSumCombine(retval, m_part_name);
    CheckSums::CheckSumCombine(retval, m_meter);
    CheckSums::CheckSumCombine(retval, m_low);
    CheckSums::CheckSumCombine(retval, m_high);

    TraceLogger() << "GetCheckSum(ShipPartMeterValue): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// EmpireMeterValue                                      //
///////////////////////////////////////////////////////////
EmpireMeterValue::EmpireMeterValue(const std::string& meter,
                                   std::unique_ptr<ValueRef::ValueRefBase<double>>&& low,
                                   std::unique_ptr<ValueRef::ValueRefBase<double>>&& high) :
    ConditionBase(),
    m_empire_id(nullptr),
    m_meter(meter),
    m_low(std::move(low)),
    m_high(std::move(high))
{}

EmpireMeterValue::EmpireMeterValue(std::unique_ptr<ValueRef::ValueRefBase<int>>&& empire_id,
                                   const std::string& meter,
                                   std::unique_ptr<ValueRef::ValueRefBase<double>>&& low,
                                   std::unique_ptr<ValueRef::ValueRefBase<double>>&& high) :
    ConditionBase(),
    m_empire_id(std::move(empire_id)),
    m_meter(meter),
    m_low(std::move(low)),
    m_high(std::move(high))
{}

bool EmpireMeterValue::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const EmpireMeterValue& rhs_ = static_cast<const EmpireMeterValue&>(rhs);

    if (m_empire_id != rhs_.m_empire_id)
        return false;

    if (m_meter != rhs_.m_meter)
        return false;

    CHECK_COND_VREF_MEMBER(m_low)
    CHECK_COND_VREF_MEMBER(m_high)

    return true;
}

namespace {
    struct EmpireMeterValueSimpleMatch {
        EmpireMeterValueSimpleMatch(int empire_id, float low, float high, const std::string& meter) :
            m_empire_id(empire_id),
            m_low(low),
            m_high(high),
            m_meter(meter)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;
            const Empire* empire = GetEmpire(m_empire_id);
            if (!empire)
                return false;
            const Meter* meter = empire->GetMeter(m_meter);
            if (!meter)
                return false;
            float meter_current = meter->Current();
            return (m_low <= meter_current && meter_current <= m_high);
        }

        int         m_empire_id;
        float      m_low;
        float      m_high;
        std::string m_meter;
    };
}

void EmpireMeterValue::Eval(const ScriptingContext& parent_context,
                            ObjectSet& matches, ObjectSet& non_matches,
                            SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = ((m_empire_id && m_empire_id->LocalCandidateInvariant()) &&
                             (!m_low || m_low->LocalCandidateInvariant()) &&
                             (!m_high || m_high->LocalCandidateInvariant()) &&
                             (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate number limits once, use to match all candidates
        std::shared_ptr<const UniverseObject> no_object;
        ScriptingContext local_context(parent_context, no_object);
        int empire_id = m_empire_id->Eval(local_context);   // if m_empire_id not set, default to local candidate's owner, which is not target invariant
        float low = (m_low ? m_low->Eval(local_context) : -Meter::LARGE_VALUE);
        float high = (m_high ? m_high->Eval(local_context) : Meter::LARGE_VALUE);
        EvalImpl(matches, non_matches, search_domain, EmpireMeterValueSimpleMatch(empire_id, low, high, m_meter));
    } else {
        // re-evaluate allowed turn range for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool EmpireMeterValue::RootCandidateInvariant() const {
    return (!m_empire_id || m_empire_id->RootCandidateInvariant()) &&
           (!m_low || m_low->RootCandidateInvariant()) &&
           (!m_high || m_high->RootCandidateInvariant());
}

bool EmpireMeterValue::TargetInvariant() const {
    return (!m_empire_id || m_empire_id->TargetInvariant()) &&
           (!m_low || m_low->TargetInvariant()) &&
           (!m_high || m_high->TargetInvariant());
}

bool EmpireMeterValue::SourceInvariant() const {
    return (!m_empire_id || m_empire_id->SourceInvariant()) &&
           (!m_low || m_low->SourceInvariant()) &&
           (!m_high || m_high->SourceInvariant());
}

std::string EmpireMeterValue::Description(bool negated/* = false*/) const {
    std::string empire_str;
    if (m_empire_id) {
        int empire_id = ALL_EMPIRES;
        if (m_empire_id->ConstantExpr())
            empire_id = m_empire_id->Eval();
        if (const Empire* empire = GetEmpire(empire_id))
            empire_str = empire->Name();
        else
            empire_str = m_empire_id->Description();
    }
    std::string low_str = (m_low ? (m_low->ConstantExpr() ?
                                    std::to_string(m_low->Eval()) :
                                    m_low->Description())
                                 : std::to_string(-Meter::LARGE_VALUE));
    std::string high_str = (m_high ? (m_high->ConstantExpr() ?
                                      std::to_string(m_high->Eval()) :
                                      m_high->Description())
                                   : std::to_string(Meter::LARGE_VALUE));
    return str(FlexibleFormat((!negated)
        ? UserString("DESC_EMPIRE_METER_VALUE_CURRENT")
        : UserString("DESC_EMPIRE_METER_VALUE_CURRENT_NOT"))
               % UserString(m_meter)
               % low_str
               % high_str
               % empire_str);
}

std::string EmpireMeterValue::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "EmpireMeterValue";
    if (m_empire_id)
        retval += " empire = " + m_empire_id->Dump(ntabs);
    retval += " meter = " + m_meter;
    if (m_low)
        retval += " low = " + m_low->Dump(ntabs);
    if (m_high)
        retval += " high = " + m_high->Dump(ntabs);
    retval += "\n";
    return retval;
}

bool EmpireMeterValue::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "EmpireMeterValue::Match passed no candidate object";
        return false;
    }
    int empire_id = (m_empire_id ? m_empire_id->Eval(local_context) : candidate->Owner());
    if (empire_id == ALL_EMPIRES)
        return false;
    float low = (m_low ? m_low->Eval(local_context) : -Meter::LARGE_VALUE);
    float high = (m_high ? m_high->Eval(local_context) : Meter::LARGE_VALUE);
    return EmpireMeterValueSimpleMatch(empire_id, low, high, m_meter)(candidate);
}

void EmpireMeterValue::SetTopLevelContent(const std::string& content_name) {
    if (m_empire_id)
        m_empire_id->SetTopLevelContent(content_name);
    if (m_low)
        m_low->SetTopLevelContent(content_name);
    if (m_high)
        m_high->SetTopLevelContent(content_name);
}

unsigned int EmpireMeterValue::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::EmpireMeterValue");
    CheckSums::CheckSumCombine(retval, m_empire_id);
    CheckSums::CheckSumCombine(retval, m_meter);
    CheckSums::CheckSumCombine(retval, m_low);
    CheckSums::CheckSumCombine(retval, m_high);

    TraceLogger() << "GetCheckSum(EmpireMeterValue): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// EmpireStockpileValue                                  //
///////////////////////////////////////////////////////////
EmpireStockpileValue::EmpireStockpileValue(ResourceType stockpile,
                                           std::unique_ptr<ValueRef::ValueRefBase<double>>&& low,
                                           std::unique_ptr<ValueRef::ValueRefBase<double>>&& high) :
    ConditionBase(),
    m_stockpile(stockpile),
    m_low(std::move(low)),
    m_high(std::move(high))
{}

bool EmpireStockpileValue::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const EmpireStockpileValue& rhs_ = static_cast<const EmpireStockpileValue&>(rhs);

    if (m_stockpile != rhs_.m_stockpile)
        return false;

    CHECK_COND_VREF_MEMBER(m_low)
    CHECK_COND_VREF_MEMBER(m_high)

    return true;
}

namespace {
    struct EmpireStockpileValueSimpleMatch {
        EmpireStockpileValueSimpleMatch(float low, float high, ResourceType stockpile) :
            m_low(low),
            m_high(high),
            m_stockpile(stockpile)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;

            if (candidate->Unowned())
                return false;

            const Empire* empire = GetEmpire(candidate->Owner());
            if (!empire)
                return false;

            if (m_stockpile == RE_TRADE) {
                float amount = empire->ResourceStockpile(m_stockpile);
                return (m_low <= amount && amount <= m_high);
            }

            return false;
        }

        float m_low;
        float m_high;
        ResourceType m_stockpile;
    };
}

void EmpireStockpileValue::Eval(const ScriptingContext& parent_context,
                                ObjectSet& matches, ObjectSet& non_matches,
                                SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = (m_low->LocalCandidateInvariant() && m_high->LocalCandidateInvariant() &&
                             (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate number limits once, use to match all candidates
        std::shared_ptr<const UniverseObject> no_object;
        ScriptingContext local_context(parent_context, no_object);
        float low = m_low->Eval(local_context);
        float high = m_high->Eval(local_context);
        EvalImpl(matches, non_matches, search_domain, EmpireStockpileValueSimpleMatch(low, high, m_stockpile));
    } else {
        // re-evaluate allowed turn range for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool EmpireStockpileValue::RootCandidateInvariant() const
{ return (m_low->RootCandidateInvariant() && m_high->RootCandidateInvariant()); }

bool EmpireStockpileValue::TargetInvariant() const
{ return (m_low->TargetInvariant() && m_high->TargetInvariant()); }

bool EmpireStockpileValue::SourceInvariant() const
{ return (m_low->SourceInvariant() && m_high->SourceInvariant()); }

std::string EmpireStockpileValue::Description(bool negated/* = false*/) const {
    std::string low_str = m_low->ConstantExpr() ?
                            std::to_string(m_low->Eval()) :
                            m_low->Description();
    std::string high_str = m_high->ConstantExpr() ?
                            std::to_string(m_high->Eval()) :
                            m_high->Description();
    return str(FlexibleFormat((!negated)
        ? UserString("DESC_EMPIRE_STOCKPILE_VALUE")
        : UserString("DESC_EMPIRE_STOCKPILE_VALUE_NOT"))
               % UserString(boost::lexical_cast<std::string>(m_stockpile))
               % low_str
               % high_str);
}

std::string EmpireStockpileValue::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs);
    switch (m_stockpile) {
    case RE_TRADE:      retval += "OwnerTradeStockpile";    break;
    case RE_RESEARCH:   retval += "OwnerResearchStockpile"; break;
    case RE_INDUSTRY:   retval += "OwnerIndustryStockpile"; break;
    default: retval += "?"; break;
    }
    retval += " low = " + m_low->Dump(ntabs) + " high = " + m_high->Dump(ntabs) + "\n";
    return retval;
}

bool EmpireStockpileValue::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "EmpireStockpileValue::Match passed no candidate object";
        return false;
    }

    float low = m_low->Eval(local_context);
    float high = m_high->Eval(local_context);
    return EmpireStockpileValueSimpleMatch(low, high, m_stockpile)(candidate);
}

void EmpireStockpileValue::SetTopLevelContent(const std::string& content_name) {
    if (m_low)
        m_low->SetTopLevelContent(content_name);
    if (m_high)
        m_high->SetTopLevelContent(content_name);
}

unsigned int EmpireStockpileValue::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::EmpireStockpileValue");
    CheckSums::CheckSumCombine(retval, m_stockpile);
    CheckSums::CheckSumCombine(retval, m_low);
    CheckSums::CheckSumCombine(retval, m_high);

    TraceLogger() << "GetCheckSum(EmpireStockpileValue): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// OwnerHasTech                                          //
///////////////////////////////////////////////////////////
OwnerHasTech::OwnerHasTech(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& name) :
    ConditionBase(),
    m_name(std::move(name))
{}

bool OwnerHasTech::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const OwnerHasTech& rhs_ = static_cast<const OwnerHasTech&>(rhs);

    CHECK_COND_VREF_MEMBER(m_name)

    return true;
}

namespace {
    struct OwnerHasTechSimpleMatch {
        OwnerHasTechSimpleMatch(const std::string& name) :
            m_name(name)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;

            if (candidate->Unowned())
                return false;

            if (const Empire* empire = GetEmpire(candidate->Owner()))
                return empire->TechResearched(m_name);

            return false;
        }

        std::string m_name;
    };
}

void OwnerHasTech::Eval(const ScriptingContext& parent_context,
                        ObjectSet& matches, ObjectSet& non_matches,
                        SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = (!m_name || m_name->LocalCandidateInvariant()) &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant());
    if (simple_eval_safe) {
        // evaluate number limits once, use to match all candidates
        std::shared_ptr<const UniverseObject> no_object;
        ScriptingContext local_context(parent_context, no_object);
        std::string name = m_name ? m_name->Eval(local_context) : "";
        EvalImpl(matches, non_matches, search_domain, OwnerHasTechSimpleMatch(name));
    } else {
        // re-evaluate allowed turn range for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool OwnerHasTech::RootCandidateInvariant() const
{ return !m_name || m_name->RootCandidateInvariant(); }

bool OwnerHasTech::TargetInvariant() const
{ return !m_name || m_name->TargetInvariant(); }

bool OwnerHasTech::SourceInvariant() const
{ return !m_name || m_name->SourceInvariant(); }

std::string OwnerHasTech::Description(bool negated/* = false*/) const {
    std::string name_str;
    if (m_name) {
        name_str = m_name->Description();
        if (m_name->ConstantExpr() && UserStringExists(name_str))
            name_str = UserString(name_str);
    }
    return str(FlexibleFormat((!negated)
        ? UserString("DESC_OWNER_HAS_TECH")
        : UserString("DESC_OWNER_HAS_TECH_NOT"))
        % name_str);
}

std::string OwnerHasTech::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "OwnerHasTech";
    if (m_name)
        retval += " name = " + m_name->Dump(ntabs);
    retval += "\n";
    return retval;
}

bool OwnerHasTech::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "OwnerHasTech::Match passed no candidate object";
        return false;
    }

    std::string name = m_name ? m_name->Eval(local_context) : "";
    return OwnerHasTechSimpleMatch(name)(candidate);
}

void OwnerHasTech::SetTopLevelContent(const std::string& content_name) {
    if (m_name)
        m_name->SetTopLevelContent(content_name);
}

unsigned int OwnerHasTech::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::OwnerHasTech");
    CheckSums::CheckSumCombine(retval, m_name);

    TraceLogger() << "GetCheckSum(OwnerHasTech): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// OwnerHasBuildingTypeAvailable                         //
///////////////////////////////////////////////////////////
OwnerHasBuildingTypeAvailable::OwnerHasBuildingTypeAvailable(const std::string& name) :
    ConditionBase(),
    // TODO: Use std::make_unique when adopting C++14
    m_name(new ValueRef::Constant<std::string>(name))
{}

OwnerHasBuildingTypeAvailable::OwnerHasBuildingTypeAvailable(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& name) :
    ConditionBase(),
    m_name(std::move(name))
{}

bool OwnerHasBuildingTypeAvailable::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const OwnerHasBuildingTypeAvailable& rhs_ = static_cast<const OwnerHasBuildingTypeAvailable&>(rhs);

    CHECK_COND_VREF_MEMBER(m_name)

    return true;
}

namespace {
    struct OwnerHasBuildingTypeAvailableSimpleMatch {
        OwnerHasBuildingTypeAvailableSimpleMatch(const std::string& name) :
            m_name(name)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;

            if (candidate->Unowned())
                return false;

            if (const Empire* empire = GetEmpire(candidate->Owner()))
                return empire->BuildingTypeAvailable(m_name);

            return false;
        }

        std::string m_name;
    };
}

void OwnerHasBuildingTypeAvailable::Eval(const ScriptingContext& parent_context,
                                         ObjectSet& matches, ObjectSet& non_matches,
                                         SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = (!m_name || m_name->LocalCandidateInvariant()) &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant());
    if (simple_eval_safe) {
        // evaluate number limits once, use to match all candidates
        std::shared_ptr<const UniverseObject> no_object;
        ScriptingContext local_context(parent_context, no_object);
        std::string name = m_name ? m_name->Eval(local_context) : "";
        EvalImpl(matches, non_matches, search_domain, OwnerHasBuildingTypeAvailableSimpleMatch(name));
    } else {
        // re-evaluate allowed turn range for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool OwnerHasBuildingTypeAvailable::RootCandidateInvariant() const
{ return !m_name || m_name->RootCandidateInvariant(); }

bool OwnerHasBuildingTypeAvailable::TargetInvariant() const
{ return !m_name || m_name->TargetInvariant(); }

bool OwnerHasBuildingTypeAvailable::SourceInvariant() const
{ return !m_name || m_name->SourceInvariant(); }

std::string OwnerHasBuildingTypeAvailable::Description(bool negated/* = false*/) const {
    // used internally for a tooltip where context is apparent, so don't need
    // to name builing type here
    return (!negated)
        ? UserString("DESC_OWNER_HAS_BUILDING_TYPE")
        : UserString("DESC_OWNER_HAS_BUILDING_TYPE_NOT");
}

std::string OwnerHasBuildingTypeAvailable::Dump(unsigned short ntabs) const {
    std::string retval= DumpIndent(ntabs) + "OwnerHasBuildingTypeAvailable";
    if (m_name)
        retval += " name = " + m_name->Dump(ntabs);
    retval += "\n";
    return retval;
}

bool OwnerHasBuildingTypeAvailable::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "OwnerHasTech::Match passed no candidate object";
        return false;
    }

    std::string name = m_name ? m_name->Eval(local_context) : "";
    return OwnerHasBuildingTypeAvailableSimpleMatch(name)(candidate);
}

void OwnerHasBuildingTypeAvailable::SetTopLevelContent(const std::string& content_name) {
    if (m_name)
        m_name->SetTopLevelContent(content_name);
}

unsigned int OwnerHasBuildingTypeAvailable::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::OwnerHasBuildingTypeAvailable");
    CheckSums::CheckSumCombine(retval, m_name);

    TraceLogger() << "GetCheckSum(OwnerHasBuildingTypeAvailable): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// OwnerHasShipDesignAvailable                           //
///////////////////////////////////////////////////////////
OwnerHasShipDesignAvailable::OwnerHasShipDesignAvailable(int id) :
    ConditionBase(),
    // TODO: Use std::make_unique when adopting C++14
    m_id(new ValueRef::Constant<int>(id))
{}

OwnerHasShipDesignAvailable::OwnerHasShipDesignAvailable(std::unique_ptr<ValueRef::ValueRefBase<int>>&& id) :
    ConditionBase(),
    m_id(std::move(id))
{}

bool OwnerHasShipDesignAvailable::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const OwnerHasShipDesignAvailable& rhs_ = static_cast<const OwnerHasShipDesignAvailable&>(rhs);

    CHECK_COND_VREF_MEMBER(m_id)

    return true;
}

namespace {
    struct OwnerHasShipDesignAvailableSimpleMatch {
        OwnerHasShipDesignAvailableSimpleMatch(int id) :
            m_id(id)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;

            if (candidate->Unowned())
                return false;

            if (const Empire* empire = GetEmpire(candidate->Owner()))
                return empire->ShipDesignAvailable(m_id);

            return false;
        }

        int m_id;
    };
}

void OwnerHasShipDesignAvailable::Eval(const ScriptingContext& parent_context,
                                       ObjectSet& matches, ObjectSet& non_matches,
                                       SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = (!m_id || m_id->LocalCandidateInvariant()) &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant());
    if (simple_eval_safe) {
        // evaluate number limits once, use to match all candidates
        std::shared_ptr<const UniverseObject> no_object;
        ScriptingContext local_context(parent_context, no_object);
        int id = m_id ? m_id->Eval(local_context) : INVALID_DESIGN_ID;
        EvalImpl(matches, non_matches, search_domain, OwnerHasShipDesignAvailableSimpleMatch(id));
    } else {
        // re-evaluate allowed turn range for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool OwnerHasShipDesignAvailable::RootCandidateInvariant() const
{ return !m_id || m_id->RootCandidateInvariant(); }

bool OwnerHasShipDesignAvailable::TargetInvariant() const
{ return !m_id || m_id->TargetInvariant(); }

bool OwnerHasShipDesignAvailable::SourceInvariant() const
{ return !m_id || m_id->SourceInvariant(); }

std::string OwnerHasShipDesignAvailable::Description(bool negated/* = false*/) const {
    // used internally for a tooltip where context is apparent, so don't need
    // to specify design here
    return (!negated)
        ? UserString("DESC_OWNER_HAS_SHIP_DESIGN")
        : UserString("DESC_OWNER_HAS_SHIP_DESIGN_NOT");
}

std::string OwnerHasShipDesignAvailable::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "OwnerHasShipDesignAvailable";
    if (m_id)
        retval += " id = " + m_id->Dump(ntabs);
    retval += "\n";
    return retval;
}

bool OwnerHasShipDesignAvailable::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "OwnerHasTech::Match passed no candidate object";
        return false;
    }

    int id = m_id ? m_id->Eval(local_context) : INVALID_DESIGN_ID;
    return OwnerHasShipDesignAvailableSimpleMatch(id)(candidate);
}

void OwnerHasShipDesignAvailable::SetTopLevelContent(const std::string& content_name) {
    if (m_id)
        m_id->SetTopLevelContent(content_name);
}

unsigned int OwnerHasShipDesignAvailable::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::OwnerHasShipDesignAvailable");
    CheckSums::CheckSumCombine(retval, m_id);

    TraceLogger() << "GetCheckSum(OwnerHasShipDesignAvailable): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// OwnerHasShipPartAvailable                             //
///////////////////////////////////////////////////////////
OwnerHasShipPartAvailable::OwnerHasShipPartAvailable(const std::string& name) :
    ConditionBase(),
    // TODO: Use std::make_unique when adopting C++14
    m_name(new ValueRef::Constant<std::string>(name))
{}

OwnerHasShipPartAvailable::OwnerHasShipPartAvailable(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& name) :
    ConditionBase(),
    m_name(std::move(name))
{}

bool OwnerHasShipPartAvailable::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const OwnerHasShipPartAvailable& rhs_ =
        static_cast<const OwnerHasShipPartAvailable&>(rhs);

    CHECK_COND_VREF_MEMBER(m_name)

    return true;
}

namespace {
    struct OwnerHasShipPartAvailableSimpleMatch {
        OwnerHasShipPartAvailableSimpleMatch(const std::string& name) :
            m_name(name)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;

            if (candidate->Unowned())
                return false;

            if (const Empire* empire = GetEmpire(candidate->Owner()))
                return empire->ShipPartAvailable(m_name);

            return false;
        }

        std::string m_name;
    };
}

void OwnerHasShipPartAvailable::Eval(const ScriptingContext& parent_context,
                                     ObjectSet& matches, ObjectSet& non_matches,
                                     SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = (!m_name || m_name->LocalCandidateInvariant()) &&
                            (parent_context.condition_root_candidate ||
                             RootCandidateInvariant());
    if (simple_eval_safe) {
        // evaluate number limits once, use to match all candidates
        std::shared_ptr<const UniverseObject> no_object;
        ScriptingContext local_context(parent_context, no_object);
        std::string name = m_name ? m_name->Eval(local_context) : "";
        EvalImpl(matches, non_matches, search_domain,
                 OwnerHasShipPartAvailableSimpleMatch(name));
    } else {
        // re-evaluate allowed turn range for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool OwnerHasShipPartAvailable::RootCandidateInvariant() const
{ return !m_name || m_name->RootCandidateInvariant(); }

bool OwnerHasShipPartAvailable::TargetInvariant() const
{ return !m_name || m_name->TargetInvariant(); }

bool OwnerHasShipPartAvailable::SourceInvariant() const
{ return !m_name || m_name->SourceInvariant(); }

std::string OwnerHasShipPartAvailable::Description(bool negated/* = false*/) const {
    return (!negated)
        ? UserString("DESC_OWNER_HAS_SHIP_PART")
        : UserString("DESC_OWNER_HAS_SHIP_PART_NOT");
}

std::string OwnerHasShipPartAvailable::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "OwnerHasShipPartAvailable";
    if (m_name)
        retval += " name = " + m_name->Dump(ntabs);
    retval += "\n";
    return retval;
}

bool OwnerHasShipPartAvailable::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "OwnerHasShipPart::Match passed no candidate object";
        return false;
    }

    std::string name = m_name ? m_name->Eval(local_context) : "";
    return OwnerHasShipPartAvailableSimpleMatch(name)(candidate);
}

void OwnerHasShipPartAvailable::SetTopLevelContent(const std::string& content_name) {
    if (m_name)
        m_name->SetTopLevelContent(content_name);
}

unsigned int OwnerHasShipPartAvailable::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::OwnerHasShipPartAvailable");
    CheckSums::CheckSumCombine(retval, m_name);

    TraceLogger() << "GetCheckSum(OwnerHasShipPartAvailable): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// VisibleToEmpire                                       //
///////////////////////////////////////////////////////////
VisibleToEmpire::VisibleToEmpire(std::unique_ptr<ValueRef::ValueRefBase<int>>&& empire_id) :
    ConditionBase(),
    m_empire_id(std::move(empire_id))
{}

bool VisibleToEmpire::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const VisibleToEmpire& rhs_ = static_cast<const VisibleToEmpire&>(rhs);

    CHECK_COND_VREF_MEMBER(m_empire_id)

    return true;
}

namespace {
    struct VisibleToEmpireSimpleMatch {
        VisibleToEmpireSimpleMatch(int empire_id,
                                   const Universe::EmpireObjectVisibilityMap& vis_map) :
            m_empire_id(empire_id),
            vis_map(vis_map)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;

            // if override is empty, use universe state
            if (vis_map.empty())
                return candidate->GetVisibility(m_empire_id) > VIS_NO_VISIBILITY;

            // if override specified, get visibility info from it
            auto empire_it = vis_map.find(m_empire_id);
            if (empire_it == vis_map.end())
                return false;
            const auto& object_map = empire_it->second;
            auto object_it = object_map.find(candidate->ID());
            if (object_it == object_map.end())
                return false;
            return object_it->second > VIS_NO_VISIBILITY;
        }

        int m_empire_id;
        const Universe::EmpireObjectVisibilityMap& vis_map;
    };
}

void VisibleToEmpire::Eval(const ScriptingContext& parent_context,
                           ObjectSet& matches, ObjectSet& non_matches,
                           SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = m_empire_id->ConstantExpr() ||
                            (m_empire_id->LocalCandidateInvariant() &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate empire id once, and use to check all candidate objects
        std::shared_ptr<const UniverseObject> no_object;
        int empire_id = m_empire_id->Eval(ScriptingContext(parent_context, no_object));
        EvalImpl(matches, non_matches, search_domain,
                 VisibleToEmpireSimpleMatch(empire_id, parent_context.empire_object_vis_map_override));
    } else {
        // re-evaluate empire id for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool VisibleToEmpire::RootCandidateInvariant() const
{ return m_empire_id->RootCandidateInvariant(); }

bool VisibleToEmpire::TargetInvariant() const
{ return m_empire_id->TargetInvariant(); }

bool VisibleToEmpire::SourceInvariant() const
{ return m_empire_id->SourceInvariant(); }

std::string VisibleToEmpire::Description(bool negated/* = false*/) const {
    std::string empire_str;
    if (m_empire_id) {
        int empire_id = ALL_EMPIRES;
        if (m_empire_id->ConstantExpr())
            empire_id = m_empire_id->Eval();
        if (const Empire* empire = GetEmpire(empire_id))
            empire_str = empire->Name();
        else
            empire_str = m_empire_id->Description();
    }

    return str(FlexibleFormat((!negated)
        ? UserString("DESC_VISIBLE_TO_EMPIRE")
        : UserString("DESC_VISIBLE_TO_EMPIRE_NOT"))
               % empire_str);
}

std::string VisibleToEmpire::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "VisibleToEmpire empire = " + m_empire_id->Dump(ntabs) + "\n"; }

bool VisibleToEmpire::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "VisibleToEmpire::Match passed no candidate object";
        return false;
    }

    int empire_id = m_empire_id->Eval(local_context);
    return VisibleToEmpireSimpleMatch(empire_id, local_context.empire_object_vis_map_override)(candidate);
}

void VisibleToEmpire::SetTopLevelContent(const std::string& content_name) {
    if (m_empire_id)
        m_empire_id->SetTopLevelContent(content_name);
}

unsigned int VisibleToEmpire::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::VisibleToEmpire");
    CheckSums::CheckSumCombine(retval, m_empire_id);

    TraceLogger() << "GetCheckSum(VisibleToEmpire): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// WithinDistance                                        //
///////////////////////////////////////////////////////////
WithinDistance::WithinDistance(std::unique_ptr<ValueRef::ValueRefBase<double>>&& distance,
                               std::unique_ptr<ConditionBase>&& condition) :
    ConditionBase(),
    m_distance(std::move(distance)),
    m_condition(std::move(condition))
{}

bool WithinDistance::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const WithinDistance& rhs_ = static_cast<const WithinDistance&>(rhs);

    CHECK_COND_VREF_MEMBER(m_distance)
    CHECK_COND_VREF_MEMBER(m_condition)

    return true;
}

namespace {
    struct WithinDistanceSimpleMatch {
        WithinDistanceSimpleMatch(const ObjectSet& from_objects, double distance) :
            m_from_objects(from_objects),
            m_distance2(distance*distance)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;

            // is candidate object close enough to any of the passed-in objects?
            for (auto& obj : m_from_objects) {
                double delta_x = candidate->X() - obj->X();
                double delta_y = candidate->Y() - obj->Y();
                if (delta_x*delta_x + delta_y*delta_y <= m_distance2)
                    return true;
            }

            return false;
        }

        const ObjectSet& m_from_objects;
        double m_distance2;
    };
}

void WithinDistance::Eval(const ScriptingContext& parent_context,
                          ObjectSet& matches, ObjectSet& non_matches,
                          SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = m_distance->LocalCandidateInvariant()
        && (parent_context.condition_root_candidate || RootCandidateInvariant());
    if (simple_eval_safe) {
        // evaluate contained objects once and check for all candidates

        std::shared_ptr<const UniverseObject> no_object;
        ScriptingContext local_context(parent_context, no_object);

        // get subcondition matches
        ObjectSet subcondition_matches;
        m_condition->Eval(local_context, subcondition_matches);

        double distance = m_distance->Eval(local_context);

        EvalImpl(matches, non_matches, search_domain, WithinDistanceSimpleMatch(subcondition_matches, distance));
    } else {
        // re-evaluate contained objects for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool WithinDistance::RootCandidateInvariant() const
{ return m_distance->RootCandidateInvariant() && m_condition->RootCandidateInvariant(); }

bool WithinDistance::TargetInvariant() const
{ return m_distance->TargetInvariant() && m_condition->TargetInvariant(); }

bool WithinDistance::SourceInvariant() const
{ return m_distance->SourceInvariant() && m_condition->SourceInvariant(); }

std::string WithinDistance::Description(bool negated/* = false*/) const {
    std::string value_str = m_distance->ConstantExpr() ?
                                std::to_string(m_distance->Eval()) :
                                m_distance->Description();
    return str(FlexibleFormat((!negated)
        ? UserString("DESC_WITHIN_DISTANCE")
        : UserString("DESC_WITHIN_DISTANCE_NOT"))
               % value_str
               % m_condition->Description());
}

std::string WithinDistance::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "WithinDistance distance = " + m_distance->Dump(ntabs) + " condition =\n";
    retval += m_condition->Dump(ntabs+1);
    return retval;
}

bool WithinDistance::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "WithinDistance::Match passed no candidate object";
        return false;
    }

    // get subcondition matches
    ObjectSet subcondition_matches;
    m_condition->Eval(local_context, subcondition_matches);
    if (subcondition_matches.empty())
        return false;

    return WithinDistanceSimpleMatch(subcondition_matches, m_distance->Eval(local_context))(candidate);
}

void WithinDistance::SetTopLevelContent(const std::string& content_name) {
    if (m_distance)
        m_distance->SetTopLevelContent(content_name);
    if (m_condition)
        m_condition->SetTopLevelContent(content_name);
}

unsigned int WithinDistance::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::WithinDistance");
    CheckSums::CheckSumCombine(retval, m_distance);
    CheckSums::CheckSumCombine(retval, m_condition);

    TraceLogger() << "GetCheckSum(WithinDistance): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// WithinStarlaneJumps                                   //
///////////////////////////////////////////////////////////
WithinStarlaneJumps::WithinStarlaneJumps(std::unique_ptr<ValueRef::ValueRefBase<int>>&& jumps,
                                         std::unique_ptr<ConditionBase>&& condition) :
    ConditionBase(),
    m_jumps(std::move(jumps)),
    m_condition(std::move(condition))
{}

bool WithinStarlaneJumps::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const WithinStarlaneJumps& rhs_ = static_cast<const WithinStarlaneJumps&>(rhs);

    CHECK_COND_VREF_MEMBER(m_jumps)
    CHECK_COND_VREF_MEMBER(m_condition)

    return true;
}

void WithinStarlaneJumps::Eval(const ScriptingContext& parent_context,
                               ObjectSet& matches, ObjectSet& non_matches,
                               SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = m_jumps->LocalCandidateInvariant()
        && (parent_context.condition_root_candidate || RootCandidateInvariant());
    if (simple_eval_safe) {
        // evaluate contained objects once and check for all candidates
        std::shared_ptr<const UniverseObject> no_object;
        ScriptingContext local_context(parent_context, no_object);

        // get subcondition matches
        ObjectSet subcondition_matches;
        m_condition->Eval(local_context, subcondition_matches);
        int jump_limit = m_jumps->Eval(local_context);
        ObjectSet &from_set(search_domain == Condition::MATCHES ? matches : non_matches);

        std::tie(matches, non_matches) = GetPathfinder()->WithinJumpsOfOthers(jump_limit, from_set, subcondition_matches);

    } else {
        // re-evaluate contained objects for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool WithinStarlaneJumps::RootCandidateInvariant() const
{ return m_jumps->RootCandidateInvariant() && m_condition->RootCandidateInvariant(); }

bool WithinStarlaneJumps::TargetInvariant() const
{ return m_jumps->TargetInvariant() && m_condition->TargetInvariant(); }

bool WithinStarlaneJumps::SourceInvariant() const
{ return m_jumps->SourceInvariant() && m_condition->SourceInvariant(); }

std::string WithinStarlaneJumps::Description(bool negated/* = false*/) const {
    std::string value_str = m_jumps->ConstantExpr() ? std::to_string(m_jumps->Eval()) : m_jumps->Description();
    return str(FlexibleFormat((!negated)
        ? UserString("DESC_WITHIN_STARLANE_JUMPS")
        : UserString("DESC_WITHIN_STARLANE_JUMPS_NOT"))
               % value_str
               % m_condition->Description());
}

std::string WithinStarlaneJumps::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "WithinStarlaneJumps jumps = " + m_jumps->Dump(ntabs) + " condition =\n";
    retval += m_condition->Dump(ntabs+1);
    return retval;
}

bool WithinStarlaneJumps::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "WithinStarlaneJumps::Match passed no candidate object";
        return false;
    }

    // get subcondition matches
    ObjectSet subcondition_matches;
    m_condition->Eval(local_context, subcondition_matches);
    if (subcondition_matches.empty())
        return false;

    int jump_limit = m_jumps->Eval(local_context);
    if (jump_limit < 0)
        return false;

    ObjectSet candidate_set{candidate};

    // candidate objects within jumps of subcondition_matches objects
    ObjectSet near_objs;

    std::tie(near_objs, std::ignore) =
        GetPathfinder()->WithinJumpsOfOthers(jump_limit, candidate_set, subcondition_matches);
    return !near_objs.empty();
}

void WithinStarlaneJumps::SetTopLevelContent(const std::string& content_name) {
    if (m_jumps)
        m_jumps->SetTopLevelContent(content_name);
    if (m_condition)
        m_condition->SetTopLevelContent(content_name);
}

unsigned int WithinStarlaneJumps::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::WithinStarlaneJumps");
    CheckSums::CheckSumCombine(retval, m_jumps);
    CheckSums::CheckSumCombine(retval, m_condition);

    TraceLogger() << "GetCheckSum(WithinStarlaneJumps): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// CanAddStarlaneConnection                              //
///////////////////////////////////////////////////////////
bool CanAddStarlaneConnection::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const CanAddStarlaneConnection& rhs_ = static_cast<const CanAddStarlaneConnection&>(rhs);

    CHECK_COND_VREF_MEMBER(m_condition)

    return true;
}

namespace {
    // check if two destination systems, connected to the same origin system
    // would have starlanes too close angularly to eachother
    bool LanesAngularlyTooClose(std::shared_ptr<const UniverseObject> sys1,
                                std::shared_ptr<const UniverseObject> lane1_sys2,
                                std::shared_ptr<const UniverseObject> lane2_sys2)
    {
        if (!sys1 || !lane1_sys2 || !lane2_sys2)
            return true;
        if (sys1 == lane1_sys2 || sys1 == lane2_sys2 || lane1_sys2 == lane2_sys2)
            return true;

        float dx1 = lane1_sys2->X() - sys1->X();
        float dy1 = lane1_sys2->Y() - sys1->Y();
        float mag = std::sqrt(dx1*dx1 + dy1*dy1);
        if (mag == 0.0f)
            return true;
        dx1 /= mag;
        dy1 /= mag;

        float dx2 = lane2_sys2->X() - sys1->X();
        float dy2 = lane2_sys2->Y() - sys1->Y();
        mag = std::sqrt(dx2*dx2 + dy2*dy2);
        if (mag == 0.0f)
            return true;
        dx2 /= mag;
        dy2 /= mag;


        const float MAX_LANE_DOT_PRODUCT = 0.98f;   // magic limit copied from CullAngularlyTooCloseLanes in UniverseGenerator

        float dp = (dx1 * dx2) + (dy1 * dy2);
        //TraceLogger() << "systems: " << sys1->UniverseObject::Name() << "  " << lane1_sys2->UniverseObject::Name() << "  " << lane2_sys2->UniverseObject::Name() << "  dp: " << dp << std::endl;

        return dp >= MAX_LANE_DOT_PRODUCT;   // if dot product too high after normalizing vectors, angles are adequately separated
    }

    // check the distance between a system and a (possibly nonexistant)
    // starlane between two other systems. distance here is how far the third
    // system is from the line passing through the lane endpoint systems, as
    // long as the third system is closer to either end point than the endpoints
    // are to eachother. if the third system is further than the endpoints, than
    // the distance to the line is not considered and the lane is considered
    // acceptable
    bool ObjectTooCloseToLane(std::shared_ptr<const UniverseObject> lane_end_sys1,
                              std::shared_ptr<const UniverseObject> lane_end_sys2,
                              std::shared_ptr<const UniverseObject> obj)
    {
        if (!lane_end_sys1 || !lane_end_sys2 || !obj)
            return true;
        if (lane_end_sys1 == lane_end_sys2 || obj == lane_end_sys1 || obj == lane_end_sys2)
            return true;

        // check distances (squared) between object and lane-end systems
        float v_12_x = lane_end_sys2->X() - lane_end_sys1->X();
        float v_12_y = lane_end_sys2->Y() - lane_end_sys1->Y();
        float v_o1_x = lane_end_sys1->X() - obj->X();
        float v_o1_y = lane_end_sys1->Y() - obj->Y();
        float v_o2_x = lane_end_sys2->X() - obj->X();
        float v_o2_y = lane_end_sys2->Y() - obj->Y();

        float dist2_12 = v_12_x*v_12_x + v_12_y*v_12_y;
        float dist2_o1 = v_o1_x*v_o1_x + v_o1_y*v_o1_y;
        float dist2_o2 = v_o2_x*v_o2_x + v_o2_y*v_o2_y;

        // object to zero-length lanes
        if (dist2_12 == 0.0f || dist2_o1 == 0.0f || dist2_o2 == 0.0f)
            return true;

        // if object is further from either of the lane end systems than they
        // are from eachother, it is fine, regardless of the right-angle
        // distance to the line between the systems
        if (dist2_12 < dist2_o1 || dist2_12 < dist2_o2)
            return false;


        // check right-angle distance between obj and lane

        // normalize vector components of lane vector
        float mag_12 = std::sqrt(dist2_12);
        if (mag_12 == 0.0f)
            return true;
        v_12_x /= mag_12;
        v_12_y /= mag_12;

        // distance to point from line from vector projection / dot products
        //.......O
        //      /|
        //     / |
        //    /  |d
        //   /   |
        //  /a___|___
        // 1         2
        // (1O).(12) = |1O| |12| cos(a)
        // d = |1O| cos(a) = (1O).(12) / |12|
        // d = -(O1).(12 / |12|)

        const float MIN_PERP_DIST = 10; // magic limit, in units of universe units (uu)

        float perp_dist = std::abs(v_o1_x*v_12_x + v_o1_y*v_12_y);

        return perp_dist < MIN_PERP_DIST;
    }

    inline float CrossProduct(float dx1, float dy1, float dx2, float dy2)
    { return dx1*dy2 - dy1*dx2; }

    bool LanesCross(std::shared_ptr<const System> lane1_end_sys1,
                    std::shared_ptr<const System> lane1_end_sys2,
                    std::shared_ptr<const System> lane2_end_sys1,
                    std::shared_ptr<const System> lane2_end_sys2)
    {
        // are all endpoints valid systems?
        if (!lane1_end_sys1 || !lane1_end_sys2 || !lane2_end_sys1 || !lane2_end_sys2)
            return false;

        // is either lane degenerate (same start and endpoints)
        if (lane1_end_sys1 == lane1_end_sys2 || lane2_end_sys1 == lane2_end_sys2)
            return false;

        // do the two lanes share endpoints?
        bool share_endpoint_1 = lane1_end_sys1 == lane2_end_sys1 || lane1_end_sys1 == lane2_end_sys2;
        bool share_endpoint_2 = lane1_end_sys2 == lane2_end_sys1 || lane1_end_sys2 == lane2_end_sys2;
        if (share_endpoint_1 && share_endpoint_2)
            return true;    // two copies of the same lane?
        if (share_endpoint_1 || share_endpoint_2)
            return false;   // one common endpoing, but not both common, so can't cross in middle

        // calculate vector components for lanes
        // lane 1
        float v_11_12_x = lane1_end_sys2->X() - lane1_end_sys1->X();
        float v_11_12_y = lane1_end_sys2->Y() - lane1_end_sys1->Y();
        // lane 2
        float v_21_22_x = lane2_end_sys2->X() - lane2_end_sys1->X();
        float v_21_22_y = lane2_end_sys2->Y() - lane2_end_sys1->Y();

        // calculate vector components from lane 1 system 1 to lane 2 endpoints
        // lane 1 endpoint 1 to lane 2 endpoint 1
        float v_11_21_x = lane2_end_sys1->X() - lane1_end_sys1->X();
        float v_11_21_y = lane2_end_sys1->Y() - lane1_end_sys1->Y();
        // lane 1 endpoint 1 to lane 2 endpoint 2
        float v_11_22_x = lane2_end_sys2->X() - lane1_end_sys1->X();
        float v_11_22_y = lane2_end_sys2->Y() - lane1_end_sys1->Y();

        // find cross products of vectors to check on which sides of lane 1 the
        // endpoints of lane 2 are located...
        float cp_1_21 = CrossProduct(v_11_12_x, v_11_12_y, v_11_21_x, v_11_21_y);
        float cp_1_22 = CrossProduct(v_11_12_x, v_11_12_y, v_11_22_x, v_11_22_y);
        if (cp_1_21*cp_1_22 >= 0) // product of same sign numbers is positive, of different sign numbers is negative
            return false;   // if same sign, points are on same side of line, so can't cross it

        // calculate vector components from lane 2 system 1 to lane 1 endpoints
        // lane 2 endpoint 1 to lane 1 endpoint 1
        float v_21_11_x = -v_11_21_x;
        float v_21_11_y = -v_11_21_y;
        // lane 2 endpoint 1 to lane 1 endpoint 2
        float v_21_12_x = lane1_end_sys2->X() - lane2_end_sys1->X();
        float v_21_12_y = lane1_end_sys2->Y() - lane2_end_sys1->Y();

        // find cross products of vectors to check on which sides of lane 2 the
        // endpoints of lane 1 are located...
        float cp_2_11 = CrossProduct(v_21_22_x, v_21_22_y, v_21_11_x, v_21_11_y);
        float cp_2_12 = CrossProduct(v_21_22_x, v_21_22_y, v_21_12_x, v_21_12_y);
        if (cp_2_11*cp_2_12 >= 0)
            return false;

        // endpoints of both lines are on opposite sides of the other line, so
        // the lines must cross

        return true;
    }

    bool LaneCrossesExistingLane(std::shared_ptr<const System> lane_end_sys1,
                                 std::shared_ptr<const System> lane_end_sys2)
    {
        if (!lane_end_sys1 || !lane_end_sys2 || lane_end_sys1 == lane_end_sys2)
            return true;

        const ObjectMap& objects = Objects();

        // loop over all existing lanes in all systems, checking if a lane
        // beween the specified systems would cross any of the existing lanes
        for (auto& system : objects.FindObjects<System>()) {
            if (system == lane_end_sys1 || system == lane_end_sys2)
                continue;

            const auto& sys_existing_lanes = system->StarlanesWormholes();

            // check all existing lanes of currently-being-checked system
            for (const auto& lane : sys_existing_lanes) {
                auto lane_end_sys3 = GetSystem(lane.first);
                if (!lane_end_sys3)
                    continue;
                // don't need to check against existing lanes that include one
                // of the endpoints of the lane is one of the specified systems
                if (lane_end_sys3 == lane_end_sys1 || lane_end_sys3 == lane_end_sys2)
                    continue;

                if (LanesCross(lane_end_sys1, lane_end_sys2, system, lane_end_sys3)) {
                    //TraceLogger() << "... ... ... lane from: " << lane_end_sys1->UniverseObject::Name() << " to: " << lane_end_sys2->UniverseObject::Name()
                    //          << " crosses lane from: " << system->UniverseObject::Name() << " to: " << lane_end_sys3->UniverseObject::Name() << std::endl;
                    return true;
                }
            }
        }

        return false;
    }

    bool LaneTooCloseToOtherSystem(std::shared_ptr<const System> lane_end_sys1,
                                   std::shared_ptr<const System> lane_end_sys2)
    {
        if (!lane_end_sys1 || !lane_end_sys2 || lane_end_sys1 == lane_end_sys2)
            return true;

        const ObjectMap& objects = Objects();
        auto systems = objects.FindObjects<System>();

        // loop over all existing systems, checking if each is too close to a
        // lane between the specified lane endpoints
        for (auto& system : systems) {
            if (system == lane_end_sys1 || system == lane_end_sys2)
                continue;

            if (ObjectTooCloseToLane(lane_end_sys1, lane_end_sys2, system))
                return true;
        }

        return false;
    }

    struct CanAddStarlaneConnectionSimpleMatch {
        CanAddStarlaneConnectionSimpleMatch(const ObjectSet& destination_objects) :
            m_destination_systems()
        {
            // get (one of each of) set of systems that are or that contain any
            // destination objects
            std::set<std::shared_ptr<const System>> dest_systems;
            for (auto& obj : destination_objects) {
                if (auto sys = GetSystem(obj->SystemID()))
                    dest_systems.insert(sys);
            }
            std::copy(dest_systems.begin(), dest_systems.end(), std::inserter(m_destination_systems, m_destination_systems.end()));
        }

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;

            // get system from candidate
            auto candidate_sys = std::dynamic_pointer_cast<const System>(candidate);
            if (!candidate_sys)
                candidate_sys = GetSystem(candidate->SystemID());
            if (!candidate_sys)
                return false;


            // check if candidate is one of the destination systems
            for (auto& destination : m_destination_systems) {
                if (candidate_sys->ID() == destination->ID())
                    return false;
            }


            // check if candidate already has a lane to any of the destination systems
            for (auto& destination : m_destination_systems) {
                if (candidate_sys->HasStarlaneTo(destination->ID()))
                    return false;
            }

            // check if any of the proposed lanes are too close to any already-
            // present lanes of the candidate system
            //TraceLogger() << "... Checking lanes of candidate system: " << candidate->UniverseObject::Name() << std::endl;
            for (const auto& lane : candidate_sys->StarlanesWormholes()) {
                auto candidate_existing_lane_end_sys = GetSystem(lane.first);
                if (!candidate_existing_lane_end_sys)
                    continue;

                // check this existing lane against potential lanes to all destination systems
                for (auto& dest_sys : m_destination_systems) {
                    if (LanesAngularlyTooClose(candidate_sys, candidate_existing_lane_end_sys, dest_sys)) {
                        //TraceLogger() << " ... ... can't add lane from candidate: " << candidate_sys->UniverseObject::Name() << " to " << dest_sys->UniverseObject::Name() << " due to existing lane to " << candidate_existing_lane_end_sys->UniverseObject::Name() << std::endl;
                        return false;
                    }
                }
            }


            // check if any of the proposed lanes are too close to any already-
            // present lanes of any of the destination systems
            //TraceLogger() << "... Checking lanes of destination systems:" << std::endl;
            for (auto& dest_sys : m_destination_systems) {
                // check this destination system's existing lanes against a lane
                // to the candidate system
                for (const auto& dest_lane : dest_sys->StarlanesWormholes()) {
                    auto dest_lane_end_sys = GetSystem(dest_lane.first);
                    if (!dest_lane_end_sys)
                        continue;

                    if (LanesAngularlyTooClose(dest_sys, candidate_sys, dest_lane_end_sys)) {
                        //TraceLogger() << " ... ... can't add lane from candidate: " << candidate_sys->UniverseObject::Name() << " to " << dest_sys->UniverseObject::Name() << " due to existing lane from dest to " << dest_lane_end_sys->UniverseObject::Name() << std::endl;
                        return false;
                    }
                }
            }


            // check if any of the proposed lanes are too close to eachother
            //TraceLogger() << "... Checking proposed lanes against eachother" << std::endl;
            for (auto it1 = m_destination_systems.begin();
                 it1 != m_destination_systems.end(); ++it1)
            {
                auto dest_sys1 = *it1;

                // don't need to check a lane in both directions, so start at one past it1
                auto it2 = it1;
                ++it2;
                for (; it2 != m_destination_systems.end(); ++it2) {
                    auto dest_sys2 = *it2;
                    if (LanesAngularlyTooClose(candidate_sys, dest_sys1, dest_sys2)) {
                        //TraceLogger() << " ... ... can't add lane from candidate: " << candidate_sys->UniverseObject::Name() << " to " << dest_sys1->UniverseObject::Name() << " and also to " << dest_sys2->UniverseObject::Name() << std::endl;
                        return false;
                    }
                }
            }


            // check that the proposed lanes are not too close to any existing
            // system they are not connected to
            //TraceLogger() << "... Checking proposed lanes for proximity to other systems" <<std::endl;
            for (auto& dest_sys : m_destination_systems) {
                if (LaneTooCloseToOtherSystem(candidate_sys, dest_sys)) {
                    //TraceLogger() << " ... ... can't add lane from candidate: " << candidate_sys->Name() << " to " << dest_sys->Name() << " due to proximity to another system." << std::endl;
                    return false;
                }
            }


            // check that there are no lanes already existing that cross the proposed lanes
            //TraceLogger() << "... Checking for potential lanes crossing existing lanes" << std::endl;
            for (auto& dest_sys : m_destination_systems) {
                if (LaneCrossesExistingLane(candidate_sys, dest_sys)) {
                    //TraceLogger() << " ... ... can't add lane from candidate: " << candidate_sys->Name() << " to " << dest_sys->Name() << " due to crossing an existing lane." << std::endl;
                    return false;
                }
            }

            return true;
        }

        std::vector<std::shared_ptr<const System>> m_destination_systems;
    };
}

void CanAddStarlaneConnection::Eval(const ScriptingContext& parent_context,
                                    ObjectSet& matches, ObjectSet& non_matches,
                                    SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = parent_context.condition_root_candidate || RootCandidateInvariant();
    if (simple_eval_safe) {
        // evaluate contained objects once and check for all candidates
        std::shared_ptr<const UniverseObject> no_object;
        ScriptingContext local_context(parent_context, no_object);

        // get subcondition matches
        ObjectSet subcondition_matches;
        m_condition->Eval(local_context, subcondition_matches);

        EvalImpl(matches, non_matches, search_domain, CanAddStarlaneConnectionSimpleMatch(subcondition_matches));
    } else {
        // re-evaluate contained objects for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool CanAddStarlaneConnection::RootCandidateInvariant() const
{ return m_condition->RootCandidateInvariant(); }

bool CanAddStarlaneConnection::TargetInvariant() const
{ return m_condition->TargetInvariant(); }

bool CanAddStarlaneConnection::SourceInvariant() const
{ return m_condition->SourceInvariant(); }

std::string CanAddStarlaneConnection::Description(bool negated/* = false*/) const {
    return str(FlexibleFormat((!negated)
        ? UserString("DESC_CAN_ADD_STARLANE_CONNECTION") : UserString("DESC_CAN_ADD_STARLANE_CONNECTION_NOT"))
        % m_condition->Description());
}

std::string CanAddStarlaneConnection::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "CanAddStarlanesTo condition =\n";
    retval += m_condition->Dump(ntabs+1);
    return retval;
}

bool CanAddStarlaneConnection::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "CanAddStarlaneConnection::Match passed no candidate object";
        return false;
    }

    // get subcondition matches
    ObjectSet subcondition_matches;
    m_condition->Eval(local_context, subcondition_matches);

    return CanAddStarlaneConnectionSimpleMatch(subcondition_matches)(candidate);
}

void CanAddStarlaneConnection::SetTopLevelContent(const std::string& content_name) {
    if (m_condition)
        m_condition->SetTopLevelContent(content_name);
}

unsigned int CanAddStarlaneConnection::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::CanAddStarlaneConnection");
    CheckSums::CheckSumCombine(retval, m_condition);

    TraceLogger() << "GetCheckSum(CanAddStarlaneConnection): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// ExploredByEmpire                                      //
///////////////////////////////////////////////////////////
ExploredByEmpire::ExploredByEmpire(std::unique_ptr<ValueRef::ValueRefBase<int>>&& empire_id) :
    ConditionBase(),
    m_empire_id(std::move(empire_id))
{}

bool ExploredByEmpire::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const ExploredByEmpire& rhs_ = static_cast<const ExploredByEmpire&>(rhs);

    CHECK_COND_VREF_MEMBER(m_empire_id)

    return true;
}

namespace {
    struct ExploredByEmpireSimpleMatch {
        ExploredByEmpireSimpleMatch(int empire_id) :
            m_empire_id(empire_id)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;

            const Empire* empire = GetEmpire(m_empire_id);
            if (!empire)
                return false;
            return empire->HasExploredSystem(candidate->ID());
        }

        int m_empire_id;
    };
}

void ExploredByEmpire::Eval(const ScriptingContext& parent_context,
                            ObjectSet& matches, ObjectSet& non_matches,
                            SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = m_empire_id->ConstantExpr() ||
                            (m_empire_id->LocalCandidateInvariant() &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate empire id once, and use to check all candidate objects
        std::shared_ptr<const UniverseObject> no_object;
        int empire_id = m_empire_id->Eval(ScriptingContext(parent_context, no_object));
        EvalImpl(matches, non_matches, search_domain, ExploredByEmpireSimpleMatch(empire_id));
    } else {
        // re-evaluate empire id for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool ExploredByEmpire::RootCandidateInvariant() const
{ return m_empire_id->RootCandidateInvariant(); }

bool ExploredByEmpire::TargetInvariant() const
{ return m_empire_id->TargetInvariant(); }

bool ExploredByEmpire::SourceInvariant() const
{ return m_empire_id->SourceInvariant(); }

std::string ExploredByEmpire::Description(bool negated/* = false*/) const {
    std::string empire_str;
    if (m_empire_id) {
        int empire_id = ALL_EMPIRES;
        if (m_empire_id->ConstantExpr())
            empire_id = m_empire_id->Eval();
        if (const Empire* empire = GetEmpire(empire_id))
            empire_str = empire->Name();
        else
            empire_str = m_empire_id->Description();
    }

    return str(FlexibleFormat((!negated)
               ? UserString("DESC_EXPLORED_BY_EMPIRE")
               : UserString("DESC_EXPLORED_BY_EMPIRE_NOT"))
               % empire_str);
}

std::string ExploredByEmpire::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "ExploredByEmpire empire_id = " + m_empire_id->Dump(ntabs); }

bool ExploredByEmpire::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "ExploredByEmpire::Match passed no candidate object";
        return false;
    }

    return ExploredByEmpireSimpleMatch(m_empire_id->Eval(local_context))(candidate);
}

void ExploredByEmpire::SetTopLevelContent(const std::string& content_name) {
    if (m_empire_id)
        m_empire_id->SetTopLevelContent(content_name);
}

unsigned int ExploredByEmpire::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::ExploredByEmpire");
    CheckSums::CheckSumCombine(retval, m_empire_id);

    TraceLogger() << "GetCheckSum(ExploredByEmpire): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// Stationary                                            //
///////////////////////////////////////////////////////////
bool Stationary::operator==(const ConditionBase& rhs) const
{ return ConditionBase::operator==(rhs); }

std::string Stationary::Description(bool negated/* = false*/) const {
    return (!negated)
        ? UserString("DESC_STATIONARY")
        : UserString("DESC_STATIONARY_NOT");
}

std::string Stationary::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "Stationary\n"; }

bool Stationary::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "Stationary::Match passed no candidate object";
        return false;
    }

    // the only objects that can move are fleets and the ships in them.  so,
    // attempt to cast the candidate object to a fleet or ship, and if it's a ship
    // get the fleet of that ship
    auto fleet = std::dynamic_pointer_cast<const Fleet>(candidate);
    if (!fleet)
        if (auto ship = std::dynamic_pointer_cast<const Ship>(candidate))
            fleet = GetFleet(ship->FleetID());

    if (fleet) {
        // if a fleet is available, it is "moving", or not stationary, if it's
        // next system is a system and isn't the current system.  This will
        // mean fleets that have arrived at a system on the current turn will
        // be stationary, but fleets departing won't be stationary.
        int next_id = fleet->NextSystemID();
        int cur_id = fleet->SystemID();
        if (next_id != INVALID_OBJECT_ID && next_id != cur_id)
            return false;
    }

    return true;
}

unsigned int Stationary::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::Stationary");

    TraceLogger() << "GetCheckSum(Stationary): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// Aggressive                                            //
///////////////////////////////////////////////////////////
bool Aggressive::operator==(const ConditionBase& rhs) const
{ return ConditionBase::operator==(rhs); }

std::string Aggressive::Description(bool negated/* = false*/) const {
    if (m_aggressive)
        return (!negated)
            ? UserString("DESC_AGGRESSIVE")
            : UserString("DESC_AGGRESSIVE_NOT");
    else
        return (!negated)
            ? UserString("DESC_PASSIVE")
            : UserString("DESC_PASSIVE_NOT");
}

std::string Aggressive::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + (m_aggressive ? "Aggressive\n" : "Passive\n"); }

bool Aggressive::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "Aggressive::Match passed no candidate object";
        return false;
    }

    // the only objects that can be aggressive are fleets and the ships in them.
    // so, attempt to cast the candidate object to a fleet or ship, and if it's
    // a ship get the fleet of that ship
    auto fleet = std::dynamic_pointer_cast<const Fleet>(candidate);
    if (!fleet)
        if (auto ship = std::dynamic_pointer_cast<const Ship>(candidate))
            fleet = GetFleet(ship->FleetID());

    if (!fleet)
        return false;

    return m_aggressive == fleet->Aggressive();
}

unsigned int Aggressive::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::Aggressive");
    CheckSums::CheckSumCombine(retval, m_aggressive);

    TraceLogger() << "GetCheckSum(Aggressive): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// FleetSupplyableByEmpire                               //
///////////////////////////////////////////////////////////
FleetSupplyableByEmpire::FleetSupplyableByEmpire(std::unique_ptr<ValueRef::ValueRefBase<int>>&& empire_id) :
    ConditionBase(),
    m_empire_id(std::move(empire_id))
{}

bool FleetSupplyableByEmpire::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const FleetSupplyableByEmpire& rhs_ = static_cast<const FleetSupplyableByEmpire&>(rhs);

    CHECK_COND_VREF_MEMBER(m_empire_id)

    return true;
}

namespace {
    struct FleetSupplyableSimpleMatch {
        FleetSupplyableSimpleMatch(int empire_id) :
            m_empire_id(empire_id)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;

            const Empire* empire = GetEmpire(m_empire_id);
            if (!empire)
                return false;

            const SupplyManager& supply = GetSupplyManager();
            const auto& empire_supplyable_systems = supply.FleetSupplyableSystemIDs();
            auto it = empire_supplyable_systems.find(m_empire_id);
            if (it == empire_supplyable_systems.end())
                return false;
            return it->second.count(candidate->SystemID());
        }

        int m_empire_id;
    };
}

void FleetSupplyableByEmpire::Eval(const ScriptingContext& parent_context,
                                   ObjectSet& matches, ObjectSet& non_matches,
                                   SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = m_empire_id->ConstantExpr() ||
                            (m_empire_id->LocalCandidateInvariant() &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate empire id once, and use to check all candidate objects
        std::shared_ptr<const UniverseObject> no_object;
        int empire_id = m_empire_id->Eval(ScriptingContext(parent_context, no_object));
        EvalImpl(matches, non_matches, search_domain, FleetSupplyableSimpleMatch(empire_id));
    } else {
        // re-evaluate empire id for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool FleetSupplyableByEmpire::RootCandidateInvariant() const
{ return m_empire_id->RootCandidateInvariant(); }

bool FleetSupplyableByEmpire::TargetInvariant() const
{ return m_empire_id->TargetInvariant(); }

bool FleetSupplyableByEmpire::SourceInvariant() const
{ return m_empire_id->SourceInvariant(); }

std::string FleetSupplyableByEmpire::Description(bool negated/* = false*/) const {
    std::string empire_str;
    if (m_empire_id) {
        int empire_id = ALL_EMPIRES;
        if (m_empire_id->ConstantExpr())
            empire_id = m_empire_id->Eval();
        if (const Empire* empire = GetEmpire(empire_id))
            empire_str = empire->Name();
        else
            empire_str = m_empire_id->Description();
    }

    return str(FlexibleFormat((!negated)
               ? UserString("DESC_SUPPLY_CONNECTED_FLEET")
               : UserString("DESC_SUPPLY_CONNECTED_FLEET_NOT"))
               % empire_str);
}

std::string FleetSupplyableByEmpire::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "ResupplyableBy empire_id = " + m_empire_id->Dump(ntabs); }

bool FleetSupplyableByEmpire::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "FleetSupplyableByEmpire::Match passed no candidate object";
        return false;
    }

    int empire_id = m_empire_id->Eval(local_context);

    return FleetSupplyableSimpleMatch(empire_id)(candidate);
}

void FleetSupplyableByEmpire::SetTopLevelContent(const std::string& content_name) {
    if (m_empire_id)
        m_empire_id->SetTopLevelContent(content_name);
}

unsigned int FleetSupplyableByEmpire::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::FleetSupplyableByEmpire");
    CheckSums::CheckSumCombine(retval, m_empire_id);

    TraceLogger() << "GetCheckSum(FleetSupplyableByEmpire): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// ResourceSupplyConnectedByEmpire                       //
///////////////////////////////////////////////////////////
ResourceSupplyConnectedByEmpire::ResourceSupplyConnectedByEmpire(
    std::unique_ptr<ValueRef::ValueRefBase<int>>&& empire_id,
    std::unique_ptr<ConditionBase>&& condition) :
    ConditionBase(),
    m_empire_id(std::move(empire_id)),
    m_condition(std::move(condition))
{}

bool ResourceSupplyConnectedByEmpire::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const ResourceSupplyConnectedByEmpire& rhs_ = static_cast<const ResourceSupplyConnectedByEmpire&>(rhs);

    CHECK_COND_VREF_MEMBER(m_empire_id)

    return true;
}

namespace {
    struct ResourceSupplySimpleMatch {
        ResourceSupplySimpleMatch(int empire_id, const ObjectSet& from_objects) :
            m_empire_id(empire_id),
            m_from_objects(from_objects)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;
            if (m_from_objects.empty())
                return false;
            const auto& groups = GetSupplyManager().ResourceSupplyGroups(m_empire_id);
            if (groups.empty())
                return false;

            // is candidate object connected to a subcondition matching object by resource supply?
            // first check if candidate object is (or is a building on) a blockaded planet
            // "isolated" objects are anything not in a non-blockaded system
            bool is_isolated = true;
            for (const auto& group : groups) {
                if (group.count(candidate->SystemID())) {
                    is_isolated = false;
                    break;
                }
            }
            if (is_isolated) {
                // planets are still supply-connected to themselves even if blockaded
                auto candidate_planet = std::dynamic_pointer_cast<const Planet>(candidate);
                std::shared_ptr<const ::Building> building;
                if (!candidate_planet && (building = std::dynamic_pointer_cast<const ::Building>(candidate)))
                    candidate_planet = GetPlanet(building->PlanetID());
                if (candidate_planet) {
                    int candidate_planet_id = candidate_planet->ID();
                    // can only match if the from_object is (or is on) the same planet
                    for (auto& from_object : m_from_objects) {
                        auto from_obj_planet = std::dynamic_pointer_cast<const Planet>(from_object);
                        std::shared_ptr<const ::Building> from_building;
                        if (!from_obj_planet && (from_building = std::dynamic_pointer_cast<const ::Building>(from_object)))
                            from_obj_planet = GetPlanet(from_building->PlanetID());
                        if (from_obj_planet && from_obj_planet->ID() == candidate_planet_id)
                            return true;
                    }
                }
                // candidate is isolated, but did not match planet for any test object
                return false;
            }
            // candidate is not blockaded, so check for system group matches
            for (auto& from_object : m_from_objects) {
                for (const auto& group : groups) {
                    if (group.count(from_object->SystemID())) {
                        // found resource sharing group containing test object system.  Does it also contain candidate?
                        if (group.count(candidate->SystemID()))
                            return true;    // test object and candidate object are in same resourse sharing group
                        else
                            // test object is not in resource sharing group with candidate
                            // as each object can be in only one group, no need to check the remaining groups
                            break;
                    }
                    // current subcondition-matching object is not in this resource sharing group
                }
                // current subcondition-matching object is not in any resource sharing group for this empire
            }

            return false;
        }

        int m_empire_id;
        const ObjectSet& m_from_objects;
    };
}

void ResourceSupplyConnectedByEmpire::Eval(const ScriptingContext& parent_context,
                                           ObjectSet& matches, ObjectSet& non_matches,
                                           SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = m_empire_id->ConstantExpr() ||
                            (m_empire_id->LocalCandidateInvariant() &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate contained objects once and check for all candidates
        std::shared_ptr<const UniverseObject> no_object;
        ScriptingContext local_context(parent_context, no_object);

        // get objects to be considering for matching against subcondition
        ObjectSet subcondition_matches;
        m_condition->Eval(local_context, subcondition_matches);

        int empire_id = m_empire_id->Eval(local_context);

        EvalImpl(matches, non_matches, search_domain, ResourceSupplySimpleMatch(empire_id, subcondition_matches));
    } else {
        // re-evaluate empire id for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool ResourceSupplyConnectedByEmpire::RootCandidateInvariant() const
{ return m_empire_id->RootCandidateInvariant() && m_condition->RootCandidateInvariant(); }

bool ResourceSupplyConnectedByEmpire::TargetInvariant() const
{ return m_empire_id->TargetInvariant() && m_condition->TargetInvariant(); }

bool ResourceSupplyConnectedByEmpire::SourceInvariant() const
{ return m_empire_id->SourceInvariant() && m_condition->SourceInvariant(); }

bool ResourceSupplyConnectedByEmpire::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "ResourceSupplyConnectedByEmpire::Match passed no candidate object";
        return false;
    }

    // get subcondition matches
    ObjectSet subcondition_matches;
    m_condition->Eval(local_context, subcondition_matches);
    int empire_id = m_empire_id->Eval(local_context);

    return ResourceSupplySimpleMatch(empire_id, subcondition_matches)(candidate);
}

std::string ResourceSupplyConnectedByEmpire::Description(bool negated/* = false*/) const {
    std::string empire_str;
    if (m_empire_id) {
        int empire_id = ALL_EMPIRES;
        if (m_empire_id->ConstantExpr())
            empire_id = m_empire_id->Eval();
        if (const Empire* empire = GetEmpire(empire_id))
            empire_str = empire->Name();
        else
            empire_str = m_empire_id->Description();
    }

    return str(FlexibleFormat((!negated)
               ? UserString("DESC_SUPPLY_CONNECTED_RESOURCE")
               : UserString("DESC_SUPPLY_CONNECTED_RESOURCE_NOT"))
               % empire_str
               % m_condition->Description());
}

std::string ResourceSupplyConnectedByEmpire::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "ResourceSupplyConnectedBy empire_id = "
        + m_empire_id->Dump(ntabs) + " condition = \n";
    retval += m_condition->Dump(ntabs+1);
    return retval;
}

void ResourceSupplyConnectedByEmpire::SetTopLevelContent(const std::string& content_name) {
    if (m_empire_id)
        m_empire_id->SetTopLevelContent(content_name);
    if (m_condition)
        m_condition->SetTopLevelContent(content_name);
}

unsigned int ResourceSupplyConnectedByEmpire::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::ResourceSupplyConnectedByEmpire");
    CheckSums::CheckSumCombine(retval, m_empire_id);
    CheckSums::CheckSumCombine(retval, m_condition);

    TraceLogger() << "GetCheckSum(ResourceSupplyConnectedByEmpire): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// CanColonize                                           //
///////////////////////////////////////////////////////////
bool CanColonize::operator==(const ConditionBase& rhs) const
{ return ConditionBase::operator==(rhs); }

std::string CanColonize::Description(bool negated/* = false*/) const {
    return str(FlexibleFormat((!negated)
        ? UserString("DESC_CAN_COLONIZE")
        : UserString("DESC_CAN_COLONIZE_NOT")));
}

std::string CanColonize::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "CanColonize\n"; }

bool CanColonize::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "CanColonize::Match passed no candidate object";
        return false;
    }

    // is it a ship, a planet, or a building on a planet?
    std::string species_name;
    if (candidate->ObjectType() == OBJ_PLANET) {
        auto planet = std::dynamic_pointer_cast<const Planet>(candidate);
        if (!planet) {
            ErrorLogger() << "CanColonize couldn't cast supposedly planet candidate";
            return false;
        }
        species_name = planet->SpeciesName();

    } else if (candidate->ObjectType() == OBJ_BUILDING) {
        auto building = std::dynamic_pointer_cast<const ::Building>(candidate);
        if (!building) {
            ErrorLogger() << "CanColonize couldn't cast supposedly building candidate";
            return false;
        }
        auto planet = GetPlanet(building->PlanetID());
        if (!planet) {
            ErrorLogger() << "CanColonize couldn't get building's planet";
            return false;
        }
        species_name = planet->SpeciesName();

    } else if (candidate->ObjectType() == OBJ_SHIP) {
        auto ship = std::dynamic_pointer_cast<const Ship>(candidate);
        if (!ship) {
            ErrorLogger() << "CanColonize couldn't cast supposedly ship candidate";
            return false;
        }
        species_name = ship->SpeciesName();
    }

    if (species_name.empty())
        return false;
    auto species = GetSpecies(species_name);
    if (!species) {
        ErrorLogger() << "CanColonize couldn't get species: " << species_name;
        return false;
    }
    return species->CanColonize();
}

unsigned int CanColonize::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::CanColonize");

    TraceLogger() << "GetCheckSum(CanColonize): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// CanProduceShips                                       //
///////////////////////////////////////////////////////////
bool CanProduceShips::operator==(const ConditionBase& rhs) const
{ return ConditionBase::operator==(rhs); }

std::string CanProduceShips::Description(bool negated/* = false*/) const {
    return str(FlexibleFormat((!negated)
        ? UserString("DESC_CAN_PRODUCE_SHIPS")
        : UserString("DESC_CAN_PRODUCE_SHIPS_NOT")));
}

std::string CanProduceShips::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "CanColonize\n"; }

bool CanProduceShips::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "CanProduceShips::Match passed no candidate object";
        return false;
    }

    // is it a ship, a planet, or a building on a planet?
    std::string species_name;
    if (candidate->ObjectType() == OBJ_PLANET) {
        auto planet = std::dynamic_pointer_cast<const Planet>(candidate);
        if (!planet) {
            ErrorLogger() << "CanProduceShips couldn't cast supposedly planet candidate";
            return false;
        }
        species_name = planet->SpeciesName();

    } else if (candidate->ObjectType() == OBJ_BUILDING) {
        auto building = std::dynamic_pointer_cast<const ::Building>(candidate);
        if (!building) {
            ErrorLogger() << "CanProduceShips couldn't cast supposedly building candidate";
            return false;
        }
        auto planet = GetPlanet(building->PlanetID());
        if (!planet) {
            ErrorLogger() << "CanProduceShips couldn't get building's planet";
            return false;
        }
        species_name = planet->SpeciesName();

    } else if (candidate->ObjectType() == OBJ_SHIP) {
        auto ship = std::dynamic_pointer_cast<const Ship>(candidate);
        if (!ship) {
            ErrorLogger() << "CanProduceShips couldn't cast supposedly ship candidate";
            return false;
        }
        species_name = ship->SpeciesName();
    }

    if (species_name.empty())
        return false;
    auto species = GetSpecies(species_name);
    if (!species) {
        ErrorLogger() << "CanProduceShips couldn't get species: " << species_name;
        return false;
    }
    return species->CanProduceShips();
}

unsigned int CanProduceShips::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::CanProduceShips");

    TraceLogger() << "GetCheckSum(CanProduceShips): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// OrderedBombarded                                      //
///////////////////////////////////////////////////////////
OrderedBombarded::OrderedBombarded(std::unique_ptr<ConditionBase>&& by_object_condition) :
    ConditionBase(),
    m_by_object_condition(std::move(by_object_condition))
{}

bool OrderedBombarded::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const OrderedBombarded& rhs_ = static_cast<const OrderedBombarded&>(rhs);

    CHECK_COND_VREF_MEMBER(m_by_object_condition)

    return true;
}

namespace {
    struct OrderedBombardedSimpleMatch {
        OrderedBombardedSimpleMatch(const ObjectSet& by_objects) :
            m_by_objects(by_objects)
        {}

        bool operator()(std::shared_ptr<const UniverseObject> candidate) const {
            if (!candidate)
                return false;
            if (m_by_objects.empty())
                return false;
            auto planet = std::dynamic_pointer_cast<const Planet>(candidate);
            if (!planet)
                return false;
            int planet_id = planet->ID();
            if (planet_id == INVALID_OBJECT_ID)
                return false;

            // check if any of the by_objects is ordered to bombard the candidate planet
            for (auto& obj : m_by_objects) {
                auto ship = std::dynamic_pointer_cast<const Ship>(obj);
                if (!ship)
                    continue;
                if (ship->OrderedBombardPlanet() == planet_id)
                    return true;
            }
            return false;
        }

        const ObjectSet& m_by_objects;
    };
}

void OrderedBombarded::Eval(const ScriptingContext& parent_context,
                            ObjectSet& matches, ObjectSet& non_matches,
                            SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = parent_context.condition_root_candidate || RootCandidateInvariant();
    if (simple_eval_safe) {
        // evaluate contained objects once and check for all candidates
        std::shared_ptr<const UniverseObject> no_object;
        ScriptingContext local_context(parent_context, no_object);

        // get subcondition matches
        ObjectSet subcondition_matches;
        m_by_object_condition->Eval(local_context, subcondition_matches);

        EvalImpl(matches, non_matches, search_domain, OrderedBombardedSimpleMatch(subcondition_matches));
    } else {
        // re-evaluate contained objects for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool OrderedBombarded::RootCandidateInvariant() const
{ return m_by_object_condition->RootCandidateInvariant(); }

bool OrderedBombarded::TargetInvariant() const
{ return m_by_object_condition->TargetInvariant(); }

bool OrderedBombarded::SourceInvariant() const
{ return m_by_object_condition->SourceInvariant(); }

std::string OrderedBombarded::Description(bool negated/* = false*/) const {
    std::string by_str;
    if (m_by_object_condition)
        by_str = m_by_object_condition->Description();

    return str(FlexibleFormat((!negated)
               ? UserString("DESC_ORDERED_BOMBARDED")
               : UserString("DESC_ORDERED_BOMBARDED_NOT"))
               % by_str);
}

std::string OrderedBombarded::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "OrderedBombarded by_object = " + m_by_object_condition->Dump(ntabs); }

bool OrderedBombarded::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "OrderedBombarded::Match passed no candidate object";
        return false;
    }

    // get subcondition matches
    ObjectSet subcondition_matches;
    m_by_object_condition->Eval(local_context, subcondition_matches);

    return OrderedBombardedSimpleMatch(subcondition_matches)(candidate);
}

void OrderedBombarded::SetTopLevelContent(const std::string& content_name) {
    if (m_by_object_condition)
        m_by_object_condition->SetTopLevelContent(content_name);
}

unsigned int OrderedBombarded::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::OrderedBombarded");
    CheckSums::CheckSumCombine(retval, m_by_object_condition);

    TraceLogger() << "GetCheckSum(OrderedBombarded): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// ValueTest                                             //
///////////////////////////////////////////////////////////
namespace {
    bool Comparison(float val1, ComparisonType comp, float val2) {
        switch (comp) {
            case EQUAL:                 return val1 == val2;
            case GREATER_THAN:          return val1 > val2;
            case GREATER_THAN_OR_EQUAL: return val1 >= val2;
            case LESS_THAN:             return val1 < val2;
            case LESS_THAN_OR_EQUAL:    return val1 <= val2;
            case NOT_EQUAL:             return val1 != val2;
            case INVALID_COMPARISON:
            default:                    return false;
        }
    }
    bool Comparison(const std::string& val1, ComparisonType comp,
                    const std::string& val2)
    {
        switch (comp) {
            case EQUAL:                 return val1 == val2;
            case NOT_EQUAL:             return val1 != val2;
            case INVALID_COMPARISON:
            default:                    return false;
        }
    }

    std::string CompareTypeString(ComparisonType comp) {
        switch (comp) {
        case EQUAL:                 return "=";
        case GREATER_THAN:          return ">";
        case GREATER_THAN_OR_EQUAL: return ">=";
        case LESS_THAN:             return "<";
        case LESS_THAN_OR_EQUAL:    return "<=";
        case NOT_EQUAL:             return "!=";
        case INVALID_COMPARISON:
        default:                    return "";
        }
    }
}

ValueTest::ValueTest(std::unique_ptr<ValueRef::ValueRefBase<double>>&& value_ref1,
                     ComparisonType comp1,
                     std::unique_ptr<ValueRef::ValueRefBase<double>>&& value_ref2,
                     ComparisonType comp2,
                     std::unique_ptr<ValueRef::ValueRefBase<double>>&& value_ref3) :
    ConditionBase(),
    m_value_ref1(std::move(value_ref1)),
    m_value_ref2(std::move(value_ref2)),
    m_value_ref3(std::move(value_ref3)),
    m_compare_type1(comp1),
    m_compare_type2(comp2)
{}

ValueTest::ValueTest(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& value_ref1,
                     ComparisonType comp1,
                     std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& value_ref2,
                     ComparisonType comp2,
                     std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& value_ref3) :
    ConditionBase(),
    m_string_value_ref1(std::move(value_ref1)),
    m_string_value_ref2(std::move(value_ref2)),
    m_string_value_ref3(std::move(value_ref3)),
    m_compare_type1(comp1),
    m_compare_type2(comp2)
{
    /*DebugLogger() << "String ValueTest(" << value_ref1->Dump(ntabs) << " "
                                         << CompareTypeString(comp1) << " "
                                         << value_ref2->Dump(ntabs) << ")";*/
}

ValueTest::ValueTest(std::unique_ptr<ValueRef::ValueRefBase<int>>&& value_ref1,
                     ComparisonType comp1,
                     std::unique_ptr<ValueRef::ValueRefBase<int>>&& value_ref2,
                     ComparisonType comp2,
                     std::unique_ptr<ValueRef::ValueRefBase<int>>&& value_ref3) :
    ConditionBase(),
    m_int_value_ref1(std::move(value_ref1)),
    m_int_value_ref2(std::move(value_ref2)),
    m_int_value_ref3(std::move(value_ref3)),
    m_compare_type1(comp1),
    m_compare_type2(comp2)
{
    //DebugLogger() << "ValueTest(double)";
}

bool ValueTest::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const ValueTest& rhs_ = static_cast<const ValueTest&>(rhs);

    CHECK_COND_VREF_MEMBER(m_value_ref1)
    CHECK_COND_VREF_MEMBER(m_value_ref2)
    CHECK_COND_VREF_MEMBER(m_value_ref3)
    CHECK_COND_VREF_MEMBER(m_string_value_ref1)
    CHECK_COND_VREF_MEMBER(m_string_value_ref2)
    CHECK_COND_VREF_MEMBER(m_string_value_ref3)
    CHECK_COND_VREF_MEMBER(m_int_value_ref1)
    CHECK_COND_VREF_MEMBER(m_int_value_ref2)
    CHECK_COND_VREF_MEMBER(m_int_value_ref3)

    if (m_compare_type1 != rhs_.m_compare_type1)
        return false;
    if (m_compare_type2 != rhs_.m_compare_type2)
        return false;

    return true;
}

void ValueTest::Eval(const ScriptingContext& parent_context,
                     ObjectSet& matches, ObjectSet& non_matches,
                     SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = ((!m_value_ref1         || m_value_ref1->LocalCandidateInvariant()) &&
                             (!m_value_ref2         || m_value_ref2->LocalCandidateInvariant()) &&
                             (!m_value_ref3         || m_value_ref3->LocalCandidateInvariant()) &&
                             (!m_string_value_ref1  || m_string_value_ref1->LocalCandidateInvariant()) &&
                             (!m_string_value_ref2  || m_string_value_ref2->LocalCandidateInvariant()) &&
                             (!m_string_value_ref3  || m_string_value_ref3->LocalCandidateInvariant()) &&
                             (!m_int_value_ref1     || m_int_value_ref1->LocalCandidateInvariant()) &&
                             (!m_int_value_ref2     || m_int_value_ref2->LocalCandidateInvariant()) &&
                             (!m_int_value_ref3     || m_int_value_ref3->LocalCandidateInvariant()) &&
                             (parent_context.condition_root_candidate || RootCandidateInvariant()));

    if (simple_eval_safe) {
        // evaluate value and range limits once, use to match all candidates
        std::shared_ptr<const UniverseObject> no_object;
        ScriptingContext local_context(parent_context, no_object);
        bool passed = false;

        if (m_value_ref1) {
            float val1, val2, val3;
            passed = /*m_value_ref1 &&*/ m_value_ref2.get();
            if (passed) {
                val1 = m_value_ref1->Eval(local_context);
                val2 = m_value_ref2->Eval(local_context);
                passed = Comparison(val1, m_compare_type1, val2);
            }
            if (passed && m_value_ref3) {
                val3 = m_value_ref3->Eval(local_context);
                passed = Comparison(val2, m_compare_type2, val3);
            }

        } else if (m_string_value_ref1) {
            std::string val1, val2, val3;
            passed = /*m_string_value_ref1 &&*/ m_string_value_ref2.get();
            if (passed) {
                val1 = m_string_value_ref1->Eval(local_context);
                val2 = m_string_value_ref2->Eval(local_context);
                passed = Comparison(val1, m_compare_type1, val2);
            }
            if (passed && m_string_value_ref3) {
                val3 = m_string_value_ref3->Eval(local_context);
                passed = Comparison(val2, m_compare_type2, val3);
            }

        } else if (m_int_value_ref1) {
            int val1, val2, val3;
            passed = /*m_int_value_ref1 &&*/ m_int_value_ref2.get();
            if (passed) {
                val1 = m_int_value_ref1->Eval(local_context);
                val2 = m_int_value_ref2->Eval(local_context);
                passed = Comparison(val1, m_compare_type1, val2);
            }
            if (passed && m_int_value_ref3) {
                val3 = m_int_value_ref3->Eval(local_context);
                passed = Comparison(val2, m_compare_type2, val3);
            }
        } else {
            // passed remains false
        }

        // transfer objects to or from candidate set, according to whether the value comparisons were true
        if (search_domain == MATCHES && !passed) {
            non_matches.insert(non_matches.end(), matches.begin(), matches.end());
            matches.clear();
        }
        if (search_domain == NON_MATCHES && passed) {
            matches.insert(matches.end(), non_matches.begin(), non_matches.end());
            non_matches.clear();
        }


    } else {
        // re-evaluate value and ranges for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool ValueTest::RootCandidateInvariant() const {
    return (!m_value_ref1           || m_value_ref1->RootCandidateInvariant()) &&
           (!m_value_ref2           || m_value_ref2->RootCandidateInvariant()) &&
           (!m_value_ref3           || m_value_ref3->RootCandidateInvariant()) &&
           (!m_string_value_ref1    || m_string_value_ref1->RootCandidateInvariant()) &&
           (!m_string_value_ref2    || m_string_value_ref2->RootCandidateInvariant()) &&
           (!m_string_value_ref3    || m_string_value_ref3->RootCandidateInvariant()) &&
           (!m_int_value_ref1       || m_int_value_ref1->RootCandidateInvariant()) &&
           (!m_int_value_ref2       || m_int_value_ref2->RootCandidateInvariant()) &&
           (!m_int_value_ref3       || m_int_value_ref3->RootCandidateInvariant());
}

bool ValueTest::TargetInvariant() const {
    return (!m_value_ref1           || m_value_ref1->TargetInvariant()) &&
           (!m_value_ref2           || m_value_ref2->TargetInvariant()) &&
           (!m_value_ref3           || m_value_ref3->TargetInvariant()) &&
           (!m_string_value_ref1    || m_string_value_ref1->TargetInvariant()) &&
           (!m_string_value_ref2    || m_string_value_ref2->TargetInvariant()) &&
           (!m_string_value_ref3    || m_string_value_ref3->TargetInvariant()) &&
           (!m_int_value_ref1       || m_int_value_ref1->TargetInvariant()) &&
           (!m_int_value_ref2       || m_int_value_ref2->TargetInvariant()) &&
           (!m_int_value_ref3       || m_int_value_ref3->TargetInvariant());
}

bool ValueTest::SourceInvariant() const {
    return (!m_value_ref1           || m_value_ref1->SourceInvariant()) &&
           (!m_value_ref2           || m_value_ref2->SourceInvariant()) &&
           (!m_value_ref3           || m_value_ref3->SourceInvariant()) &&
           (!m_string_value_ref1    || m_string_value_ref1->SourceInvariant()) &&
           (!m_string_value_ref2    || m_string_value_ref2->SourceInvariant()) &&
           (!m_string_value_ref3    || m_string_value_ref3->SourceInvariant()) &&
           (!m_int_value_ref1       || m_int_value_ref1->SourceInvariant()) &&
           (!m_int_value_ref2       || m_int_value_ref2->SourceInvariant()) &&
           (!m_int_value_ref3       || m_int_value_ref3->SourceInvariant());
}

std::string ValueTest::Description(bool negated/* = false*/) const {
    std::string value_str1, value_str2, value_str3;
    if (m_value_ref1)
        value_str1 = m_value_ref1->Description();
    else if (m_string_value_ref1)
        value_str1 = m_string_value_ref1->Description();
    else if (m_int_value_ref1)
        value_str1 = m_int_value_ref1->Description();

    if (m_value_ref2)
        value_str2 = m_value_ref2->Description();
    else if (m_string_value_ref2)
        value_str2 = m_string_value_ref2->Description();
    else if (m_int_value_ref2)
        value_str2 = m_int_value_ref2->Description();

    if (m_value_ref3)
        value_str3 = m_value_ref3->Description();
    else if (m_string_value_ref3)
        value_str3 = m_string_value_ref3->Description();
    else if (m_int_value_ref3)
        value_str3 = m_int_value_ref3->Description();

    std::string comp_str1 = CompareTypeString(m_compare_type1);
    std::string comp_str2 = CompareTypeString(m_compare_type2);

    std::string composed_comparison = value_str1 + " " + comp_str1 + " " + value_str2;
    if (!comp_str2.empty())
        composed_comparison += " " + comp_str2;
    if (!value_str3.empty())
        composed_comparison += +" " + value_str3;

    return str(FlexibleFormat((!negated)
               ? UserString("DESC_VALUE_TEST")
               : UserString("DESC_VALUE_TEST_NOT"))
               % composed_comparison);
}

std::string ValueTest::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "(";
    if (m_value_ref1)
        retval += m_value_ref1->Dump(ntabs);
    else if (m_string_value_ref1)
        retval += m_string_value_ref1->Dump(ntabs);
    else if (m_int_value_ref1)
        retval += m_int_value_ref1->Dump(ntabs);

    if (m_compare_type1 != INVALID_COMPARISON)
        retval += " " + CompareTypeString(m_compare_type1);

    if (m_value_ref2)
        retval += " " + m_value_ref2->Dump(ntabs);
    else if (m_string_value_ref2)
        retval += " " + m_string_value_ref2->Dump(ntabs);
    else if (m_int_value_ref2)
        retval += " " + m_int_value_ref2->Dump(ntabs);

    if (m_compare_type2 != INVALID_COMPARISON)
        retval += " " + CompareTypeString(m_compare_type2);

    if (m_value_ref3)
        retval += " " + m_value_ref3->Dump(ntabs);
    else if (m_string_value_ref3)
        retval += " " + m_string_value_ref3->Dump(ntabs);
    else if (m_int_value_ref3)
        retval += " " + m_int_value_ref3->Dump(ntabs);

    retval += ")\n";
    return retval;
}

bool ValueTest::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "ValueTest::Match passed no candidate object";
        return false;
    }
    if (m_compare_type1 == INVALID_COMPARISON)
        return false;

    if (m_value_ref1) {
        if (!m_value_ref2)
            return false;

        float val1 = m_value_ref1->Eval(local_context);
        float val2 = m_value_ref2->Eval(local_context);
        if (!Comparison(val1, m_compare_type1, val2))
            return false;

        if (m_compare_type2 == INVALID_COMPARISON || !m_value_ref3)
            return true;

        float val3 = m_value_ref3->Eval(local_context);
        return Comparison(val2, m_compare_type1, val3);

    } else if (m_string_value_ref1) {
        if (!m_string_value_ref2)
            return false;

        std::string val1 = m_string_value_ref1->Eval(local_context);
        std::string val2 = m_string_value_ref2->Eval(local_context);
        if (!Comparison(val1, m_compare_type1, val2))
            return false;

        if (m_compare_type2 == INVALID_COMPARISON || !m_value_ref3)
            return true;

        std::string val3 = m_string_value_ref3->Eval(local_context);
        return Comparison(val2, m_compare_type1, val3);
    } else if (m_int_value_ref1) {
        if (!m_int_value_ref2)
            return false;

        int val1 = m_int_value_ref1->Eval(local_context);
        int val2 = m_int_value_ref2->Eval(local_context);
        if (!Comparison(val1, m_compare_type1, val2))
            return false;

        if (m_compare_type2 == INVALID_COMPARISON || !m_value_ref3)
            return true;

        int val3 = m_int_value_ref3->Eval(local_context);
        return Comparison(val2, m_compare_type1, val3);
    }
    return false;
}

void ValueTest::SetTopLevelContent(const std::string& content_name) {
    if (m_value_ref1)
        m_value_ref1->SetTopLevelContent(content_name);
    if (m_value_ref2)
        m_value_ref2->SetTopLevelContent(content_name);
    if (m_value_ref3)
        m_value_ref3->SetTopLevelContent(content_name);
    if (m_string_value_ref1)
        m_string_value_ref1->SetTopLevelContent(content_name);
    if (m_string_value_ref2)
        m_string_value_ref2->SetTopLevelContent(content_name);
    if (m_string_value_ref3)
        m_string_value_ref3->SetTopLevelContent(content_name);
    if (m_int_value_ref1)
        m_int_value_ref1->SetTopLevelContent(content_name);
    if (m_int_value_ref2)
        m_int_value_ref2->SetTopLevelContent(content_name);
    if (m_int_value_ref3)
        m_int_value_ref3->SetTopLevelContent(content_name);
}

unsigned int ValueTest::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::ValueTest");
    CheckSums::CheckSumCombine(retval, m_value_ref1);
    CheckSums::CheckSumCombine(retval, m_value_ref2);
    CheckSums::CheckSumCombine(retval, m_value_ref3);
    CheckSums::CheckSumCombine(retval, m_string_value_ref1);
    CheckSums::CheckSumCombine(retval, m_string_value_ref2);
    CheckSums::CheckSumCombine(retval, m_string_value_ref3);
    CheckSums::CheckSumCombine(retval, m_int_value_ref1);
    CheckSums::CheckSumCombine(retval, m_int_value_ref2);
    CheckSums::CheckSumCombine(retval, m_int_value_ref3);
    CheckSums::CheckSumCombine(retval, m_compare_type1);
    CheckSums::CheckSumCombine(retval, m_compare_type2);

    TraceLogger() << "GetCheckSum(ValueTest): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// Location                                              //
///////////////////////////////////////////////////////////
namespace {
    const ConditionBase* GetLocationCondition(ContentType content_type,
                                              const std::string& name1,
                                              const std::string& name2)
    {
        if (name1.empty())
            return nullptr;
        switch (content_type) {
        case CONTENT_BUILDING: {
            if (auto bt = GetBuildingType(name1))
                return bt->Location();
            break;
        }
        case CONTENT_SPECIES: {
            if (auto s = GetSpecies(name1))
                return s->Location();
            break;
        }
        case CONTENT_SHIP_HULL: {
            if (auto h = GetHullType(name1))
                return h->Location();
            break;
        }
        case CONTENT_SHIP_PART: {
            if (auto p = GetPartType(name1))
                return p->Location();
            break;
        }
        case CONTENT_SPECIAL: {
            if (auto s = GetSpecial(name1))
                return s->Location();
            break;
        }
        case CONTENT_FOCUS: {
            if (name2.empty())
                return nullptr;
            // get species, then focus from that species
            if (auto s = GetSpecies(name1)) {
                for (auto& f : s->Foci()) {
                    if (f.Name() == name2)
                        return f.Location();
                }
            }
            break;
        }
        default:
            return nullptr;
        }
        return nullptr;
    }

    const std::string& GetContentTypeName(ContentType content_type) {
        switch (content_type) {
        case CONTENT_BUILDING:  return UserString("UIT_BUILDING");          break;
        case CONTENT_SPECIES:   return UserString("ENC_SPECIES");           break;
        case CONTENT_SHIP_HULL: return UserString("UIT_SHIP_HULL");         break;
        case CONTENT_SHIP_PART: return UserString("UIT_SHIP_PART");         break;
        case CONTENT_SPECIAL:   return UserString("ENC_SPECIAL");           break;
        case CONTENT_FOCUS:     return UserString("PLANETARY_FOCUS_TITLE"); break;
        default:                return EMPTY_STRING;                        break;
        }
    }
}

Location::Location(ContentType content_type,
                   std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& name1,
                   std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& name2) :
    ConditionBase(),
    m_name1(std::move(name1)),
    m_name2(std::move(name2)),
    m_content_type(content_type)
{}

bool Location::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const Location& rhs_ = static_cast<const Location&>(rhs);

    if (m_content_type != rhs_.m_content_type)
        return false;

    CHECK_COND_VREF_MEMBER(m_name1)
    CHECK_COND_VREF_MEMBER(m_name2)

    return true;
}

void Location::Eval(const ScriptingContext& parent_context,
                    ObjectSet& matches, ObjectSet& non_matches,
                    SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = ((!m_name1 || m_name1->LocalCandidateInvariant()) &&
                             (!m_name2 || m_name2->LocalCandidateInvariant()) &&
                             (parent_context.condition_root_candidate || RootCandidateInvariant()));

    if (simple_eval_safe) {
        // evaluate value and range limits once, use to match all candidates
        std::shared_ptr<const UniverseObject> no_object;
        ScriptingContext local_context(parent_context, no_object);

        std::string name1 = (m_name1 ? m_name1->Eval(local_context) : "");
        std::string name2 = (m_name2 ? m_name2->Eval(local_context) : "");

        // get condition from content, apply to matches / non_matches
        const auto condition = GetLocationCondition(m_content_type, name1, name2);
        if (condition && condition != this) {
            condition->Eval(parent_context, matches, non_matches, search_domain);
        } else {
            // if somehow in a cyclical loop because some content's location
            // was defined as Location or if there is no location
            // condition, match nothing
            if (search_domain == MATCHES) {
                non_matches.insert(non_matches.end(), matches.begin(), matches.end());
                matches.clear();
            }
        }

    } else {
        // re-evaluate value and ranges for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Location::RootCandidateInvariant() const {
    return (!m_name1    || m_name1->RootCandidateInvariant()) &&
           (!m_name2    || m_name2->RootCandidateInvariant());
}

bool Location::TargetInvariant() const {
    return (!m_name1    || m_name1->TargetInvariant()) &&
           (!m_name2    || m_name2->TargetInvariant());
}

bool Location::SourceInvariant() const {
    return (!m_name1    || m_name1->SourceInvariant()) &&
           (!m_name2    || m_name2->SourceInvariant());
}

std::string Location::Description(bool negated/* = false*/) const {
    std::string name1_str;
    if (m_name1)
        name1_str = m_name1->Description();

    std::string name2_str;
    if (m_name2)
        name2_str = m_name2->Description();

    std::string content_type_str = GetContentTypeName(m_content_type);
    std::string name_str = (m_content_type == CONTENT_FOCUS ? name2_str : name1_str);

    return str(FlexibleFormat((!negated)
               ? UserString("DESC_LOCATION")
               : UserString("DESC_LOCATION_NOT"))
               % content_type_str
               % name_str);
}

std::string Location::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "Location content_type = ";

    switch (m_content_type) {
    case CONTENT_BUILDING:  retval += "Building";   break;
    case CONTENT_FOCUS:     retval += "Focus";      break;
    case CONTENT_SHIP_HULL: retval += "Hull";       break;
    case CONTENT_SHIP_PART: retval += "Part";       break;
    case CONTENT_SPECIAL:   retval += "Special";    break;
    case CONTENT_SPECIES:   retval += "Species";    break;
    default:                retval += "???";
    }

    if (m_name1)
        retval += " name1 = " + m_name1->Dump(ntabs);
    if (m_name2)
        retval += " name2 = " + m_name2->Dump(ntabs);
    return retval;
}

bool Location::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "Location::Match passed no candidate object";
        return false;
    }

    std::string name1 = (m_name1 ? m_name1->Eval(local_context) : "");
    std::string name2 = (m_name2 ? m_name2->Eval(local_context) : "");

    const auto condition = GetLocationCondition(m_content_type, name1, name2);
    if (!condition || condition == this)
        return false;

    // other Conditions' Match functions not directly callable, so can't do any
    // better than just calling Eval for each candidate...
    return condition->Eval(local_context, candidate);
}

void Location::SetTopLevelContent(const std::string& content_name) {
    if (m_name1)
        m_name1->SetTopLevelContent(content_name);
    if (m_name2)
        m_name2->SetTopLevelContent(content_name);
}

unsigned int Location::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::Location");
    CheckSums::CheckSumCombine(retval, m_name1);
    CheckSums::CheckSumCombine(retval, m_name2);
    CheckSums::CheckSumCombine(retval, m_content_type);

    TraceLogger() << "GetCheckSum(Location): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// CombatTarget                                          //
///////////////////////////////////////////////////////////
namespace {
    const ConditionBase* GetCombatTargetCondition(
        ContentType content_type, const std::string& name)
    {
        if (name.empty())
            return nullptr;
        switch (content_type) {
        case CONTENT_SPECIES: {
            if (auto s = GetSpecies(name))
                return s->CombatTargets();
            break;
        }
        case CONTENT_SHIP_PART: {
            if (auto p = GetPartType(name))
                return p->CombatTargets();
            break;
        }
        case CONTENT_BUILDING:
        case CONTENT_SHIP_HULL:
        case CONTENT_SPECIAL:
        case CONTENT_FOCUS:
        default:
            return nullptr;
        }
        return nullptr;
    }
}

CombatTarget::CombatTarget(ContentType content_type,
                           std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& name) :
    ConditionBase(),
    m_name(std::move(name)),
    m_content_type(content_type)
{}

bool CombatTarget::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const CombatTarget& rhs_ = static_cast<const CombatTarget&>(rhs);

    if (m_content_type != rhs_.m_content_type)
        return false;

    CHECK_COND_VREF_MEMBER(m_name)

    return true;
}

void CombatTarget::Eval(const ScriptingContext& parent_context,
                        ObjectSet& matches, ObjectSet& non_matches,
                        SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = ((!m_name || m_name->LocalCandidateInvariant()) &&
                             (parent_context.condition_root_candidate || RootCandidateInvariant()));

    if (simple_eval_safe) {
        // evaluate value and range limits once, use to match all candidates
        std::shared_ptr<const UniverseObject> no_object;
        ScriptingContext local_context(parent_context, no_object);

        std::string name = (m_name ? m_name->Eval(local_context) : "");

        // get condition from content, apply to matches / non_matches
        const auto condition = GetCombatTargetCondition(m_content_type, name);
        if (condition && condition != this) {
            condition->Eval(parent_context, matches, non_matches, search_domain);
        } else {
            // if somehow in a cyclical loop because some content's location
            // was defined as CombatTarget or if there is no available combat
            // targetting condition (eg. in valid content type, or name of
            // a bit of content that doesn't exist), match nothing
            if (search_domain == MATCHES) {
                non_matches.insert(non_matches.end(), matches.begin(), matches.end());
                matches.clear();
            }
        }

    } else {
        // re-evaluate value and ranges for each candidate object
        ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool CombatTarget::RootCandidateInvariant() const
{ return (!m_name || m_name->RootCandidateInvariant()); }

bool CombatTarget::TargetInvariant() const
{ return (!m_name|| m_name->TargetInvariant()); }

bool CombatTarget::SourceInvariant() const
{ return (!m_name || m_name->SourceInvariant()); }

std::string CombatTarget::Description(bool negated/* = false*/) const {
    std::string name_str;
    if (m_name)
        name_str = m_name->Description();

    std::string content_type_str = GetContentTypeName(m_content_type);

    return str(FlexibleFormat((!negated)
               ? UserString("DESC_COMBAT_TARGET")
               : UserString("DESC_COMBAT_TARGET_NOT"))
               % content_type_str
               % name_str);
}

std::string CombatTarget::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "CombatTarget content_type = ";

    switch (m_content_type) {
    case CONTENT_BUILDING:  retval += "Building";   break;
    case CONTENT_FOCUS:     retval += "Focus";      break;
    case CONTENT_SHIP_HULL: retval += "Hull";       break;
    case CONTENT_SHIP_PART: retval += "Part";       break;
    case CONTENT_SPECIAL:   retval += "Special";    break;
    case CONTENT_SPECIES:   retval += "Species";    break;
    default:                retval += "???";
    }

    if (m_name)
        retval += " name = " + m_name->Dump(ntabs);
    return retval;
}

bool CombatTarget::Match(const ScriptingContext& local_context) const {
    auto candidate = local_context.condition_local_candidate;
    if (!candidate) {
        ErrorLogger() << "CombatTarget::Match passed no candidate object";
        return false;
    }

    std::string name = (m_name ? m_name->Eval(local_context) : "");

    const auto condition = GetCombatTargetCondition(m_content_type, name);
    if (!condition || condition == this)
        return false;

    // other Conditions' Match functions not directly callable, so can't do any
    // better than just calling Eval for each candidate...
    return condition->Eval(local_context, candidate);
}

void CombatTarget::SetTopLevelContent(const std::string& content_name) {
    if (m_name)
        m_name->SetTopLevelContent(content_name);
}

unsigned int CombatTarget::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::CombatTarget");
    CheckSums::CheckSumCombine(retval, m_name);
    CheckSums::CheckSumCombine(retval, m_content_type);

    TraceLogger() << "GetCheckSum(CombatTarget): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// And                                                   //
///////////////////////////////////////////////////////////
And::And(std::vector<std::unique_ptr<ConditionBase>>&& operands) :
    ConditionBase(),
    m_operands(std::move(operands))
{}

And::And(std::unique_ptr<ConditionBase>&& operand1, std::unique_ptr<ConditionBase>&& operand2,
         std::unique_ptr<ConditionBase>&& operand3, std::unique_ptr<ConditionBase>&& operand4) :
    ConditionBase()
{
    // would prefer to initialize the vector m_operands in the initializer list, but this is difficult with non-copyable unique_ptr parameters
    if (operand1)
        m_operands.push_back(std::move(operand1));
    if (operand2)
        m_operands.push_back(std::move(operand2));
    if (operand3)
        m_operands.push_back(std::move(operand3));
    if (operand4)
        m_operands.push_back(std::move(operand4));
}

bool And::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const And& rhs_ = static_cast<const And&>(rhs);

    if (m_operands.size() != rhs_.m_operands.size())
        return false;
    for (unsigned int i = 0; i < m_operands.size(); ++i) {
        CHECK_COND_VREF_MEMBER(m_operands.at(i))
    }

    return true;
}

void And::Eval(const ScriptingContext& parent_context, ObjectSet& matches,
               ObjectSet& non_matches, SearchDomain search_domain/* = NON_MATCHES*/) const
{
    std::shared_ptr<const UniverseObject> no_object;
    ScriptingContext local_context(parent_context, no_object);

    if (m_operands.empty()) {
        ErrorLogger() << "And::Eval given no operands!";
        return;
    }
    for (auto& operand : m_operands) {
        if (!operand) {
            ErrorLogger() << "And::Eval given null operand!";
            return;
        }
    }

    if (search_domain == NON_MATCHES) {
        ObjectSet partly_checked_non_matches;
        partly_checked_non_matches.reserve(non_matches.size());

        // move items in non_matches set that pass first operand condition into
        // partly_checked_non_matches set
        m_operands[0]->Eval(local_context, partly_checked_non_matches, non_matches, NON_MATCHES);

        // move items that don't pass one of the other conditions back to non_matches
        for (unsigned int i = 1; i < m_operands.size(); ++i) {
            if (partly_checked_non_matches.empty()) break;
            m_operands[i]->Eval(local_context, partly_checked_non_matches, non_matches, MATCHES);
        }

        // merge items that passed all operand conditions into matches
        matches.insert(matches.end(), partly_checked_non_matches.begin(), partly_checked_non_matches.end());

        // items already in matches set are not checked, and remain in matches set even if
        // they don't match one of the operand conditions

    } else /*(search_domain == MATCHES)*/ {
        // check all operand conditions on all objects in the matches set, moving those
        // that don't pass a condition to the non-matches set

        for (auto& operand : m_operands) {
            if (matches.empty()) break;
            operand->Eval(local_context, matches, non_matches, MATCHES);
        }

        // items already in non_matches set are not checked, and remain in non_matches set
        // even if they pass all operand conditions
    }
}

bool And::RootCandidateInvariant() const {
    if (m_root_candidate_invariant != UNKNOWN_INVARIANCE)
        return m_root_candidate_invariant == INVARIANT;

    for (auto& operand : m_operands) {
        if (!operand->RootCandidateInvariant()) {
            m_root_candidate_invariant = VARIANT;
            return false;
        }
    }
    m_root_candidate_invariant = INVARIANT;
    return true;
}

bool And::TargetInvariant() const {
    if (m_target_invariant != UNKNOWN_INVARIANCE)
        return m_target_invariant == INVARIANT;

    for (auto& operand : m_operands) {
        if (!operand->TargetInvariant()) {
            m_target_invariant = VARIANT;
            return false;
        }
    }
    m_target_invariant = INVARIANT;
    return true;
}

bool And::SourceInvariant() const {
    if (m_source_invariant != UNKNOWN_INVARIANCE)
        return m_source_invariant == INVARIANT;

    for (auto& operand : m_operands) {
        if (!operand->SourceInvariant()) {
            m_source_invariant = VARIANT;
            return false;
        }
    }
    m_source_invariant = INVARIANT;
    return true;
}

std::string And::Description(bool negated/* = false*/) const {
    std::string values_str;
    if (m_operands.size() == 1) {
        values_str += (!negated)
            ? UserString("DESC_AND_BEFORE_SINGLE_OPERAND")
            : UserString("DESC_NOT_AND_BEFORE_SINGLE_OPERAND");
        // Pushing the negation to the enclosed conditions
        values_str += m_operands[0]->Description(negated);
        values_str += (!negated)
            ? UserString("DESC_AND_AFTER_SINGLE_OPERAND")
            : UserString("DESC_NOT_AND_AFTER_SINGLE_OPERAND");
    } else {
        values_str += (!negated)
            ? UserString("DESC_AND_BEFORE_OPERANDS")
            : UserString("DESC_NOT_AND_BEFORE_OPERANDS");
        for (unsigned int i = 0; i < m_operands.size(); ++i) {
            // Pushing the negation to the enclosed conditions
            values_str += m_operands[i]->Description(negated);
            if (i != m_operands.size() - 1) {
                values_str += (!negated)
                    ? UserString("DESC_AND_BETWEEN_OPERANDS")
                    : UserString("DESC_NOT_AND_BETWEEN_OPERANDS");
            }
        }
        values_str += (!negated)
            ? UserString("DESC_AND_AFTER_OPERANDS")
            : UserString("DESC_NOT_AND_AFTER_OPERANDS");
    }
    return values_str;
}

std::string And::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "And [\n";
    for (auto& operand : m_operands)
        retval += operand->Dump(ntabs+1);
    retval += DumpIndent(ntabs) + "]\n";
    return retval;
}

void And::GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context, ObjectSet& condition_non_targets) const {
    if (!m_operands.empty()) {
        m_operands[0]->GetDefaultInitialCandidateObjects(parent_context, condition_non_targets); // gets condition_non_targets from first operand condition
    } else {
        ConditionBase::GetDefaultInitialCandidateObjects(parent_context, condition_non_targets);
    }
}

void And::SetTopLevelContent(const std::string& content_name) {
    for (auto& operand : m_operands) {
        operand->SetTopLevelContent(content_name);
    }
}

unsigned int And::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::And");
    CheckSums::CheckSumCombine(retval, m_operands);

    TraceLogger() << "GetCheckSum(And): retval: " << retval;
    return retval;
}

const std::vector<ConditionBase*> And::Operands() const {
    std::vector<ConditionBase*> retval(m_operands.size());
    std::transform(m_operands.begin(), m_operands.end(), retval.begin(),
                   [](const std::unique_ptr<ConditionBase>& xx) {return xx.get();});
    return retval;
}

///////////////////////////////////////////////////////////
// Or                                                    //
///////////////////////////////////////////////////////////
Or::Or(std::vector<std::unique_ptr<ConditionBase>>&& operands) :
    ConditionBase(),
    m_operands(std::move(operands))
{}

Or::Or(std::unique_ptr<ConditionBase>&& operand1,
       std::unique_ptr<ConditionBase>&& operand2,
       std::unique_ptr<ConditionBase>&& operand3,
       std::unique_ptr<ConditionBase>&& operand4) :
    ConditionBase()
{
    // would prefer to initialize the vector m_operands in the initializer list, but this is difficult with non-copyable unique_ptr parameters
    if (operand1)
        m_operands.push_back(std::move(operand1));
    if (operand2)
        m_operands.push_back(std::move(operand2));
    if (operand3)
        m_operands.push_back(std::move(operand3));
    if (operand4)
        m_operands.push_back(std::move(operand4));
}

bool Or::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const Or& rhs_ = static_cast<const Or&>(rhs);

    if (m_operands.size() != rhs_.m_operands.size())
        return false;
    for (unsigned int i = 0; i < m_operands.size(); ++i) {
        CHECK_COND_VREF_MEMBER(m_operands.at(i))
    }

    return true;
}

void Or::Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain/* = NON_MATCHES*/) const
{
    std::shared_ptr<const UniverseObject> no_object;
    ScriptingContext local_context(parent_context, no_object);

    if (m_operands.empty()) {
        ErrorLogger() << "Or::Eval given no operands!";
        return;
    }
    for (auto& operand : m_operands) {
        if (!operand) {
            ErrorLogger() << "Or::Eval given null operand!";
            return;
        }
    }

    if (search_domain == NON_MATCHES) {
        // check each item in the non-matches set against each of the operand conditions
        // if a non-candidate item matches an operand condition, move the item to the
        // matches set.

        for (auto& operand : m_operands) {
            if (non_matches.empty()) break;
            operand->Eval(local_context, matches, non_matches, NON_MATCHES);
        }

        // items already in matches set are not checked and remain in the
        // matches set even if they fail all the operand conditions

    } else {
        ObjectSet partly_checked_matches;
        partly_checked_matches.reserve(matches.size());

        // move items in matches set the fail the first operand condition into 
        // partly_checked_matches set
        m_operands[0]->Eval(local_context, matches, partly_checked_matches, MATCHES);

        // move items that pass any of the other conditions back into matches
        for (auto& operand : m_operands) {
            if (partly_checked_matches.empty()) break;
            operand->Eval(local_context, matches, partly_checked_matches, NON_MATCHES);
        }

        // merge items that failed all operand conditions into non_matches
        non_matches.insert(non_matches.end(), partly_checked_matches.begin(), partly_checked_matches.end());

        // items already in non_matches set are not checked and remain in
        // non_matches set even if they pass one or more of the operand 
        // conditions
    }
}

bool Or::RootCandidateInvariant() const {
    if (m_root_candidate_invariant != UNKNOWN_INVARIANCE)
        return m_root_candidate_invariant == INVARIANT;

    for (auto& operand : m_operands) {
        if (!operand->RootCandidateInvariant()) {
            m_root_candidate_invariant = VARIANT;
            return false;
        }
    }
    m_root_candidate_invariant = INVARIANT;
    return true;
}

bool Or::TargetInvariant() const {
    if (m_target_invariant != UNKNOWN_INVARIANCE)
        return m_target_invariant == INVARIANT;

    for (auto& operand : m_operands) {
        if (!operand->TargetInvariant()) {
            m_target_invariant = VARIANT;
            return false;
        }
    }
    m_target_invariant = INVARIANT;
    return true;
}

bool Or::SourceInvariant() const {
    if (m_source_invariant != UNKNOWN_INVARIANCE)
        return m_source_invariant == INVARIANT;

    for (auto& operand : m_operands) {
        if (!operand->SourceInvariant()) {
            m_source_invariant = VARIANT;
            return false;
        }
    }
    m_source_invariant = INVARIANT;
    return true;
}

std::string Or::Description(bool negated/* = false*/) const {
    std::string values_str;
    if (m_operands.size() == 1) {
        values_str += (!negated)
            ? UserString("DESC_OR_BEFORE_SINGLE_OPERAND")
            : UserString("DESC_NOT_OR_BEFORE_SINGLE_OPERAND");
        // Pushing the negation to the enclosed conditions
        values_str += m_operands[0]->Description(negated);
        values_str += (!negated)
            ? UserString("DESC_OR_AFTER_SINGLE_OPERAND")
            : UserString("DESC_NOT_OR_AFTER_SINGLE_OPERAND");
    } else {
        // TODO: use per-operand-type connecting language
        values_str += (!negated)
            ? UserString("DESC_OR_BEFORE_OPERANDS")
            : UserString("DESC_NOT_OR_BEFORE_OPERANDS");
        for (unsigned int i = 0; i < m_operands.size(); ++i) {
            // Pushing the negation to the enclosed conditions
            values_str += m_operands[i]->Description(negated);
            if (i != m_operands.size() - 1) {
                values_str += (!negated)
                    ? UserString("DESC_OR_BETWEEN_OPERANDS")
                    : UserString("DESC_NOT_OR_BETWEEN_OPERANDS");
            }
        }
        values_str += (!negated)
            ? UserString("DESC_OR_AFTER_OPERANDS")
            : UserString("DESC_NOT_OR_AFTER_OPERANDS");
    }
    return values_str;
}

std::string Or::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "Or [\n";
    for (auto& operand : m_operands)
        retval += operand->Dump(ntabs+1);
    retval += "\n" + DumpIndent(ntabs) + "]\n";
    return retval;
}

void Or::SetTopLevelContent(const std::string& content_name) {
    for (auto& operand : m_operands) {
        operand->SetTopLevelContent(content_name);
    }
}

unsigned int Or::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::Or");
    CheckSums::CheckSumCombine(retval, m_operands);

    TraceLogger() << "GetCheckSum(Or): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// Not                                                   //
///////////////////////////////////////////////////////////
Not::Not(std::unique_ptr<ConditionBase>&& operand) :
    ConditionBase(),
    m_operand(std::move(operand))
{}

bool Not::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const Not& rhs_ = static_cast<const Not&>(rhs);

    CHECK_COND_VREF_MEMBER(m_operand)

    return true;
}

void Not::Eval(const ScriptingContext& parent_context, ObjectSet& matches, ObjectSet& non_matches,
               SearchDomain search_domain/* = NON_MATCHES*/) const
{
    std::shared_ptr<const UniverseObject> no_object;
    ScriptingContext local_context(parent_context, no_object);

    if (!m_operand) {
        ErrorLogger() << "Not::Eval found no subcondition to evaluate!";
        return;
    }

    if (search_domain == NON_MATCHES) {
        // search non_matches set for items that don't meet the operand
        // condition, and move those to the matches set
        m_operand->Eval(local_context, non_matches, matches, MATCHES); // swapping order of matches and non_matches set parameters and MATCHES / NON_MATCHES search domain effects NOT on requested search domain
    } else {
        // search matches set for items that meet the operand condition
        // condition, and move those to the non_matches set
        m_operand->Eval(local_context, non_matches, matches, NON_MATCHES);
    }
}

bool Not::RootCandidateInvariant() const {
    if (m_root_candidate_invariant != UNKNOWN_INVARIANCE)
        return m_root_candidate_invariant == INVARIANT;
    m_root_candidate_invariant = m_operand->RootCandidateInvariant() ? INVARIANT: VARIANT;
    return m_root_candidate_invariant == INVARIANT;
}

bool Not::TargetInvariant() const {
    if (m_target_invariant != UNKNOWN_INVARIANCE)
        return m_target_invariant == INVARIANT;
    m_target_invariant = m_operand->TargetInvariant() ? INVARIANT: VARIANT;
    return m_target_invariant == INVARIANT;
}

bool Not::SourceInvariant() const {
    if (m_source_invariant != UNKNOWN_INVARIANCE)
        return m_source_invariant == INVARIANT;
    m_source_invariant = m_operand->SourceInvariant() ? INVARIANT: VARIANT;
    return m_source_invariant == INVARIANT;
}

std::string Not::Description(bool negated/* = false*/) const
{ return m_operand->Description(!negated); }

std::string Not::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "Not\n";
    retval += m_operand->Dump(ntabs+1);
    return retval;
}

void Not::SetTopLevelContent(const std::string& content_name) {
    if (m_operand)
        m_operand->SetTopLevelContent(content_name);
}

unsigned int Not::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::Not");
    CheckSums::CheckSumCombine(retval, m_operand);

    TraceLogger() << "GetCheckSum(Not): retval: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// OrderedAlternativesOf
///////////////////////////////////////////////////////////
void FCMoveContent(Condition::ObjectSet& from_set, Condition::ObjectSet& to_set) {
    to_set.insert(to_set.end(),
                  std::make_move_iterator(from_set.begin()),
                  std::make_move_iterator(from_set.end()));
    from_set.clear();
}

OrderedAlternativesOf::OrderedAlternativesOf(
    std::vector<std::unique_ptr<ConditionBase>>&& operands) :
    ConditionBase(),
    m_operands(std::move(operands))
{}

bool OrderedAlternativesOf::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const OrderedAlternativesOf& rhs_ = static_cast<const OrderedAlternativesOf&>(rhs);

    if (m_operands.size() != rhs_.m_operands.size())
        return false;
    for (unsigned int i = 0; i < m_operands.size(); ++i) {
        CHECK_COND_VREF_MEMBER(m_operands.at(i))
    }

    return true;
}

void OrderedAlternativesOf::Eval(const ScriptingContext& parent_context,
                                 ObjectSet& matches, ObjectSet& non_matches,
                                 SearchDomain search_domain/* = NON_MATCHES*/) const
{
    std::shared_ptr<const UniverseObject> no_object;
    ScriptingContext local_context(parent_context, no_object);

    if (m_operands.empty()) {
        ErrorLogger() << "OrderedAlternativesOf::Eval given no operands!";
        return;
    }
    for (auto& operand : m_operands) {
        if (!operand) {
            ErrorLogger() << "OrderedAlternativesOf::Eval given null operand!";
            return;
        }
    }

    // OrderedAlternativesOf [ A B C ] matches all candidates which match the topmost condition.
    // If any candidate matches A, then all candidates that match A are matched,
    // or if no candidate matches A but any candidate matches B, then all candidates that match B are matched,
    // or if no candidate matches A or B but any candidate matches C, then all candidates that match C are matched.
    // If no candidate matches A, B, or C, then nothing is matched.
    //
    // Not OrderedAlternativesOf [ A B C ] finds the topmost condition which has matches and then matches its non-matches.
    // If any candidate matches A, then all candidates that do not match A are matched,
    // or if no candidates match A but any candidate matches B, then all candidates that do not match B are matched,
    // or if no candidate matches A or B but any candidate matches C, then all candidates that do not match C are matched.
    // If no candidate matches A, B, or C, then all candidates are matched.
    if (search_domain == NON_MATCHES) {
        // Check each operand condition on objects in the input matches and non_matches sets, until an operand condition matches an object.
        // If an operand condition is selected, apply it to the input non_matches set, moving matching candidates to matches.
        // If no operand condition is selected, because no candidate is matched by any operand condition, then do nothing.
        ObjectSet temp_objects;
        temp_objects.reserve(std::max(matches.size(),non_matches.size()));

        for (auto& operand : m_operands) {
            operand->Eval(local_context, temp_objects, non_matches, NON_MATCHES);
            if (!temp_objects.empty()) {
                // Select the current operand condition. Apply it to the NON_MATCHES candidate set.
                // We alread did the application, so we use the results
                matches.reserve(temp_objects.size() + matches.size());
                FCMoveContent(temp_objects, matches);
                return;
            }
            // Check if the operand condition matches an object in the other input set
            operand->Eval(local_context, matches, temp_objects, MATCHES);
            if (!matches.empty()) {
                // Select the current operand condition. Apply it to the NON_MATCHES candidate set.
                // We already did the application before, but there were no matches.
                // restore state before checking the operand
                FCMoveContent(temp_objects, matches);
                return;
            }

            // restore state before checking the operand
            FCMoveContent(temp_objects, matches);
            // try the next operand
        }

        // No operand condition was selected. State is restored. Nothing should be moved to matches input set
    } else /*(search_domain == MATCHES)*/ {
        // Check each operand condition on objects in the input matches and non_matches sets, until an operand condition matches an object.
        // If an operand condition is selected, apply it to the input matches set, moving non-matching candidates to non_matches.
        // If no operand condition is selected, because no candidate is matched by any operand condition, then move all of the input matches into non_matches.
        ObjectSet temp_objects;
        temp_objects.reserve(std::max(matches.size(),non_matches.size()));

        for (auto& operand : m_operands) {
            // Apply the current operand optimistically. Select it if there are any matching objects in the input sets 
            operand->Eval(local_context, temp_objects, matches, NON_MATCHES);
            // temp_objects are objects from input matches set which also match the operand
            // matches are objects from input matches set which do not match the operand
            if (!temp_objects.empty()) {
                // Select and apply this operand. Objects in matches do not match this condition.
                non_matches.reserve(matches.size() + non_matches.size());
                FCMoveContent(matches, non_matches);
                FCMoveContent(temp_objects, matches);
                return;
            }
            // Select this operand if there are matching objects in the non_matches input set.
            operand->Eval(local_context, temp_objects, non_matches, NON_MATCHES);
            if (!temp_objects.empty()) {
                // Select and apply this operand. But no matching objects exist in the matches input set,
                // so all objects need to be moved into the non_matches set
                non_matches.reserve(matches.size() + non_matches.size() + temp_objects.size());
                FCMoveContent(matches, non_matches);
                FCMoveContent(temp_objects, non_matches);
                return;
            }

            // Operand was not selected. Restore state before. Try next operand.
            FCMoveContent(temp_objects, matches);
        }

        // No operand condition was selected. Objects in matches input set do not match, so move those to non_matches input set.
        non_matches.reserve(matches.size() + non_matches.size());
        FCMoveContent(matches, non_matches);
    }
}

bool OrderedAlternativesOf::RootCandidateInvariant() const {
    if (m_root_candidate_invariant != UNKNOWN_INVARIANCE)
        return m_root_candidate_invariant == INVARIANT;

    for (auto& operand : m_operands) {
        if (!operand->RootCandidateInvariant()) {
            m_root_candidate_invariant = VARIANT;
            return false;
        }
    }
    m_root_candidate_invariant = INVARIANT;
    return true;
}

bool OrderedAlternativesOf::TargetInvariant() const {
    if (m_target_invariant != UNKNOWN_INVARIANCE)
        return m_target_invariant == INVARIANT;

    for (auto& operand : m_operands) {
        if (!operand->TargetInvariant()) {
            m_target_invariant = VARIANT;
            return false;
        }
    }
    m_target_invariant = INVARIANT;
    return true;
}

bool OrderedAlternativesOf::SourceInvariant() const {
    if (m_source_invariant != UNKNOWN_INVARIANCE)
        return m_source_invariant == INVARIANT;

    for (auto& operand : m_operands) {
        if (!operand->SourceInvariant()) {
            m_source_invariant = VARIANT;
            return false;
        }
    }
    m_source_invariant = INVARIANT;
    return true;
}

std::string OrderedAlternativesOf::Description(bool negated/* = false*/) const {
    std::string values_str;
    if (m_operands.size() == 1) {
        values_str += (!negated)
            ? UserString("DESC_ORDERED_ALTERNATIVES_BEFORE_SINGLE_OPERAND")
            : UserString("DESC_NOT_ORDERED_ALTERNATIVES_BEFORE_SINGLE_OPERAND");
        // Pushing the negation of matches to the enclosed conditions
        values_str += m_operands[0]->Description(negated);
        values_str += (!negated)
            ? UserString("DESC_ORDERED_ALTERNATIVES_AFTER_SINGLE_OPERAND")
            : UserString("DESC_NOT_ORDERED_ALTERNATIVES_AFTER_SINGLE_OPERAND");
    } else {
        // TODO: use per-operand-type connecting language
        values_str += (!negated)
            ? UserString("DESC_ORDERED_ALTERNATIVES_BEFORE_OPERANDS")
            : UserString("DESC_NOT_ORDERED_ALTERNATIVES_BEFORE_OPERANDS");
        for (unsigned int i = 0; i < m_operands.size(); ++i) {
            // Pushing the negation of matches to the enclosed conditions
            values_str += m_operands[i]->Description(negated);
            if (i != m_operands.size() - 1) {
                values_str += (!negated)
                    ? UserString("DESC_ORDERED_ALTERNATIVES_BETWEEN_OPERANDS")
                    : UserString("DESC_NOT_ORDERED_ALTERNATIVES_BETWEEN_OPERANDS");
            }
        }
        values_str += (!negated)
            ? UserString("DESC_ORDERED_ALTERNATIVES_AFTER_OPERANDS")
            : UserString("DESC_NOT_ORDERED_ALTERNATIVES_AFTER_OPERANDS");
    }
    return values_str;
}

std::string OrderedAlternativesOf::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "OrderedAlternativesOf [\n";
    for (auto& operand : m_operands)
        retval += operand->Dump(ntabs+1);
    retval += DumpIndent(ntabs) + "]\n";
    return retval;
}

void OrderedAlternativesOf::SetTopLevelContent(const std::string& content_name) {
    for (auto& operand : m_operands) {
        operand->SetTopLevelContent(content_name);
    }
}

unsigned int OrderedAlternativesOf::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::OrderedAlternativesOf");
    CheckSums::CheckSumCombine(retval, m_operands);

    TraceLogger() << "GetCheckSum(OrderedAlternativesOf): retval: " << retval;
    return retval;
}

const std::vector<ConditionBase*> OrderedAlternativesOf::Operands() const {
    std::vector<ConditionBase*> retval(m_operands.size());
    std::transform(m_operands.begin(), m_operands.end(), retval.begin(),
                   [](const std::unique_ptr<ConditionBase>& xx) {return xx.get();});
    return retval;
}

///////////////////////////////////////////////////////////
// Described                                             //
///////////////////////////////////////////////////////////
bool Described::operator==(const ConditionBase& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const Described& rhs_ = static_cast<const Described&>(rhs);

   if (m_desc_stringtable_key != rhs_.m_desc_stringtable_key)
        return false;

    CHECK_COND_VREF_MEMBER(m_condition)

    return true;
}

void Described::Eval(const ScriptingContext& parent_context, ObjectSet& matches, ObjectSet& non_matches,
                     SearchDomain search_domain/* = NON_MATCHES*/) const
{
    std::shared_ptr<const UniverseObject> no_object;
    ScriptingContext local_context(parent_context, no_object);

    if (!m_condition) {
        ErrorLogger() << "Described::Eval found no subcondition to evaluate!";
        return;
    }

    return m_condition->Eval(parent_context, matches, non_matches, search_domain);
}

std::string Described::Description(bool negated/* = false*/) const {
    if (!m_desc_stringtable_key.empty() && UserStringExists(m_desc_stringtable_key))
        return UserString(m_desc_stringtable_key);
    if (m_condition)
        return m_condition->Description(negated);
    return "";
}

bool Described::RootCandidateInvariant() const
{ return !m_condition || m_condition->RootCandidateInvariant(); }

bool Described::TargetInvariant() const
{ return !m_condition || m_condition->TargetInvariant(); }

bool Described::SourceInvariant() const
{ return !m_condition || m_condition->SourceInvariant(); }

void Described::SetTopLevelContent(const std::string& content_name) {
    if (m_condition)
        m_condition->SetTopLevelContent(content_name);
}

unsigned int Described::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Condition::Described");
    CheckSums::CheckSumCombine(retval, m_condition);
    CheckSums::CheckSumCombine(retval, m_desc_stringtable_key);

    TraceLogger() << "GetCheckSum(Described): retval: " << retval;
    return retval;
}
} // namespace Condition
