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
    const Fleet* FleetFromObject(const UniverseObject* obj)
    {
        const ObjectMap& objects = GetMainObjectMap();
        const Fleet* retval = universe_object_cast<const Fleet*>(obj);
        if (!retval) {
            if (const Ship* ship = universe_object_cast<const Ship*>(obj))
                retval = objects.Object<Fleet>(ship->FleetID());
        }
        return retval;
    }
}

///////////////////////////////////////////////////////////
// Condition::ConditionBase                              //
///////////////////////////////////////////////////////////
Condition::ConditionBase::ConditionBase()
{}

Condition::ConditionBase::~ConditionBase()
{}

void Condition::ConditionBase::Eval(const ScriptingContext& parent_context, ObjectSet& matches, ObjectSet& non_matches, SearchDomain search_domain/* = NON_MATCHES*/) const
{
    ObjectSet& from_set = search_domain == MATCHES ? matches : non_matches;
    ObjectSet& to_set = search_domain == MATCHES ? non_matches : matches;
    ObjectSet::iterator it = from_set.begin();
    ObjectSet::iterator end_it = from_set.end();
    for ( ; it != end_it; ) {
        ObjectSet::iterator temp = it++;
        bool match = Match(ScriptingContext(parent_context, *temp));
        if ((search_domain == MATCHES && !match) || (search_domain == NON_MATCHES && match)) {
            to_set.insert(*temp);
            from_set.erase(temp);
        }
    }
}

void Condition::ConditionBase::Eval(ObjectSet& matches, ObjectSet& non_matches, SearchDomain search_domain/* = NON_MATCHES*/) const
{
    Eval(ScriptingContext(), matches, non_matches, search_domain);
}

std::string Condition::ConditionBase::Description(bool negated/* = false*/) const
{
    return "";
}

std::string Condition::ConditionBase::Dump() const
{
    return "";
}

bool Condition::ConditionBase::Match(const ScriptingContext& local_context) const
{
    return false;
}

///////////////////////////////////////////////////////////
// Number                                                //
///////////////////////////////////////////////////////////
Condition::Number::Number(const ValueRef::ValueRefBase<int>* low, const ValueRef::ValueRefBase<int>* high, const ConditionBase* condition) :
    m_low(low),
    m_high(high),
    m_condition(condition)
{}

Condition::Number::~Number()
{
    delete m_low;
    delete m_high;
    delete m_condition;
}

std::string Condition::Number::Description(bool negated/* = false*/) const
{
    std::string low_str = ValueRef::ConstantExpr(m_low) ?
                            boost::lexical_cast<std::string>(m_low->Eval()) :
                            m_low->Description();
    std::string high_str = ValueRef::ConstantExpr(m_high) ?
                                boost::lexical_cast<std::string>(m_high->Eval()) :
                                m_high->Description();
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
    std::string retval = DumpIndent() + "Number low = " + m_low->Dump() + "Number high = " + m_high->Dump() + " condition =\n";
    ++g_indent;
    retval += m_condition->Dump();
    --g_indent;
    return retval;
}

void Condition::Number::Eval(const ScriptingContext& parent_context, ObjectSet& matches, ObjectSet& non_matches, SearchDomain search_domain/* = NON_MATCHES*/) const
{
    // Number does not have a single valid local candidate to be matched, as it
    // will match anything if the proper number of objects match the
    // subcondition.  So, the local context that is passed to the subcondition
    // needs to have a null local candidate.
    const UniverseObject* no_object(0);
    ScriptingContext local_context(parent_context, no_object);

    // get set of all UniverseObjects that satisfy m_condition
    ObjectSet condition_targets;
    ObjectSet condition_non_targets;
    ObjectMap& objects = GetUniverse().Objects();
    for (ObjectMap::iterator uit = objects.begin(); uit != objects.end(); ++uit) {
        condition_non_targets.insert(uit->second);
    }
    m_condition->Eval(local_context, condition_targets, condition_non_targets, NON_MATCHES);

    // compare number of objects that satisfy m_condition to the acceptable range of such objects
    int matched = condition_targets.size();
    int low = m_low->Eval(local_context);
    int high = m_high->Eval(local_context);
    bool in_range = (low <= matched && matched < high);

    // transfer objects to or from candidate set, according to whether number of matches was within
    // the requested range.
    if (search_domain == MATCHES && !in_range) {
        non_matches.insert(matches.begin(), matches.end());
        matches.clear();
    }
    if (search_domain == NON_MATCHES && in_range) {
        matches.insert(non_matches.begin(), non_matches.end());
        non_matches.clear();
    }
}

///////////////////////////////////////////////////////////
// Turn                                                  //
///////////////////////////////////////////////////////////
Condition::Turn::Turn(const ValueRef::ValueRefBase<int>* low, const ValueRef::ValueRefBase<int>* high) :
    m_low(low),
    m_high(high)
{}

Condition::Turn::~Turn()
{
    delete m_low;
    delete m_high;
}

std::string Condition::Turn::Description(bool negated/* = false*/) const
{
    std::string low_str = ValueRef::ConstantExpr(m_low) ?
                            boost::lexical_cast<std::string>(m_low->Eval()) :
                            m_low->Description();
    std::string high_str = ValueRef::ConstantExpr(m_high) ?
                            boost::lexical_cast<std::string>(m_high->Eval()) :
                            m_high->Description();
    std::string description_str = "DESC_TURN";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str))
               % low_str
               % high_str);
}

std::string Condition::Turn::Dump() const
{
    return DumpIndent() + "Turn low = " + m_low->Dump() + " high = " + m_high->Dump() + "\n";
}

bool Condition::Turn::Match(const ScriptingContext& local_context) const
{
    double low = std::max(0, m_low->Eval(local_context));
    double high = std::min(m_high->Eval(local_context), IMPOSSIBLY_LARGE_TURN);
    int turn = CurrentTurn();

    return (low <= turn && turn < high);
}

///////////////////////////////////////////////////////////
// SortedNumberOf                                        //
///////////////////////////////////////////////////////////
Condition::SortedNumberOf::SortedNumberOf(const ValueRef::ValueRefBase<int>* number,
                                          const ConditionBase* condition) :
    m_number(number),
    m_sort_key(0),
    m_sorting_method(Condition::SORT_RANDOM),
    m_condition(condition)
{}

Condition::SortedNumberOf::SortedNumberOf(const ValueRef::ValueRefBase<int>* number,
                                          const ValueRef::ValueRefBase<double>* sort_key,
                                          SortingMethod sorting_method,
                                          const ConditionBase* condition) :
    m_number(number),
    m_sort_key(sort_key),
    m_sorting_method(sorting_method),
    m_condition(condition)
{}

Condition::SortedNumberOf::~SortedNumberOf()
{
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
        for (Condition::ObjectSet::iterator it = from_set.begin() ; it != from_set.end(); ++i) {
            Condition::ObjectSet::iterator temp = it++;
            if (transfer_flags[i]) {
                to_set.insert(*temp);
                from_set.erase(temp);
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
                Condition::ObjectSet::iterator from_it = from_set.find(object_to_transfer);
                if (from_it != from_set.end()) {
                    from_set.erase(from_it);
                    to_set.insert(object_to_transfer);
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
                Condition::ObjectSet::iterator from_it = from_set.find(object_to_transfer);
                if (from_it != from_set.end()) {
                    from_set.erase(from_it);
                    to_set.insert(object_to_transfer);
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
                for (std::multimap<double, const UniverseObject*>::const_iterator sorted_it = sort_key_objects.begin();
                     sorted_it != sort_key_objects.end(); ++sorted_it)
                {
                    const UniverseObject* object_to_transfer = sorted_it->second;
                    Condition::ObjectSet::iterator from_it = from_set.find(object_to_transfer);
                    if (from_it != from_set.end()) {
                        from_set.erase(from_it);
                        to_set.insert(object_to_transfer);
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
    ObjectSet subcondition_matching_targets;
    m_condition->Eval(local_context, subcondition_matching_targets, matches, NON_MATCHES);

    // remaining input matches don't match the subcondition...
    ObjectSet subcondition_non_matching_targets = matches;
    matches.clear();    // to be refilled later

    // which input non_matches match the subcondition?
    ObjectSet subcondition_matching_non_targets;
    m_condition->Eval(local_context, subcondition_matching_non_targets, non_matches, NON_MATCHES);

    // remaining input non_matches don't match the subcondition...
    ObjectSet subcondition_non_matching_non_targets = non_matches;
    non_matches.clear();    // to be refilled later

    // assemble single set of subcondition matching objects
    ObjectSet all_subcondition_matches = subcondition_matching_targets;
    all_subcondition_matches.insert(subcondition_matching_non_targets.begin(), subcondition_matching_non_targets.end());

    // how many subcondition matches to select as matches to this condition
    int number = m_number->Eval(local_context);

    // compile single set of all objects that are matched by this condition.
    // these are the objects that should be transferred from non_matches into
    // matches, or those left in matches while the rest are moved into non_matches
    ObjectSet matched_objects;
    TransferSortedObjects(number, m_sort_key, local_context, m_sorting_method, all_subcondition_matches, matched_objects);

    // put objects back into matches and non_target sets as output...

    if (search_domain == NON_MATCHES) {
        // put matched objects that are in subcondition_matching_non_targets into matches
        for (ObjectSet::const_iterator match_it = matched_objects.begin(); match_it != matched_objects.end(); ++match_it) {
            const UniverseObject* matched_object = *match_it;

            // is this matched object in subcondition_matching_non_targets?
            ObjectSet::iterator smnt_it = subcondition_matching_non_targets.find(matched_object);
            if (smnt_it != subcondition_matching_non_targets.end()) {
                // yes; move object to matches
                subcondition_matching_non_targets.erase(smnt_it);
                matches.insert(matched_object);
            }
        }

        // put remaining (non-matched) objects in subcondition_matching_non_targets back into non_matches
        non_matches.insert( subcondition_matching_non_targets.begin(),      subcondition_matching_non_targets.end());
        // put objects in subcondition_non_matching_non_targets back into non_matches
        non_matches.insert( subcondition_non_matching_non_targets.begin(),  subcondition_non_matching_non_targets.end());
        // put objects in subcondition_matching_targets and subcondition_non_matching_targets back into matches
        matches.insert(     subcondition_matching_targets.begin(),          subcondition_matching_targets.end());
        matches.insert(     subcondition_non_matching_targets.begin(),      subcondition_non_matching_targets.end());
        // this leaves the original contents of matches unchanged, other than
        // possibly having transferred some objects into matches from non_matches

    } else { /*(search_domain == MATCHES)*/
        // put matched objecs that are in subcondition_matching_targets back into matches
        for (ObjectSet::const_iterator match_it = matched_objects.begin(); match_it != matched_objects.end(); ++match_it) {
            const UniverseObject* matched_object = *match_it;

            // is this matched object in subcondition_matching_targets?
            ObjectSet::iterator smt_it = subcondition_matching_targets.find(matched_object);
            if (smt_it != subcondition_matching_targets.end()) {
                // yes; move back into matches
                subcondition_matching_targets.erase(smt_it);
                matches.insert(matched_object);
            }
        }

        // put remaining (non-matched) objects in subcondition_matching_targets) into non_matches
        non_matches.insert( subcondition_matching_targets.begin(),          subcondition_matching_targets.end());
        // put objects in subcondition_non_matching_targets into non_matches
        non_matches.insert( subcondition_non_matching_targets.begin(),      subcondition_non_matching_targets.end());
        // put objects in subcondition_matching_non_targets and subcondition_non_matching_non_targets back into non_matches
        non_matches.insert( subcondition_matching_non_targets.begin(),      subcondition_matching_non_targets.end());
        non_matches.insert( subcondition_non_matching_non_targets.begin(),  subcondition_non_matching_non_targets.end());
        // this leaves the original contents of non_matches unchanged, other than
        // possibly having transferred some objects into non_matches from matches
    }
}

std::string Condition::SortedNumberOf::Description(bool negated/* = false*/) const
{
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

std::string Condition::SortedNumberOf::Dump() const
{
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
Condition::All::All()
{}

void Condition::All::Eval(const ScriptingContext& parent_context, ObjectSet& matches, ObjectSet& non_matches, SearchDomain search_domain/* = NON_MATCHES*/) const
{
    if (search_domain == NON_MATCHES) {
        // move all objects from non_matches to matches
        matches.insert(non_matches.begin(), non_matches.end());
        non_matches.clear();
    }
    // if search_comain is MATCHES, do nothing: all objects in matches set
    // match this condition, so should remain in mathes set
}

std::string Condition::All::Description(bool negated/* = false*/) const
{
    std::string description_str = "DESC_ALL";
    if (negated)
        description_str += "_NOT";
    return UserString(description_str);
}

std::string Condition::All::Dump() const
{
    return DumpIndent() + "All\n";
}

///////////////////////////////////////////////////////////
// EmpireAffiliation                                     //
///////////////////////////////////////////////////////////
Condition::EmpireAffiliation::EmpireAffiliation(const ValueRef::ValueRefBase<int>* empire_id, EmpireAffiliationType affiliation, bool exclusive) :
    m_empire_id(empire_id),
    m_affiliation(affiliation),
    m_exclusive(exclusive)
{}

Condition::EmpireAffiliation::~EmpireAffiliation()
{
    delete m_empire_id;
}

std::string Condition::EmpireAffiliation::Description(bool negated/* = false*/) const
{
    std::string value_str = ValueRef::ConstantExpr(m_empire_id) ?
                                Empires().Lookup(m_empire_id->Eval())->Name() :
                                m_empire_id->Description();
    if (m_affiliation == AFFIL_SELF) {
        std::string description_str = m_exclusive ? "DESC_EMPIRE_AFFILIATION_SELF_EXCLUSIVE" : "DESC_EMPIRE_AFFILIATION_SELF";
        if (negated)
            description_str += "_NOT";
        return str(FlexibleFormat(UserString(description_str)) % value_str);
    } else {
        std::string description_str = m_exclusive ? "DESC_EMPIRE_AFFILIATION_EXCLUSIVE" : "DESC_EMPIRE_AFFILIATION";
        if (negated)
            description_str += "_NOT";
        return str(FlexibleFormat(UserString(description_str))
                   % UserString(boost::lexical_cast<std::string>(m_affiliation))
                   % value_str);
    }
}

std::string Condition::EmpireAffiliation::Dump() const
{
    std::string retval = DumpIndent() + (m_exclusive ? "OwnedExclusivelyBy" : "OwnedBy");
    retval += " affiliation = ";
    switch (m_affiliation) {
    case AFFIL_SELF:  retval += "TheEmpire"; break;
    case AFFIL_ENEMY: retval += "EnemyOf"; break;
    case AFFIL_ALLY:  retval += "AllyOf"; break;
    default: retval += "?"; break;
    }
    retval += " empire = " + m_empire_id->Dump() + "\n";
    return retval;
}

bool Condition::EmpireAffiliation::Match(const ScriptingContext& local_context) const
{
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "EmpireAffiliation::Match passed no candidate object";
        return false;
    }

    int empire_id = m_empire_id->Eval(local_context);

    switch (m_affiliation) {
    case AFFIL_SELF:
        if (m_exclusive) {
            // candidate object owned only by specified empire
            return candidate->WhollyOwnedBy(empire_id);
        } else {
            // candidate object owned by specified empire, and possibly others
            return candidate->OwnedBy(empire_id);
        }
        break;
    case AFFIL_ENEMY:
        if (m_exclusive) {
            // candidate has an owner, but isn't owned by specified empire
            return (!candidate->Owners().empty() && !candidate->OwnedBy(empire_id));
        } else {
            // at least one of candidate's owners is not specified empire, but specified empire may also own candidate
            return (candidate->Owners().size() > 1 ||
                    (!candidate->Owners().empty() && !candidate->OwnedBy(empire_id))
                   );
        }
        break;
    case AFFIL_ALLY:
        // TODO
        break;
    default:
        break;
    }
    return false;
}

///////////////////////////////////////////////////////////
// Source                                                //
///////////////////////////////////////////////////////////
Condition::Source::Source()
{}

std::string Condition::Source::Description(bool negated/* = false*/) const
{
    std::string description_str = "DESC_SELF";
    if (negated)
        description_str += "_NOT";
    return UserString(description_str);
}

std::string Condition::Source::Dump() const
{
    return DumpIndent() + "Source\n";
}

bool Condition::Source::Match(const ScriptingContext& local_context) const
{
    if (!local_context.condition_local_candidate)
        return false;
    return local_context.source == local_context.condition_local_candidate;
}

///////////////////////////////////////////////////////////
// Target                                                //
///////////////////////////////////////////////////////////
Condition::Target::Target()
{}

std::string Condition::Target::Description(bool negated/* = false*/) const
{
    std::string description_str = "DESC_TARGET";
    if (negated)
        description_str += "_NOT";
    return UserString(description_str);
}

std::string Condition::Target::Dump() const
{
    return DumpIndent() + "Target\n";
}

bool Condition::Target::Match(const ScriptingContext& local_context) const
{
    if (!local_context.effect_target)
        return false;
    return local_context.effect_target == local_context.condition_local_candidate;
}

///////////////////////////////////////////////////////////
// Homeworld                                             //
///////////////////////////////////////////////////////////
Condition::Homeworld::Homeworld() :
    m_names()
{}

Condition::Homeworld::Homeworld(const std::vector<const ValueRef::ValueRefBase<std::string>*>& names) :
    m_names(names)
{}

Condition::Homeworld::~Homeworld()
{
    for (unsigned int i = 0; i < m_names.size(); ++i)
        delete m_names[i];
}

std::string Condition::Homeworld::Description(bool negated/* = false*/) const
{
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

std::string Condition::Homeworld::Dump() const
{
    std::string retval = DumpIndent() + "HomeWorld name = ";
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

bool Condition::Homeworld::Match(const ScriptingContext& local_context) const
{
    const ObjectMap& objects = GetMainObjectMap();

    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "Homeworld::Match passed no candidate object";
        return false;
    }

    // is it a planet or a building on a planet?
    const Planet* planet = universe_object_cast<const Planet*>(candidate);
    const ::Building* building = 0;
    if (!planet && (building = universe_object_cast<const ::Building*>(candidate))) {
        planet = objects.Object<Planet>(building->PlanetID());
    }
    if (!planet)
        return false;

    int planet_id = planet->ID();

    if (m_names.empty()) {
        // match homeworlds for any species
        const SpeciesManager& manager = GetSpeciesManager();
        for (SpeciesManager::iterator species_it = manager.begin(); species_it != manager.end(); ++species_it) {
            if (const ::Species* species = species_it->second) {
                const std::set<int>& homeworld_ids = species->Homeworlds();
                if (homeworld_ids.find(planet_id) != homeworld_ids.end())
                    return true;
            }
        }

    } else {
        // match any of the species specified
        for (unsigned int i = 0; i < m_names.size(); ++i) {
            std::string species_name = m_names[i]->Eval(local_context);
            if (const ::Species* species = GetSpecies(species_name)) {
                const std::set<int>& homeworld_ids = species->Homeworlds();
                if (homeworld_ids.find(planet_id) != homeworld_ids.end())
                    return true;
            }
        }
    }
    return false;
}

///////////////////////////////////////////////////////////
// Capital                                               //
///////////////////////////////////////////////////////////
Condition::Capital::Capital()
{}

std::string Condition::Capital::Description(bool negated/* = false*/) const
{
    std::string description_str = "DESC_CAPITAL";
    if (negated)
        description_str += "_NOT";
    return UserString(description_str);
}

std::string Condition::Capital::Dump() const
{
    return DumpIndent() + "Capital\n";
}

bool Condition::Capital::Match(const ScriptingContext& local_context) const
{
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
// Type                                                  //
///////////////////////////////////////////////////////////
Condition::Type::Type(const ValueRef::ValueRefBase<UniverseObjectType>* type) :
    m_type(type)
{}

Condition::Type::~Type()
{
    delete m_type;
}

std::string Condition::Type::Description(bool negated/* = false*/) const
{
    std::string value_str = ValueRef::ConstantExpr(m_type) ?
                                UserString(boost::lexical_cast<std::string>(m_type->Eval())) :
                                m_type->Description();
    std::string description_str = "DESC_TYPE";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str)) % value_str);
}

std::string Condition::Type::Dump() const
{
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

bool Condition::Type::Match(const ScriptingContext& local_context) const
{
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "Type::Match passed no candidate object";
        return false;
    }
    UniverseObjectType type = m_type->Eval(local_context);

    switch (type) {
    case OBJ_BUILDING:
        return universe_object_cast<const ::Building*>(candidate);
        break;
    case OBJ_SHIP:
        return universe_object_cast<const Ship*>(candidate);
        break;
    case OBJ_FLEET:
        return universe_object_cast<const Fleet*>(candidate);
        break;
    case OBJ_PLANET:
        return universe_object_cast<const Planet*>(candidate);
        break;
    case OBJ_POP_CENTER:
        return dynamic_cast<const PopCenter*>(candidate);
        break;
    case OBJ_PROD_CENTER:
        return dynamic_cast<const ResourceCenter*>(candidate);
        break;
    case OBJ_SYSTEM:
        return universe_object_cast<const System*>(candidate);
        break;
    default:
        break;
    }
    return false;
}

///////////////////////////////////////////////////////////
// Building                                              //
///////////////////////////////////////////////////////////
Condition::Building::Building(const std::vector<const ValueRef::ValueRefBase<std::string>*>& names) :
    m_names(names)
{}

Condition::Building::~Building()
{
    for (unsigned int i = 0; i < m_names.size(); ++i) {
        delete m_names[i];
    }
}

std::string Condition::Building::Description(bool negated/* = false*/) const
{
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

std::string Condition::Building::Dump() const
{
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

bool Condition::Building::Match(const ScriptingContext& local_context) const
{
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "Building::Match passed no candidate object";
        return false;
    }

    // is it a building?
    const ::Building* building = universe_object_cast<const ::Building*>(candidate);
    if (building) {
        for (unsigned int i = 0; i < m_names.size(); ++i) {
            if (m_names[i]->Eval(local_context) == building->BuildingTypeName())
                return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////
// HasSpecial                                            //
///////////////////////////////////////////////////////////
Condition::HasSpecial::HasSpecial(const std::string& name) :
    m_name(name)
{}

std::string Condition::HasSpecial::Description(bool negated/* = false*/) const
{
    std::string description_str = "DESC_SPECIAL";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str)) % UserString(m_name));
}

std::string Condition::HasSpecial::Dump() const
{
    return DumpIndent() + "HasSpecial name = \"" + m_name + "\"\n";
}

bool Condition::HasSpecial::Match(const ScriptingContext& local_context) const
{
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "HasSpecial::Match passed no candidate object";
        return false;
    }

    return (m_name == "All" && !candidate->Specials().empty()) ||
           (candidate->Specials().find(m_name) != candidate->Specials().end());
}

///////////////////////////////////////////////////////////
// Contains                                              //
///////////////////////////////////////////////////////////
Condition::Contains::Contains(const ConditionBase* condition) :
    m_condition(condition)
{}

Condition::Contains::~Contains()
{
    delete m_condition;
}

std::string Condition::Contains::Description(bool negated/* = false*/) const
{
    std::string description_str = "DESC_CONTAINS";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str)) % m_condition->Description());
}

std::string Condition::Contains::Dump() const
{
    std::string retval = DumpIndent() + "Contains condition =\n";
    ++g_indent;
    retval += m_condition->Dump();
    --g_indent;
    return retval;
}

bool Condition::Contains::Match(const ScriptingContext& local_context) const
{
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "Contains::Match passed no candidate object";
        return false;
    }

    ObjectMap& objects = GetUniverse().Objects();

    // get objects to be considering for matching against subcondition
    ObjectSet subcondition_non_matches;
    for (ObjectMap::iterator it = objects.begin(); it != objects.end(); ++it)
        subcondition_non_matches.insert(it->second);
    ObjectSet subcondition_matches;

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
Condition::ContainedBy::ContainedBy(const ConditionBase* condition) :
    m_condition(condition)
{}

Condition::ContainedBy::~ContainedBy()
{
    delete m_condition;
}

std::string Condition::ContainedBy::Description(bool negated/* = false*/) const
{
    std::string description_str = "DESC_CONTAINED_BY";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str)) % m_condition->Description());
}

std::string Condition::ContainedBy::Dump() const
{
    std::string retval = DumpIndent() + "ContainedBy condition =\n";
    ++g_indent;
        retval += m_condition->Dump();
    --g_indent;
    return retval;
}

bool Condition::ContainedBy::Match(const ScriptingContext& local_context) const
{
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "ContainedBy::Match passed no candidate object";
        return false;
    }

    ObjectMap& objects = GetUniverse().Objects();

    // get objects to be considering for matching against subcondition
    ObjectSet subcondition_non_matches;
    for (ObjectMap::iterator it = objects.begin(); it != objects.end(); ++it)
        subcondition_non_matches.insert(it->second);
    ObjectSet subcondition_matches;

    m_condition->Eval(local_context, subcondition_matches, subcondition_non_matches);

    // is candidate object contained by any subcondition matches?
    for (ObjectSet::iterator subcon_it = subcondition_matches.begin(); subcon_it != subcondition_matches.end(); ++subcon_it)
        if ((*subcon_it)->Contains(candidate->ID()))
            return true;

    return false;
}

///////////////////////////////////////////////////////////
// PlanetType                                            //
///////////////////////////////////////////////////////////
Condition::PlanetType::PlanetType(const std::vector<const ValueRef::ValueRefBase< ::PlanetType>*>& types) :
    m_types(types)
{}

Condition::PlanetType::~PlanetType()
{
    for (unsigned int i = 0; i < m_types.size(); ++i) {
        delete m_types[i];
    }
}

std::string Condition::PlanetType::Description(bool negated/* = false*/) const
{
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

std::string Condition::PlanetType::Dump() const
{
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

bool Condition::PlanetType::Match(const ScriptingContext& local_context) const
{
    const ObjectMap& objects = GetMainObjectMap();

    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "PlanetType::Match passed no candidate object";
        return false;
    }

    const Planet* planet = universe_object_cast<const Planet*>(candidate);
    const ::Building* building = 0;
    if (!planet && (building = universe_object_cast<const ::Building*>(candidate))) {
        planet = objects.Object<Planet>(building->PlanetID());
    }
    if (planet) {
        for (unsigned int i = 0; i < m_types.size(); ++i) {
            if (m_types[i]->Eval(ScriptingContext(local_context)) == planet->Type())
                return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////
// PlanetSize                                            //
///////////////////////////////////////////////////////////
Condition::PlanetSize::PlanetSize(const std::vector<const ValueRef::ValueRefBase< ::PlanetSize>*>& sizes) :
    m_sizes(sizes)
{}

Condition::PlanetSize::~PlanetSize()
{
    for (unsigned int i = 0; i < m_sizes.size(); ++i) {
        delete m_sizes[i];
    }
}

std::string Condition::PlanetSize::Description(bool negated/* = false*/) const
{
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

std::string Condition::PlanetSize::Dump() const
{
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

bool Condition::PlanetSize::Match(const ScriptingContext& local_context) const
{
    const ObjectMap& objects = GetMainObjectMap();

    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "PlanetSize::Match passed no candidate object";
        return false;
    }

    const Planet* planet = universe_object_cast<const Planet*>(candidate);
    const ::Building* building = 0;
    if (!planet && (building = universe_object_cast<const ::Building*>(candidate))) {
        planet = objects.Object<Planet>(building->PlanetID());
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
Condition::PlanetEnvironment::PlanetEnvironment(const std::vector<const ValueRef::ValueRefBase< ::PlanetEnvironment>*>& environments) :
    m_environments(environments)
{}

Condition::PlanetEnvironment::~PlanetEnvironment()
{
    for (unsigned int i = 0; i < m_environments.size(); ++i) {
        delete m_environments[i];
    }
}

std::string Condition::PlanetEnvironment::Description(bool negated/* = false*/) const
{
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

std::string Condition::PlanetEnvironment::Dump() const
{
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

bool Condition::PlanetEnvironment::Match(const ScriptingContext& local_context) const
{
    const ObjectMap& objects = GetMainObjectMap();

    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "PlanetEnvironment::Match passed no candidate object";
        return false;
    }

    const Planet* planet = universe_object_cast<const Planet*>(candidate);
    const ::Building* building = 0;
    if (!planet && (building = universe_object_cast<const ::Building*>(candidate))) {
        planet = objects.Object<Planet>(building->PlanetID());
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
Condition::Species::Species(const std::vector<const ValueRef::ValueRefBase<std::string>*>& names) :
    m_names(names)
{}

Condition::Species::~Species()
{
    for (unsigned int i = 0; i < m_names.size(); ++i) {
        delete m_names[i];
    }
}

std::string Condition::Species::Description(bool negated/* = false*/) const
{
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

std::string Condition::Species::Dump() const
{
    std::string retval = DumpIndent() + "Species name = ";
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

bool Condition::Species::Match(const ScriptingContext& local_context) const
{
    const ObjectMap& objects = GetMainObjectMap();

    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "Species::Match passed no candidate object";
        return false;
    }

    // is it a planet or a building on a planet?
    const Planet* planet = universe_object_cast<const Planet*>(candidate);
    const ::Building* building = 0;
    if (!planet && (building = universe_object_cast<const ::Building*>(candidate))) {
        planet = objects.Object<Planet>(building->PlanetID());
    }
    if (planet) {
        for (unsigned int i = 0; i < m_names.size(); ++i) {
            if (m_names[i]->Eval(local_context) == planet->SpeciesName())
                return true;
        }
    }
    // is it a ship?
    const Ship* ship = universe_object_cast<const Ship*>(candidate);
    if (ship) {
        for (unsigned int i = 0; i < m_names.size(); ++i) {
            if (m_names[i]->Eval(local_context) == ship->SpeciesName())
                return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////
// FocusType                                             //
///////////////////////////////////////////////////////////
Condition::FocusType::FocusType(const std::vector<const ValueRef::ValueRefBase<std::string>*>& names) :
    m_names(names)
{}

Condition::FocusType::~FocusType()
{
    for (unsigned int i = 0; i < m_names.size(); ++i) {
        delete m_names[i];
    }
}

std::string Condition::FocusType::Description(bool negated/* = false*/) const
{
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

std::string Condition::FocusType::Dump() const
{
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

bool Condition::FocusType::Match(const ScriptingContext& local_context) const
{
    const ObjectMap& objects = GetMainObjectMap();

    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "FocusType::Match passed no candidate object";
        return false;
    }

    // is it a ResourceCenter or a Building on a Planet (that is a ResourceCenter)
    const ResourceCenter* res_center = dynamic_cast<const ResourceCenter*>(candidate);
    const ::Building* building = 0;
    if (!res_center && (building = universe_object_cast<const ::Building*>(candidate))) {
        if (const Planet* planet = objects.Object<Planet>(building->PlanetID()))
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
Condition::StarType::StarType(const std::vector<const ValueRef::ValueRefBase< ::StarType>*>& types) :
    m_types(types)
{}

Condition::StarType::~StarType()
{
    for (unsigned int i = 0; i < m_types.size(); ++i) {
        delete m_types[i];
    }
}

std::string Condition::StarType::Description(bool negated/* = false*/) const
{
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

std::string Condition::StarType::Dump() const
{
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

bool Condition::StarType::Match(const ScriptingContext& local_context) const
{
    const ObjectMap& objects = GetMainObjectMap();

    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "StarType::Match passed no candidate object";
        return false;
    }

    const System* system = objects.Object<System>(candidate->SystemID());
    if (system || (system = universe_object_cast<const System*>(candidate))) {
        for (unsigned int i = 0; i < m_types.size(); ++i) {
            if (m_types[i]->Eval(local_context) == system->GetStarType())
                return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////
// DesignHasHull                                              //
///////////////////////////////////////////////////////////
Condition::DesignHasHull::DesignHasHull(const std::string& name) :
    m_name(name)
{}

std::string Condition::DesignHasHull::Description(bool negated/* = false*/) const
{
    std::string description_str = "DESC_DESIGN_HAS_HULL";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str)) % UserString(m_name));
}

std::string Condition::DesignHasHull::Dump() const
{
    return DumpIndent() + "DesignHasHull name = \"" + m_name + "\"\n";
}

bool Condition::DesignHasHull::Match(const ScriptingContext& local_context) const
{
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
Condition::DesignHasPart::DesignHasPart(const ValueRef::ValueRefBase<int>* low, const ValueRef::ValueRefBase<int>* high, const std::string& name) :
    m_low(low),
    m_high(high),
    m_name(name)
{}

Condition::DesignHasPart::~DesignHasPart()
{
    delete m_low;
    delete m_high;
}

std::string Condition::DesignHasPart::Description(bool negated/* = false*/) const
{
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

std::string Condition::DesignHasPart::Dump() const{
    return DumpIndent() + "DesignHasPart low = " + m_low->Dump() + " high = " + m_high->Dump() + " name = " + m_name;
}

bool Condition::DesignHasPart::Match(const ScriptingContext& local_context) const
{
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "DesignHasPart::Match passed no candidate object";
        return false;
    }

    if (const Ship* ship = universe_object_cast<const Ship*>(candidate)) {
        if (const ShipDesign* design = ship->Design()) {
            const std::vector<std::string>& parts = design->Parts();
            int count = 0;
            for (std::vector<std::string>::const_iterator it = parts.begin(); it != parts.end(); ++it)
                if (*it == m_name)
                    ++count;
            int low = m_low->Eval(local_context);
            int high = m_high->Eval(local_context);
            return (low <= count && count < high);
        }
    }
    return false;
}

///////////////////////////////////////////////////////////
// DesignHasPartClass                                    //
///////////////////////////////////////////////////////////
Condition::DesignHasPartClass::DesignHasPartClass(const ValueRef::ValueRefBase<int>* low, const ValueRef::ValueRefBase<int>* high, ShipPartClass part_class) :
    m_low(low),
    m_high(high),
    m_class(part_class)
{}

Condition::DesignHasPartClass::~DesignHasPartClass()
{
    delete m_low;
    delete m_high;
}

std::string Condition::DesignHasPartClass::Description(bool negated/* = false*/) const
{
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

std::string Condition::DesignHasPartClass::Dump() const{
    return DumpIndent() + "DesignHasPartClass low = " + m_low->Dump() + " high = " + m_high->Dump() + " class = " + UserString(boost::lexical_cast<std::string>(m_class));
}

bool Condition::DesignHasPartClass::Match(const ScriptingContext& local_context) const
{
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "DesignHasPartClass::Match passed no candidate object";
        return false;
    }

    if (const Ship* ship = universe_object_cast<const Ship*>(candidate)) {
        if (const ShipDesign* design = ship->Design()) {
            const std::vector<std::string>& parts = design->Parts();
            int count = 0;
            for (std::vector<std::string>::const_iterator it = parts.begin(); it != parts.end(); ++it) {
                if (const PartType* part_type = GetPartType(*it)) {
                    if (part_type->Class() == m_class)
                        ++count;
                }
            }
            int low = m_low->Eval(local_context);
            int high = m_high->Eval(local_context);
            return (low <= count && count < high);
        }
    }
    return false;
}

///////////////////////////////////////////////////////////
// Chance                                                //
///////////////////////////////////////////////////////////
Condition::Chance::Chance(const ValueRef::ValueRefBase<double>* chance) :
    m_chance(chance)
{}

Condition::Chance::~Chance()
{
    delete m_chance;
}

std::string Condition::Chance::Description(bool negated/* = false*/) const
{
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
{
    return DumpIndent() + "Random probability = " + m_chance->Dump() + "\n";
}

bool Condition::Chance::Match(const ScriptingContext& local_context) const
{
    double chance = std::max(0.0, std::min(m_chance->Eval(local_context), 1.0));
    return RandZeroToOne() <= chance;
}

///////////////////////////////////////////////////////////
// MeterValue                                            //
///////////////////////////////////////////////////////////
Condition::MeterValue::MeterValue(MeterType meter, const ValueRef::ValueRefBase<double>* low, const ValueRef::ValueRefBase<double>* high) :
    m_meter(meter),
    m_low(low),
    m_high(high)
{}

Condition::MeterValue::~MeterValue()
{
    delete m_low;
    delete m_high;
}

std::string Condition::MeterValue::Description(bool negated/* = false*/) const
{
    std::string low_str = ValueRef::ConstantExpr(m_low) ?
                            boost::lexical_cast<std::string>(m_low->Eval()) :
                            m_low->Description();
    std::string high_str = ValueRef::ConstantExpr(m_high) ?
                            boost::lexical_cast<std::string>(m_high->Eval()) :
                            m_high->Description();
    std::string description_str = "DESC_METER_VALUE_CURRENT";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str))
               % UserString(boost::lexical_cast<std::string>(m_meter))
               % low_str
               % high_str);
}

std::string Condition::MeterValue::Dump() const
{
    std::string retval = DumpIndent();
    switch (m_meter) {
    case INVALID_METER_TYPE:        retval += "INVALID_METER_TYPE"; break;
    case METER_TARGET_POPULATION:   retval += "TargetPopulation";   break;
    case METER_TARGET_HEALTH:       retval += "TargetHealth";       break;
    case METER_TARGET_FARMING:      retval += "TargetFarming";      break;
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
    case METER_HEALTH:              retval += "Health";             break;
    case METER_FARMING:             retval += "Farming";            break;
    case METER_INDUSTRY:            retval += "Industry";           break;
    case METER_RESEARCH:            retval += "Research";           break;
    case METER_TRADE:               retval += "Trade";              break;
    case METER_MINING:              retval += "Mining";             break;
    case METER_CONSTRUCTION:        retval += "Construction";       break;
    case METER_FUEL:                retval += "Fuel";               break;
    case METER_SHIELD:              retval += "Shield";             break;
    case METER_STRUCTURE:           retval += "Structure";          break;
    case METER_DEFENSE:             retval += "Defense";            break;
    case METER_FOOD_CONSUMPTION:    retval += "FoodConsumption";    break;
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
    retval += " low = " + m_low->Dump() + " high = " + m_high->Dump() + "\n";
    return retval;
}

bool Condition::MeterValue::Match(const ScriptingContext& local_context) const
{
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "MeterValue::Match passed no candidate object";
        return false;
    }

    double low = m_low->Eval(local_context);
    double high = m_high->Eval(local_context);
    if (const Meter* meter = candidate->GetMeter(m_meter)) {
        double value = meter->Initial();
        return low <= value && value < high;
    } else {
        Logger().errorStream() << "Condition::MeterValue::Match couldn't get meter of requested type";
    }
    return false;
}

///////////////////////////////////////////////////////////
// EmpireStockpileValue                                  //
///////////////////////////////////////////////////////////
Condition::EmpireStockpileValue::EmpireStockpileValue(ResourceType stockpile, const ValueRef::ValueRefBase<double>* low, const ValueRef::ValueRefBase<double>* high) :
    m_stockpile(stockpile),
    m_low(low),
    m_high(high)
{}

Condition::EmpireStockpileValue::~EmpireStockpileValue()
{
    delete m_low;
    delete m_high;
}

std::string Condition::EmpireStockpileValue::Description(bool negated/* = false*/) const
{
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

std::string Condition::EmpireStockpileValue::Dump() const
{
    std::string retval = DumpIndent();
    switch (m_stockpile) {
    case RE_FOOD:       retval += "OwnerFoodStockpile";     break;
    case RE_MINERALS:   retval += "OwnerMineralStockpile";  break;
    case RE_TRADE:      retval += "OwnerTradeStockpile";    break;
    case RE_RESEARCH:   retval += "OwnerResearchStockpile"; break;
    case RE_INDUSTRY:   retval += "OwnerIndustryStockpile"; break;
    default: retval += "?"; break;
    }
    retval += " low = " + m_low->Dump() + " high = " + m_high->Dump() + "\n";
    return retval;
}

bool Condition::EmpireStockpileValue::Match(const ScriptingContext& local_context) const
{
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "MeterValue::Match passed no candidate object";
        return false;
    }

    if (candidate->Owners().size() != 1)
        return false;

    if (const Empire* empire = Empires().Lookup(*candidate->Owners().begin()))
        if (m_stockpile == RE_FOOD || m_stockpile == RE_MINERALS || m_stockpile == RE_TRADE) {
            double stockpile = empire->ResourceStockpile(m_stockpile);
            return (m_low->Eval(local_context) <= stockpile && stockpile <= m_high->Eval(local_context));
        }
    return false;
}

///////////////////////////////////////////////////////////
// OwnerHasTech                                          //
///////////////////////////////////////////////////////////
Condition::OwnerHasTech::OwnerHasTech(const std::string& name) :
    m_name(name)
{}

std::string Condition::OwnerHasTech::Description(bool negated/* = false*/) const
{
    std::string description_str = "DESC_OWNER_HAS_TECH";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str)) % UserString(m_name));
}

std::string Condition::OwnerHasTech::Dump() const
{
    return DumpIndent() + "OwnerHasTech name = \"" + m_name + "\"\n";
}

bool Condition::OwnerHasTech::Match(const ScriptingContext& local_context) const
{
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "OwnerHasTech::Match passed no candidate object";
        return false;
    }

    if (candidate->Owners().size() != 1)
        return false;

    if (const Empire* empire = Empires().Lookup(*candidate->Owners().begin()))
        return empire->TechResearched(m_name);
    else
        return false;
}

///////////////////////////////////////////////////////////
// VisibleToEmpire                                       //
///////////////////////////////////////////////////////////
Condition::VisibleToEmpire::VisibleToEmpire(const std::vector<const ValueRef::ValueRefBase<int>*>& empire_ids) :
    m_empire_ids(empire_ids)
{}

Condition::VisibleToEmpire::~VisibleToEmpire()
{
    for (unsigned int i = 0; i < m_empire_ids.size(); ++i) {
        delete m_empire_ids[i];
    }
}

std::string Condition::VisibleToEmpire::Description(bool negated/* = false*/) const
{
    if (m_empire_ids.size() == 1) {
        std::string value_str = ValueRef::ConstantExpr(m_empire_ids[0]) ?
                                    Empires().Lookup(m_empire_ids[0]->Eval())->Name() :
                                    m_empire_ids[0]->Description();
        std::string description_str = "DESC_VISIBLE_TO_SINGLE_EMPIRE";
        if (negated)
            description_str += "_NOT";
        return str(FlexibleFormat(UserString(description_str)) % value_str);
    } else {
        std::string values_str;
        for (unsigned int i = 0; i < m_empire_ids.size(); ++i) {
            values_str += ValueRef::ConstantExpr(m_empire_ids[i]) ?
                            Empires().Lookup(m_empire_ids[i]->Eval())->Name() :
                            m_empire_ids[i]->Description();
            if (2 <= m_empire_ids.size() && i < m_empire_ids.size() - 2) {
                values_str += ", ";
            } else if (i == m_empire_ids.size() - 2) {
                values_str += m_empire_ids.size() < 3 ? " " : ", ";
                values_str += UserString("OR");
                values_str += " ";
            }
        }
        std::string description_str = "DESC_VISIBLE_TO_EMPIRES";
        if (negated)
            description_str += "_NOT";
        return str(FlexibleFormat(UserString(description_str)) % values_str);
    }
}

std::string Condition::VisibleToEmpire::Dump() const
{
    std::string retval = DumpIndent() + "VisibleToEmpire empire = ";
    if (m_empire_ids.size() == 1) {
        retval += m_empire_ids[0]->Dump() + "\n";
    } else {
        retval += "[ ";
        for (unsigned int i = 0; i < m_empire_ids.size(); ++i) {
            retval += m_empire_ids[i]->Dump() + " " ;
        }
        retval += "]\n";
    }
    return retval;
}

bool Condition::VisibleToEmpire::Match(const ScriptingContext& local_context) const
{
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "VisibleToEmpire::Match passed no candidate object";
        return false;
    }

    bool retval = false;
    for (unsigned int i = 0; i < m_empire_ids.size(); ++i) {
        if (candidate->GetVisibility(m_empire_ids[i]->Eval(local_context)) != VIS_NO_VISIBILITY)
            return true;
    }
    return retval;
}

///////////////////////////////////////////////////////////
// WithinDistance                                        //
///////////////////////////////////////////////////////////
Condition::WithinDistance::WithinDistance(const ValueRef::ValueRefBase<double>* distance, const ConditionBase* condition) :
    m_distance(distance),
    m_condition(condition)
{}

Condition::WithinDistance::~WithinDistance()
{
    delete m_distance;
    delete m_condition;
}

std::string Condition::WithinDistance::Description(bool negated/* = false*/) const
{
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

std::string Condition::WithinDistance::Dump() const
{
    std::string retval = DumpIndent() + "WithinDistance distance = " + m_distance->Dump() + " condition =\n";
    ++g_indent;
    retval += m_condition->Dump();
    --g_indent;
    return retval;
}

bool Condition::WithinDistance::Match(const ScriptingContext& local_context) const
{
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "WithinDistance::Match passed no candidate object";
        return false;
    }

    ObjectMap& objects = GetUniverse().Objects();

    // get objects to be considering for matching against subcondition
    ObjectSet subcondition_non_matches;
    for (ObjectMap::iterator it = objects.begin(); it != objects.end(); ++it)
        subcondition_non_matches.insert(it->second);
    ObjectSet subcondition_matches;

    m_condition->Eval(local_context, subcondition_matches, subcondition_non_matches);
    if (subcondition_matches.empty())
        return false;

    double distance_limit = m_distance->Eval(local_context);
    double distance_limit2 = distance_limit * distance_limit;

    // is candidate object close enough to any subcondition matches?
    for (ObjectSet::iterator subcon_it = subcondition_matches.begin(); subcon_it != subcondition_matches.end(); ++subcon_it) {
        const UniverseObject* subcondition_match(*subcon_it);

        double delta_x = candidate->X() - subcondition_match->X();
        double delta_y = candidate->Y() - subcondition_match->Y();
        if (delta_x * delta_x + delta_y* delta_y < distance_limit2)
            return true;
    }

    return false;
}

///////////////////////////////////////////////////////////
// WithinStarlaneJumps                                   //
///////////////////////////////////////////////////////////
Condition::WithinStarlaneJumps::WithinStarlaneJumps(const ValueRef::ValueRefBase<int>* jumps, const ConditionBase* condition) :
    m_jumps(jumps),
    m_condition(condition)
{}

Condition::WithinStarlaneJumps::~WithinStarlaneJumps()
{
    delete m_jumps;
    delete m_condition;
}

std::string Condition::WithinStarlaneJumps::Description(bool negated/* = false*/) const
{
    std::string value_str = ValueRef::ConstantExpr(m_jumps) ? boost::lexical_cast<std::string>(m_jumps->Eval()) : m_jumps->Description();
    std::string description_str = "DESC_WITHIN_STARLANE_JUMPS";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str))
               % value_str
               % m_condition->Description());
}

std::string Condition::WithinStarlaneJumps::Dump() const
{
    std::string retval = DumpIndent() + "WithinStarlaneJumps jumps = " + m_jumps->Dump() + " condition =\n";
    ++g_indent;
    retval += m_condition->Dump();
    --g_indent;
    return retval;
}

namespace {
    int MANY_JUMPS(999999);

    int JumpsBetweenObjects(const UniverseObject* one, const UniverseObject* two) {
        ObjectMap& objects = GetUniverse().Objects();

        if (!one || !two)
            return MANY_JUMPS;

        // determine if objects are systems
        const System* system_one = objects.Object<System>(one->SystemID());
        const System* system_two = objects.Object<System>(two->SystemID());

        // need various special cases for whether the condition-matching object
        // and candidate objects are or are in systems.
        if (system_one && system_two) {
            // both condition-matching object and candidate are / in systems.
            // can just find the shortest path between the two systems
            std::pair<std::list<int>, double> path = GetUniverse().LeastJumpsPath(system_one->ID(), system_two->ID());
            if (!path.first.empty())    // if path.first is empty, no path exists between the systems
                return static_cast<int>(path.first.size());

        } else if (system_one) {
            // just object one is / in a system.
            if (const Fleet* fleet = FleetFromObject(two)) {
                // other object is a fleet that is between systems
                // need to check shortest path from systems on either side of starlane fleet is on
                std::pair<std::list<int>, double> path1 = GetUniverse().LeastJumpsPath(system_one->ID(), fleet->PreviousSystemID());
                std::pair<std::list<int>, double> path2 = GetUniverse().LeastJumpsPath(system_one->ID(), fleet->NextSystemID());
                if (int jumps = static_cast<int>(std::max(path1.first.size(), path2.first.size())))
                    return jumps - 1;
            }

        } else if (system_two) {
            // just object two is a system.
            if (const Fleet* fleet = FleetFromObject(two)) {
                // other object is a fleet that is between systems
                // need to check shortest path from systems on either side of starlane fleet is on
                std::pair<std::list<int>, double> path1 = GetUniverse().LeastJumpsPath(system_two->ID(), fleet->PreviousSystemID());
                std::pair<std::list<int>, double> path2 = GetUniverse().LeastJumpsPath(system_two->ID(), fleet->NextSystemID());
                if (int jumps = static_cast<int>(std::max(path1.first.size(), path2.first.size())))
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

                std::pair<std::list<int>, int> path1 = GetUniverse().LeastJumpsPath(fleet_one_prev_system_id, fleet_two_prev_system_id);
                std::pair<std::list<int>, int> path2 = GetUniverse().LeastJumpsPath(fleet_one_prev_system_id, fleet_two_next_system_id);
                std::pair<std::list<int>, int> path3 = GetUniverse().LeastJumpsPath(fleet_one_next_system_id, fleet_two_prev_system_id);
                std::pair<std::list<int>, int> path4 = GetUniverse().LeastJumpsPath(fleet_one_next_system_id, fleet_two_next_system_id);
                if (int jumps = static_cast<int>(std::max(std::max(path1.second, path2.second),
                                                          std::max(path3.second, path4.second))))
                    return jumps - 1;
            }

        }
        return MANY_JUMPS;
    }
}

bool Condition::WithinStarlaneJumps::Match(const ScriptingContext& local_context) const
{
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "WithinStarlaneJumps::Match passed no candidate object";
        return false;
    }

    ObjectMap& objects = GetUniverse().Objects();

    // get objects to be considering for matching against subcondition
    ObjectSet subcondition_non_matches;
    for (ObjectMap::iterator it = objects.begin(); it != objects.end(); ++it)
        subcondition_non_matches.insert(it->second);
    ObjectSet subcondition_matches;

    m_condition->Eval(local_context, subcondition_matches, subcondition_non_matches);
    if (subcondition_matches.empty())
        return false;

    int jump_limit = m_jumps->Eval(local_context);
    if (jump_limit < 0)
        return false;

    // is candidate object close enough to any subcondition matches?
    for (ObjectSet::iterator subcon_it = subcondition_matches.begin(); subcon_it != subcondition_matches.end(); ++subcon_it) {
        const UniverseObject* subcondition_match(*subcon_it);

        if (jump_limit == 0) {
            // special case, since LeastJumpsPath() doesn't expect the start point to be the end point
            double delta_x = subcondition_match->X() - candidate->X();
            double delta_y = subcondition_match->Y() - candidate->Y();
            if (delta_x * delta_x + delta_y * delta_y == 0)
                return true;
            continue;
        }

        int jumps = JumpsBetweenObjects(subcondition_match, candidate);
        if (jumps <= jump_limit)
            return true;
    }

    return false;
}

///////////////////////////////////////////////////////////
// CanAddStarlaneConnection                              //
///////////////////////////////////////////////////////////
Condition::CanAddStarlaneConnection::CanAddStarlaneConnection(const ConditionBase* condition) :
    m_condition(condition)
{}

Condition::CanAddStarlaneConnection::~CanAddStarlaneConnection()
{
    delete m_condition;
}

std::string Condition::CanAddStarlaneConnection::Description(bool negated/* = false*/) const
{
    std::string description_str = "DESC_CAN_ADD_STARLANE_CONNECTION";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str)) % m_condition->Description());
}

std::string Condition::CanAddStarlaneConnection::Dump() const
{
    std::string retval = DumpIndent() + "CanAddStarlaneConnection condition =\n";
    ++g_indent;
        retval += m_condition->Dump();
    --g_indent;
    return retval;
}

bool Condition::CanAddStarlaneConnection::Match(const ScriptingContext& local_context) const
{
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "CanAddStarlaneConnection::Match passed no candidate object";
        return false;
    }

    ObjectMap& objects = GetUniverse().Objects();

    // get objects to be considering for matching against subcondition
    ObjectSet subcondition_non_matches;
    for (ObjectMap::iterator it = objects.begin(); it != objects.end(); ++it)
        subcondition_non_matches.insert(it->second);
    ObjectSet subcondition_matches;

    m_condition->Eval(local_context, subcondition_matches, subcondition_non_matches);

    // assemble all systems in or containing objects in subcondition matches
    ObjectSet destination_systems;
    for (ObjectSet::const_iterator it = subcondition_matches.begin(); it != subcondition_matches.end(); ++it)
        if (const System* system = objects.Object<System>((*it)->SystemID()))
            destination_systems.insert(system);

    if (destination_systems.empty())
        return false;

    // can the candidate object have starlanes to all destination systems?
    for (ObjectSet::const_iterator it = destination_systems.begin(); it != destination_systems.end(); ++it) {
        return false;
    }

    return true;
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

Condition::CanRemoveStarlaneConnection::CanRemoveStarlaneConnection(const ConditionBase* condition) :
    m_condition(condition)
{}

Condition::CanRemoveStarlaneConnection::~CanRemoveStarlaneConnection()
{
    delete m_condition;
}

std::string Condition::CanRemoveStarlaneConnection::Description(bool negated/* = false*/) const
{
    std::string description_str = "DESC_CAN_REMOVE_STARLANE_CONNECTION";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str)) % m_condition->Description());
}

std::string Condition::CanRemoveStarlaneConnection::Dump() const
{
    std::string retval = DumpIndent() + "CanRemoveStarlaneConnection condition =\n";
    ++g_indent;
        retval += m_condition->Dump();
    --g_indent;
    return retval;
}

bool Condition::CanRemoveStarlaneConnection::Match(const ScriptingContext& local_context) const
{
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "CanRemoveStarlaneConnection::Match passed no candidate object";
        return false;
    }

    const ObjectMap& objects = GetMainObjectMap();

    // get system for candidate object
    int candidate_system_id = candidate->SystemID();
    const System* candidate_system = objects.Object<System>(candidate_system_id);
    if (!candidate_system)
        return false;

    // get objects to be considering for matching against subcondition
    ObjectSet subcondition_non_matches;
    for (ObjectMap::const_iterator it = objects.const_begin(); it != objects.const_end(); ++it)
        subcondition_non_matches.insert(it->second);
    ObjectSet subcondition_matches;

    m_condition->Eval(local_context, subcondition_matches, subcondition_non_matches);

    if (subcondition_matches.empty())
        return false;

    // assemble all systems in or containing objects in subcondition matches
    std::set<const System*> destination_systems;
    for (ObjectSet::const_iterator it = subcondition_matches.begin(); it != subcondition_matches.end(); ++it)
        if (const System* system = objects.Object<System>((*it)->SystemID()))
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

    //SystemsConnected(graph_impl
    return true;
}

///////////////////////////////////////////////////////////
// ExploredByEmpire                                      //
///////////////////////////////////////////////////////////
Condition::ExploredByEmpire::ExploredByEmpire(const std::vector<const ValueRef::ValueRefBase<int>*>& empire_ids) :
    m_empire_ids(empire_ids)
{}

Condition::ExploredByEmpire::~ExploredByEmpire()
{
    for (unsigned int i = 0; i < m_empire_ids.size(); ++i) {
        delete m_empire_ids[i];
    }
}

std::string Condition::ExploredByEmpire::Description(bool negated/* = false*/) const
{
    if (m_empire_ids.size() == 1) {
        std::string value_str = ValueRef::ConstantExpr(m_empire_ids[0]) ?
                                    Empires().Lookup(m_empire_ids[0]->Eval())->Name() :
                                    m_empire_ids[0]->Description();
        std::string description_str = "DESC_EXPLORED_BY_SINGLE_EMPIRE";
        if (negated)
            description_str += "_NOT";
        return str(FlexibleFormat(UserString(description_str)) % value_str);
    } else {
        std::string values_str;
        for (unsigned int i = 0; i < m_empire_ids.size(); ++i) {
            values_str += ValueRef::ConstantExpr(m_empire_ids[i]) ?
                            Empires().Lookup(m_empire_ids[i]->Eval())->Name() :
                            m_empire_ids[i]->Description();
            if (2 <= m_empire_ids.size() && i < m_empire_ids.size() - 2) {
                values_str += ", ";
            } else if (i == m_empire_ids.size() - 2) {
                values_str += m_empire_ids.size() < 3 ? " " : ", ";
                values_str += UserString("OR");
                values_str += " ";
            }
        }
        std::string description_str = "DESC_EXPLORED_BY_EMPIRES";
        if (negated)
            description_str += "_NOT";
        return str(FlexibleFormat(UserString(description_str)) % values_str);
    }
}

std::string Condition::ExploredByEmpire::Dump() const
{
    std::string retval = DumpIndent() + "ExploredByEmpire empire = ";
    if (m_empire_ids.size() == 1) {
        retval += m_empire_ids[0]->Dump() + "\n";
    } else {
        retval += "[ ";
        for (unsigned int i = 0; i < m_empire_ids.size(); ++i) {
            retval += m_empire_ids[i]->Dump() + " " ;
        }
        retval += "]\n";
    }
    return retval;
}

bool Condition::ExploredByEmpire::Match(const ScriptingContext& local_context) const
{
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "ExploredByEmpire::Match passed no candidate object";
        return false;
    }

    const EmpireManager& empires = Empires();
    for (unsigned int i = 0; i < m_empire_ids.size(); ++i) {
        if (const Empire* empire = empires.Lookup(m_empire_ids[i]->Eval(local_context)))
            if (empire->HasExploredSystem(candidate->ID()))
                return true;
    }
    return false;
}

///////////////////////////////////////////////////////////
// Stationary                                      //
///////////////////////////////////////////////////////////
Condition::Stationary::Stationary()
{}

std::string Condition::Stationary::Description(bool negated/* = false*/) const
{
    std::string description_str = "DESC_STATIONARY";
    if (negated)
        description_str += "_NOT";
    return UserString(description_str);
}

std::string Condition::Stationary::Dump() const
{
    return DumpIndent() + "Stationary\n";
}

bool Condition::Stationary::Match(const ScriptingContext& local_context) const
{
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "Stationary::Match passed no candidate object";
        return false;
    }

    const ObjectMap& objects = GetMainObjectMap();
    // the only objects that can move are fleets and the ships in them.  so,
    // attempt to cast the candidate object to a fleet or ship, and if it's a ship
    // get the fleet of that ship
    const Fleet* fleet = universe_object_cast<const Fleet*>(candidate);
    if (!fleet)
        if (const Ship* ship = universe_object_cast<const Ship*>(candidate))
            fleet = objects.Object<Fleet>(ship->FleetID());

    if (fleet) {
        // if a fleet is available, it is "moving", or not stationary, if it's
        // next system is a system and isn't the current system.  This will
        // mean fleets that have arrived at a system on the current turn will
        // be stationary, but fleets departing won't be stationary.
        int next_id = fleet->NextSystemID();
        int cur_id = fleet->SystemID();
        if (next_id != UniverseObject::INVALID_OBJECT_ID && next_id != cur_id)
            return false;
    }

    return true;
}

///////////////////////////////////////////////////////////
// FleetSupplyableByEmpire                               //
///////////////////////////////////////////////////////////
Condition::FleetSupplyableByEmpire::FleetSupplyableByEmpire(const ValueRef::ValueRefBase<int>* empire_id) :
    m_empire_id(empire_id)
{}

Condition::FleetSupplyableByEmpire::~FleetSupplyableByEmpire()
{
    delete m_empire_id;
}

std::string Condition::FleetSupplyableByEmpire::Description(bool negated/* = false*/) const
{
    std::string empire_str = ValueRef::ConstantExpr(m_empire_id) ?
                                Empires().Lookup(m_empire_id->Eval())->Name() :
                                m_empire_id->Description();

    std::string description_str = "DESC_SUPPLY_CONNECTED_FLEET";
    if (negated)
        description_str += "_NOT";

    return str(FlexibleFormat(UserString(description_str))
               % empire_str);
}

std::string Condition::FleetSupplyableByEmpire::Dump() const
{
    return DumpIndent() + "FleetSupplyableByEmpire empire_id = " + m_empire_id->Dump();
}

bool Condition::FleetSupplyableByEmpire::Match(const ScriptingContext& local_context) const
{
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "FleetSupplyableByEmpire::Match passed no candidate object";
        return false;
    }

    const EmpireManager& empires = Empires();
    if (const Empire* empire = empires.Lookup(m_empire_id->Eval(local_context))) {
        const std::set<int>& supplyable_systems = empire->FleetSupplyableSystemIDs();
        if (supplyable_systems.find(candidate->SystemID()) != supplyable_systems.end())
            return true;
    }
    return false;
}

///////////////////////////////////////////////////////////
// ResourceSupplyConnectedByEmpire                       //
///////////////////////////////////////////////////////////
Condition::ResourceSupplyConnectedByEmpire::ResourceSupplyConnectedByEmpire(const ValueRef::ValueRefBase<int>* empire_id,
                                                                            const ConditionBase* condition) :
    m_empire_id(empire_id),
    m_condition(condition)
{}

Condition::ResourceSupplyConnectedByEmpire::~ResourceSupplyConnectedByEmpire()
{
    delete m_empire_id;
    delete m_condition;
}

bool Condition::ResourceSupplyConnectedByEmpire::Match(const ScriptingContext& local_context) const
{
    const UniverseObject* candidate = local_context.condition_local_candidate;
    if (!candidate) {
        Logger().errorStream() << "ResourceSupplyConnectedByEmpire::Match passed no candidate object";
        return false;
    }

    ObjectMap& objects = GetUniverse().Objects();

    // get objects to be considering for matching against subcondition
    ObjectSet subcondition_non_matches;
    for (ObjectMap::iterator it = objects.begin(); it != objects.end(); ++it)
        subcondition_non_matches.insert(it->second);
    ObjectSet subcondition_matches;

    m_condition->Eval(local_context, subcondition_matches, subcondition_non_matches);
    if (subcondition_matches.empty())
        return false;

    const Empire* empire = Empires().Lookup(m_empire_id->Eval(local_context));
    if (!empire)
        return false;

    // is candidate object connected to a subcondition matching object by resource supply?
    for (ObjectSet::iterator subcon_it = subcondition_matches.begin(); subcon_it != subcondition_matches.end(); ++subcon_it) {
        const UniverseObject* subcondition_match(*subcon_it);

        const std::set<std::set<int> >& groups = empire->ResourceSupplyGroups();
        for (std::set<std::set<int> >::const_iterator groups_it = groups.begin(); groups_it != groups.end(); ++groups_it) {
            const std::set<int>& group = *groups_it;
            if (group.find(subcondition_match->SystemID()) != group.end()) {
                // found resource sharing group containing subcondition-matching object.  Does it also contain candidate?
                if (group.find(candidate->SystemID()) != group.end())
                    return true;    // object matching m_condition and candidate object are in same resourse sharing group
                else
                    return false;   // object matching m_condition is not in resource sharing group with candidate (each object can be in only one group)
            }
            // current subcondition-matching object is not in this resource sharing group
        }
        // current subcondition-matching object is not in any resource sharing group for this empire
    }

    return false;
}

std::string Condition::ResourceSupplyConnectedByEmpire::Description(bool negated/* = false*/) const
{
    std::string empire_str = ValueRef::ConstantExpr(m_empire_id) ?
                                Empires().Lookup(m_empire_id->Eval())->Name() :
                                m_empire_id->Description();

    std::string description_str = "DESC_SUPPLY_CONNECTED_RESOURCE";
    if (negated)
        description_str += "_NOT";

    return str(FlexibleFormat(UserString(description_str))
               % empire_str
               % m_condition->Description());
}

std::string Condition::ResourceSupplyConnectedByEmpire::Dump() const
{
    std::string retval = DumpIndent() + "ResourceSupplyConnectedByEmpire empire_id = " + m_empire_id->Dump() +
                                        " condition = \n";
    ++g_indent;
    retval += m_condition->Dump();
    --g_indent;
    return retval;
}

///////////////////////////////////////////////////////////
// And                                                   //
///////////////////////////////////////////////////////////
Condition::And::And(const std::vector<const ConditionBase*>& operands) :
    m_operands(operands)
{
    assert(!m_operands.empty());
}

Condition::And::~And()
{
    for (unsigned int i = 0; i < m_operands.size(); ++i) {
        delete m_operands[i];
    }
}

bool Condition::And::Match(const ScriptingContext& local_context) const
{
    ObjectSet non_match;
    ObjectSet match;        match.insert(local_context.condition_local_candidate);
    // candidate must match all subconditions
    for (std::vector<const ConditionBase*>::const_iterator it = m_operands.begin(); it != m_operands.end(); ++it) {
        (*it)->Eval(local_context, match, non_match, MATCHES);
        if (match.empty())
            return false;
    }
    return true;
}

std::string Condition::And::Description(bool negated/* = false*/) const
{
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

std::string Condition::And::Dump() const
{
    std::string retval = DumpIndent() + "And [\n";
    ++g_indent;
    for (unsigned int i = 0; i < m_operands.size(); ++i) {
        retval += m_operands[i]->Dump();
    }
    --g_indent;
    retval += DumpIndent() + "]\n";
    return retval;
}

///////////////////////////////////////////////////////////
// Or                                                    //
///////////////////////////////////////////////////////////
Condition::Or::Or(const std::vector<const ConditionBase*>& operands) :
    m_operands(operands)
{
    assert(!m_operands.empty());
}

Condition::Or::~Or()
{
    for (unsigned int i = 0; i < m_operands.size(); ++i) {
        delete m_operands[i];
    }
}

bool Condition::Or::Match(const ScriptingContext& local_context) const
{
    ObjectSet non_match;    non_match.insert(local_context.condition_local_candidate);
    ObjectSet match;
    // candidate must match at least one subcondition
    for (std::vector<const ConditionBase*>::const_iterator it = m_operands.begin(); it != m_operands.end(); ++it) {
        (*it)->Eval(local_context, match, non_match);
        if (non_match.empty())
            return true;
    }
    return false;
}

std::string Condition::Or::Description(bool negated/* = false*/) const
{
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

std::string Condition::Or::Dump() const
{
    std::string retval = DumpIndent() + "Or [\n";
    ++g_indent;
    for (unsigned int i = 0; i < m_operands.size(); ++i) {
        retval += m_operands[i]->Dump();
    }
    --g_indent;
    retval += DumpIndent() + "]\n";
    return retval;
}

///////////////////////////////////////////////////////////
// Not                                                   //
///////////////////////////////////////////////////////////
Condition::Not::Not(const ConditionBase* operand) :
    m_operand(operand)
{
    assert(m_operand);
}

Condition::Not::~Not()
{
    delete m_operand;
}

bool Condition::Not::Match(const ScriptingContext& local_context) const
{
    ObjectSet non_match;    non_match.insert(local_context.condition_local_candidate);
    ObjectSet match;
    m_operand->Eval(local_context, match, non_match);
    return match.empty();
}

std::string Condition::Not::Description(bool negated/* = false*/) const
{
    return m_operand->Description(true);
}

std::string Condition::Not::Dump() const
{
    std::string retval = DumpIndent() + "Not\n";
    ++g_indent;
    retval += m_operand->Dump();
    --g_indent;
    return retval;
}
