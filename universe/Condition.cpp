#include "Condition.h"

#include "../util/AppInterface.h"
#include "../util/Random.h"
#include "UniverseObject.h"
#include "Building.h"
#include "Fleet.h"
#include "Ship.h"
#include "Planet.h"
#include "System.h"
#include "Species.h"
#include "Meter.h"
#include "ValueRef.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/st_connected.hpp>

using boost::io::str;

extern int g_indent;

namespace {
    const Fleet* FleetFromObject(const UniverseObject* obj) {
        const Fleet* retval = universe_object_cast<const Fleet*>(obj);
        if (!retval) {
            if (const Ship* ship = universe_object_cast<const Ship*>(obj))
                retval = GetFleet(ship->FleetID());
        }
        return retval;
    }

    template <class Pred>
    void EvalImpl(Condition::ObjectSet& matches, Condition::ObjectSet& non_matches,
                  Condition::SearchDomain search_domain, const Pred& pred)
    {
        Condition::ObjectSet& from_set = search_domain == Condition::MATCHES ? matches : non_matches;
        Condition::ObjectSet& to_set = search_domain == Condition::MATCHES ? non_matches : matches;
        for (Condition::ObjectSet::iterator it = from_set.begin(); it != from_set.end(); ) {
            bool match = pred(*it);
            if ((search_domain == Condition::MATCHES && !match) || (search_domain == Condition::NON_MATCHES && match)) {
                to_set.push_back(*it);
                *it = from_set.back();
                from_set.pop_back();
            } else {
                ++it;
            }
        }
    }

    std::vector<const Condition::ConditionBase*> FlattenAndNestedConditions(
        const std::vector<const Condition::ConditionBase*>& input_conditions)
    {
        std::vector<const Condition::ConditionBase*> retval;
        for (std::vector<const Condition::ConditionBase*>::const_iterator it = input_conditions.begin();
             it != input_conditions.end(); ++it)
        {
            if (const Condition::And* and_condition = dynamic_cast<const Condition::And*>(*it)) {
                std::vector<const Condition::ConditionBase*> flattened_operands =
                    FlattenAndNestedConditions(and_condition->Operands());
                std::copy(flattened_operands.begin(), flattened_operands.end(), std::back_inserter(retval));
            } else {
                retval.push_back(*it);
            }
        }
        return retval;
    }

    std::map<std::string, bool> ConditionDescriptionAndTest(const std::vector<const Condition::ConditionBase*>& conditions,
                                                            const ScriptingContext& parent_context,
                                                            const UniverseObject* candidate_object/* = 0*/)
    {
        std::map<std::string, bool> retval;

        std::vector<const Condition::ConditionBase*> flattened_conditions;
        if (conditions.empty())
            return retval;
        else if (conditions.size() > 1 || dynamic_cast<const Condition::And*>(*conditions.begin()))
            flattened_conditions = FlattenAndNestedConditions(conditions);
        //else if (dynamic_cast<const Condition::Or*>(*conditions.begin()))
        //    flattened_conditions = FlattenOrNestedConditions(conditions);
        else
            flattened_conditions = conditions;

        for (std::vector<const Condition::ConditionBase*>::const_iterator it = flattened_conditions.begin();
             it != flattened_conditions.end(); ++it)
        {
            const Condition::ConditionBase* condition = *it;
            retval[condition->Description()] = condition->Eval(parent_context, candidate_object);
        }
        return retval;
    }
}

std::string ConditionDescription(const std::vector<const Condition::ConditionBase*>& conditions,
                                 const UniverseObject* candidate_object/* = 0*/,
                                 const UniverseObject* source_object/* = 0*/)
{
    if (conditions.empty())
        return UserString("NONE");

    ScriptingContext parent_context(source_object);
    // test candidate against all input conditions, and store descriptions of each
    std::map<std::string, bool> condition_description_and_test_results =
        ConditionDescriptionAndTest(conditions, parent_context, candidate_object);
    bool all_conditions_match_candidate = true, at_least_one_condition_matches_candidate = false;
    for (std::map<std::string, bool>::const_iterator it = condition_description_and_test_results.begin();
         it != condition_description_and_test_results.end(); ++it)
    {
        all_conditions_match_candidate = all_conditions_match_candidate && it->second;
        at_least_one_condition_matches_candidate = at_least_one_condition_matches_candidate || it->second;
    }

    // concatenate (non-duplicated) single-description results
    std::string retval;
    if (conditions.size() > 1 || dynamic_cast<const Condition::And*>(*conditions.begin())) {
        retval += UserString("ALL_OF") + " ";
        retval += (all_conditions_match_candidate ? UserString("PASSED") : UserString("FAILED")) + "\n";
    } else if (dynamic_cast<const Condition::Or*>(*conditions.begin())) {
        retval += UserString("ANY_OF") + " ";
        retval += (at_least_one_condition_matches_candidate ? UserString("PASSED") : UserString("FAILED")) + "\n";
    }
    // else just output single condition description and PASS/FAIL text

    for (std::map<std::string, bool>::const_iterator it = condition_description_and_test_results.begin();
         it != condition_description_and_test_results.end(); ++it)
    {
        retval += (it->second ? UserString("PASSED") : UserString("FAILED"));
        retval += " " + it->first + "\n";
    }
    return retval;
}

///////////////////////////////////////////////////////////
// Condition::ConditionBase                              //
///////////////////////////////////////////////////////////
struct Condition::ConditionBase::MatchHelper {
    MatchHelper(const Condition::ConditionBase* this_, const ScriptingContext& parent_context) :
        m_this(this_),
        m_parent_context(parent_context)
    {}
    bool operator()(const UniverseObject* candidate) const
    { return m_this->Match(ScriptingContext(m_parent_context, candidate)); }
    const Condition::ConditionBase* m_this;
    const ScriptingContext& m_parent_context;
};

Condition::ConditionBase::~ConditionBase()
{}

void Condition::ConditionBase::Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                                    ObjectSet& non_matches, SearchDomain search_domain/* = NON_MATCHES*/) const
{ EvalImpl(matches, non_matches, search_domain, MatchHelper(this, parent_context)); }

void Condition::ConditionBase::Eval(ObjectSet& matches, ObjectSet& non_matches,
                                    SearchDomain search_domain/* = NON_MATCHES*/) const
{ Eval(ScriptingContext(), matches, non_matches, search_domain); }

void Condition::ConditionBase::Eval(const ScriptingContext& parent_context,
                                    Condition::ObjectSet& matches) const
{
    matches.clear();
    // evaluate condition on all objects in Universe
    Condition::ObjectSet condition_non_targets;
    condition_non_targets.reserve(RESERVE_SET_SIZE);
    for (ObjectMap::const_iterator it = Objects().const_begin(); it != Objects().const_end(); ++it)
        condition_non_targets.push_back(it->second);
    Eval(parent_context, matches, condition_non_targets);
}

void Condition::ConditionBase::Eval(Condition::ObjectSet& matches) const
{ Eval(ScriptingContext(), matches); }

bool Condition::ConditionBase::Eval(const ScriptingContext& parent_context,
                                    const UniverseObject* candidate) const
{
    if (!candidate)
        return false;
    Condition::ObjectSet non_matches, matches;
    non_matches.push_back(candidate);
    Eval(parent_context, matches, non_matches);
    return non_matches.empty(); // if candidate has been matched, non_matches will now be empty
}

bool Condition::ConditionBase::Eval(const UniverseObject* candidate) const {
    if (!candidate)
        return false;
    Condition::ObjectSet non_matches, matches;
    non_matches.push_back(candidate);
    Eval(ScriptingContext(), matches, non_matches);
    return non_matches.empty(); // if candidate has been matched, non_matches will now be empty
}

std::string Condition::ConditionBase::Description(bool negated/* = false*/) const
{ return ""; }

std::string Condition::ConditionBase::Dump() const
{ return ""; }

bool Condition::ConditionBase::Match(const ScriptingContext& local_context) const
{ return false; }

///////////////////////////////////////////////////////////
// Number                                                //
///////////////////////////////////////////////////////////
Condition::Number::~Number()
{
    delete m_low;
    delete m_high;
    delete m_condition;
}

std::string Condition::Number::Description(bool negated/* = false*/) const
{
    std::string low_str = (m_low ? (ValueRef::ConstantExpr(m_low) ?
                                    boost::lexical_cast<std::string>(m_low->Eval()) :
                                    m_low->Description())
                                 : "0");
    std::string high_str = (m_high ? (ValueRef::ConstantExpr(m_high) ?
                                      boost::lexical_cast<std::string>(m_high->Eval()) :
                                      m_high->Description())
                                   : boost::lexical_cast<std::string>(INT_MAX));
    std::string description_str = "DESC_NUMBER";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str))
               % low_str
               % high_str
               % m_condition->Description());
}

std::string Condition::Number::Dump() const
{
    std::string retval = DumpIndent() + "Number";
    if (m_low)
        retval += " low = " + m_low->Dump();
    if (m_high)
        retval += " high = " + m_high->Dump();
    retval += " condition =\n";
    ++g_indent;
        retval += m_condition->Dump();
    --g_indent;
    return retval;
}

void Condition::Number::Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain/* = NON_MATCHES*/) const
{
    // Number does not have a single valid local candidate to be matched, as it
    // will match anything if the proper number of objects match the
    // subcondition.  So, the local context that is passed to the subcondition
    // needs to have a null local candidate.
    const UniverseObject* no_object(0);
    ScriptingContext local_context(parent_context, no_object);

    if (!(
                (!m_low || m_low->LocalCandidateInvariant())
             && (!m_high || m_high->LocalCandidateInvariant())
       ))
    {
        Logger().errorStream() << "Condition::Number::Eval has local candidate-dependent ValueRefs, but no valid local candidate!";
    } else if (
                !local_context.condition_root_candidate
                && !(
                        (!m_low || m_low->RootCandidateInvariant())
                     && (!m_high || m_high->RootCandidateInvariant())
                    )
              )
    {
        Logger().errorStream() << "Condition::Number::Eval has root candidate-dependent ValueRefs, but expects local candidate to be the root candidate, and has no valid local candidate!";
    }

    if (!local_context.condition_root_candidate && !this->RootCandidateInvariant()) {
        // no externally-defined root candidate, so each object matched must
        // separately act as a root candidate, and sub-condition must be re-
        // evaluated for each tested object and the number of objects matched
        // checked for each object being tested
        Condition::ConditionBase::Eval(local_context, matches, non_matches, search_domain);

    } else {
        // parameters for number of subcondition objects that needs to be matched
        int low = (m_low ? m_low->Eval(local_context) : 0);
        int high = (m_high ? m_high->Eval(local_context) : INT_MAX);

        // get set of all UniverseObjects that satisfy m_condition
        ObjectSet condition_matches;
        condition_matches.reserve(RESERVE_SET_SIZE);
        ObjectSet condition_non_matches;
        condition_non_matches.reserve(RESERVE_SET_SIZE);
        ObjectMap& objects = GetUniverse().Objects();
        for (ObjectMap::iterator uit = objects.begin(); uit != objects.end(); ++uit)
            condition_non_matches.push_back(uit->second);
        // can evaluate subcondition once for all objects being tested by this condition
        m_condition->Eval(local_context, condition_matches, condition_non_matches, NON_MATCHES);
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

bool Condition::Number::RootCandidateInvariant() const {
    return (!m_low || m_low->RootCandidateInvariant()) &&
           (!m_high || m_high->RootCandidateInvariant()) &&
           m_condition->RootCandidateInvariant();
}

bool Condition::Number::TargetInvariant() const
{ return (!m_low || m_low->TargetInvariant()) && (!m_high || m_high->TargetInvariant()) && m_condition->TargetInvariant(); }

bool Condition::Number::Match(const ScriptingContext& local_context) const {
    // get acceptable range of subcondition matches for candidate
    double low = (m_low ? std::max(0, m_low->Eval(local_context)) : 0);
    double high = (m_high ? std::min(m_high->Eval(local_context), INT_MAX) : INT_MAX);

    // get set of all UniverseObjects that satisfy m_condition
    ObjectSet condition_matches;
    condition_matches.reserve(RESERVE_SET_SIZE);
    ObjectSet condition_non_matches;
    condition_non_matches.reserve(RESERVE_SET_SIZE);
    ObjectMap& objects = GetUniverse().Objects();
    for (ObjectMap::iterator uit = objects.begin(); uit != objects.end(); ++uit)
        condition_non_matches.push_back(uit->second);
    m_condition->Eval(local_context, condition_matches, condition_non_matches, NON_MATCHES);

    // compare number of objects that satisfy m_condition to the acceptable range of such objects
    int matched = condition_matches.size();
    bool in_range = (low <= matched && matched <= high);
    return in_range;
}


///////////////////////////////////////////////////////////
// Turn                                                  //
///////////////////////////////////////////////////////////
Condition::Turn::~Turn() {
    delete m_low;
    delete m_high;
}

void Condition::Turn::Eval(const ScriptingContext& parent_context, ObjectSet& matches, ObjectSet& non_matches, SearchDomain search_domain/* = NON_MATCHES*/) const {
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
        const UniverseObject* no_object(0);
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
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::Turn::RootCandidateInvariant() const
{ return (!m_low || m_low->RootCandidateInvariant()) && (!m_high || m_high->RootCandidateInvariant()); }

bool Condition::Turn::TargetInvariant() const
{ return (!m_low || m_low->TargetInvariant()) && (!m_high || m_high->TargetInvariant()); }

std::string Condition::Turn::Description(bool negated/* = false*/) const {
    std::string low_str;
    if (m_low)
        low_str = (ValueRef::ConstantExpr(m_low) ?
                   boost::lexical_cast<std::string>(m_low->Eval()) :
                   m_low->Description());
    std::string high_str;
    if (m_high)
        high_str = (ValueRef::ConstantExpr(m_high) ?
                    boost::lexical_cast<std::string>(m_high->Eval()) :
                    m_high->Description());
    std::string description_str;
    if (m_low && m_high)
        description_str = "DESC_TURN";
    else if (m_low)
        description_str = "DESC_TURN_MIN_ONLY";
    else if (m_high)
        description_str = "DESC_TURN_MAX_ONLY";
    else
        description_str = "DESC_TURN_ANY";
    if (negated)
        description_str += "_NOT";

    if (m_low && m_high)
        return str(FlexibleFormat(UserString(description_str))
                   % low_str
                   % high_str);
    else if (m_low)
        return str(FlexibleFormat(UserString(description_str))
                   % low_str);
    else if (m_high)
        return str(FlexibleFormat(UserString(description_str))
                   % high_str);
    else
        return UserString(description_str);
}

std::string Condition::Turn::Dump() const {
    std::string retval = DumpIndent() + "Turn";
    if (m_low)
        retval += " low = " + m_low->Dump();
    if (m_high)
        retval += " high = " + m_high->Dump();
    retval += "\n";
    return retval;
}

bool Condition::Turn::Match(const ScriptingContext& local_context) const {
    double low = (m_low ? std::max(0, m_low->Eval(local_context)) : 0);
    double high = (m_high ? std::min(m_high->Eval(local_context), IMPOSSIBLY_LARGE_TURN) : IMPOSSIBLY_LARGE_TURN);
    int turn = CurrentTurn();
    return (low <= turn && turn <= high);
}

///////////////////////////////////////////////////////////
// SortedNumberOf                                        //
///////////////////////////////////////////////////////////
Condition::SortedNumberOf::~SortedNumberOf() {
    delete m_number;
    delete m_sort_key;
    delete m_condition;
}

namespace {
    /** Random number genrator function to use with random_shuffle */
    int CustomRandInt(int max_plus_one) {
        return RandSmallInt(0, max_plus_one - 1);
    }
    int (*CRI)(int) = CustomRandInt;

    /** Transfers the indicated \a number of objects, randomly selected from from_set to to_set */
    void TransferRandomObjects(unsigned int number, Condition::ObjectSet& from_set, Condition::ObjectSet& to_set) {
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
        for (Condition::ObjectSet::iterator it = from_set.begin(); it != from_set.end(); ++i) {
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
    void TransferSortedObjects(unsigned int number, const ValueRef::ValueRefBase<double>* sort_key,
                               const ScriptingContext& context, Condition::SortingMethod sorting_method,
                               Condition::ObjectSet& from_set, Condition::ObjectSet& to_set)
    {
        // handle random case, which doesn't need sorting key
        if (sorting_method == Condition::SORT_RANDOM) {
            TransferRandomObjects(number, from_set, to_set);
            return;
        }

        // for other SoringMethods, need sort key values
        if (!sort_key) {
            Logger().errorStream() << "TransferSortedObjects given null sort_key";
            return;
        }

        // get sort key values for all objects in from_set, and sort by inserting into map
        std::multimap<double, const UniverseObject*> sort_key_objects;
        for (Condition::ObjectSet::const_iterator it = from_set.begin(); it != from_set.end(); ++it) {
            double sort_value = sort_key->Eval(ScriptingContext(context, *it));
            sort_key_objects.insert(std::make_pair(sort_value, *it));
        }

        // how many objects to select?
        number = std::min<unsigned int>(number, sort_key_objects.size());
        if (number == 0)
            return;
        unsigned int number_transferred(0);

        // pick max / min / most common values
        if (sorting_method == Condition::SORT_MIN) {
            // move (number) objects with smallest sort key (at start of map)
            // from the from_set into the to_set.
            for (std::multimap<double, const UniverseObject*>::const_iterator sorted_it = sort_key_objects.begin();
                 sorted_it != sort_key_objects.end(); ++sorted_it)
            {
                const UniverseObject* object_to_transfer = sorted_it->second;
                Condition::ObjectSet::iterator from_it = std::find(from_set.begin(), from_set.end(), object_to_transfer);
                if (from_it != from_set.end()) {
                    *from_it = from_set.back();
                    from_set.pop_back();
                    to_set.push_back(object_to_transfer);
                    number_transferred++;
                    if (number_transferred >= number)
                        return;
                }
            }
        } else if (sorting_method == Condition::SORT_MAX) {
            // move (number) objects with largest sort key (at end of map)
            // from the from_set into the to_set.
            for (std::multimap<double, const UniverseObject*>::reverse_iterator sorted_it = sort_key_objects.rbegin();  // would use const_reverse_iterator but this causes a compile error in some compilers
                 sorted_it != sort_key_objects.rend(); ++sorted_it)
            {
                const UniverseObject* object_to_transfer = sorted_it->second;
                Condition::ObjectSet::iterator from_it = std::find(from_set.begin(), from_set.end(), object_to_transfer);
                if (from_it != from_set.end()) {
                    *from_it = from_set.back();
                    from_set.pop_back();
                    to_set.push_back(object_to_transfer);
                    number_transferred++;
                    if (number_transferred >= number)
                        return;                }
            }
        } else if (sorting_method == Condition::SORT_MODE) {
            // compile histogram of of number of times each sort key occurs
            std::map<double, unsigned int> histogram;
            for (std::multimap<double, const UniverseObject*>::const_iterator sorted_it = sort_key_objects.begin();
                 sorted_it != sort_key_objects.end(); ++sorted_it)
            {
                histogram[sorted_it->first]++;
            }
            // invert histogram to index by number of occurances
            std::multimap<unsigned int, double> inv_histogram;
            for (std::map<double, unsigned int>::const_iterator hist_it = histogram.begin();
                 hist_it != histogram.end(); ++hist_it)
            {
                inv_histogram.insert(std::make_pair(hist_it->second, hist_it->first));
            }
            // reverse-loop through inverted histogram to find which sort keys
            // occurred most frequently, and transfer objects with those sort
            // keys from from_set to to_set.
            for (std::multimap<unsigned int, double>::reverse_iterator inv_hist_it = inv_histogram.rbegin();  // would use const_reverse_iterator but this causes a compile error in some compilers
                 inv_hist_it != inv_histogram.rend(); ++inv_hist_it)
            {
                double cur_sort_key = inv_hist_it->second;

                // get range of objects with the current sort key
                std::pair<std::multimap<double, const UniverseObject*>::const_iterator,
                          std::multimap<double, const UniverseObject*>::const_iterator> key_range =
                    sort_key_objects.equal_range(cur_sort_key);

                // loop over range, selecting objects to transfer from from_set to to_set
                for (std::multimap<double, const UniverseObject*>::const_iterator sorted_it = key_range.first;
                     sorted_it != key_range.second; ++sorted_it)
                {
                    const UniverseObject* object_to_transfer = sorted_it->second;
                    Condition::ObjectSet::iterator from_it = std::find(from_set.begin(), from_set.end(), object_to_transfer);
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
            Logger().debugStream() << "TransferSortedObjects given unknown sort method";
        }
    }
}

void Condition::SortedNumberOf::Eval(const ScriptingContext& parent_context, Condition::ObjectSet& matches,
                                     Condition::ObjectSet& non_matches, SearchDomain search_domain/* = NON_MATCHES*/) const
{
    // Most conditions match objects independently of the other objects being
    // tested, but the number parameter for NumberOf conditions makes things
    // more complicated.  In order to match some number of the potential
    // matchs property, both the matches and non_matches need to be checked
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
    const UniverseObject* no_object(0);
    ScriptingContext local_context(parent_context, no_object);

    // which input matches match the subcondition?
    ObjectSet subcondition_matching_matches;
    subcondition_matching_matches.reserve(RESERVE_SET_SIZE);
    m_condition->Eval(local_context, subcondition_matching_matches, matches, NON_MATCHES);

    // remaining input matches don't match the subcondition...
    ObjectSet subcondition_non_matching_matches = matches;
    matches.clear();    // to be refilled later

    // which input non_matches match the subcondition?
    ObjectSet subcondition_matching_non_matches;
    subcondition_matching_non_matches.reserve(RESERVE_SET_SIZE);
    m_condition->Eval(local_context, subcondition_matching_non_matches, non_matches, NON_MATCHES);

    // remaining input non_matches don't match the subcondition...
    ObjectSet subcondition_non_matching_non_matches = non_matches;
    non_matches.clear();    // to be refilled later

    // assemble single set of subcondition matching objects
    ObjectSet all_subcondition_matches = subcondition_matching_matches;
    all_subcondition_matches.reserve(RESERVE_SET_SIZE);
    all_subcondition_matches.insert(all_subcondition_matches.end(), subcondition_matching_non_matches.begin(), subcondition_matching_non_matches.end());

    // how many subcondition matches to select as matches to this condition
    int number = m_number->Eval(local_context);

    // compile single set of all objects that are matched by this condition.
    // these are the objects that should be transferred from non_matches into
    // matches, or those left in matches while the rest are moved into non_matches
    ObjectSet matched_objects;
    matched_objects.reserve(RESERVE_SET_SIZE);
    TransferSortedObjects(number, m_sort_key, local_context, m_sorting_method, all_subcondition_matches, matched_objects);

    // put objects back into matches and non_target sets as output...

    if (search_domain == NON_MATCHES) {
        // put matched objects that are in subcondition_matching_non_matches into matches
        for (ObjectSet::const_iterator match_it = matched_objects.begin(); match_it != matched_objects.end(); ++match_it) {
            const UniverseObject* matched_object = *match_it;

            // is this matched object in subcondition_matching_non_matches?
            ObjectSet::iterator smnt_it = std::find(subcondition_matching_non_matches.begin(), subcondition_matching_non_matches.end(), matched_object);
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
        for (ObjectSet::const_iterator match_it = matched_objects.begin(); match_it != matched_objects.end(); ++match_it) {
            const UniverseObject* matched_object = *match_it;

            // is this matched object in subcondition_matching_matches?
            ObjectSet::iterator smt_it = std::find(subcondition_matching_matches.begin(), subcondition_matching_matches.end(), matched_object);
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

std::string Condition::SortedNumberOf::Description(bool negated/* = false*/) const {
    std::string number_str = ValueRef::ConstantExpr(m_number) ? boost::lexical_cast<std::string>(m_number->Dump()) : m_number->Description();

    if (m_sorting_method == SORT_RANDOM) {
        std::string description_str = "DESC_NUMBER_OF";
        if (negated)
            description_str += "_NOT";
        return str(FlexibleFormat(UserString(description_str))
                   % number_str
                   % m_condition->Description());
    } else {
        std::string sort_key_str = ValueRef::ConstantExpr(m_sort_key) ? boost::lexical_cast<std::string>(m_sort_key->Dump()) : m_sort_key->Description();

        std::string description_str, temp;
        switch (m_sorting_method) {
        case SORT_MAX:
            temp = "DESC_MAX_NUMBER_OF";
            if (negated)
                temp += "_NOT";
            description_str = UserString(temp);
            break;

        case SORT_MIN:
            temp = "DESC_MIN_NUMBER_OF";
            if (negated)
                temp += "_NOT";
            description_str = UserString(temp);
            break;

        case SORT_MODE:
            temp = "DESC_MODE_NUMBER_OF";
            if (negated)
                temp += "_NOT";
            description_str = UserString(temp);
            break;
        default:
            break;
        }

        return str(FlexibleFormat(UserString(description_str))
                   % number_str
                   % sort_key_str
                   % m_condition->Description());
    }
}

std::string Condition::SortedNumberOf::Dump() const {
    std::string retval = DumpIndent();
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

    retval += " number = " + m_number->Dump();

    if (m_sort_key)
         retval += " sortby = " + m_sort_key->Dump();

    retval += " condition =\n";
    ++g_indent;
        retval += m_condition->Dump();
    --g_indent;

    return retval;
}

///////////////////////////////////////////////////////////
// All                                                   //
///////////////////////////////////////////////////////////
void Condition::All::Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                          ObjectSet& non_matches, SearchDomain search_domain/* = NON_MATCHES*/) const
{
    if (search_domain == NON_MATCHES) {
        // move all objects from non_matches to matches
        matches.insert(matches.end(), non_matches.begin(), non_matches.end());
        non_matches.clear();
    }
    // if search_comain is MATCHES, do nothing: all objects in matches set
    // match this condition, so should remain in matches set
}

std::string Condition::All::Description(bool negated/* = false*/) const {
    std::string description_str = "DESC_ALL";
    if (negated)
        description_str += "_NOT";
    return UserString(description_str);
}

std::string Condition::All::Dump() const
{ return DumpIndent() + "All\n"; }

///////////////////////////////////////////////////////////
// EmpireAffiliation                                     //
///////////////////////////////////////////////////////////
Condition::EmpireAffiliation::~EmpireAffiliation() {
    if (m_empire_id)
        delete m_empire_id;
}

namespace {
    struct EmpireAffiliationSimpleMatch {
        EmpireAffiliationSimpleMatch(int empire_id, EmpireAffiliationType affiliation) :
            m_empire_id(empire_id),
            m_affiliation(affiliation)
        {}

        bool operator()(const UniverseObject* candidate) const {
            if (!candidate)
                return false;

            switch (m_affiliation) {
            case AFFIL_SELF:
                return m_empire_id != ALL_EMPIRES && candidate->OwnedBy(m_empire_id);
                break;
            case AFFIL_ENEMY:
                return m_empire_id != ALL_EMPIRES && !candidate->Unowned() && !candidate->OwnedBy(m_empire_id);
                break;
            case AFFIL_ANY:
                return !candidate->Unowned();
                break;
            default:
                return false;
                break;
            }
        }

        int m_empire_id;
        EmpireAffiliationType m_affiliation;
    };
}

void Condition::EmpireAffiliation::Eval(const ScriptingContext& parent_context, ObjectSet& matches, ObjectSet& non_matches,
                                        SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = (!m_empire_id || ValueRef::ConstantExpr(m_empire_id)) ||
                            ((!m_empire_id || m_empire_id->LocalCandidateInvariant()) &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate empire id once, and use to check all candidate objects
        const UniverseObject* no_object(0);
        int empire_id = m_empire_id ? m_empire_id->Eval(ScriptingContext(parent_context, no_object)) : ALL_EMPIRES;
        EvalImpl(matches, non_matches, search_domain, EmpireAffiliationSimpleMatch(empire_id, m_affiliation));
    } else {
        // re-evaluate empire id for each candidate object
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::EmpireAffiliation::RootCandidateInvariant() const
{ return m_empire_id ? m_empire_id->RootCandidateInvariant() : true; }

bool Condition::EmpireAffiliation::TargetInvariant() const
{ return m_empire_id ? m_empire_id->TargetInvariant() : true; }

std::string Condition::EmpireAffiliation::Description(bool negated/* = false*/) const {
    std::string empire_str;
    if (m_empire_id) {
        int empire_id = ALL_EMPIRES;
        if (ValueRef::ConstantExpr(m_empire_id))
            empire_id = m_empire_id->Eval();
        if (const Empire* empire = Empires().Lookup(empire_id))
            empire_str = empire->Name();
        else
            empire_str = m_empire_id->Description();
    }

    if (m_affiliation == AFFIL_SELF) {
        std::string description_str = "DESC_EMPIRE_AFFILIATION_SELF";
        if (negated)
            description_str += "_NOT";
        return str(FlexibleFormat(UserString(description_str)) % empire_str);
    } else if (m_affiliation == AFFIL_ANY) {
        std::string description_str = "DESC_EMPIRE_AFFILIATION_ANY";
        if (negated)
            description_str += "_NOT";
        return UserString(description_str);
    } else {
        std::string description_str = "DESC_EMPIRE_AFFILIATION";
        if (negated)
            description_str += "_NOT";
        return str(FlexibleFormat(UserString(description_str))
                   % UserString(boost::lexical_cast<std::string>(m_affiliation))
                   % empire_str);
    }
}

std::string Condition::EmpireAffiliation::Dump() const {
    std::string retval = DumpIndent() + "OwnedBy";
    retval += " affiliation = ";
    switch (m_affiliation) {
    case AFFIL_SELF:    retval += "TheEmpire";  break;
    case AFFIL_ENEMY:   retval += "EnemyOf";    break;
    case AFFIL_ALLY:    retval += "AllyOf";     break;
    case AFFIL_ANY:     retval += "AnyEmpire";  break;
    default:            retval += "?";          break;
    }
    if (m_empire_id)
        retval += " empire = " + m_empire_id->Dump() + "\n";
    return retval;
}

bool Condition::EmpireAffiliation::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "EmpireAffiliation::Match passed no candidate object";
        return false;
    }

    int empire_id = m_empire_id ? m_empire_id->Eval(local_context) : ALL_EMPIRES;

    return EmpireAffiliationSimpleMatch(empire_id, m_affiliation)(candidate);
}

///////////////////////////////////////////////////////////
// Source                                                //
///////////////////////////////////////////////////////////
std::string Condition::Source::Description(bool negated/* = false*/) const {
    std::string description_str = "DESC_SOURCE";
    if (negated)
        description_str += "_NOT";
    return UserString(description_str);
}

std::string Condition::Source::Dump() const
{ return DumpIndent() + "Source\n"; }

bool Condition::Source::Match(const ScriptingContext& local_context) const {
    if (!local_context.source)
        return false;
    return local_context.source == local_context.condition_local_candidate;
}

///////////////////////////////////////////////////////////
// RootCandidate                                         //
///////////////////////////////////////////////////////////
std::string Condition::RootCandidate::Description(bool negated/* = false*/) const {
    std::string description_str = "DESC_ROOT_CANDIDATE";
    if (negated)
        description_str += "_NOT";
    return UserString(description_str);
}

std::string Condition::RootCandidate::Dump() const
{ return DumpIndent() + "RootCandidate\n"; }

bool Condition::RootCandidate::Match(const ScriptingContext& local_context) const {
    if (!local_context.condition_root_candidate)
        return false;
    return local_context.condition_root_candidate == local_context.condition_local_candidate;
}

///////////////////////////////////////////////////////////
// Target                                                //
///////////////////////////////////////////////////////////
std::string Condition::Target::Description(bool negated/* = false*/) const {
    std::string description_str = "DESC_TARGET";
    if (negated)
        description_str += "_NOT";
    return UserString(description_str);
}

std::string Condition::Target::Dump() const
{ return DumpIndent() + "Target\n"; }

bool Condition::Target::Match(const ScriptingContext& local_context) const {
    if (!local_context.effect_target)
        return false;
    return local_context.effect_target == local_context.condition_local_candidate;
}

///////////////////////////////////////////////////////////
// Homeworld                                             //
///////////////////////////////////////////////////////////
Condition::Homeworld::~Homeworld() {
    for (unsigned int i = 0; i < m_names.size(); ++i)
        delete m_names[i];
}

namespace {
    struct HomeworldSimpleMatch {
        HomeworldSimpleMatch(const std::vector<std::string>& names) :
            m_names(names)
        {}

        bool operator()(const UniverseObject* candidate) const {
            if (!candidate)
                return false;

            // is it a planet or a building on a planet?
            const Planet* planet = universe_object_cast<const Planet*>(candidate);
            const ::Building* building = 0;
            if (!planet && (building = universe_object_cast<const ::Building*>(candidate))) {
                planet = GetPlanet(building->PlanetID());
            }
            if (!planet)
                return false;

            int planet_id = planet->ID();
            const SpeciesManager& manager = GetSpeciesManager();

            if (m_names.empty()) {
                // match homeworlds for any species
                for (SpeciesManager::iterator species_it = manager.begin(); species_it != manager.end(); ++species_it) {
                    if (const ::Species* species = species_it->second) {
                        const std::set<int>& homeworld_ids = species->Homeworlds();
                        if (homeworld_ids.find(planet_id) != homeworld_ids.end())
                            return true;
                    }
                }

            } else {
                // match any of the species specified
                for (std::vector<std::string>::const_iterator it = m_names.begin(); it != m_names.end(); ++it) {
                    if (const ::Species* species = GetSpecies(*it)) {
                        const std::set<int>& homeworld_ids = species->Homeworlds();
                        if (homeworld_ids.find(planet_id) != homeworld_ids.end())
                            return true;
                    }
                }
            }

            return false;
        }

        const std::vector<std::string>& m_names;
    };
}

void Condition::Homeworld::Eval(const ScriptingContext& parent_context, ObjectSet& matches, ObjectSet& non_matches,
                                SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = parent_context.condition_root_candidate || RootCandidateInvariant();
    if (simple_eval_safe) {
        // check each valueref for invariance to local candidate
        for (std::vector<const ValueRef::ValueRefBase<std::string>*>::const_iterator it = m_names.begin();
             it != m_names.end(); ++it)
        {
            if (!(*it)->LocalCandidateInvariant()) {
                simple_eval_safe = false;
                break;
            }
        }
    }
    if (simple_eval_safe) {
        // evaluate names once, and use to check all candidate objects
        std::vector<std::string> names;
        // get all names from valuerefs
        for (std::vector<const ValueRef::ValueRefBase<std::string>*>::const_iterator it = m_names.begin();
             it != m_names.end(); ++it)
        {
            names.push_back((*it)->Eval(parent_context));
        }
        EvalImpl(matches, non_matches, search_domain, HomeworldSimpleMatch(names));
    } else {
        // re-evaluate allowed names for each candidate object
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::Homeworld::RootCandidateInvariant() const {
    for (std::vector<const ValueRef::ValueRefBase<std::string>*>::const_iterator it = m_names.begin();
         it != m_names.end(); ++it)
    {
        if (!(*it)->RootCandidateInvariant())
            return false;
    }
    return true;
}

bool Condition::Homeworld::TargetInvariant() const {
    for (std::vector<const ValueRef::ValueRefBase<std::string>*>::const_iterator it = m_names.begin();
         it != m_names.end(); ++it)
    {
        if (!(*it)->TargetInvariant())
            return false;
    }
    return true;
}

std::string Condition::Homeworld::Description(bool negated/* = false*/) const {
    std::string values_str;
    for (unsigned int i = 0; i < m_names.size(); ++i) {
        values_str += ValueRef::ConstantExpr(m_names[i]) ?
                        UserString(boost::lexical_cast<std::string>(m_names[i]->Eval())) :
                        m_names[i]->Description();
        if (2 <= m_names.size() && i < m_names.size() - 2) {
            values_str += ", ";
        } else if (i == m_names.size() - 2) {
            values_str += m_names.size() < 3 ? " " : ", ";
            values_str += UserString("OR");
            values_str += " ";
        }
    }
    std::string description_str = "DESC_HOMEWORLD";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str)) % values_str);
}

std::string Condition::Homeworld::Dump() const {
    std::string retval = DumpIndent() + "HomeWorld";
    if (m_names.size() == 1) {
        retval += " name = " + m_names[0]->Dump();
    } else if (!m_names.empty()) {
        retval += " name = [ ";
        for (unsigned int i = 0; i < m_names.size(); ++i) {
            retval += m_names[i]->Dump() + " ";
        }
        retval += "]";
    }
    return retval;
}

bool Condition::Homeworld::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "Homeworld::Match passed no candidate object";
        return false;
    }

    // is it a planet or a building on a planet?
    const Planet* planet = universe_object_cast<const Planet*>(candidate);
    const ::Building* building = 0;
    if (!planet && (building = universe_object_cast<const ::Building*>(candidate))) {
        planet = GetPlanet(building->PlanetID());
    }
    if (!planet)
        return false;

    int planet_id = planet->ID();
    const SpeciesManager& manager = GetSpeciesManager();

    if (m_names.empty()) {
        // match homeworlds for any species
        for (SpeciesManager::iterator species_it = manager.begin(); species_it != manager.end(); ++species_it) {
            if (const ::Species* species = species_it->second) {
                const std::set<int>& homeworld_ids = species->Homeworlds();
                if (homeworld_ids.find(planet_id) != homeworld_ids.end())   // is this planet the homeworld for this species?
                    return true;
            }
        }

    } else {
        // match any of the species specified
        for (std::vector<const ValueRef::ValueRefBase<std::string>*>::const_iterator it = m_names.begin();
             it != m_names.end(); ++it)
        {
            const std::string& species_name = (*it)->Eval(local_context);
            if (const ::Species* species = manager.GetSpecies(species_name)) {
                const std::set<int>& homeworld_ids = species->Homeworlds();
                if (homeworld_ids.find(planet_id) != homeworld_ids.end())   // is this planet the homeworld for this species?
                    return true;
            }
        }
    }

    return false;
}

///////////////////////////////////////////////////////////
// Capital                                               //
///////////////////////////////////////////////////////////
std::string Condition::Capital::Description(bool negated/* = false*/) const {
    std::string description_str = "DESC_CAPITAL";
    if (negated)
        description_str += "_NOT";
    return UserString(description_str);
}

std::string Condition::Capital::Dump() const
{ return DumpIndent() + "Capital\n"; }

bool Condition::Capital::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "Capital::Match passed no candidate object";
        return false;
    }
    int candidate_id = candidate->ID();

    // check if any empire's capital's ID is that candidate object's id.
    // if it is, the candidate object is a capital.
    const EmpireManager& empires = Empires();
    for (EmpireManager::const_iterator it = empires.begin(); it != empires.end(); ++it)
        if (it->second->CapitalID() == candidate_id)
            return true;
    return false;
}

///////////////////////////////////////////////////////////
// Monster                                               //
///////////////////////////////////////////////////////////
std::string Condition::Monster::Description(bool negated/* = false*/) const {
    std::string description_str = "DESC_MONSTER";
    if (negated)
        description_str += "_NOT";
    return UserString(description_str);
}

std::string Condition::Monster::Dump() const
{ return DumpIndent() + "Monster\n"; }

bool Condition::Monster::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "Monster::Match passed no candidate object";
        return false;
    }

    if (const Ship* ship = universe_object_cast<const Ship*>(candidate))
        if (ship->IsMonster())
            return true;

    return false;
}

///////////////////////////////////////////////////////////
// Armed                                                 //
///////////////////////////////////////////////////////////
std::string Condition::Armed::Description(bool negated/* = false*/) const {
    std::string description_str = "DESC_ARMED";
    if (negated)
        description_str += "_NOT";
    return UserString(description_str);
}

std::string Condition::Armed::Dump() const
{ return DumpIndent() + "Armed\n"; }

bool Condition::Armed::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "Armed::Match passed no candidate object";
        return false;
    }

    if (const Ship* ship = universe_object_cast<const Ship*>(candidate))
        if (ship->IsArmed())
            return true;

    return false;
}

///////////////////////////////////////////////////////////
// Type                                                  //
///////////////////////////////////////////////////////////
Condition::Type::~Type()
{ delete m_type; }

namespace {
    struct TypeSimpleMatch {
        TypeSimpleMatch(UniverseObjectType type) :
            m_type(type)
        {}

        bool operator()(const UniverseObject* candidate) const {
            if (!candidate)
                return false;

            switch (m_type) {
            case OBJ_BUILDING:
                return universe_object_cast<const ::Building*>(candidate) != 0;
                break;
            case OBJ_SHIP:
                return universe_object_cast<const Ship*>(candidate) != 0;
                break;
            case OBJ_FLEET:
                return universe_object_cast<const Fleet*>(candidate) != 0;
                break;
            case OBJ_PLANET:
                return universe_object_cast<const Planet*>(candidate) != 0;
                break;
            case OBJ_POP_CENTER:
                return dynamic_cast<const PopCenter*>(candidate) != 0;
                break;
            case OBJ_PROD_CENTER:
                return dynamic_cast<const ResourceCenter*>(candidate) != 0;
                break;
            case OBJ_SYSTEM:
                return universe_object_cast<const System*>(candidate) != 0;
                break;
            default:
                break;
            }
            return false;
        }

        UniverseObjectType m_type;
    };
}

void Condition::Type::Eval(const ScriptingContext& parent_context, ObjectSet& matches, ObjectSet& non_matches,
                           SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = ValueRef::ConstantExpr(m_type) ||
                            (m_type->LocalCandidateInvariant() &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        UniverseObjectType type = m_type->Eval(parent_context);
        EvalImpl(matches, non_matches, search_domain, TypeSimpleMatch(type));
    } else {
        // re-evaluate allowed turn range for each candidate object
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::Type::RootCandidateInvariant() const
{ return m_type->RootCandidateInvariant(); }

bool Condition::Type::TargetInvariant() const
{ return m_type->TargetInvariant(); }

std::string Condition::Type::Description(bool negated/* = false*/) const {
    std::string value_str = ValueRef::ConstantExpr(m_type) ?
                                UserString(boost::lexical_cast<std::string>(m_type->Eval())) :
                                m_type->Description();
    std::string description_str = "DESC_TYPE";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str)) % value_str);
}

std::string Condition::Type::Dump() const {
    std::string retval = DumpIndent();
    if (dynamic_cast<const ValueRef::Constant<UniverseObjectType>*>(m_type)) {
        switch (m_type->Eval()) {
        case OBJ_BUILDING:    retval += "Building\n"; break;
        case OBJ_SHIP:        retval += "Ship\n"; break;
        case OBJ_FLEET:       retval += "Fleet\n"; break;
        case OBJ_PLANET:      retval += "Planet\n"; break;
        case OBJ_POP_CENTER:  retval += "PopulationCenter\n"; break;
        case OBJ_PROD_CENTER: retval += "ProductionCenter\n"; break;
        case OBJ_SYSTEM:      retval += "System\n"; break;
        default: retval += "?\n"; break;
        }
    } else {
        retval += "ObjectType type = " + m_type->Dump() + "\n";
    }
    return retval;
}

bool Condition::Type::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "Type::Match passed no candidate object";
        return false;
    }

    return TypeSimpleMatch(m_type->Eval(local_context))(candidate);
}

///////////////////////////////////////////////////////////
// Building                                              //
///////////////////////////////////////////////////////////
Condition::Building::~Building() {
    for (unsigned int i = 0; i < m_names.size(); ++i) {
        delete m_names[i];
    }
}

namespace {
    struct BuildingSimpleMatch
    {
        BuildingSimpleMatch(const std::vector<std::string>& names) :
            m_names(names)
        {}

        bool operator()(const UniverseObject* candidate) const {
            if (!candidate)
                return false;

            // is it a building?
            const ::Building* building = universe_object_cast<const ::Building*>(candidate);
            if (!building)
                return false;

            // if no name supplied, match any building
            if (m_names.empty())
                return true;

            // is it one of the specified building types?
            return std::find(m_names.begin(), m_names.end(), building->BuildingTypeName()) != m_names.end();
        }

        const std::vector<std::string>& m_names;
    };
}

void Condition::Building::Eval(const ScriptingContext& parent_context, ObjectSet& matches, ObjectSet& non_matches,
                               SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = parent_context.condition_root_candidate || RootCandidateInvariant();
    if (simple_eval_safe) {
        // check each valueref for invariance to local candidate
        for (std::vector<const ValueRef::ValueRefBase<std::string>*>::const_iterator it = m_names.begin();
             it != m_names.end(); ++it)
        {
            if (!(*it)->LocalCandidateInvariant()) {
                simple_eval_safe = false;
                break;
            }
        }
    }
    if (simple_eval_safe) {
        // evaluate names once, and use to check all candidate objects
        std::vector<std::string> names;
        // get all names from valuerefs
        for (std::vector<const ValueRef::ValueRefBase<std::string>*>::const_iterator it = m_names.begin();
             it != m_names.end(); ++it)
        {
            names.push_back((*it)->Eval(parent_context));
        }
        EvalImpl(matches, non_matches, search_domain, BuildingSimpleMatch(names));
    } else {
        // re-evaluate allowed building types range for each candidate object
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::Building::RootCandidateInvariant() const {
    for (std::vector<const ValueRef::ValueRefBase<std::string>*>::const_iterator it = m_names.begin();
         it != m_names.end(); ++it)
    {
        if (!(*it)->RootCandidateInvariant())
            return false;
    }
    return true;
}

bool Condition::Building::TargetInvariant() const {
    for (std::vector<const ValueRef::ValueRefBase<std::string>*>::const_iterator it = m_names.begin();
         it != m_names.end(); ++it)
    {
        if (!(*it)->TargetInvariant())
            return false;
    }
    return true;
}

std::string Condition::Building::Description(bool negated/* = false*/) const {
    std::string values_str;
    for (unsigned int i = 0; i < m_names.size(); ++i) {
        values_str += ValueRef::ConstantExpr(m_names[i]) ?
                        UserString(boost::lexical_cast<std::string>(m_names[i]->Eval())) :
                        m_names[i]->Description();
        if (2 <= m_names.size() && i < m_names.size() - 2) {
            values_str += ", ";
        } else if (i == m_names.size() - 2) {
            values_str += m_names.size() < 3 ? " " : ", ";
            values_str += UserString("OR");
            values_str += " ";
        }
    }
    std::string description_str = "DESC_BUILDING";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str)) % values_str);
}

std::string Condition::Building::Dump() const {
    std::string retval = DumpIndent() + "Building name = ";
    if (m_names.size() == 1) {
        retval += m_names[0]->Dump() + "\n";
    } else {
        retval += "[ ";
        for (unsigned int i = 0; i < m_names.size(); ++i) {
            retval += m_names[i]->Dump() + " ";
        }
        retval += "]\n";
    }
    return retval;
}

bool Condition::Building::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "Building::Match passed no candidate object";
        return false;
    }

    // is it a building?
    const ::Building* building = universe_object_cast<const ::Building*>(candidate);
    if (building) {
        // match any building type?
        if (m_names.empty())
            return true;

        // match one of the specified building types
        for (std::vector<const ValueRef::ValueRefBase<std::string>*>::const_iterator it = m_names.begin();
             it != m_names.end(); ++it)
        {
            if ((*it)->Eval(local_context) == building->BuildingTypeName())
                return true;
        }
    }

    return false;
}

///////////////////////////////////////////////////////////
// HasSpecial                                            //
///////////////////////////////////////////////////////////
Condition::HasSpecial::~HasSpecial() {
    delete m_since_turn_low;
    delete m_since_turn_high;
}

namespace {
    struct HasSpecialSimpleMatch {
        HasSpecialSimpleMatch(const std::string& name, int low_turn, int high_turn) :
            m_name(name),
            m_low_turn(low_turn),
            m_high_turn(high_turn)
        {}

        bool operator()(const UniverseObject* candidate) const {
            if (!candidate)
                return false;

            if (m_name.empty())
                return !candidate->Specials().empty();

            std::map<std::string, int>::const_iterator it = candidate->Specials().find(m_name);
            if (it == candidate->Specials().end())
                return false;

            int special_since_turn = it->second;

            return m_low_turn <= special_since_turn && special_since_turn <= m_high_turn;
        }

        const std::string& m_name;
        int m_low_turn;
        int m_high_turn;
    };
}

void Condition::HasSpecial::Eval(const ScriptingContext& parent_context, ObjectSet& matches, ObjectSet& non_matches,
                                 SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = ((!m_since_turn_low || m_since_turn_low->LocalCandidateInvariant()) &&
                             (!m_since_turn_high || m_since_turn_high->LocalCandidateInvariant()) &&
                             (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate turn limits once, pass to simple match for all candidates
        const UniverseObject* no_object(0);
        ScriptingContext local_context(parent_context, no_object);
        int low = (m_since_turn_low ? m_since_turn_low->Eval(local_context) : BEFORE_FIRST_TURN);
        int high = (m_since_turn_high ? m_since_turn_high->Eval(local_context) : IMPOSSIBLY_LARGE_TURN);
        EvalImpl(matches, non_matches, search_domain, HasSpecialSimpleMatch(m_name, low, high));
    } else {
        // re-evaluate allowed turn range for each candidate object
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::HasSpecial::RootCandidateInvariant() const
{ return ((!m_since_turn_low || m_since_turn_low->RootCandidateInvariant()) &&
          (!m_since_turn_high || m_since_turn_high->RootCandidateInvariant())); }

bool Condition::HasSpecial::TargetInvariant() const
{ return ((!m_since_turn_low || m_since_turn_low->TargetInvariant()) &&
          (!m_since_turn_high || m_since_turn_high->TargetInvariant())); }

std::string Condition::HasSpecial::Description(bool negated/* = false*/) const {
    if (!m_since_turn_low && !m_since_turn_high) {
        std::string description_str = "DESC_SPECIAL";
        if (negated)
            description_str += "_NOT";
        return str(FlexibleFormat(UserString(description_str))
                   % UserString(m_name));
    }

    // turn ranges have been specified; must indicate in description
    std::string low_str = boost::lexical_cast<std::string>(BEFORE_FIRST_TURN);
    if (m_since_turn_low) {
        low_str = ValueRef::ConstantExpr(m_since_turn_low) ?
                  boost::lexical_cast<std::string>(m_since_turn_low->Eval()) :
                  m_since_turn_low->Description();
    }
    std::string high_str = boost::lexical_cast<std::string>(IMPOSSIBLY_LARGE_TURN);
    if (m_since_turn_high) {
        high_str = ValueRef::ConstantExpr(m_since_turn_high) ?
                   boost::lexical_cast<std::string>(m_since_turn_high->Eval()) :
                   m_since_turn_high->Description();
    }

    std::string description_str = "DESC_SPECIAL_TURN_RANGE";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str))
               % UserString(m_name)
               % low_str
               % high_str);
}

std::string Condition::HasSpecial::Dump() const {
    if (!m_since_turn_low && !m_since_turn_high)
        return DumpIndent() + "HasSpecial name = \"" + m_name + "\"\n";

    std::string low_dump = (m_since_turn_low ? m_since_turn_low->Dump() : boost::lexical_cast<std::string>(BEFORE_FIRST_TURN));
    std::string high_dump = (m_since_turn_high ? m_since_turn_high->Dump() : boost::lexical_cast<std::string>(IMPOSSIBLY_LARGE_TURN));
    return DumpIndent() + "HasSpecialSinceTurn name = \"" + m_name + "\" low = " + low_dump + " high = " + high_dump;
}

bool Condition::HasSpecial::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "HasSpecial::Match passed no candidate object";
        return false;
    }
    int low = (m_since_turn_low ? m_since_turn_low->Eval(local_context) : BEFORE_FIRST_TURN);
    int high = (m_since_turn_high ? m_since_turn_high->Eval(local_context) : IMPOSSIBLY_LARGE_TURN);
    return HasSpecialSimpleMatch(m_name, low, high)(candidate);
}

///////////////////////////////////////////////////////////
// HasTag                                                //
///////////////////////////////////////////////////////////
std::string Condition::HasTag::Description(bool negated/* = false*/) const {
    std::string description_str = "DESC_HAS_TAG";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str)) % UserString(m_name));
}

std::string Condition::HasTag::Dump() const
{ return DumpIndent() + "HasTag name = \"" + m_name + "\"\n"; }

bool Condition::HasTag::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "HasTag::Match passed no candidate object";
        return false;
    }

    return candidate->HasTag(m_name);
}


///////////////////////////////////////////////////////////
// CreatedOnTurn                                         //
///////////////////////////////////////////////////////////
Condition::CreatedOnTurn::~CreatedOnTurn() {
    delete m_low;
    delete m_high;
}

namespace {
    struct CreatedOnTurnSimpleMatch {
        CreatedOnTurnSimpleMatch(int low, int high) :
            m_low(low),
            m_high(high)
        {}

        bool operator()(const UniverseObject* candidate) const {
            if (!candidate)
                return false;
            int turn = candidate->CreationTurn();
            return m_low <= turn && turn <= m_high;
        }

        int m_low;
        int m_high;
    };
}

void Condition::CreatedOnTurn::Eval(const ScriptingContext& parent_context, ObjectSet& matches, ObjectSet& non_matches,
                                    SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = ((!m_low || m_low->LocalCandidateInvariant()) &&
                             (!m_high || m_high->LocalCandidateInvariant()) &&
                             (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        const UniverseObject* no_object(0);
        ScriptingContext local_context(parent_context, no_object);
        int low = (m_low ? m_low->Eval(local_context) : BEFORE_FIRST_TURN);
        int high = (m_high ? m_high->Eval(local_context) : IMPOSSIBLY_LARGE_TURN);
        EvalImpl(matches, non_matches, search_domain, CreatedOnTurnSimpleMatch(low, high));
    } else {
        // re-evaluate allowed turn range for each candidate object
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::CreatedOnTurn::RootCandidateInvariant() const
{ return ((!m_low || m_low->RootCandidateInvariant()) && (!m_high || m_high->RootCandidateInvariant())); }

bool Condition::CreatedOnTurn::TargetInvariant() const
{ return ((!m_low || m_low->TargetInvariant()) && (!m_high || m_high->TargetInvariant())); }

std::string Condition::CreatedOnTurn::Description(bool negated/* = false*/) const {
    std::string low_str = (m_low ? (ValueRef::ConstantExpr(m_low) ?
                                    boost::lexical_cast<std::string>(m_low->Eval()) :
                                    m_low->Description())
                                 : boost::lexical_cast<std::string>(BEFORE_FIRST_TURN));
    std::string high_str = (m_high ? (ValueRef::ConstantExpr(m_high) ?
                                      boost::lexical_cast<std::string>(m_high->Eval()) :
                                      m_high->Description())
                                   : boost::lexical_cast<std::string>(IMPOSSIBLY_LARGE_TURN));
    std::string description_str = "DESC_CREATED_ON_TURN";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str))
               % low_str
               % high_str);
}

std::string Condition::CreatedOnTurn::Dump() const {
    std::string retval = DumpIndent() + "CreatedOnTurn";
    if (m_low)
        retval += " low = " + m_low->Dump();
    if (m_high)
        retval += " high = " + m_high->Dump();
    retval += "\n";
    return retval;
}

bool Condition::CreatedOnTurn::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "CreatedOnTurn::Match passed no candidate object";
        return false;
    }
    double low = (m_low ? std::max(0, m_low->Eval(local_context)) : BEFORE_FIRST_TURN);
    double high = (m_high ? std::min(m_high->Eval(local_context), IMPOSSIBLY_LARGE_TURN) : IMPOSSIBLY_LARGE_TURN);
    return CreatedOnTurnSimpleMatch(low, high)(candidate);
}

///////////////////////////////////////////////////////////
// Contains                                              //
///////////////////////////////////////////////////////////
Condition::Contains::~Contains()
{ delete m_condition; }

namespace {
    struct ContainsSimpleMatch {
        ContainsSimpleMatch(const Condition::ObjectSet& subcondition_matches) :
            m_subcondition_matches(subcondition_matches)
        {}

        bool operator()(const UniverseObject* candidate) const {
            if (!candidate)
                return false;

            bool match = false;
            for (Condition::ObjectSet::const_iterator it = m_subcondition_matches.begin(); it != m_subcondition_matches.end(); ++it) {
                if (candidate->Contains((*it)->ID())) {
                    match = true;
                    break;
                }
            }
            return match;
        }

        const Condition::ObjectSet& m_subcondition_matches;
    };
}

void Condition::Contains::Eval(const ScriptingContext& parent_context, ObjectSet& matches, ObjectSet& non_matches,
                               SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = parent_context.condition_root_candidate || RootCandidateInvariant();
    if (simple_eval_safe) {
        // evaluate contained objects once and check for all candidates
        ObjectMap& objects = GetUniverse().Objects();

        const UniverseObject* no_object(0);
        ScriptingContext local_context(parent_context, no_object);

        // get objects to be considering for matching against subcondition
        ObjectSet subcondition_non_matches;
        subcondition_non_matches.reserve(RESERVE_SET_SIZE);
        for (ObjectMap::iterator it = objects.begin(); it != objects.end(); ++it)
            subcondition_non_matches.push_back(it->second);
        ObjectSet subcondition_matches;
        subcondition_non_matches.reserve(RESERVE_SET_SIZE);

        m_condition->Eval(local_context, subcondition_matches, subcondition_non_matches);

        EvalImpl(matches, non_matches, search_domain, ContainsSimpleMatch(subcondition_matches));
    } else {
        // re-evaluate contained objects for each candidate object
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::Contains::RootCandidateInvariant() const
{ return m_condition->RootCandidateInvariant(); }

bool Condition::Contains::TargetInvariant() const
{ return m_condition->TargetInvariant(); }

std::string Condition::Contains::Description(bool negated/* = false*/) const {
    std::string description_str = "DESC_CONTAINS";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str)) % m_condition->Description());
}

std::string Condition::Contains::Dump() const {
    std::string retval = DumpIndent() + "Contains condition =\n";
    ++g_indent;
    retval += m_condition->Dump();
    --g_indent;
    return retval;
}

bool Condition::Contains::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "Contains::Match passed no candidate object";
        return false;
    }

    ObjectMap& objects = GetUniverse().Objects();

    // get objects to be considering for matching against subcondition
    ObjectSet subcondition_non_matches;
    subcondition_non_matches.reserve(RESERVE_SET_SIZE);
    for (ObjectMap::iterator it = objects.begin(); it != objects.end(); ++it)
        subcondition_non_matches.push_back(it->second);
    ObjectSet subcondition_matches;
    subcondition_matches.reserve(RESERVE_SET_SIZE);

    m_condition->Eval(local_context, subcondition_matches, subcondition_non_matches);

    // does candidate object contain any subcondition matches?
    for (ObjectSet::iterator subcon_it = subcondition_matches.begin(); subcon_it != subcondition_matches.end(); ++subcon_it)
        if (candidate->Contains((*subcon_it)->ID()))
            return true;

    return false;
}

///////////////////////////////////////////////////////////
// ContainedBy                                           //
///////////////////////////////////////////////////////////
Condition::ContainedBy::~ContainedBy()
{ delete m_condition; }

namespace {
    struct ContainedBySimpleMatch {
        ContainedBySimpleMatch(const Condition::ObjectSet& subcondition_matches) :
            m_subcondition_matches(subcondition_matches)
    {}

        bool operator()(const UniverseObject* candidate) const {
            if (!candidate)
                return false;

            bool match = false;
            for (Condition::ObjectSet::const_iterator it = m_subcondition_matches.begin(); it != m_subcondition_matches.end(); ++it) {
                if ((*it)->Contains(candidate->ID())) {
                    match = true;
                    break;
                }
            }
            return match;
        }

        const Condition::ObjectSet& m_subcondition_matches;
    };
}

void Condition::ContainedBy::Eval(const ScriptingContext& parent_context, ObjectSet& matches, ObjectSet& non_matches,
                                  SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = parent_context.condition_root_candidate || RootCandidateInvariant();
    if (simple_eval_safe) {
        // evaluate contained objects once and check for all candidates
        ObjectMap& objects = GetUniverse().Objects();

        const UniverseObject* no_object(0);
        ScriptingContext local_context(parent_context, no_object);

        // get objects to be considering for matching against subcondition
        ObjectSet subcondition_non_matches;
        subcondition_non_matches.reserve(RESERVE_SET_SIZE);
        for (ObjectMap::iterator it = objects.begin(); it != objects.end(); ++it)
            subcondition_non_matches.push_back(it->second);
        ObjectSet subcondition_matches;
        subcondition_matches.reserve(RESERVE_SET_SIZE);

        m_condition->Eval(local_context, subcondition_matches, subcondition_non_matches);

        EvalImpl(matches, non_matches, search_domain, ContainedBySimpleMatch(subcondition_matches));
    } else {
        // re-evaluate contained objects for each candidate object
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::ContainedBy::RootCandidateInvariant() const
{ return m_condition->RootCandidateInvariant(); }

bool Condition::ContainedBy::TargetInvariant() const
{ return m_condition->TargetInvariant(); }

std::string Condition::ContainedBy::Description(bool negated/* = false*/) const {
    std::string description_str = "DESC_CONTAINED_BY";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str)) % m_condition->Description());
}

std::string Condition::ContainedBy::Dump() const {
    std::string retval = DumpIndent() + "ContainedBy condition =\n";
    ++g_indent;
        retval += m_condition->Dump();
    --g_indent;
    return retval;
}

bool Condition::ContainedBy::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "ContainedBy::Match passed no candidate object";
        return false;
    }

    ObjectMap& objects = GetUniverse().Objects();

    // get objects to be considering for matching against subcondition
    ObjectSet subcondition_non_matches;
    subcondition_non_matches.reserve(RESERVE_SET_SIZE);
    for (ObjectMap::iterator it = objects.begin(); it != objects.end(); ++it)
        subcondition_non_matches.push_back(it->second);
    ObjectSet subcondition_matches;
    subcondition_matches.reserve(RESERVE_SET_SIZE);

    m_condition->Eval(local_context, subcondition_matches, subcondition_non_matches);

    // is candidate object contained by any subcondition matches?
    for (ObjectSet::iterator subcon_it = subcondition_matches.begin(); subcon_it != subcondition_matches.end(); ++subcon_it)
        if ((*subcon_it)->Contains(candidate->ID()))
            return true;

    return false;
}

///////////////////////////////////////////////////////////
// InSystem                                              //
///////////////////////////////////////////////////////////
Condition::InSystem::~InSystem()
{ delete m_system_id; }

namespace {
    struct InSystemSimpleMatch {
        InSystemSimpleMatch(int system_id) :
            m_system_id(system_id)
        {}

        bool operator()(const UniverseObject* candidate) const {
            if (!candidate)
                return false;
            if (m_system_id == INVALID_OBJECT_ID)
                return candidate->SystemID() != INVALID_OBJECT_ID;  // match objects in any system
            else
                return candidate->SystemID() == m_system_id;                        // match objects in specified system
        }

        int m_system_id;
    };
}

void Condition::InSystem::Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                               ObjectSet& non_matches, SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = !m_system_id || ValueRef::ConstantExpr(m_system_id) ||
                            (m_system_id->LocalCandidateInvariant() &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate empire id once, and use to check all candidate objects
        const UniverseObject* no_object(0);
        int system_id = (m_system_id ? m_system_id->Eval(ScriptingContext(parent_context, no_object)) : INVALID_OBJECT_ID);
        EvalImpl(matches, non_matches, search_domain, InSystemSimpleMatch(system_id));
    } else {
        // re-evaluate empire id for each candidate object
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::InSystem::RootCandidateInvariant() const
{ return !m_system_id || m_system_id->RootCandidateInvariant(); }

bool Condition::InSystem::TargetInvariant() const
{ return !m_system_id || m_system_id->TargetInvariant(); }

std::string Condition::InSystem::Description(bool negated/* = false*/) const {
    std::string system_str;
    int system_id = INVALID_OBJECT_ID;
    if (m_system_id && ValueRef::ConstantExpr(m_system_id))
        system_id = m_system_id->Eval();
    if (const System* system = GetSystem(system_id))
        system_str = system->Name();
    else if (m_system_id)
        system_str = m_system_id->Description();

    std::string description_str = (!system_str.empty() ? "DESC_IN_SYSTEM" : "DESC_IN_SYSTEM_SIMPLE");
    if (negated)
        description_str += "_NOT";

    return str(FlexibleFormat(UserString(description_str))
               % system_str);
}

std::string Condition::InSystem::Dump() const {
    std::string retval = DumpIndent() + "InSystem";
    if (m_system_id)
        retval += " id = " + m_system_id->Dump();
    retval += "\n";
    return retval;
}

bool Condition::InSystem::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "InSystem::Match passed no candidate object";
        return false;
    }
    int system_id = (m_system_id ? m_system_id->Eval(local_context) : INVALID_OBJECT_ID);
    return InSystemSimpleMatch(system_id)(candidate);
}

///////////////////////////////////////////////////////////
// ObjectID                                              //
///////////////////////////////////////////////////////////
Condition::ObjectID::~ObjectID()
{ delete m_object_id; }

namespace {
    struct ObjectIDSimpleMatch {
        ObjectIDSimpleMatch(int object_id) :
            m_object_id(object_id)
        {}

        bool operator()(const UniverseObject* candidate) const {
            return candidate &&
                m_object_id != INVALID_OBJECT_ID &&
                candidate->ID() == m_object_id;
        }

        int m_object_id;
    };
}

void Condition::ObjectID::Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                               ObjectSet& non_matches, SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = !m_object_id || ValueRef::ConstantExpr(m_object_id) ||
                            (m_object_id->LocalCandidateInvariant() &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate empire id once, and use to check all candidate objects
        const UniverseObject* no_object(0);
        int object_id = (m_object_id ? m_object_id->Eval(ScriptingContext(parent_context, no_object)) : INVALID_OBJECT_ID);
        EvalImpl(matches, non_matches, search_domain, ObjectIDSimpleMatch(object_id));
    } else {
        // re-evaluate empire id for each candidate object
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::ObjectID::RootCandidateInvariant() const
{ return !m_object_id || m_object_id->RootCandidateInvariant(); }

bool Condition::ObjectID::TargetInvariant() const
{ return !m_object_id || m_object_id->TargetInvariant(); }

std::string Condition::ObjectID::Description(bool negated/* = false*/) const {
    std::string object_str;
    int object_id = INVALID_OBJECT_ID;
    if (m_object_id && ValueRef::ConstantExpr(m_object_id))
        object_id = m_object_id->Eval();
    if (const System* system = GetSystem(object_id))
        object_str = system->Name();
    else if (m_object_id)
        object_str = m_object_id->Description();
    else
        object_str = UserString("ERROR");   // should always have a valid ID for this condition

    std::string description_str = "DESC_OBJECT_ID";
    if (negated)
        description_str += "_NOT";

    return str(FlexibleFormat(UserString(description_str))
               % object_str);
}

std::string Condition::ObjectID::Dump() const
{ return DumpIndent() + "Object id = " + m_object_id->Dump(); }

bool Condition::ObjectID::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "ObjectID::Match passed no candidate object";
        return false;
    }

    return ObjectIDSimpleMatch(m_object_id->Eval(local_context))(candidate);
}

///////////////////////////////////////////////////////////
// PlanetType                                            //
///////////////////////////////////////////////////////////
Condition::PlanetType::~PlanetType() {
    for (unsigned int i = 0; i < m_types.size(); ++i) {
        delete m_types[i];
    }
}

namespace {
    struct PlanetTypeSimpleMatch {
        PlanetTypeSimpleMatch(const std::vector< ::PlanetType>& types) :
            m_types(types)
        {}

        bool operator()(const UniverseObject* candidate) const {
            if (!candidate)
                return false;

            // is it a planet or on a planet?
            const Planet* planet = universe_object_cast<const Planet*>(candidate);
            const ::Building* building = 0;
            if (!planet && (building = universe_object_cast<const ::Building*>(candidate))) {
                planet = GetPlanet(building->PlanetID());
            }
            if (planet) {
                // is it one of the specified building types?
                return std::find(m_types.begin(), m_types.end(), planet->Type()) != m_types.end();
            }

            return false;
        }

        const std::vector< ::PlanetType>& m_types;
    };
}

void Condition::PlanetType::Eval(const ScriptingContext& parent_context, ObjectSet& matches, ObjectSet& non_matches,
                                 SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = parent_context.condition_root_candidate || RootCandidateInvariant();
    if (simple_eval_safe) {
        // check each valueref for invariance to local candidate
        for (std::vector<const ValueRef::ValueRefBase< ::PlanetType>*>::const_iterator it = m_types.begin();
            it != m_types.end(); ++it)
        {
            if (!(*it)->LocalCandidateInvariant()) {
                simple_eval_safe = false;
                break;
            }
        }
    }
    if (simple_eval_safe) {
        // evaluate types once, and use to check all candidate objects
        std::vector< ::PlanetType> types;
        // get all types from valuerefs
        for (std::vector<const ValueRef::ValueRefBase< ::PlanetType>*>::const_iterator it = m_types.begin();
             it != m_types.end(); ++it)
        {
            types.push_back((*it)->Eval(parent_context));
        }
        EvalImpl(matches, non_matches, search_domain, PlanetTypeSimpleMatch(types));
    } else {
        // re-evaluate contained objects for each candidate object
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::PlanetType::RootCandidateInvariant() const {
    for (std::vector<const ValueRef::ValueRefBase< ::PlanetType>*>::const_iterator it = m_types.begin();
         it != m_types.end(); ++it)
    {
        if (!(*it)->RootCandidateInvariant())
            return false;
    }
    return true;
}

bool Condition::PlanetType::TargetInvariant() const {
    for (std::vector<const ValueRef::ValueRefBase< ::PlanetType>*>::const_iterator it = m_types.begin();
         it != m_types.end(); ++it)
    {
        if (!(*it)->TargetInvariant())
            return false;
    }
    return true;
}

std::string Condition::PlanetType::Description(bool negated/* = false*/) const {
    std::string values_str;
    for (unsigned int i = 0; i < m_types.size(); ++i) {
        values_str += ValueRef::ConstantExpr(m_types[i]) ?
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
    std::string description_str = "DESC_PLANET_TYPE";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str)) % values_str);
}

std::string Condition::PlanetType::Dump() const {
    std::string retval = DumpIndent() + "Planet type = ";
    if (m_types.size() == 1) {
        retval += m_types[0]->Dump() + "\n";
    } else {
        retval += "[ ";
        for (unsigned int i = 0; i < m_types.size(); ++i) {
            retval += m_types[i]->Dump() + " ";
        }
        retval += "]\n";
    }
    return retval;
}

bool Condition::PlanetType::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "PlanetType::Match passed no candidate object";
        return false;
    }

    const Planet* planet = universe_object_cast<const Planet*>(candidate);
    const ::Building* building = 0;
    if (!planet && (building = universe_object_cast<const ::Building*>(candidate))) {
        planet = GetPlanet(building->PlanetID());
    }
    if (planet) {
        for (std::vector<const ValueRef::ValueRefBase< ::PlanetType>*>::const_iterator it = m_types.begin();
             it != m_types.end(); ++it)
        {
            if ((*it)->Eval(ScriptingContext(local_context)) == planet->Type())
                return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////
// PlanetSize                                            //
///////////////////////////////////////////////////////////
Condition::PlanetSize::~PlanetSize() {
    for (unsigned int i = 0; i < m_sizes.size(); ++i) {
        delete m_sizes[i];
    }
}

namespace {
    struct PlanetSizeSimpleMatch {
        PlanetSizeSimpleMatch(const std::vector< ::PlanetSize>& sizes) :
            m_sizes(sizes)
        {}

        bool operator()(const UniverseObject* candidate) const {
            if (!candidate)
                return false;

            // is it a planet or on a planet?
            const Planet* planet = universe_object_cast<const Planet*>(candidate);
            const ::Building* building = 0;
            if (!planet && (building = universe_object_cast<const ::Building*>(candidate))) {
                planet = GetPlanet(building->PlanetID());
            }
            if (planet) {
                // is it one of the specified building types?
                for (std::vector< ::PlanetSize>::const_iterator it = m_sizes.begin();
                        it != m_sizes.end(); ++it)
                {
                    if (planet->Size() == *it)
                        return true;
                }
            }

            return false;
        }

        const std::vector< ::PlanetSize>& m_sizes;
    };
}

void Condition::PlanetSize::Eval(const ScriptingContext& parent_context, ObjectSet& matches, ObjectSet& non_matches,
                                 SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = parent_context.condition_root_candidate || RootCandidateInvariant();
    if (simple_eval_safe) {
        // check each valueref for invariance to local candidate
        for (std::vector<const ValueRef::ValueRefBase< ::PlanetSize>*>::const_iterator it = m_sizes.begin();
            it != m_sizes.end(); ++it)
        {
            if (!(*it)->LocalCandidateInvariant()) {
                simple_eval_safe = false;
                break;
            }
        }
    }
    if (simple_eval_safe) {
        // evaluate types once, and use to check all candidate objects
        std::vector< ::PlanetSize> sizes;
        // get all types from valuerefs
        for (std::vector<const ValueRef::ValueRefBase< ::PlanetSize>*>::const_iterator it = m_sizes.begin();
             it != m_sizes.end(); ++it)
        {
            sizes.push_back((*it)->Eval(parent_context));
        }
        EvalImpl(matches, non_matches, search_domain, PlanetSizeSimpleMatch(sizes));
    } else {
        // re-evaluate contained objects for each candidate object
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::PlanetSize::RootCandidateInvariant() const {
    for (std::vector<const ValueRef::ValueRefBase< ::PlanetSize>*>::const_iterator it = m_sizes.begin();
         it != m_sizes.end(); ++it)
    {
        if (!(*it)->RootCandidateInvariant())
            return false;
    }
    return true;
}

bool Condition::PlanetSize::TargetInvariant() const {
    for (std::vector<const ValueRef::ValueRefBase< ::PlanetSize>*>::const_iterator it = m_sizes.begin();
         it != m_sizes.end(); ++it)
    {
        if (!(*it)->TargetInvariant())
            return false;
    }
    return true;
}

std::string Condition::PlanetSize::Description(bool negated/* = false*/) const {
    std::string values_str;
    for (unsigned int i = 0; i < m_sizes.size(); ++i) {
        values_str += ValueRef::ConstantExpr(m_sizes[i]) ?
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
    std::string description_str = "DESC_PLANET_SIZE";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str)) % values_str);
}

std::string Condition::PlanetSize::Dump() const {
    std::string retval = DumpIndent() + "Planet size = ";
    if (m_sizes.size() == 1) {
        retval += m_sizes[0]->Dump() + "\n";
    } else {
        retval += "[ ";
        for (unsigned int i = 0; i < m_sizes.size(); ++i) {
            retval += m_sizes[i]->Dump() + " ";
        }
        retval += "]\n";
    }
    return retval;
}

bool Condition::PlanetSize::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "PlanetSize::Match passed no candidate object";
        return false;
    }

    const Planet* planet = universe_object_cast<const Planet*>(candidate);
    const ::Building* building = 0;
    if (!planet && (building = universe_object_cast<const ::Building*>(candidate))) {
        planet = GetPlanet(building->PlanetID());
    }
    if (planet) {
        for (unsigned int i = 0; i < m_sizes.size(); ++i) {
            if (m_sizes[i]->Eval(local_context) == planet->Size())
                return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////
// PlanetEnvironment                                     //
///////////////////////////////////////////////////////////
Condition::PlanetEnvironment::~PlanetEnvironment() {
    for (unsigned int i = 0; i < m_environments.size(); ++i) {
        delete m_environments[i];
    }
}

namespace {
    struct PlanetEnvironmentSimpleMatch {
        PlanetEnvironmentSimpleMatch(const std::vector< ::PlanetEnvironment>& environments) :
            m_environments(environments)
        {}

        bool operator()(const UniverseObject* candidate) const {
            if (!candidate)
                return false;

            // is it a planet or on a planet?
            const Planet* planet = universe_object_cast<const Planet*>(candidate);
            const ::Building* building = 0;
            if (!planet && (building = universe_object_cast<const ::Building*>(candidate))) {
                planet = GetPlanet(building->PlanetID());
            }
            if (planet) {
                // is it one of the specified building types?
                for (std::vector< ::PlanetEnvironment>::const_iterator it = m_environments.begin();
                        it != m_environments.end(); ++it)
                {
                    if (planet->EnvironmentForSpecies() == *it)
                        return true;
                }
            }

            return false;
        }

        const std::vector< ::PlanetEnvironment>& m_environments;
    };
}

void Condition::PlanetEnvironment::Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                                        ObjectSet& non_matches, SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = parent_context.condition_root_candidate || RootCandidateInvariant();
    if (simple_eval_safe) {
        // check each valueref for invariance to local candidate
        for (std::vector<const ValueRef::ValueRefBase< ::PlanetEnvironment>*>::const_iterator it = m_environments.begin();
             it != m_environments.end(); ++it)
        {
            if (!(*it)->LocalCandidateInvariant()) {
                simple_eval_safe = false;
                break;
            }
        }
    }
    if (simple_eval_safe) {
        // evaluate types once, and use to check all candidate objects
        std::vector< ::PlanetEnvironment> environments;
        // get all types from valuerefs
        for (std::vector<const ValueRef::ValueRefBase< ::PlanetEnvironment>*>::const_iterator it = m_environments.begin();
             it != m_environments.end(); ++it)
        {
            environments.push_back((*it)->Eval(parent_context));
        }
        EvalImpl(matches, non_matches, search_domain, PlanetEnvironmentSimpleMatch(environments));
    } else {
        // re-evaluate contained objects for each candidate object
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::PlanetEnvironment::RootCandidateInvariant() const {
    for (std::vector<const ValueRef::ValueRefBase< ::PlanetEnvironment>*>::const_iterator it = m_environments.begin();
         it != m_environments.end(); ++it)
    {
        if (!(*it)->RootCandidateInvariant())
            return false;
    }
    return true;
}

bool Condition::PlanetEnvironment::TargetInvariant() const {
    for (std::vector<const ValueRef::ValueRefBase< ::PlanetEnvironment>*>::const_iterator it = m_environments.begin();
         it != m_environments.end(); ++it)
    {
        if (!(*it)->TargetInvariant())
            return false;
    }
    return true;
}

std::string Condition::PlanetEnvironment::Description(bool negated/* = false*/) const {
    std::string values_str;
    for (unsigned int i = 0; i < m_environments.size(); ++i) {
        values_str += ValueRef::ConstantExpr(m_environments[i]) ?
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
    std::string description_str = "DESC_PLANET_ENVIRONMENT";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str)) % values_str);
}

std::string Condition::PlanetEnvironment::Dump() const {
    std::string retval = DumpIndent() + "Planet environment = ";
    if (m_environments.size() == 1) {
        retval += m_environments[0]->Dump() + "\n";
    } else {
        retval += "[ ";
        for (unsigned int i = 0; i < m_environments.size(); ++i) {
            retval += m_environments[i]->Dump() + " ";
        }
        retval += "]\n";
    }
    return retval;
}

bool Condition::PlanetEnvironment::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "PlanetEnvironment::Match passed no candidate object";
        return false;
    }

    const Planet* planet = universe_object_cast<const Planet*>(candidate);
    const ::Building* building = 0;
    if (!planet && (building = universe_object_cast<const ::Building*>(candidate))) {
        planet = GetPlanet(building->PlanetID());
    }
    if (planet) {
        ::PlanetEnvironment env_for_planets_species = planet->EnvironmentForSpecies();
        for (unsigned int i = 0; i < m_environments.size(); ++i) {
            if (m_environments[i]->Eval(local_context) == env_for_planets_species)
                return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////
// Species                                              //
///////////////////////////////////////////////////////////
Condition::Species::~Species() {
    for (unsigned int i = 0; i < m_names.size(); ++i) {
        delete m_names[i];
    }
}

namespace {
    struct SpeciesSimpleMatch {
        SpeciesSimpleMatch(const std::vector<std::string>& names) :
            m_names(names)
        {}

        bool operator()(const UniverseObject* candidate) const {
            if (!candidate)
                return false;

            // is it a population centre?
            if (const ::PopCenter* pop = dynamic_cast<const ::PopCenter*>(candidate)) {
                const std::string& species_name = pop->SpeciesName();
                // if the popcenter has a species and that species is one of those specified...
                return !species_name.empty() && (m_names.empty() || (std::find(m_names.begin(), m_names.end(), species_name) != m_names.end()));
            }
            // is it a ship?
            if (const ::Ship* ship = universe_object_cast<const ::Ship*>(candidate)) {
                // if the ship has a species and that species is one of those specified...
                const std::string& species_name = ship->SpeciesName();
                return !species_name.empty() && (m_names.empty() || (std::find(m_names.begin(), m_names.end(), species_name) != m_names.end()));
            }
            // is it a building on a planet?
            if (const ::Building* building = universe_object_cast<const ::Building*>(candidate)) {
                const ::Planet* planet = GetPlanet(building->PlanetID());
                const std::string& species_name = planet->SpeciesName();
                // if the planet (which IS a popcenter) has a species and that species is one of those specified...
                return !species_name.empty() && (m_names.empty() || (std::find(m_names.begin(), m_names.end(), species_name) != m_names.end()));
            }

            return false;
        }

        const std::vector<std::string>& m_names;
    };
}

void Condition::Species::Eval(const ScriptingContext& parent_context, ObjectSet& matches, ObjectSet& non_matches,
                              SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = parent_context.condition_root_candidate || RootCandidateInvariant();
    if (simple_eval_safe) {
        // check each valueref for invariance to local candidate
        for (std::vector<const ValueRef::ValueRefBase<std::string>*>::const_iterator it = m_names.begin();
             it != m_names.end(); ++it)
        {
            if (!(*it)->LocalCandidateInvariant()) {
                simple_eval_safe = false;
                break;
            }
        }
    }
    if (simple_eval_safe) {
        // evaluate names once, and use to check all candidate objects
        std::vector<std::string> names;
        // get all names from valuerefs
        for (std::vector<const ValueRef::ValueRefBase<std::string>*>::const_iterator it = m_names.begin();
             it != m_names.end(); ++it)
        {
            names.push_back((*it)->Eval(parent_context));
        }
        EvalImpl(matches, non_matches, search_domain, SpeciesSimpleMatch(names));
    } else {
        // re-evaluate allowed building types range for each candidate object
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::Species::RootCandidateInvariant() const {
    for (std::vector<const ValueRef::ValueRefBase<std::string>*>::const_iterator it = m_names.begin();
         it != m_names.end(); ++it)
    {
        if (!(*it)->RootCandidateInvariant())
            return false;
    }
    return true;
}

bool Condition::Species::TargetInvariant() const {
    for (std::vector<const ValueRef::ValueRefBase<std::string>*>::const_iterator it = m_names.begin();
         it != m_names.end(); ++it)
    {
        if (!(*it)->TargetInvariant())
            return false;
    }
    return true;
}

std::string Condition::Species::Description(bool negated/* = false*/) const {
    std::string values_str;
    for (unsigned int i = 0; i < m_names.size(); ++i) {
        values_str += ValueRef::ConstantExpr(m_names[i]) ?
                        UserString(boost::lexical_cast<std::string>(m_names[i]->Eval())) :
                        m_names[i]->Description();
        if (2 <= m_names.size() && i < m_names.size() - 2) {
            values_str += ", ";
        } else if (i == m_names.size() - 2) {
            values_str += m_names.size() < 3 ? " " : ", ";
            values_str += UserString("OR");
            values_str += " ";
        }
    }
    std::string description_str = "DESC_SPECIES";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str)) % values_str);
}

std::string Condition::Species::Dump() const {
    std::string retval = DumpIndent() + "Species";
    if (m_names.empty()) {
        // do nothing else
    } else if (m_names.size() == 1) {
        retval += " name = " + m_names[0]->Dump() + "\n";
    } else {
        retval += " name = [ ";
        for (unsigned int i = 0; i < m_names.size(); ++i) {
            retval += m_names[i]->Dump() + " ";
        }
        retval += "]\n";
    }
    return retval;
}

bool Condition::Species::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "Species::Match passed no candidate object";
        return false;
    }

    // is it a planet or a building on a planet?
    const Planet* planet = universe_object_cast<const Planet*>(candidate);
    const ::Building* building = 0;
    if (!planet && (building = universe_object_cast<const ::Building*>(candidate))) {
        planet = GetPlanet(building->PlanetID());
    }
    if (planet) {
        if (m_names.empty()) {
            return !planet->SpeciesName().empty();  // match any species name
        } else {
            // match only specified species names
            for (unsigned int i = 0; i < m_names.size(); ++i)
                if (m_names[i]->Eval(local_context) == planet->SpeciesName())
                    return true;
        }
    }
    // is it a ship?
    const Ship* ship = universe_object_cast<const Ship*>(candidate);
    if (ship) {
        if (m_names.empty()) {
            return !ship->SpeciesName().empty();    // match any species name
        } else {
            // match only specified species names
            for (unsigned int i = 0; i < m_names.size(); ++i)
                if (m_names[i]->Eval(local_context) == ship->SpeciesName())
                    return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////
// FocusType                                             //
///////////////////////////////////////////////////////////
Condition::FocusType::~FocusType() {
    for (unsigned int i = 0; i < m_names.size(); ++i) {
        delete m_names[i];
    }
}

namespace {
    struct FocusTypeSimpleMatch {
        FocusTypeSimpleMatch(const std::vector<std::string>& names) :
            m_names(names)
        {}

        bool operator()(const UniverseObject* candidate) const {
            if (!candidate)
                return false;

            // is it a ResourceCenter or a Building on a Planet (that is a ResourceCenter)
            const ResourceCenter* res_center = dynamic_cast<const ResourceCenter*>(candidate);
            const ::Building* building = 0;
            if (!res_center && (building = universe_object_cast<const ::Building*>(candidate))) {
                if (const Planet* planet = GetPlanet(building->PlanetID()))
                    res_center = dynamic_cast<const ResourceCenter*>(planet);
            }
            if (res_center) {
                return !res_center->Focus().empty() && (std::find(m_names.begin(), m_names.end(), res_center->Focus()) != m_names.end());

            }

            return false;
        }

        const std::vector<std::string>& m_names;
    };
}

void Condition::FocusType::Eval(const ScriptingContext& parent_context, ObjectSet& matches, ObjectSet& non_matches,
                                SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = parent_context.condition_root_candidate || RootCandidateInvariant();
    if (simple_eval_safe) {
        // check each valueref for invariance to local candidate
        for (std::vector<const ValueRef::ValueRefBase<std::string>*>::const_iterator it = m_names.begin();
             it != m_names.end(); ++it)
        {
            if (!(*it)->LocalCandidateInvariant()) {
                simple_eval_safe = false;
                break;
            }
        }
    }
    if (simple_eval_safe) {
        // evaluate names once, and use to check all candidate objects
        std::vector<std::string> names;
        // get all names from valuerefs
        for (std::vector<const ValueRef::ValueRefBase<std::string>*>::const_iterator it = m_names.begin();
             it != m_names.end(); ++it)
        {
            names.push_back((*it)->Eval(parent_context));
        }
        EvalImpl(matches, non_matches, search_domain, FocusTypeSimpleMatch(names));
    } else {
        // re-evaluate allowed building types range for each candidate object
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::FocusType::RootCandidateInvariant() const {
    for (std::vector<const ValueRef::ValueRefBase<std::string>*>::const_iterator it = m_names.begin();
         it != m_names.end(); ++it)
    {
        if (!(*it)->RootCandidateInvariant())
            return false;
    }
    return true;
}

bool Condition::FocusType::TargetInvariant() const {
    for (std::vector<const ValueRef::ValueRefBase<std::string>*>::const_iterator it = m_names.begin();
         it != m_names.end(); ++it)
    {
        if (!(*it)->TargetInvariant())
            return false;
    }
    return true;
}

std::string Condition::FocusType::Description(bool negated/* = false*/) const {
    std::string values_str;
    for (unsigned int i = 0; i < m_names.size(); ++i) {
        values_str += ValueRef::ConstantExpr(m_names[i]) ?
            UserString(boost::lexical_cast<std::string>(m_names[i]->Eval())) :
            m_names[i]->Description();
        if (2 <= m_names.size() && i < m_names.size() - 2) {
            values_str += ", ";
        } else if (i == m_names.size() - 2) {
            values_str += m_names.size() < 3 ? " " : ", ";
            values_str += UserString("OR");
            values_str += " ";
        }
    }
    std::string description_str = "DESC_FOCUS_TYPE";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str)) % values_str);
}

std::string Condition::FocusType::Dump() const {
    std::string retval = DumpIndent() + "Focus name = ";
    if (m_names.size() == 1) {
        retval += m_names[0]->Dump() + "\n";
    } else {
        retval += "[ ";
        for (unsigned int i = 0; i < m_names.size(); ++i) {
            retval += m_names[i]->Dump() + " ";
        }
        retval += "]\n";
    }
    return retval;
}

bool Condition::FocusType::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "FocusType::Match passed no candidate object";
        return false;
    }

    // is it a ResourceCenter or a Building on a Planet (that is a ResourceCenter)
    const ResourceCenter* res_center = dynamic_cast<const ResourceCenter*>(candidate);
    const ::Building* building = 0;
    if (!res_center && (building = universe_object_cast<const ::Building*>(candidate))) {
        if (const Planet* planet = GetPlanet(building->PlanetID()))
            res_center = dynamic_cast<const ResourceCenter*>(planet);
    }
    if (res_center) {
        for (unsigned int i = 0; i < m_names.size(); ++i) {
            if (m_names[i]->Eval(local_context) == res_center->Focus())
                return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////
// StarType                                              //
///////////////////////////////////////////////////////////
Condition::StarType::~StarType() {
    for (unsigned int i = 0; i < m_types.size(); ++i) {
        delete m_types[i];
    }
}

namespace {
    struct StarTypeSimpleMatch {
        StarTypeSimpleMatch(const std::vector< ::StarType>& types) :
            m_types(types)
        {}

        bool operator()(const UniverseObject* candidate) const {
            if (!candidate)
                return false;

            const System* system = GetSystem(candidate->SystemID());
            if (system || (system = universe_object_cast<const System*>(candidate)))
                return !m_types.empty() && (std::find(m_types.begin(), m_types.end(), system->GetStarType()) != m_types.end());

            return false;
        }

        const std::vector< ::StarType>& m_types;
    };
}

void Condition::StarType::Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                               ObjectSet& non_matches, SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = parent_context.condition_root_candidate || RootCandidateInvariant();
    if (simple_eval_safe) {
        // check each valueref for invariance to local candidate
        for (std::vector<const ValueRef::ValueRefBase< ::StarType>*>::const_iterator it = m_types.begin();
            it != m_types.end(); ++it)
        {
            if (!(*it)->LocalCandidateInvariant()) {
                simple_eval_safe = false;
                break;
            }
        }
    }
    if (simple_eval_safe) {
        // evaluate types once, and use to check all candidate objects
        std::vector< ::StarType> types;
        // get all types from valuerefs
        for (std::vector<const ValueRef::ValueRefBase< ::StarType>*>::const_iterator it = m_types.begin();
             it != m_types.end(); ++it)
        {
            types.push_back((*it)->Eval(parent_context));
        }
        EvalImpl(matches, non_matches, search_domain, StarTypeSimpleMatch(types));
    } else {
        // re-evaluate contained objects for each candidate object
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::StarType::RootCandidateInvariant() const {
    for (std::vector<const ValueRef::ValueRefBase< ::StarType>*>::const_iterator it = m_types.begin();
         it != m_types.end(); ++it)
    {
        if (!(*it)->RootCandidateInvariant())
            return false;
    }
    return true;
}

bool Condition::StarType::TargetInvariant() const {
    for (std::vector<const ValueRef::ValueRefBase< ::StarType>*>::const_iterator it = m_types.begin();
         it != m_types.end(); ++it)
    {
        if (!(*it)->TargetInvariant())
            return false;
    }
    return true;
}

std::string Condition::StarType::Description(bool negated/* = false*/) const {
    std::string values_str;
    for (unsigned int i = 0; i < m_types.size(); ++i) {
        values_str += ValueRef::ConstantExpr(m_types[i]) ?
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
    std::string description_str = "DESC_STAR_TYPE";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str)) % values_str);
}

std::string Condition::StarType::Dump() const {
    std::string retval = DumpIndent() + "Star type = ";
    if (m_types.size() == 1) {
        retval += m_types[0]->Dump() + "\n";
    } else {
        retval += "[ ";
        for (unsigned int i = 0; i < m_types.size(); ++i) {
            retval += m_types[i]->Dump() + " ";
        }
        retval += "]\n";
    }
    return retval;
}

bool Condition::StarType::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "StarType::Match passed no candidate object";
        return false;
    }

    const System* system = GetSystem(candidate->SystemID());
    if (system || (system = universe_object_cast<const System*>(candidate))) {
        for (unsigned int i = 0; i < m_types.size(); ++i) {
            if (m_types[i]->Eval(local_context) == system->GetStarType())
                return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////
// DesignHasHull                                         //
///////////////////////////////////////////////////////////
std::string Condition::DesignHasHull::Description(bool negated/* = false*/) const {
    std::string description_str = "DESC_DESIGN_HAS_HULL";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str)) % UserString(m_name));
}

std::string Condition::DesignHasHull::Dump() const
{ return DumpIndent() + "DesignHasHull name = \"" + m_name + "\"\n"; }

bool Condition::DesignHasHull::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "DesignHasHull::Match passed no candidate object";
        return false;
    }

    if (const Ship* ship = universe_object_cast<const Ship*>(candidate))
        if (const ShipDesign* design = ship->Design())
            return (design->Hull() == m_name);
    return false;
}

///////////////////////////////////////////////////////////
// DesignHasPart                                         //
///////////////////////////////////////////////////////////
Condition::DesignHasPart::~DesignHasPart() {
    delete m_low;
    delete m_high;
}

namespace {
    struct DesignHasPartSimpleMatch {
        DesignHasPartSimpleMatch(int low, int high, const std::string& name) :
            m_low(low),
            m_high(high),
            m_name(name)
        {}

        bool operator()(const UniverseObject* candidate) const {
            if (!candidate)
                return false;

            // is it a ship?
            const ::Ship* ship = universe_object_cast<const ::Ship*>(candidate);
            if (!ship)
                return false;
            // with a valid design?
            const ShipDesign* design = ship->Design();
            if (!design)
                return false;

            const std::vector<std::string>& parts = design->Parts();
            int count = 0;
            for (std::vector<std::string>::const_iterator it = parts.begin(); it != parts.end(); ++it)
                if (*it == m_name || (m_name.empty() && !(*it).empty()))    // # of copies of specified part, or total number of parts if no part name specified
                    ++count;
            return (m_low <= count && count <= m_high);
        }

        int m_low;
        int m_high;
        const std::string& m_name;
    };
}

void Condition::DesignHasPart::Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                                    ObjectSet& non_matches, SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = (m_low->LocalCandidateInvariant() && m_high->LocalCandidateInvariant() &&
                             (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate number limits once, use to match all candidates
        const UniverseObject* no_object(0);
        ScriptingContext local_context(parent_context, no_object);
        int low =  std::max(0, m_low->Eval(local_context));
        int high = std::min(m_high->Eval(local_context), IMPOSSIBLY_LARGE_TURN);
        EvalImpl(matches, non_matches, search_domain, DesignHasPartSimpleMatch(low, high, m_name));
    } else {
        // re-evaluate allowed turn range for each candidate object
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::DesignHasPart::RootCandidateInvariant() const
{ return (m_low->RootCandidateInvariant() && m_high->RootCandidateInvariant()); }

bool Condition::DesignHasPart::TargetInvariant() const
{ return (m_low->TargetInvariant() && m_high->TargetInvariant()); }

std::string Condition::DesignHasPart::Description(bool negated/* = false*/) const {
    std::string low_str = ValueRef::ConstantExpr(m_low) ?
                            boost::lexical_cast<std::string>(m_low->Eval()) :
                            m_low->Description();
    std::string high_str = ValueRef::ConstantExpr(m_high) ?
                            boost::lexical_cast<std::string>(m_high->Eval()) :
                            m_high->Description();
    std::string description_str = "DESC_DESIGN_HAS_PART";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str))
               % low_str
               % high_str
               % m_name);
}

std::string Condition::DesignHasPart::Dump() const
{ return DumpIndent() + "DesignHasPart low = " + m_low->Dump() + " high = " + m_high->Dump() + " name = " + m_name; }

bool Condition::DesignHasPart::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "DesignHasPart::Match passed no candidate object";
        return false;
    }

    int low =  std::max(0, m_low->Eval(local_context));
    int high = std::min(m_high->Eval(local_context), IMPOSSIBLY_LARGE_TURN);

    return DesignHasPartSimpleMatch(low, high, m_name)(candidate);
}

///////////////////////////////////////////////////////////
// DesignHasPartClass                                    //
///////////////////////////////////////////////////////////
Condition::DesignHasPartClass::~DesignHasPartClass() {
    delete m_low;
    delete m_high;
}

namespace {
    struct DesignHasPartClassSimpleMatch {
        DesignHasPartClassSimpleMatch(int low, int high, ShipPartClass part_class) :
            m_low(low),
            m_high(high),
            m_part_class(part_class)
        {}

        bool operator()(const UniverseObject* candidate) const {
            if (!candidate)
                return false;

            // is it a ship?
            const ::Ship* ship = universe_object_cast<const ::Ship*>(candidate);
            if (!ship)
                return false;
            // with a valid design?
            const ShipDesign* design = ship->Design();
            if (!design)
                return false;


            const std::vector<std::string>& parts = design->Parts();
            int count = 0;
            for (std::vector<std::string>::const_iterator it = parts.begin(); it != parts.end(); ++it) {
                if (const PartType* part_type = GetPartType(*it)) {
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

void Condition::DesignHasPartClass::Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                                         ObjectSet& non_matches, SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = (m_low->LocalCandidateInvariant() && m_high->LocalCandidateInvariant() &&
                             (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate number limits once, use to match all candidates
        const UniverseObject* no_object(0);
        ScriptingContext local_context(parent_context, no_object);
        int low =  std::max(0, m_low->Eval(local_context));
        int high = std::min(m_high->Eval(local_context), IMPOSSIBLY_LARGE_TURN);
        EvalImpl(matches, non_matches, search_domain, DesignHasPartClassSimpleMatch(low, high, m_class));
    } else {
        // re-evaluate allowed turn range for each candidate object
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::DesignHasPartClass::RootCandidateInvariant() const
{ return (m_low->RootCandidateInvariant() && m_high->RootCandidateInvariant()); }

bool Condition::DesignHasPartClass::TargetInvariant() const
{ return (m_low->TargetInvariant() && m_high->TargetInvariant()); }

std::string Condition::DesignHasPartClass::Description(bool negated/* = false*/) const {
    std::string low_str = ValueRef::ConstantExpr(m_low) ?
                            boost::lexical_cast<std::string>(m_low->Eval()) :
                            m_low->Description();
    std::string high_str = ValueRef::ConstantExpr(m_high) ?
                            boost::lexical_cast<std::string>(m_high->Eval()) :
                            m_high->Description();
    std::string description_str = "DESC_DESIGN_HAS_PART_CLASS";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str))
               % low_str
               % high_str
               % UserString(boost::lexical_cast<std::string>(m_class)));
}

std::string Condition::DesignHasPartClass::Dump() const
{ return DumpIndent() + "DesignHasPartClass low = " + m_low->Dump() + " high = " + m_high->Dump() + " class = " + UserString(boost::lexical_cast<std::string>(m_class)); }

bool Condition::DesignHasPartClass::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "DesignHasPartClass::Match passed no candidate object";
        return false;
    }

    int low =  std::max(0, m_low->Eval(local_context));
    int high = std::min(m_high->Eval(local_context), IMPOSSIBLY_LARGE_TURN);

    return DesignHasPartClassSimpleMatch(low, high, m_class)(candidate);
}

///////////////////////////////////////////////////////////
// PredefinedShipDesign                                         //
///////////////////////////////////////////////////////////
std::string Condition::PredefinedShipDesign::Description(bool negated/* = false*/) const {
    std::string description_str = "DESC_PREDEFINED_SHIP_DESIGN";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str)) % UserString(m_name));
}

std::string Condition::PredefinedShipDesign::Dump() const
{ return DumpIndent() + "PredefinedShipDesign name = \"" + m_name + "\"\n"; }

bool Condition::PredefinedShipDesign::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "PredefinedShipDesign::Match passed no candidate object";
        return false;
    }

    const Ship* ship = universe_object_cast<const Ship*>(candidate);
    if (!ship)
        return false;
    const ShipDesign* candidate_design = ship->Design();
    if (!candidate_design)
        return false;

    // ship has a valid design.  see if it is / could be a predefined ship design...

    // all predefined named designs are hard-coded in parsing to have a designed on turn 0 (before first turn)
    if (candidate_design->DesignedOnTurn() != 0)
        return false;

    // all predefined designs have a name, and that name should match the condition m_name
    return (m_name == candidate_design->Name(false));   // don't look up in stringtable; predefined designs are stored by stringtable entry key
}

///////////////////////////////////////////////////////////
// NumberedShipDesign                                      //
///////////////////////////////////////////////////////////
Condition::NumberedShipDesign::~NumberedShipDesign()
{ delete m_design_id; }

namespace {
    struct NumberedShipDesignSimpleMatch {
        NumberedShipDesignSimpleMatch(int design_id) :
            m_design_id(design_id)
        {}

        bool operator()(const UniverseObject* candidate) const {
            if (!candidate)
                return false;
            if (m_design_id == ShipDesign::INVALID_DESIGN_ID)
                return false;
            if (const Ship* ship = universe_object_cast<const Ship*>(candidate))
                if (ship->DesignID() == m_design_id)
                    return true;
            return false;
        }

        int m_design_id;
    };
}

void Condition::NumberedShipDesign::Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                                         ObjectSet& non_matches, SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = ValueRef::ConstantExpr(m_design_id) ||
                            (m_design_id->LocalCandidateInvariant() &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate empire id once, and use to check all candidate objects
        const UniverseObject* no_object(0);
        int design_id = m_design_id->Eval(ScriptingContext(parent_context, no_object));
        EvalImpl(matches, non_matches, search_domain, NumberedShipDesignSimpleMatch(design_id));
    } else {
        // re-evaluate design id for each candidate object
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::NumberedShipDesign::RootCandidateInvariant() const
{ return m_design_id->RootCandidateInvariant(); }

bool Condition::NumberedShipDesign::TargetInvariant() const
{ return m_design_id->TargetInvariant(); }

std::string Condition::NumberedShipDesign::Description(bool negated/* = false*/) const {
    std::string id_str = ValueRef::ConstantExpr(m_design_id) ?
                            boost::lexical_cast<std::string>(m_design_id->Eval()) :
                            m_design_id->Description();

    std::string description_str = "DESC_NUMBERED_SHIP_DESIGN";
    if (negated)
        description_str += "_NOT";

    return str(FlexibleFormat(UserString(description_str))
               % id_str);
}

std::string Condition::NumberedShipDesign::Dump() const
{ return DumpIndent() + "NumberedShipDesign design_id = " + m_design_id->Dump(); }

bool Condition::NumberedShipDesign::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "NumberedShipDesign::Match passed no candidate object";
        return false;
    }
    return NumberedShipDesignSimpleMatch(m_design_id->Eval(local_context))(candidate);
}

///////////////////////////////////////////////////////////
// ProducedByEmpire                                      //
///////////////////////////////////////////////////////////
Condition::ProducedByEmpire::~ProducedByEmpire()
{ delete m_empire_id; }

namespace {
    struct ProducedByEmpireSimpleMatch {
        ProducedByEmpireSimpleMatch(int empire_id) :
            m_empire_id(empire_id)
        {}

        bool operator()(const UniverseObject* candidate) const {
            if (!candidate)
                return false;

            if (const ::Ship* ship = universe_object_cast<const ::Ship*>(candidate))
                return ship->ProducedByEmpireID() == m_empire_id;
            else if (const ::Building* building = universe_object_cast<const ::Building*>(candidate))
                return building->ProducedByEmpireID() == m_empire_id;
            return false;
        }

        int m_empire_id;
    };
}

void Condition::ProducedByEmpire::Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                                       ObjectSet& non_matches, SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = ValueRef::ConstantExpr(m_empire_id) ||
                            (m_empire_id->LocalCandidateInvariant() &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate empire id once, and use to check all candidate objects
        const UniverseObject* no_object(0);
        int empire_id = m_empire_id->Eval(ScriptingContext(parent_context, no_object));
        EvalImpl(matches, non_matches, search_domain, ProducedByEmpireSimpleMatch(empire_id));
    } else {
        // re-evaluate empire id for each candidate object
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::ProducedByEmpire::RootCandidateInvariant() const
{ return m_empire_id->RootCandidateInvariant(); }

bool Condition::ProducedByEmpire::TargetInvariant() const
{ return m_empire_id->TargetInvariant(); }

std::string Condition::ProducedByEmpire::Description(bool negated/* = false*/) const {
    std::string empire_str;
    if (m_empire_id) {
        int empire_id = ALL_EMPIRES;
        if (ValueRef::ConstantExpr(m_empire_id))
            empire_id = m_empire_id->Eval();
        if (const Empire* empire = Empires().Lookup(empire_id))
            empire_str = empire->Name();
        else
            empire_str = m_empire_id->Description();
    }

    std::string description_str = "DESC_PRODUCED_BY_EMPIRE";
    if (negated)
        description_str += "_NOT";

    return str(FlexibleFormat(UserString(description_str))
               % empire_str);
}

std::string Condition::ProducedByEmpire::Dump() const
{ return DumpIndent() + "ProducedByEmpire empire_id = " + m_empire_id->Dump(); }

bool Condition::ProducedByEmpire::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "ProducedByEmpire::Match passed no candidate object";
        return false;
    }

    return ProducedByEmpireSimpleMatch(m_empire_id->Eval(local_context))(candidate);
}

///////////////////////////////////////////////////////////
// Chance                                                //
///////////////////////////////////////////////////////////
Condition::Chance::~Chance()
{ delete m_chance; }

namespace {
    struct ChanceSimpleMatch {
        ChanceSimpleMatch(double chance) :
            m_chance(chance)
        {}

        bool operator()(const UniverseObject* candidate) const
        { return RandZeroToOne() <= m_chance; }

        double m_chance;
    };
}

void Condition::Chance::Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                             ObjectSet& non_matches, SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = ValueRef::ConstantExpr(m_chance) ||
                            (m_chance->LocalCandidateInvariant() &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate empire id once, and use to check all candidate objects
        const UniverseObject* no_object(0);
        double chance = std::max(0.0, std::min(1.0, m_chance->Eval(ScriptingContext(parent_context, no_object))));
        EvalImpl(matches, non_matches, search_domain, ChanceSimpleMatch(chance));
    } else {
        // re-evaluate empire id for each candidate object
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::Chance::RootCandidateInvariant() const
{ return m_chance->RootCandidateInvariant(); }

bool Condition::Chance::TargetInvariant() const
{ return m_chance->TargetInvariant(); }

std::string Condition::Chance::Description(bool negated/* = false*/) const {
    std::string value_str;
    if (ValueRef::ConstantExpr(m_chance)) {
        std::string description_str = "DESC_CHANCE_PERCENTAGE";
        if (negated)
            description_str += "_NOT";
        return str(FlexibleFormat(UserString(description_str)) %
                                  boost::lexical_cast<std::string>(std::max(0.0, std::min(m_chance->Eval(), 1.0)) * 100));
    } else {
        std::string description_str = "DESC_CHANCE";
        if (negated)
            description_str += "_NOT";
        return str(FlexibleFormat(UserString(description_str)) % m_chance->Description());
    }
}

std::string Condition::Chance::Dump() const
{ return DumpIndent() + "Random probability = " + m_chance->Dump() + "\n"; }

bool Condition::Chance::Match(const ScriptingContext& local_context) const {
    double chance = std::max(0.0, std::min(m_chance->Eval(local_context), 1.0));
    return RandZeroToOne() <= chance;
}

///////////////////////////////////////////////////////////
// MeterValue                                            //
///////////////////////////////////////////////////////////
Condition::MeterValue::~MeterValue() {
    delete m_low;
    delete m_high;
}

namespace {
    struct MeterValueSimpleMatch {
        MeterValueSimpleMatch(double low, double high, MeterType meter_type) :
            m_low(low),
            m_high(high),
            m_meter_type(meter_type)
        {}

        bool operator()(const UniverseObject* candidate) const {
            if (!candidate)
                return false;

            if (const Meter* meter = candidate->GetMeter(m_meter_type)) {
                double value = meter->Initial();    // match Initial rather than Current to make results reproducible in a given turn, until back propegation happens
                return m_low <= value && value <= m_high;
            }

            return false;
        }

        double m_low;
        double m_high;
        MeterType m_meter_type;
    };
}

void Condition::MeterValue::Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                                 ObjectSet& non_matches, SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = ((!m_low || m_low->LocalCandidateInvariant()) &&
                             (!m_high || m_high->LocalCandidateInvariant()) &&
                             (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate number limits once, use to match all candidates
        const UniverseObject* no_object(0);
        ScriptingContext local_context(parent_context, no_object);
        double low = (m_low ? m_low->Eval(local_context) : -Meter::LARGE_VALUE);
        double high = (m_high ? m_high->Eval(local_context) : Meter::LARGE_VALUE);
        EvalImpl(matches, non_matches, search_domain, MeterValueSimpleMatch(low, high, m_meter));
    } else {
        // re-evaluate allowed turn range for each candidate object
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::MeterValue::RootCandidateInvariant() const
{ return (!m_low || m_low->RootCandidateInvariant()) && (!m_high || m_high->RootCandidateInvariant()); }

bool Condition::MeterValue::TargetInvariant() const
{ return (!m_low || m_low->TargetInvariant()) && (!m_high || m_high->TargetInvariant()); }

std::string Condition::MeterValue::Description(bool negated/* = false*/) const {
    std::string low_str = (m_low ? (ValueRef::ConstantExpr(m_low) ?
                                    boost::lexical_cast<std::string>(m_low->Eval()) :
                                    m_low->Description())
                                 : boost::lexical_cast<std::string>(-Meter::LARGE_VALUE));
    std::string high_str = (m_high ? (ValueRef::ConstantExpr(m_high) ?
                                      boost::lexical_cast<std::string>(m_high->Eval()) :
                                      m_high->Description())
                                   : boost::lexical_cast<std::string>(Meter::LARGE_VALUE));
    std::string description_str = "DESC_METER_VALUE_CURRENT";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str))
               % UserString(boost::lexical_cast<std::string>(m_meter))
               % low_str
               % high_str);
}

std::string Condition::MeterValue::Dump() const {
    std::string retval = DumpIndent();
    switch (m_meter) {
    case INVALID_METER_TYPE:        retval += "INVALID_METER_TYPE"; break;
    case METER_TARGET_POPULATION:   retval += "TargetPopulation";   break;
    case METER_TARGET_INDUSTRY:     retval += "TargetIndustry";     break;
    case METER_TARGET_RESEARCH:     retval += "TargetResearch";     break;
    case METER_TARGET_TRADE:        retval += "TargetTrade";        break;
    case METER_TARGET_MINING:       retval += "TargetMining";       break;
    case METER_TARGET_CONSTRUCTION: retval += "TargetConstruction"; break;
    case METER_MAX_FUEL:            retval += "MaxFuel";            break;
    case METER_MAX_SHIELD:          retval += "MaxShield";          break;
    case METER_MAX_STRUCTURE:       retval += "MaxStructure";       break;
    case METER_MAX_DEFENSE:         retval += "MaxDefense";         break;
    case METER_POPULATION:          retval += "Population";         break;
    case METER_INDUSTRY:            retval += "Industry";           break;
    case METER_RESEARCH:            retval += "Research";           break;
    case METER_TRADE:               retval += "Trade";              break;
    case METER_MINING:              retval += "Mining";             break;
    case METER_CONSTRUCTION:        retval += "Construction";       break;
    case METER_FUEL:                retval += "Fuel";               break;
    case METER_SHIELD:              retval += "Shield";             break;
    case METER_STRUCTURE:           retval += "Structure";          break;
    case METER_DEFENSE:             retval += "Defense";            break;
    case METER_SUPPLY:              retval += "Supply";             break;
    case METER_STEALTH:             retval += "Stealth";            break;
    case METER_DETECTION:           retval += "Detection";          break;
    case METER_BATTLE_SPEED:        retval += "BattleSpeed";        break;
    case METER_STARLANE_SPEED:      retval += "StarlaneSpeed";      break;
    case METER_DAMAGE:              retval += "Damage";             break;
    case METER_ROF:                 retval += "ROF";                break;
    case METER_RANGE:               retval += "Range";              break;
    case METER_SPEED:               retval += "Speed";              break;
    case METER_CAPACITY:            retval += "Capacity";           break;
    case METER_ANTI_SHIP_DAMAGE:    retval += "AntiShipDamage";     break;
    case METER_ANTI_FIGHTER_DAMAGE: retval += "AntiFighterDamage";  break;
    case METER_LAUNCH_RATE:         retval += "LaunchRate";         break;
    case METER_FIGHTER_WEAPON_RANGE:retval += "FighterWeaponRange"; break;
    default: retval += "?Meter?"; break;
    }
    if (m_low)
        retval += " low = " + m_low->Dump();
    if (m_high)
        retval += " high = " + m_high->Dump();
    retval += "\n";
    return retval;
}

bool Condition::MeterValue::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "MeterValue::Match passed no candidate object";
        return false;
    }
    double low = (m_low ? m_low->Eval(local_context) : -Meter::LARGE_VALUE);
    double high = (m_high ? m_high->Eval(local_context) : Meter::LARGE_VALUE);
    return MeterValueSimpleMatch(low, high, m_meter)(candidate);
}

///////////////////////////////////////////////////////////
// EmpireMeterValue                                            //
///////////////////////////////////////////////////////////
Condition::EmpireMeterValue::~EmpireMeterValue() {
    delete m_empire_id;
    delete m_low;
    delete m_high;
}

namespace {
    struct EmpireMeterValueSimpleMatch {
        EmpireMeterValueSimpleMatch(int empire_id, double low, double high, const std::string& meter) :
            m_empire_id(empire_id),
            m_low(low),
            m_high(high),
            m_meter(meter)
        {}

        bool operator()(const UniverseObject* candidate) const {
            if (!candidate)
                return false;
            const Empire* empire = Empires().Lookup(m_empire_id);
            if (!empire)
                return false;
            const Meter* meter = empire->GetMeter(m_meter);
            if (!meter)
                return false;
            double meter_current = meter->Current();
            return (m_low <= meter_current && meter_current <= m_high);
        }

        int         m_empire_id;
        double      m_low;
        double      m_high;
        std::string m_meter;
    };
}

void Condition::EmpireMeterValue::Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                                 ObjectSet& non_matches, SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = ((m_empire_id && m_empire_id->LocalCandidateInvariant()) &&
                             (!m_low || m_low->LocalCandidateInvariant()) &&
                             (!m_high || m_high->LocalCandidateInvariant()) &&
                             (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate number limits once, use to match all candidates
        const UniverseObject* no_object(0);
        ScriptingContext local_context(parent_context, no_object);
        int empire_id = m_empire_id->Eval(local_context);   // if m_empire_id not set, default to local candidate's owner, which is not target invariant
        double low = (m_low ? m_low->Eval(local_context) : -Meter::LARGE_VALUE);
        double high = (m_high ? m_high->Eval(local_context) : Meter::LARGE_VALUE);
        EvalImpl(matches, non_matches, search_domain, EmpireMeterValueSimpleMatch(empire_id, low, high, m_meter));
    } else {
        // re-evaluate allowed turn range for each candidate object
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::EmpireMeterValue::RootCandidateInvariant() const {
    return (!m_empire_id || m_empire_id->RootCandidateInvariant()) &&
           (!m_low || m_low->RootCandidateInvariant()) &&
           (!m_high || m_high->RootCandidateInvariant());
}

bool Condition::EmpireMeterValue::TargetInvariant() const {
    return (!m_empire_id || m_empire_id->TargetInvariant()) &&
           (!m_low || m_low->TargetInvariant()) &&
           (!m_high || m_high->TargetInvariant());
}

std::string Condition::EmpireMeterValue::Description(bool negated/* = false*/) const {
    std::string empire_str;
    if (m_empire_id) {
        int empire_id = ALL_EMPIRES;
        if (ValueRef::ConstantExpr(m_empire_id))
            empire_id = m_empire_id->Eval();
        if (const Empire* empire = Empires().Lookup(empire_id))
            empire_str = empire->Name();
        else
            empire_str = m_empire_id->Description();
    }
    std::string low_str = (m_low ? (ValueRef::ConstantExpr(m_low) ?
                                    boost::lexical_cast<std::string>(m_low->Eval()) :
                                    m_low->Description())
                                 : boost::lexical_cast<std::string>(-Meter::LARGE_VALUE));
    std::string high_str = (m_high ? (ValueRef::ConstantExpr(m_high) ?
                                      boost::lexical_cast<std::string>(m_high->Eval()) :
                                      m_high->Description())
                                   : boost::lexical_cast<std::string>(Meter::LARGE_VALUE));
    std::string description_str = "DESC_EMPIRE_METER_VALUE_CURRENT";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str))
               % UserString(m_meter)
               % low_str
               % high_str
               % empire_str);
}

std::string Condition::EmpireMeterValue::Dump() const {
    std::string retval = DumpIndent() + "EmpireMeterValue";
    if (m_empire_id)
        retval += " empire = " + m_empire_id->Dump();
    retval += " meter = " + m_meter;
    if (m_low)
        retval += " low = " + m_low->Dump();
    if (m_high)
        retval += " high = " + m_high->Dump();
    retval += "\n";
    return retval;
}

bool Condition::EmpireMeterValue::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "EmpireMeterValue::Match passed no candidate object";
        return false;
    }
    int empire_id = (m_empire_id ? m_empire_id->Eval(local_context) : candidate->Owner());
    if (empire_id == ALL_EMPIRES)
        return false;
    double low = (m_low ? m_low->Eval(local_context) : -Meter::LARGE_VALUE);
    double high = (m_high ? m_high->Eval(local_context) : Meter::LARGE_VALUE);
    return EmpireMeterValueSimpleMatch(empire_id, low, high, m_meter)(candidate);
}

///////////////////////////////////////////////////////////
// EmpireStockpileValue                                  //
///////////////////////////////////////////////////////////
Condition::EmpireStockpileValue::~EmpireStockpileValue() {
    delete m_low;
    delete m_high;
}

namespace {
    struct EmpireStockpileValueSimpleMatch {
        EmpireStockpileValueSimpleMatch(double low, double high, ResourceType stockpile) :
            m_low(low),
            m_high(high),
            m_stockpile(stockpile)
        {}

        bool operator()(const UniverseObject* candidate) const {
            if (!candidate)
                return false;

            if (candidate->Unowned())
                return false;

            const Empire* empire = Empires().Lookup(candidate->Owner());
            if (!empire)
                return false;

            if (m_stockpile == RE_MINERALS || m_stockpile == RE_TRADE) {
                double amount = empire->ResourceStockpile(m_stockpile);
                return (m_low <= amount && amount <= m_high);
            }

            return false;
        }

        double m_low;
        double m_high;
        ResourceType m_stockpile;
    };
}

void Condition::EmpireStockpileValue::Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                                           ObjectSet& non_matches, SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = (m_low->LocalCandidateInvariant() && m_high->LocalCandidateInvariant() &&
                             (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate number limits once, use to match all candidates
        const UniverseObject* no_object(0);
        ScriptingContext local_context(parent_context, no_object);
        double low = m_low->Eval(local_context);
        double high = m_high->Eval(local_context);
        EvalImpl(matches, non_matches, search_domain, EmpireStockpileValueSimpleMatch(low, high, m_stockpile));
    } else {
        // re-evaluate allowed turn range for each candidate object
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::EmpireStockpileValue::RootCandidateInvariant() const
{ return (m_low->RootCandidateInvariant() && m_high->RootCandidateInvariant()); }

bool Condition::EmpireStockpileValue::TargetInvariant() const
{ return (m_low->TargetInvariant() && m_high->TargetInvariant()); }

std::string Condition::EmpireStockpileValue::Description(bool negated/* = false*/) const {
    std::string low_str = ValueRef::ConstantExpr(m_low) ?
                            boost::lexical_cast<std::string>(m_low->Eval()) :
                            m_low->Description();
    std::string high_str = ValueRef::ConstantExpr(m_high) ?
                            boost::lexical_cast<std::string>(m_high->Eval()) :
                            m_high->Description();
    std::string description_str = "DESC_EMPIRE_STOCKPILE_VALUE";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str))
               % UserString(boost::lexical_cast<std::string>(m_stockpile))
               % low_str
               % high_str);
}

std::string Condition::EmpireStockpileValue::Dump() const {
    std::string retval = DumpIndent();
    switch (m_stockpile) {
    case RE_MINERALS:   retval += "OwnerMineralStockpile";  break;
    case RE_TRADE:      retval += "OwnerTradeStockpile";    break;
    case RE_RESEARCH:   retval += "OwnerResearchStockpile"; break;
    case RE_INDUSTRY:   retval += "OwnerIndustryStockpile"; break;
    default: retval += "?"; break;
    }
    retval += " low = " + m_low->Dump() + " high = " + m_high->Dump() + "\n";
    return retval;
}

bool Condition::EmpireStockpileValue::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "EmpireStockpileValue::Match passed no candidate object";
        return false;
    }

    double low = m_low->Eval(local_context);
    double high = m_high->Eval(local_context);
    return EmpireStockpileValueSimpleMatch(low, high, m_stockpile)(candidate);
}

///////////////////////////////////////////////////////////
// OwnerHasTech                                          //
///////////////////////////////////////////////////////////
std::string Condition::OwnerHasTech::Description(bool negated/* = false*/) const {
    std::string description_str = "DESC_OWNER_HAS_TECH";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str)) % UserString(m_name));
}

std::string Condition::OwnerHasTech::Dump() const
{ return DumpIndent() + "OwnerHasTech name = \"" + m_name + "\"\n"; }

bool Condition::OwnerHasTech::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "OwnerHasTech::Match passed no candidate object";
        return false;
    }

    if (candidate->Unowned())
        return false;

    if (const Empire* empire = Empires().Lookup(candidate->Owner()))
        return empire->TechResearched(m_name);
    else
        return false;
}

///////////////////////////////////////////////////////////
// VisibleToEmpire                                       //
///////////////////////////////////////////////////////////
Condition::VisibleToEmpire::~VisibleToEmpire()
{ delete m_empire_id; }

namespace {
    struct VisibleToEmpireSimpleMatch
    {
        VisibleToEmpireSimpleMatch(int empire_id) :
            m_empire_id(empire_id)
        {}

        bool operator()(const UniverseObject* candidate) const {
            if (!candidate)
                return false;
            return candidate->GetVisibility(m_empire_id) != VIS_NO_VISIBILITY;
        }

        int m_empire_id;
    };
}

void Condition::VisibleToEmpire::Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                                      ObjectSet& non_matches, SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = ValueRef::ConstantExpr(m_empire_id) ||
                            (m_empire_id->LocalCandidateInvariant() &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate empire id once, and use to check all candidate objects
        const UniverseObject* no_object(0);
        int empire_id = m_empire_id->Eval(ScriptingContext(parent_context, no_object));
        EvalImpl(matches, non_matches, search_domain, VisibleToEmpireSimpleMatch(empire_id));
    } else {
        // re-evaluate empire id for each candidate object
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::VisibleToEmpire::RootCandidateInvariant() const
{ return m_empire_id->RootCandidateInvariant(); }

bool Condition::VisibleToEmpire::TargetInvariant() const
{ return m_empire_id->TargetInvariant(); }

std::string Condition::VisibleToEmpire::Description(bool negated/* = false*/) const {
    std::string empire_str;
    if (m_empire_id) {
        int empire_id = ALL_EMPIRES;
        if (ValueRef::ConstantExpr(m_empire_id))
            empire_id = m_empire_id->Eval();
        if (const Empire* empire = Empires().Lookup(empire_id))
            empire_str = empire->Name();
        else
            empire_str = m_empire_id->Description();
    }

    std::string description_str = "DESC_VISIBLE_TO_EMPIRE";
    if (negated)
        description_str += "_NOT";

    return str(FlexibleFormat(UserString(description_str))
               % empire_str);
}

std::string Condition::VisibleToEmpire::Dump() const
{ return DumpIndent() + "VisibleToEmpire empire_id = " + m_empire_id->Dump(); }

bool Condition::VisibleToEmpire::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "VisibleToEmpire::Match passed no candidate object";
        return false;
    }

    return candidate->GetVisibility(m_empire_id->Eval(local_context)) != VIS_NO_VISIBILITY;
}

///////////////////////////////////////////////////////////
// WithinDistance                                        //
///////////////////////////////////////////////////////////
Condition::WithinDistance::~WithinDistance() {
    delete m_distance;
    delete m_condition;
}

namespace {
    struct WithinDistanceSimpleMatch {
        WithinDistanceSimpleMatch(const Condition::ObjectSet& from_objects, double distance) :
            m_from_objects(from_objects),
            m_distance(distance)
        {}

        bool operator()(const UniverseObject* candidate) const {
            if (!candidate)
                return false;

            double distance2 = m_distance*m_distance;

            // is candidate object close enough to any of the passed-in objects?
            for (Condition::ObjectSet::const_iterator it = m_from_objects.begin(); it != m_from_objects.end(); ++it) {
                double delta_x = candidate->X() - (*it)->X();
                double delta_y = candidate->Y() - (*it)->Y();
                if (delta_x*delta_x + delta_y*delta_y < distance2)
                    return true;
            }

            return false;
        }

        const Condition::ObjectSet& m_from_objects;
        double m_distance;
    };
}

void Condition::WithinDistance::Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                                     ObjectSet& non_matches, SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = m_distance->LocalCandidateInvariant() &&
                            parent_context.condition_root_candidate || RootCandidateInvariant();
    if (simple_eval_safe) {
        // evaluate contained objects once and check for all candidates
        ObjectMap& objects = GetUniverse().Objects();

        const UniverseObject* no_object(0);
        ScriptingContext local_context(parent_context, no_object);

        // get objects to be considering for matching against subcondition
        ObjectSet subcondition_non_matches;
        subcondition_non_matches.reserve(RESERVE_SET_SIZE);
        for (ObjectMap::iterator it = objects.begin(); it != objects.end(); ++it)
            subcondition_non_matches.push_back(it->second);
        ObjectSet subcondition_matches;
        subcondition_matches.reserve(RESERVE_SET_SIZE);

        m_condition->Eval(local_context, subcondition_matches, subcondition_non_matches);

        double distance = m_distance->Eval(local_context);

        EvalImpl(matches, non_matches, search_domain, WithinDistanceSimpleMatch(subcondition_matches, distance));
    } else {
        // re-evaluate contained objects for each candidate object
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::WithinDistance::RootCandidateInvariant() const
{ return m_distance->RootCandidateInvariant() && m_condition->RootCandidateInvariant(); }

bool Condition::WithinDistance::TargetInvariant() const
{ return m_distance->TargetInvariant() && m_condition->TargetInvariant(); }

std::string Condition::WithinDistance::Description(bool negated/* = false*/) const {
    std::string value_str = ValueRef::ConstantExpr(m_distance) ?
                                boost::lexical_cast<std::string>(m_distance->Eval()) :
                                m_distance->Description();
    std::string description_str = "DESC_WITHIN_DISTANCE";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str))
               % value_str
               % m_condition->Description());
}

std::string Condition::WithinDistance::Dump() const {
    std::string retval = DumpIndent() + "WithinDistance distance = " + m_distance->Dump() + " condition =\n";
    ++g_indent;
    retval += m_condition->Dump();
    --g_indent;
    return retval;
}

bool Condition::WithinDistance::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "WithinDistance::Match passed no candidate object";
        return false;
    }

    ObjectMap& objects = GetUniverse().Objects();

    // get objects to be considering for matching against subcondition
    ObjectSet subcondition_non_matches;
    subcondition_non_matches.reserve(RESERVE_SET_SIZE);
    for (ObjectMap::iterator it = objects.begin(); it != objects.end(); ++it)
        subcondition_non_matches.push_back(it->second);
    ObjectSet subcondition_matches;
    subcondition_matches.reserve(RESERVE_SET_SIZE);

    m_condition->Eval(local_context, subcondition_matches, subcondition_non_matches);
    if (subcondition_matches.empty())
        return false;

    return WithinDistanceSimpleMatch(subcondition_matches, m_distance->Eval(local_context))(candidate);
}

///////////////////////////////////////////////////////////
// WithinStarlaneJumps                                   //
///////////////////////////////////////////////////////////
Condition::WithinStarlaneJumps::~WithinStarlaneJumps() {
    delete m_jumps;
    delete m_condition;
}

namespace {
    const int MANY_JUMPS(999999);

    int JumpsBetweenObjects(const UniverseObject* one, const UniverseObject* two) {
        ObjectMap& objects = GetUniverse().Objects();

        if (!one || !two)
            return MANY_JUMPS;

        const System* system_one = GetSystem(one->SystemID());
        const System* system_two = GetSystem(two->SystemID());

        if (system_one && system_two) {
            // both condition-matching object and candidate are / in systems.
            // can just find the shortest path between the two systems
            short jumps = -1;
            try {
                jumps = GetUniverse().JumpDistance(system_one->ID(), system_two->ID());
            } catch (...) {
                Logger().errorStream() << "JumpsBetweenObjects caught exception when calling JumpDistance";
            }
            if (jumps != -1)    // if jumps is -1, no path exists between the systems
                return static_cast<int>(jumps);

        } else if (system_one) {
            // just object one is / in a system.
            if (const Fleet* fleet = FleetFromObject(two)) {
                // other object is a fleet that is between systems
                // need to check shortest path from systems on either side of starlane fleet is on
                short jumps1 = -1, jumps2 = -1;
                try {
                    jumps1 = GetUniverse().JumpDistance(system_one->ID(), fleet->PreviousSystemID());
                    jumps2 = GetUniverse().JumpDistance(system_one->ID(), fleet->NextSystemID());
                } catch (...) {
                    Logger().errorStream() << "JumpsBetweenObjects caught exception when calling JumpDistance";
                }
                int jumps = static_cast<int>(std::max(jumps1, jumps2));
                if (jumps != -1)
                    return jumps - 1;
            }

        } else if (system_two) {
            // just object two is a system.
            if (const Fleet* fleet = FleetFromObject(one)) {
                // other object is a fleet that is between systems
                // need to check shortest path from systems on either side of starlane fleet is on
                short jumps1 = -1, jumps2 = -1;
                try {
                    jumps1 = GetUniverse().JumpDistance(system_two->ID(), fleet->PreviousSystemID());
                    jumps2 = GetUniverse().JumpDistance(system_two->ID(), fleet->NextSystemID());
                } catch (...) {
                    Logger().errorStream() << "JumpsBetweenObjects caught exception when calling JumpDistance";
                }
                int jumps = static_cast<int>(std::max(jumps1, jumps2));
                if (jumps != -1)
                    return jumps - 1;
            }
        } else {
            // neither object is / in a system

            const Fleet* fleet_one = FleetFromObject(one);
            const Fleet* fleet_two = FleetFromObject(two);

            if (fleet_one && fleet_two) {
                // both objects are / in a fleet.
                // need to check all combinations of systems on either sides of
                // starlanes condition-matching object and candidate are on
                int fleet_one_prev_system_id = fleet_one->PreviousSystemID();
                int fleet_one_next_system_id = fleet_one->NextSystemID();
                int fleet_two_prev_system_id = fleet_two->PreviousSystemID();
                int fleet_two_next_system_id = fleet_two->NextSystemID();
                short jumps1 = -1, jumps2 = -1, jumps3 = -1, jumps4 = -1;
                try {
                    jumps1 = GetUniverse().JumpDistance(fleet_one_prev_system_id, fleet_two_prev_system_id);
                    jumps2 = GetUniverse().JumpDistance(fleet_one_prev_system_id, fleet_two_next_system_id);
                    jumps3 = GetUniverse().JumpDistance(fleet_one_next_system_id, fleet_two_prev_system_id);
                    jumps4 = GetUniverse().JumpDistance(fleet_one_next_system_id, fleet_two_next_system_id);
                } catch (...) {
                    Logger().errorStream() << "JumpsBetweenObjects caught exception when calling JumpDistance";
                }
                int jumps = static_cast<int>(std::max(jumps1, std::max(jumps2, std::max(jumps3, jumps4))));
                if (jumps != -1)
                    return jumps - 1;
            }
        }
        return MANY_JUMPS;
    }

    struct WithinStarlaneJumpsSimpleMatch {
        WithinStarlaneJumpsSimpleMatch(const Condition::ObjectSet& from_objects, int jump_limit) :
            m_from_objects(from_objects),
            m_jump_limit(jump_limit)
        {}

        bool operator()(const UniverseObject* candidate) const {
            if (!candidate)
                return false;
            if (m_from_objects.empty())
                return false;
            if (m_jump_limit < 0)
                return false;

            // is candidate object close enough to any subcondition matches?
            for (Condition::ObjectSet::const_iterator it = m_from_objects.begin(); it != m_from_objects.end(); ++it) {
                if (m_jump_limit == 0) {
                    // special case, since LeastJumpsPath() doesn't expect the start point to be the end point
                    double delta_x = (*it)->X() - candidate->X();
                    double delta_y = (*it)->Y() - candidate->Y();
                    if (delta_x*delta_x + delta_y*delta_y == 0)
                        return true;
                } else {
                    int jumps = JumpsBetweenObjects(*it, candidate);
                    if (jumps <= m_jump_limit)
                        return true;
                }
            }

            return false;
        }

        const Condition::ObjectSet& m_from_objects;
        int m_jump_limit;
    };
}

void Condition::WithinStarlaneJumps::Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                                          ObjectSet& non_matches,
                                          SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = m_jumps->LocalCandidateInvariant() &&
                            parent_context.condition_root_candidate || RootCandidateInvariant();
    if (simple_eval_safe) {
        // evaluate contained objects once and check for all candidates
        ObjectMap& objects = GetUniverse().Objects();

        const UniverseObject* no_object(0);
        ScriptingContext local_context(parent_context, no_object);

        // get objects to be considering for matching against subcondition
        ObjectSet subcondition_non_matches;
        subcondition_non_matches.reserve(RESERVE_SET_SIZE);
        for (ObjectMap::iterator it = objects.begin(); it != objects.end(); ++it)
            subcondition_non_matches.push_back(it->second);
        ObjectSet subcondition_matches;
        subcondition_matches.reserve(RESERVE_SET_SIZE);

        m_condition->Eval(local_context, subcondition_matches, subcondition_non_matches);
        int jump_limit = m_jumps->Eval(local_context);

        EvalImpl(matches, non_matches, search_domain, WithinStarlaneJumpsSimpleMatch(subcondition_matches, jump_limit));
    } else {
        // re-evaluate contained objects for each candidate object
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::WithinStarlaneJumps::RootCandidateInvariant() const
{ return m_jumps->RootCandidateInvariant() && m_condition->RootCandidateInvariant(); }

bool Condition::WithinStarlaneJumps::TargetInvariant() const
{ return m_jumps->TargetInvariant() && m_condition->TargetInvariant(); }

std::string Condition::WithinStarlaneJumps::Description(bool negated/* = false*/) const {
    std::string value_str = ValueRef::ConstantExpr(m_jumps) ? boost::lexical_cast<std::string>(m_jumps->Eval()) : m_jumps->Description();
    std::string description_str = "DESC_WITHIN_STARLANE_JUMPS";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str))
               % value_str
               % m_condition->Description());
}

std::string Condition::WithinStarlaneJumps::Dump() const {
    std::string retval = DumpIndent() + "WithinStarlaneJumps jumps = " + m_jumps->Dump() + " condition =\n";
    ++g_indent;
    retval += m_condition->Dump();
    --g_indent;
    return retval;
}

bool Condition::WithinStarlaneJumps::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "WithinStarlaneJumps::Match passed no candidate object";
        return false;
    }

    ObjectMap& objects = GetUniverse().Objects();

    // get objects to be considering for matching against subcondition
    ObjectSet subcondition_non_matches;
    subcondition_non_matches.reserve(RESERVE_SET_SIZE);
    for (ObjectMap::iterator it = objects.begin(); it != objects.end(); ++it)
        subcondition_non_matches.push_back(it->second);
    ObjectSet subcondition_matches;
    subcondition_matches.reserve(RESERVE_SET_SIZE);

    m_condition->Eval(local_context, subcondition_matches, subcondition_non_matches);
    int jump_limit = m_jumps->Eval(local_context);

    return WithinStarlaneJumpsSimpleMatch(subcondition_matches, jump_limit)(candidate);
}

///////////////////////////////////////////////////////////
// CanAddStarlaneConnection                              //
///////////////////////////////////////////////////////////
Condition::CanAddStarlaneConnection::~CanAddStarlaneConnection()
{ delete m_condition; }

namespace {
    struct CanAddStarlaneConnectionSimpleMatch {
        CanAddStarlaneConnectionSimpleMatch(const Condition::ObjectSet& destination_objects) :
            m_destination_objects(destination_objects)
        {}

        bool operator()(const UniverseObject* candidate) const {
            if (!candidate)
                return false;

            ObjectMap& objects = GetUniverse().Objects();

            // assemble all systems that are or that contain subcondition matches
            Condition::ObjectSet destination_systems;
            destination_systems.reserve(RESERVE_SET_SIZE);
            for (Condition::ObjectSet::const_iterator it = m_destination_objects.begin(); it != m_destination_objects.end(); ++it)
                if (const System* system = objects.Object< ::System>((*it)->SystemID()))
                    destination_systems.push_back(system);

            if (destination_systems.empty())
                return false;

            // TODO: implement this test

            // can the candidate object have starlanes added to all destination systems?
            for (Condition::ObjectSet::const_iterator it = destination_systems.begin(); it != destination_systems.end(); ++it) {
                return false;
            }

            return true;
        }

        const Condition::ObjectSet& m_destination_objects;
    };
}

void Condition::CanAddStarlaneConnection::Eval(const ScriptingContext& parent_context,
                                               ObjectSet& matches, ObjectSet& non_matches,
                                               SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = parent_context.condition_root_candidate || RootCandidateInvariant();
    if (simple_eval_safe) {
        // evaluate contained objects once and check for all candidates
        ObjectMap& objects = GetUniverse().Objects();

        const UniverseObject* no_object(0);
        ScriptingContext local_context(parent_context, no_object);

        // get objects to be considering for matching against subcondition
        ObjectSet subcondition_non_matches;
        subcondition_non_matches.reserve(RESERVE_SET_SIZE);
        for (ObjectMap::iterator it = objects.begin(); it != objects.end(); ++it)
            subcondition_non_matches.push_back(it->second);
        ObjectSet subcondition_matches;
        subcondition_matches.reserve(RESERVE_SET_SIZE);

        m_condition->Eval(local_context, subcondition_matches, subcondition_non_matches);

        EvalImpl(matches, non_matches, search_domain, CanAddStarlaneConnectionSimpleMatch(subcondition_matches));
    } else {
        // re-evaluate contained objects for each candidate object
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::CanAddStarlaneConnection::RootCandidateInvariant() const
{ return m_condition->RootCandidateInvariant(); }

bool Condition::CanAddStarlaneConnection::TargetInvariant() const
{ return m_condition->TargetInvariant(); }

std::string Condition::CanAddStarlaneConnection::Description(bool negated/* = false*/) const {
    std::string description_str = "DESC_CAN_ADD_STARLANE_CONNECTION";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str)) % m_condition->Description());
}

std::string Condition::CanAddStarlaneConnection::Dump() const {
    std::string retval = DumpIndent() + "CanAddStarlaneConnection condition =\n";
    ++g_indent;
        retval += m_condition->Dump();
    --g_indent;
    return retval;
}

bool Condition::CanAddStarlaneConnection::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "CanAddStarlaneConnection::Match passed no candidate object";
        return false;
    }

    ObjectMap& objects = GetUniverse().Objects();

    // get objects to be considering for matching against subcondition
    ObjectSet subcondition_non_matches;
    subcondition_non_matches.reserve(RESERVE_SET_SIZE);
    for (ObjectMap::iterator it = objects.begin(); it != objects.end(); ++it)
        subcondition_non_matches.push_back(it->second);
    ObjectSet subcondition_matches;
    subcondition_matches.reserve(RESERVE_SET_SIZE);

    m_condition->Eval(local_context, subcondition_matches, subcondition_non_matches);

    return CanAddStarlaneConnectionSimpleMatch(subcondition_matches)(candidate);
}

///////////////////////////////////////////////////////////
// CanRemoveStarlaneConnection                           //
///////////////////////////////////////////////////////////
namespace {
    struct vertex_system_id_t {typedef boost::vertex_property_tag kind;}; ///< a system graph property map type

    struct GraphImpl {
        typedef boost::property<vertex_system_id_t, int,
                                boost::property<boost::vertex_index_t, int> >   vertex_property_t;  ///< a system graph property map type

        // declare main graph types, including properties declared above
        typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
                                      vertex_property_t> SystemGraph;

        // declare property map types for properties declared above
        typedef boost::property_map<SystemGraph, vertex_system_id_t>::const_type        ConstSystemIDPropertyMap;
        typedef boost::property_map<SystemGraph, vertex_system_id_t>::type              SystemIDPropertyMap;

        SystemGraph system_graph;   ///< a graph in which the systems are vertices and the starlanes are edges
    };

    // returns the \a graph index for system with id \a system_id
    template <class Graph>
    int SystemGraphIndex(const Graph& graph, int system_id)
    {
        typedef typename boost::property_map<Graph, vertex_system_id_t>::const_type ConstSystemIDPropertyMap;
        ConstSystemIDPropertyMap sys_id_property_map = boost::get(vertex_system_id_t(), graph);

        for (unsigned int i = 0; i < boost::num_vertices(graph); ++i) {
            const int loop_sys_id = sys_id_property_map[i];    // get system ID of this vertex
            if (loop_sys_id == system_id)
                return i;
        }

        Logger().errorStream() << "SystemGraphIndex cannot be found due to invalid system ID: " << system_id;
        return -1;
    }

    void InitializeStarlaneGraph(GraphImpl& starlane_graph_impl,
                                 const std::map<int, std::set<int> >& system_starlanes_map)
    {
        // clean up old contents of input graph
        for (int i = static_cast<int>(boost::num_vertices(starlane_graph_impl.system_graph)) - 1; i >= 0; --i) {
            boost::clear_vertex(i, starlane_graph_impl.system_graph);
            boost::remove_vertex(i, starlane_graph_impl.system_graph);
        }

        GraphImpl::SystemIDPropertyMap sys_id_property_map =
            boost::get(vertex_system_id_t(), starlane_graph_impl.system_graph);

        std::map<int, int> system_id_graph_index_reverse_lookup_map;    // key is system ID, value is index in starlane_graph_impl.system_graph of system's vertex

        // add vertices
        unsigned int i = 0;
        for (std::map<int, std::set<int> >::const_iterator map_it = system_starlanes_map.begin();
             map_it != system_starlanes_map.end(); ++map_it, ++i)
        {
            int system_id = map_it->first;

            // add a vertex to the graph for this system, and assign it the system's universe ID as a property
            boost::add_vertex(starlane_graph_impl.system_graph);
            sys_id_property_map[i] = system_id;
            // add record of index in starlane_graph_impl.system_graph of this system
            system_id_graph_index_reverse_lookup_map[system_id] = i;
        }

        // add edges
        for (std::map<int, std::set<int> >::const_iterator map_it = system_starlanes_map.begin();
             map_it != system_starlanes_map.end(); ++map_it)
        {
            int system1_id = map_it->first;
            // find lane start graph index
            std::map<int, int>::const_iterator lookup1_it = system_id_graph_index_reverse_lookup_map.find(system1_id);
            if (lookup1_it == system_id_graph_index_reverse_lookup_map.end())
                continue;
            int system1_index = lookup1_it->second;
            // add edge for each valid starlane
            const std::set<int>& system_starlanes = map_it->second;
            for (std::set<int>::const_iterator lane_it = system_starlanes.begin();
                 lane_it != system_starlanes.end(); ++lane_it)
            {
                int system2_id = *lane_it;
                // skip null lanes
                if (system2_id == system1_id)
                    continue;
                // find lane end graph index
                std::map<int, int>::const_iterator lookup2_it = system_id_graph_index_reverse_lookup_map.find(system2_id);
                if (lookup2_it == system_id_graph_index_reverse_lookup_map.end())
                    continue;
                int system2_index = lookup2_it->second;
                // add edge between vertices
                boost::add_edge(system1_index, system2_index, starlane_graph_impl.system_graph);
            }
        }
    }

    /** Returns true or false to indicate whether the systems with ids
      * \a system1_id and system2_id are connected by a series of starlanes. */
    template <class Graph>
    bool SystemsConnected(const Graph& graph, int system1_id, int system2_id)
    {
        if (system1_id == system2_id)
            return true;    // early exit if systems are the same

        int system1_index = SystemGraphIndex(graph, system1_id);
        int system2_index = SystemGraphIndex(graph, system2_id);
        if (system1_index == -1 || system2_index == -1) {
            Logger().errorStream() << "Couldn't find valid graph index for systems " << system1_id << " or " << system2_id;
            return false;
        }

        try {
            return boost::graph::st_connected(graph, system1_index, system2_index);
        } catch(...) {
            Logger().errorStream() << "Error checking graph connectivity in Condition";
        }
        return false;
    }
}

Condition::CanRemoveStarlaneConnection::~CanRemoveStarlaneConnection()
{ delete m_condition; }

bool Condition::CanRemoveStarlaneConnection::RootCandidateInvariant() const
{ return m_condition->RootCandidateInvariant(); }

bool Condition::CanRemoveStarlaneConnection::TargetInvariant() const
{ return m_condition->TargetInvariant(); }

std::string Condition::CanRemoveStarlaneConnection::Description(bool negated/* = false*/) const {
    std::string description_str = "DESC_CAN_REMOVE_STARLANE_CONNECTION";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str)) % m_condition->Description());
}

std::string Condition::CanRemoveStarlaneConnection::Dump() const {
    std::string retval = DumpIndent() + "CanRemoveStarlaneConnection condition =\n";
    ++g_indent;
        retval += m_condition->Dump();
    --g_indent;
    return retval;
}

bool Condition::CanRemoveStarlaneConnection::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "CanRemoveStarlaneConnection::Match passed no candidate object";
        return false;
    }

    // get system for candidate object
    int candidate_system_id = candidate->SystemID();
    const System* candidate_system = GetSystem(candidate_system_id);
    if (!candidate_system)
        return false;

    // get objects to be considering for matching against subcondition
    ObjectSet subcondition_non_matches;
    subcondition_non_matches.reserve(RESERVE_SET_SIZE);
    for (ObjectMap::const_iterator it = Objects().const_begin(); it != Objects().const_end(); ++it)
        subcondition_non_matches.push_back(it->second);
    ObjectSet subcondition_matches;
    subcondition_matches.reserve(RESERVE_SET_SIZE);

    m_condition->Eval(local_context, subcondition_matches, subcondition_non_matches);

    if (subcondition_matches.empty())
        return false;

    // assemble all systems in or containing objects in subcondition matches
    std::set<const System*> destination_systems;
    for (ObjectSet::const_iterator it = subcondition_matches.begin(); it != subcondition_matches.end(); ++it)
        if (const System* system = GetSystem((*it)->SystemID()))
            destination_systems.insert(system);

    if (destination_systems.empty())
        return false;

    // does the candidate object have starlanes to all systems containing subcondition matches?
    for (std::set<const System*>::const_iterator it = destination_systems.begin(); it != destination_systems.end(); ++it) {
        if (!candidate_system->HasStarlaneTo((*it)->ID()) || !(*it)->HasStarlaneTo(candidate_system->ID()))
            return false;
    }

    // create starlane graph, fill with system info, exclusing lanes to
    // subcondition-matches from candidate
    std::map<int, std::set<int> > system_lanes;
    const ObjectMap& objects = Objects();
    std::vector<const System*> systems = objects.FindObjects<System>();

    //for (std::vector<const System*>::const_iterator system_it = systems.begin(); system_it != systems.end(); ++system_it) {
    //    const System* loop_system = *system_it;
    //    int loop_system_id = loop_system->ID();
    //    system_lanes[loop_system_id];   // ensure there's at least an empty lane set for this system

    //    // Add all lanes for each system to system_lanes map, except for lanes
    //    // connecting between the candidate and any subcondition matcing system
    //    // (and also lanes from subcondition matches to the candidate system)
    //    for (System::const_lane_iterator lane_it = loop_system->begin_lanes(); lane_it != loop_system->end_lanes(); ++lane_it) {
    //        if (loop_system_id != candidate_system_id ||
    //            destination_systems.find(*lane_it) == destination_systems.end() ||
    //            false)
    //        {
    //            system_lanes[loop_system_id].insert(*lane_it);
    //        }
    //    }
    //}

    GraphImpl graph_impl;
    InitializeStarlaneGraph(graph_impl, system_lanes);

    // In the starlanes graph that excludes the starlanes being tested, is
    // connectivity still preserved between systems connected by those starlanes?
    // To test, check if any of those pairs of systems are disconnected in the
    // test graph.
    for (std::set<const System*>::const_iterator it = destination_systems.begin(); it != destination_systems.end(); ++it) {}

    return true;
}

///////////////////////////////////////////////////////////
// ExploredByEmpire                                      //
///////////////////////////////////////////////////////////
Condition::ExploredByEmpire::~ExploredByEmpire()
{ delete m_empire_id; }

namespace {
    struct ExploredByEmpireSimpleMatch {
        ExploredByEmpireSimpleMatch(int empire_id) :
            m_empire_id(empire_id)
        {}

        bool operator()(const UniverseObject* candidate) const {
            if (!candidate)
                return false;

            const Empire* empire = Empires().Lookup(m_empire_id);
            if (!empire)
                return false;
            return empire->HasExploredSystem(candidate->ID());
        }

        int m_empire_id;
    };
}

void Condition::ExploredByEmpire::Eval(const ScriptingContext& parent_context, ObjectSet& matches, ObjectSet& non_matches,
                                       SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = ValueRef::ConstantExpr(m_empire_id) ||
                            (m_empire_id->LocalCandidateInvariant() &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate empire id once, and use to check all candidate objects
        const UniverseObject* no_object(0);
        int empire_id = m_empire_id->Eval(ScriptingContext(parent_context, no_object));
        EvalImpl(matches, non_matches, search_domain, ExploredByEmpireSimpleMatch(empire_id));
    } else {
        // re-evaluate empire id for each candidate object
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::ExploredByEmpire::RootCandidateInvariant() const
{ return m_empire_id->RootCandidateInvariant(); }

bool Condition::ExploredByEmpire::TargetInvariant() const
{ return m_empire_id->TargetInvariant(); }

std::string Condition::ExploredByEmpire::Description(bool negated/* = false*/) const {
    std::string empire_str;
    if (m_empire_id) {
        int empire_id = ALL_EMPIRES;
        if (ValueRef::ConstantExpr(m_empire_id))
            empire_id = m_empire_id->Eval();
        if (const Empire* empire = Empires().Lookup(empire_id))
            empire_str = empire->Name();
        else
            empire_str = m_empire_id->Description();
    }

    std::string description_str = "DESC_EXPLORED_BY_EMPIRE";
    if (negated)
        description_str += "_NOT";

    return str(FlexibleFormat(UserString(description_str))
               % empire_str);
}

std::string Condition::ExploredByEmpire::Dump() const
{ return DumpIndent() + "ExploredByEmpire empire_id = " + m_empire_id->Dump(); }

bool Condition::ExploredByEmpire::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "ExploredByEmpire::Match passed no candidate object";
        return false;
    }

    return ExploredByEmpireSimpleMatch(m_empire_id->Eval(local_context))(candidate);
}

///////////////////////////////////////////////////////////
// Stationary                                      //
///////////////////////////////////////////////////////////
std::string Condition::Stationary::Description(bool negated/* = false*/) const {
    std::string description_str = "DESC_STATIONARY";
    if (negated)
        description_str += "_NOT";
    return UserString(description_str);
}

std::string Condition::Stationary::Dump() const
{ return DumpIndent() + "Stationary\n"; }

bool Condition::Stationary::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "Stationary::Match passed no candidate object";
        return false;
    }

    // the only objects that can move are fleets and the ships in them.  so,
    // attempt to cast the candidate object to a fleet or ship, and if it's a ship
    // get the fleet of that ship
    const Fleet* fleet = universe_object_cast<const Fleet*>(candidate);
    if (!fleet)
        if (const Ship* ship = universe_object_cast<const Ship*>(candidate))
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

///////////////////////////////////////////////////////////
// FleetSupplyableByEmpire                               //
///////////////////////////////////////////////////////////
Condition::FleetSupplyableByEmpire::~FleetSupplyableByEmpire()
{ delete m_empire_id; }

namespace {
    struct FleetSupplyableSimpleMatch {
        FleetSupplyableSimpleMatch(int empire_id) :
            m_empire_id(empire_id)
        {}

        bool operator()(const UniverseObject* candidate) const {
            if (!candidate)
                return false;

            const EmpireManager& empires = Empires();
            const Empire* empire = empires.Lookup(m_empire_id);
            if (!empire)
                return false;

            const std::set<int>& supplyable_systems = empire->FleetSupplyableSystemIDs();
            return supplyable_systems.find(candidate->SystemID()) != supplyable_systems.end();
        }

        int m_empire_id;
    };
}

void Condition::FleetSupplyableByEmpire::Eval(const ScriptingContext& parent_context, ObjectSet& matches, ObjectSet& non_matches,
                                              SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = ValueRef::ConstantExpr(m_empire_id) ||
                            (m_empire_id->LocalCandidateInvariant() &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate empire id once, and use to check all candidate objects
        const UniverseObject* no_object(0);
        int empire_id = m_empire_id->Eval(ScriptingContext(parent_context, no_object));
        EvalImpl(matches, non_matches, search_domain, FleetSupplyableSimpleMatch(empire_id));
    } else {
        // re-evaluate empire id for each candidate object
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::FleetSupplyableByEmpire::RootCandidateInvariant() const
{ return m_empire_id->RootCandidateInvariant(); }

bool Condition::FleetSupplyableByEmpire::TargetInvariant() const
{ return m_empire_id->TargetInvariant(); }

std::string Condition::FleetSupplyableByEmpire::Description(bool negated/* = false*/) const {
    std::string empire_str;
    if (m_empire_id) {
        int empire_id = ALL_EMPIRES;
        if (ValueRef::ConstantExpr(m_empire_id))
            empire_id = m_empire_id->Eval();
        if (const Empire* empire = Empires().Lookup(empire_id))
            empire_str = empire->Name();
        else
            empire_str = m_empire_id->Description();
    }

    std::string description_str = "DESC_SUPPLY_CONNECTED_FLEET";
    if (negated)
        description_str += "_NOT";

    return str(FlexibleFormat(UserString(description_str))
               % empire_str);
}

std::string Condition::FleetSupplyableByEmpire::Dump() const
{ return DumpIndent() + "ResupplyableBy empire_id = " + m_empire_id->Dump(); }

bool Condition::FleetSupplyableByEmpire::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "FleetSupplyableByEmpire::Match passed no candidate object";
        return false;
    }

    int empire_id = m_empire_id->Eval(local_context);

    return FleetSupplyableSimpleMatch(empire_id)(candidate);
}

///////////////////////////////////////////////////////////
// ResourceSupplyConnectedByEmpire                       //
///////////////////////////////////////////////////////////
Condition::ResourceSupplyConnectedByEmpire::~ResourceSupplyConnectedByEmpire() {
    delete m_empire_id;
    delete m_condition;
}

namespace {
    struct ResourceSupplySimpleMatch {
        ResourceSupplySimpleMatch(int empire_id, const Condition::ObjectSet& from_objects) :
            m_empire_id(empire_id),
            m_from_objects(from_objects)
        {}

        bool operator()(const UniverseObject* candidate) const {
            if (!candidate)
                return false;
            if (m_from_objects.empty())
                return false;
            const Empire* empire = Empires().Lookup(m_empire_id);
            if (!empire)
                return false;
            const std::set<std::set<int> >& groups = empire->ResourceSupplyGroups();

            // is candidate object connected to a subcondition matching object by resource supply?
            for (Condition::ObjectSet::const_iterator it = m_from_objects.begin(); it != m_from_objects.end(); ++it) {
                const UniverseObject* from_object(*it);

                for (std::set<std::set<int> >::const_iterator groups_it = groups.begin(); groups_it != groups.end(); ++groups_it) {
                    const std::set<int>& group = *groups_it;
                    if (group.find(from_object->SystemID()) != group.end()) {
                        // found resource sharing group containing test object.  Does it also contain candidate?
                        if (group.find(candidate->SystemID()) != group.end())
                            return true;    // test object and candidate object are in same resourse sharing group
                        else
                            return false;   // test object is not in resource sharing group with candidate (each object can be in only one group)
                    }
                    // current subcondition-matching object is not in this resource sharing group
                }
                // current subcondition-matching object is not in any resource sharing group for this empire
            }

            return false;
        }

        int m_empire_id;
        const Condition::ObjectSet& m_from_objects;
    };
}

void Condition::ResourceSupplyConnectedByEmpire::Eval(const ScriptingContext& parent_context, ObjectSet& matches,
                                                      ObjectSet& non_matches, SearchDomain search_domain/* = NON_MATCHES*/) const
{
    bool simple_eval_safe = ValueRef::ConstantExpr(m_empire_id) ||
                            (m_empire_id->LocalCandidateInvariant() &&
                            (parent_context.condition_root_candidate || RootCandidateInvariant()));
    if (simple_eval_safe) {
        // evaluate contained objects once and check for all candidates
        ObjectMap& objects = GetUniverse().Objects();

        const UniverseObject* no_object(0);
        ScriptingContext local_context(parent_context, no_object);

        // get objects to be considering for matching against subcondition
        ObjectSet subcondition_non_matches;
        subcondition_non_matches.reserve(RESERVE_SET_SIZE);
        for (ObjectMap::iterator it = objects.begin(); it != objects.end(); ++it)
            subcondition_non_matches.push_back(it->second);
        ObjectSet subcondition_matches;
        subcondition_matches.reserve(RESERVE_SET_SIZE);
        m_condition->Eval(local_context, subcondition_matches, subcondition_non_matches);

        int empire_id = m_empire_id->Eval(local_context);

        EvalImpl(matches, non_matches, search_domain, ResourceSupplySimpleMatch(empire_id, subcondition_matches));
    } else {
        // re-evaluate empire id for each candidate object
        Condition::ConditionBase::Eval(parent_context, matches, non_matches, search_domain);
    }
}

bool Condition::ResourceSupplyConnectedByEmpire::RootCandidateInvariant() const
{ return m_empire_id->RootCandidateInvariant() && m_condition->RootCandidateInvariant(); }

bool Condition::ResourceSupplyConnectedByEmpire::TargetInvariant() const
{ return m_empire_id->TargetInvariant() && m_condition->TargetInvariant(); }

bool Condition::ResourceSupplyConnectedByEmpire::Match(const ScriptingContext& local_context) const {
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "ResourceSupplyConnectedByEmpire::Match passed no candidate object";
        return false;
    }

    ObjectMap& objects = GetUniverse().Objects();

    // get objects to be considering for matching against subcondition
    ObjectSet subcondition_non_matches;
    subcondition_non_matches.reserve(RESERVE_SET_SIZE);
    for (ObjectMap::iterator it = objects.begin(); it != objects.end(); ++it)
        subcondition_non_matches.push_back(it->second);
    ObjectSet subcondition_matches;
    subcondition_matches.reserve(RESERVE_SET_SIZE);

    m_condition->Eval(local_context, subcondition_matches, subcondition_non_matches);
    int empire_id = m_empire_id->Eval(local_context);

    return ResourceSupplySimpleMatch(empire_id, subcondition_matches)(candidate);
}

std::string Condition::ResourceSupplyConnectedByEmpire::Description(bool negated/* = false*/) const {
    std::string empire_str;
    if (m_empire_id) {
        int empire_id = ALL_EMPIRES;
        if (ValueRef::ConstantExpr(m_empire_id))
            empire_id = m_empire_id->Eval();
        if (const Empire* empire = Empires().Lookup(empire_id))
            empire_str = empire->Name();
        else
            empire_str = m_empire_id->Description();
    }

    std::string description_str = "DESC_SUPPLY_CONNECTED_RESOURCE";
    if (negated)
        description_str += "_NOT";

    return str(FlexibleFormat(UserString(description_str))
               % empire_str
               % m_condition->Description());
}

std::string Condition::ResourceSupplyConnectedByEmpire::Dump() const {
    std::string retval = DumpIndent() + "ResourceSupplyConnectedBy empire_id = " + m_empire_id->Dump() +
                                        " condition = \n";
    ++g_indent;
    retval += m_condition->Dump();
    --g_indent;
    return retval;
}

///////////////////////////////////////////////////////////
// And                                                   //
///////////////////////////////////////////////////////////
Condition::And::~And() {
    for (unsigned int i = 0; i < m_operands.size(); ++i)
        delete m_operands[i];
}

void Condition::And::Eval(const ScriptingContext& parent_context, ObjectSet& matches, ObjectSet& non_matches,
                          SearchDomain search_domain/* = NON_MATCHES*/) const
{
    const UniverseObject* no_object(0);
    ScriptingContext local_context(parent_context, no_object);

    if (search_domain == NON_MATCHES) {
        ObjectSet partly_checked_non_matches;
        partly_checked_non_matches.reserve(RESERVE_SET_SIZE);

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

        for (unsigned int i = 0; i < m_operands.size(); ++i) {
            if (matches.empty()) break;
            m_operands[i]->Eval(local_context, matches, non_matches, MATCHES);
        }

        // items already in non_matches set are not checked, and remain in non_matches set
        // even if they pass all operand conditions
    }
}

bool Condition::And::RootCandidateInvariant() const {
    for (std::vector<const ConditionBase*>::const_iterator it = m_operands.begin(); it != m_operands.end(); ++it)
        if (!(*it)->RootCandidateInvariant())
            return false;
    return true;
}

bool Condition::And::TargetInvariant() const {
    for (std::vector<const ConditionBase*>::const_iterator it = m_operands.begin(); it != m_operands.end(); ++it)
        if (!(*it)->TargetInvariant())
            return false;
    return true;
}

std::string Condition::And::Description(bool negated/* = false*/) const {
    if (m_operands.size() == 1) {
        return m_operands[0]->Description();
    } else {
        // TODO: use per-operand-type connecting language
        std::string values_str;
        for (unsigned int i = 0; i < m_operands.size(); ++i) {
            values_str += m_operands[i]->Description();
            if (i != m_operands.size() - 1) {
                values_str += UserString("DESC_AND_BETWEEN_OPERANDS");
            }
        }
        return values_str;
    }
}

std::string Condition::And::Dump() const {
    std::string retval = DumpIndent() + "And [\n";
    ++g_indent;
    for (unsigned int i = 0; i < m_operands.size(); ++i) {
        retval += m_operands[i]->Dump();
    }
    --g_indent;
    retval += "\n" + DumpIndent() + "]\n";
    return retval;
}

///////////////////////////////////////////////////////////
// Or                                                    //
///////////////////////////////////////////////////////////
Condition::Or::~Or() {
    for (unsigned int i = 0; i < m_operands.size(); ++i)
        delete m_operands[i];
}

void Condition::Or::Eval(const ScriptingContext& parent_context, ObjectSet& matches, ObjectSet& non_matches,
                         SearchDomain search_domain/* = NON_MATCHES*/) const
{
    const UniverseObject* no_object(0);
    ScriptingContext local_context(parent_context, no_object);

    if (search_domain == NON_MATCHES) {
        // check each item in the non-matches set against each of the operand conditions
        // if a non-candidate item matches an operand condition, move the item to the
        // matches set.

        for (unsigned int i = 0; i < m_operands.size(); ++i) {
            if (non_matches.empty()) break;
            m_operands[i]->Eval(local_context, matches, non_matches, NON_MATCHES);
        }

        // items already in matches set are not checked and remain in the
        // matches set even if they fail all the operand conditions

    } else {
        ObjectSet partly_checked_matches;
        partly_checked_matches.reserve(RESERVE_SET_SIZE);

        // move items in matches set the fail the first operand condition into 
        // partly_checked_matches set
        m_operands[0]->Eval(local_context, matches, partly_checked_matches, MATCHES);

        // move items that pass any of the other conditions back into matches
        for (unsigned int i = 1; i < m_operands.size(); ++i) {
            if (partly_checked_matches.empty()) break;
            m_operands[i]->Eval(local_context, matches, partly_checked_matches, NON_MATCHES);
        }

        // merge items that failed all operand conditions into non_matches
        non_matches.insert(non_matches.end(), partly_checked_matches.begin(), partly_checked_matches.end());

        // items already in non_matches set are not checked and remain in
        // non_matches set even if they pass one or more of the operand 
        // conditions
    }
}

bool Condition::Or::RootCandidateInvariant() const {
    for (std::vector<const ConditionBase*>::const_iterator it = m_operands.begin(); it != m_operands.end(); ++it)
        if (!(*it)->RootCandidateInvariant())
            return false;
    return true;
}

bool Condition::Or::TargetInvariant() const {
    for (std::vector<const ConditionBase*>::const_iterator it = m_operands.begin(); it != m_operands.end(); ++it)
        if (!(*it)->TargetInvariant())
            return false;
    return true;
}

std::string Condition::Or::Description(bool negated/* = false*/) const {
    if (m_operands.size() == 1) {
        return m_operands[0]->Description();
    } else {
        // TODO: use per-operand-type connecting language
        std::string values_str;
        for (unsigned int i = 0; i < m_operands.size(); ++i) {
            values_str += m_operands[i]->Description();
            if (i != m_operands.size() - 1) {
                values_str += UserString("DESC_OR_BETWEEN_OPERANDS");
            }
        }
        return values_str;
    }
}

std::string Condition::Or::Dump() const {
    std::string retval = DumpIndent() + "Or [\n";
    ++g_indent;
    for (unsigned int i = 0; i < m_operands.size(); ++i) {
        retval += m_operands[i]->Dump();
    }
    --g_indent;
    retval += "\n" + DumpIndent() + "]\n";
    return retval;
}

///////////////////////////////////////////////////////////
// Not                                                   //
///////////////////////////////////////////////////////////
Condition::Not::~Not()
{ delete m_operand; }

void Condition::Not::Eval(const ScriptingContext& parent_context, ObjectSet& matches, ObjectSet& non_matches,
                          SearchDomain search_domain/* = NON_MATCHES*/) const
{
    const UniverseObject* no_object(0);
    ScriptingContext local_context(parent_context, no_object);

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

bool Condition::Not::RootCandidateInvariant() const
{ return m_operand->RootCandidateInvariant(); }

bool Condition::Not::TargetInvariant() const
{ return m_operand->TargetInvariant(); }

std::string Condition::Not::Description(bool negated/* = false*/) const
{ return m_operand->Description(true); }

std::string Condition::Not::Dump() const {
    std::string retval = DumpIndent() + "Not\n";
    ++g_indent;
    retval += m_operand->Dump();
    --g_indent;
    return retval;
}
