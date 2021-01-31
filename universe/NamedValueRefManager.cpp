#include "NamedValueRefManager.h"
#include "ValueRefs.h"

#include <chrono>
#include <functional>
#include <iomanip>
#include <iterator>
#include <mutex>
#include <unordered_map>
#include <thread>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include "Enums.h"
#include "Field.h"
#include "Fighter.h"
#include "Fleet.h"
#include "Pathfinder.h"
#include "Planet.h"
#include "ShipDesign.h"
#include "ShipHull.h"
#include "ShipPart.h"
#include "Ship.h"
#include "Species.h"
#include "System.h"
#include "Tech.h"
#include "UniverseObjectVisitors.h"
#include "UniverseObject.h"
#include "Universe.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Empire.h"
#include "../Empire/Supply.h"
#include "../util/GameRules.h"
#include "../util/Logger.h"
#include "../util/MultiplayerCommon.h"
#include "../util/Random.h"

NamedValueRefManager* NamedValueRefManager::s_instance = nullptr;

NamedValueRefManager::NamedValueRefManager() {
    if (s_instance)
        throw std::runtime_error("Attempted to create more than one NamedValueRefManager.");

    // Only update the global pointer on successful construction.
    InfoLogger() << "NamedValueRefManager::NameValueRefManager constructs singleton " << this;
    s_instance = this;
}

void NamedValueRefManager::SetNamedValueRefParse(Pending::Pending<NamedValueRefParseMap>&& future)
{ m_pending_named_value_refs_focs_txt = std::move(future); }

void NamedValueRefManager::CheckPendingNamedValueRefs() const {
    if (!m_pending_named_value_refs_focs_txt)
        return;
    // we block on the asynchronous parse
    // throw away the result, the parser already registered the values
    WaitForPending(m_pending_named_value_refs_focs_txt, /*do not care about result*/true);
}


namespace {
// helper function for NamedValueRefManager::GetValueRef
template <typename V>
V* const GetValueRefImpl(std::map<NamedValueRefManager::key_type, std::unique_ptr<V>>& registry,
                         const std::string& label, const std::string& name) /*const*/
{
    TraceLogger() << "NamedValueRefManager::GetValueRef look for registered " << label << " valueref for \"" << name << '"';
    TraceLogger() << "Number of registered " << label << " ValueRefs: " << registry.size();
    const auto it = registry.find(name);
    if (it != registry.end())
        return it->second.get();
    WarnLogger() << "NamedValueRefManager::GetValueRef found no registered " << label << " valueref for \"" << name << "\". This should not happen once \"#3225 Refactor initialisation of invariants in value refs to happen after parsing\" is implemented";
    return nullptr;
}
}

// default implementation - queries the untyped registry
// will return nullptr if no such entry in the generic registry exists or if it has a different type than requested
template <typename T>
ValueRef::ValueRef<T>* const NamedValueRefManager::GetValueRef(const std::string& name, const bool wait_for_named_value_focs_txt_parse) /*const*/ {
    if (wait_for_named_value_focs_txt_parse)
        CheckPendingNamedValueRefs();
    return dynamic_cast<ValueRef::ValueRef<T>*>(GetValueRefImpl(m_value_refs, "generic", name));
}

// int specialisation - queries the ValueRef<int> registry
template <>
ValueRef::ValueRef<int>* const NamedValueRefManager::GetValueRef(const std::string& name, const bool wait_for_named_value_focs_txt_parse) /*const*/ {
    if (wait_for_named_value_focs_txt_parse)
        CheckPendingNamedValueRefs();
    return GetValueRefImpl(m_value_refs_int, "int", name);
}

// double specialisation - queries the ValueRef<double> registry
template <>
ValueRef::ValueRef<double>* const NamedValueRefManager::GetValueRef(const std::string& name,  const bool wait_for_named_value_focs_txt_parse) /*const*/ {
    if (wait_for_named_value_focs_txt_parse)
        CheckPendingNamedValueRefs();
    return GetValueRefImpl(m_value_refs_double, "double", name);
}

ValueRef::ValueRefBase* const NamedValueRefManager::GetValueRefBase(const std::string& name) const {
    auto* drefp = const_cast<NamedValueRefManager*>(this)->GetValueRef<double>(name);
    //if (auto* drefp = const_cast<NamedValueRefManager*>(this)->GetValueRef<double>(name)) // TODO C++17
    if (drefp)
        return drefp;
    auto* irefp = const_cast<NamedValueRefManager*>(this)->GetValueRef<int>(name);
    //if (auto* irefp = const_cast<NamedValueRefManager*>(this)->GetValueRef<int>(name)) // TODO C++17
    if (irefp)
        return irefp;
    CheckPendingNamedValueRefs();
    const auto it = m_value_refs.find(name);
    return it != m_value_refs.end() ? it->second.get() : nullptr;
}

NamedValueRefManager& NamedValueRefManager::GetNamedValueRefManager() {
    TraceLogger() << "NamedValueRefManager::GetNamedValueRefManager starts (check the thread)";
    static NamedValueRefManager manager; // function local 
    TraceLogger() << "NamedValueRefManager::GetNamedValueRefManager at " << &manager;
    return manager;
}


unsigned int NamedValueRefManager::GetCheckSum() const {
    CheckPendingNamedValueRefs();
    unsigned int retval{0};
    for (auto const& name_type_pair : m_value_refs)
        CheckSums::CheckSumCombine(retval, name_type_pair);
    for (auto const& name_type_pair : m_value_refs_int)
        CheckSums::CheckSumCombine(retval, name_type_pair);
    for (auto const& name_type_pair : m_value_refs_double)
        CheckSums::CheckSumCombine(retval, name_type_pair);

    DebugLogger() << "NamedValueRefManager checksum: " << retval;
    return retval;
}

NamedValueRefManager::any_container_type  NamedValueRefManager::GetItems() const {
    CheckPendingNamedValueRefs();
    auto base_to_any{[](const entry_type& kv) { return any_entry_type(kv.first, *(kv.second.get())); }};
    auto double_to_any{[](const double_entry_type& kv) { return any_entry_type(kv.first, *(kv.second.get())); }};
    auto int_to_any{[](const int_entry_type& kv) { return any_entry_type(kv.first, *(kv.second.get())); }};

    // should use C++20 ranges (or C++14, C++17 boost Range-v3 library) to avoid copying
    any_container_type aet;
    std::transform(m_value_refs_double.begin(), m_value_refs_double.end(), std::inserter(aet, aet.end()), double_to_any);
    std::transform(m_value_refs_int.begin(), m_value_refs_int.end(), std::inserter(aet, aet.end()), int_to_any);
    std::transform(m_value_refs.begin(), m_value_refs.end(), std::inserter(aet, aet.end()), base_to_any);
    return aet;
}

namespace {
    /** helper function for NamedValueRefManager::RegisterValueRef */
    template <typename R, typename VR>
    void RegisterValueRefImpl(R& container, std::mutex& mutex, const std::string& label,
                              std::string&& valueref_name, std::unique_ptr<VR>&& vref)
    {
        TraceLogger() << "Register " << label << " valueref for " << valueref_name << ": " << vref->Description();
        if (container.count(valueref_name)>0) {
            TraceLogger() << "Skip registration for already registered " << label << " valueref for " << valueref_name;
            TraceLogger() << "Number of registered " << label << " ValueRefs: " << container.size();
            return;
        }
        TraceLogger() << "RegisterValueRefImpl Check invariances for info. Then add the value ref in a thread safe way.";
        const std::lock_guard<std::mutex> lock(mutex);
        if (!(vref->RootCandidateInvariant() && vref->LocalCandidateInvariant() &&
             vref->TargetInvariant() && vref->SourceInvariant()))
        { ErrorLogger() << "Currently only invariant value refs can be named. " << valueref_name; }
        container.emplace(std::move(valueref_name), std::move(vref));
        TraceLogger() << "Number of registered " << label << " ValueRefs: " << container.size();
    }
}

template <typename T>
void NamedValueRefManager::RegisterValueRef(std::string&& valueref_name,
                                            std::unique_ptr<ValueRef::ValueRef<T>>&& vref)
{ RegisterValueRefImpl(m_value_refs, m_value_refs_mutex, "generic", std::move(valueref_name), std::move(vref)); }

// specialisation for registering to the ValueRef<int> registry
template <>
void NamedValueRefManager::RegisterValueRef(std::string&& valueref_name,
                                            std::unique_ptr<ValueRef::ValueRef<int>>&& vref)
{ RegisterValueRefImpl(m_value_refs_int, m_value_refs_int_mutex, "int", std::move(valueref_name), std::move(vref)); }

// specialisation for registering to the ValueRef<double> registry
template <>
void NamedValueRefManager::RegisterValueRef(std::string&& valueref_name,
                                            std::unique_ptr<ValueRef::ValueRef<double>>&& vref)
{
    RegisterValueRefImpl(m_value_refs_double, m_value_refs_double_mutex, "double",
                         std::move(valueref_name), std::move(vref));
}

NamedValueRefManager& GetNamedValueRefManager()
{ return NamedValueRefManager::GetNamedValueRefManager(); }

ValueRef::ValueRefBase* const GetValueRefBase(const std::string& name) {
    TraceLogger() << "NamedValueRefManager::GetValueRefBase look for registered valueref for \"" << name << '"';
    auto* vref = GetNamedValueRefManager().GetValueRefBase(name);
    if (vref)
        return vref;
    InfoLogger() << "NamedValueRefManager::GetValueRefBase could not find registered valueref for \"" << name << '"';
    return nullptr;
}

template <typename T>
ValueRef::ValueRef<T>* const GetValueRef(const std::string& name, const bool wait_for_named_value_focs_txt_parse)
{ return GetNamedValueRefManager().GetValueRef<T>(name, wait_for_named_value_focs_txt_parse); }

template <typename T>
void RegisterValueRef(std::string name, std::unique_ptr<ValueRef::ValueRef<T>>&& vref)
{ return GetNamedValueRefManager().RegisterValueRef<T>(std::move(name), std::move(vref)); }

// trigger instantiations
template ValueRef::ValueRef<int>*    const GetValueRef(const std::string& name, const bool wait_for_named_value_focs_txt_parse);
template ValueRef::ValueRef<double>* const GetValueRef(const std::string& name, const bool wait_for_named_value_focs_txt_parse);
template void RegisterValueRef(std::string name, std::unique_ptr<ValueRef::ValueRef<int>>&& vref);
template void RegisterValueRef(std::string name, std::unique_ptr<ValueRef::ValueRef<double>>&& vref);
template void RegisterValueRef(std::string name, std::unique_ptr<ValueRef::ValueRef<PlanetEnvironment>>&& vref);
template void RegisterValueRef(std::string name, std::unique_ptr<ValueRef::ValueRef<PlanetSize>>&& vref);
template void RegisterValueRef(std::string name, std::unique_ptr<ValueRef::ValueRef<PlanetType>>&& vref);
template void RegisterValueRef(std::string name, std::unique_ptr<ValueRef::ValueRef<StarType>>&& vref);
template void RegisterValueRef(std::string name, std::unique_ptr<ValueRef::ValueRef<UniverseObjectType>>&& vref);
template void RegisterValueRef(std::string name, std::unique_ptr<ValueRef::ValueRef<Visibility>>&& vref);

///////////////////////////////////////////////////////////
// NamedRef                                              //
///////////////////////////////////////////////////////////
namespace ValueRef {
template <typename T>
NamedRef<T>::NamedRef(std::string value_ref_name, bool is_lookup_only) :
    m_value_ref_name(std::move(value_ref_name)),
    m_is_lookup_only(is_lookup_only)
{
    TraceLogger() << "ctor(NamedRef<T>): " << typeid(*this).name() << "  value_ref_name: " << m_value_ref_name << "  is_lookup_only: " << m_is_lookup_only;
}


// waits for named ref to be registered and initializes invariants
// called on first use, necessary for derived values/caching
template <typename T>
bool NamedRef<T>::NamedRefInitInvariants()
{
    if (m_invariants_initialized)
        return true;
    auto* vref = GetValueRef();
    if (!vref) {
        if (!m_is_lookup_only) {
            ErrorLogger() << "NamedRef<T>::NamedRefInitInvariants() Trying to use invariants without existing value ref (which should exist in this case)";
            return false;
        }
        // there is a chance that this will be initialised, so retry
        std::chrono::milliseconds msecs(200);
        DebugLogger() << "NamedRef<T>::NamedRefInitInvariants() could not find value ref, will sleep a bit and retry.";
        std::this_thread::sleep_for(msecs);
        vref = GetValueRef();
        int tries = 5;
        for (int i = 2; i < tries; i++) {
            if (!vref) {
                TraceLogger() << "NamedRef<T>::NamedRefInitInvariants() still could not find value ref (tried " << i << "times), will sleep a bit longer and retry.";
                std::this_thread::sleep_for(i * msecs);
                vref = GetValueRef();
            }
        }
        if (!vref)
            ErrorLogger() << "NamedRef<T>::NamedRefInitInvariants() still could not find value ref after trying " << tries << " times. Giving up.";
    }
    if (vref) {
        ValueRefBase::m_root_candidate_invariant = vref->RootCandidateInvariant();
        ValueRefBase::m_local_candidate_invariant = vref->LocalCandidateInvariant();
        ValueRefBase::m_target_invariant = vref->TargetInvariant();
        ValueRefBase::m_source_invariant = vref->SourceInvariant();
        m_invariants_initialized = true;
        return true;
    }
    // m_is_lookup_only == true, no vref
    WarnLogger() << "NamedRef<T>::NamedRefInitInvariants() Trying to use invariants in a Lookup value ref without existing value ref. "
                 << "Falling back to non-invariance will prevent performance optimisations. This may be a parse race condition.";
    return false;
}

template <typename T>
bool NamedRef<T>::RootCandidateInvariant() const
{ return (const_cast<NamedRef<T>*>(this))->NamedRefInitInvariants() ? ValueRefBase::m_root_candidate_invariant : false; }

template <typename T>
bool NamedRef<T>::LocalCandidateInvariant() const
{ return (const_cast<NamedRef<T>*>(this))->NamedRefInitInvariants() ? ValueRefBase::m_local_candidate_invariant : false; }

template <typename T>
bool NamedRef<T>::TargetInvariant() const
{ return (const_cast<NamedRef<T>*>(this))->NamedRefInitInvariants() ? ValueRefBase::m_target_invariant : false; }

template <typename T>
bool NamedRef<T>::SourceInvariant() const
{ return (const_cast<NamedRef<T>*>(this))->NamedRefInitInvariants() ? ValueRefBase::m_source_invariant : false; }

template <typename T>
bool NamedRef<T>::SimpleIncrement() const
{ return (const_cast<NamedRef<T>*>(this))->NamedRefInitInvariants() ? GetValueRef()->SimpleIncrement() : false; }

template <typename T>
bool NamedRef<T>::ConstantExpr() const
{ return (const_cast<NamedRef<T>*>(this))->NamedRefInitInvariants() ? GetValueRef()->ConstantExpr() : false; }

template <typename T>
bool NamedRef<T>::operator==(const ValueRef<T>& rhs) const
{
    if (&rhs == this)
        return true;
    if (typeid(rhs) != typeid(*this))
        return false;
    const NamedRef<T>& rhs_ = static_cast<const NamedRef<T>&>(rhs);
    return (m_value_ref_name == rhs_.m_value_ref_name);
}

template <typename T>
std::string NamedRef<T>::Description() const
{ return GetValueRef() ? GetValueRef()->Description() : UserString("NAMED_REF_UNKNOWN"); }

template <typename T>
std::string NamedRef<T>::Dump(unsigned short ntabs) const
{ return GetValueRef() ? GetValueRef()->Dump() : "NAMED_REF_UNKNOWN"; }

template <typename T>
void NamedRef<T>::SetTopLevelContent(const std::string& content_name)
{
    if (m_is_lookup_only) {
        DebugLogger() << "Ignored call of SetTopLevelContent(" << content_name
                     << ") on a Lookup NamedRef for value ref " << m_value_ref_name;
        return;
    }
    // only supposed to work for named-in-the-middle-case, SetTopLevelContent checks that
    if ( GetValueRef() )
        const_cast<ValueRef<T>*>(GetValueRef())->SetTopLevelContent(content_name);
    else {
        const char* named_ref_kind = ( content_name == "THERE_IS_NO_TOP_LEVEL_CONTENT" ? "top-level" : "named-in-the-middle" );
        ErrorLogger() << "Unexpected call of SetTopLevelContent(" << content_name
                      << ") on a " << named_ref_kind
                      << " NamedRef - unexpected because no value ref " << m_value_ref_name
                      << " registered yet. Should not happen";
    }
}

template <typename T>
const ValueRef<T>* NamedRef<T>::GetValueRef() const
{
    TraceLogger() << "NamedRef<T>::GetValueRef() look for registered valueref for \"" << m_value_ref_name << '"';
    return ::GetValueRef<T>(m_value_ref_name, m_is_lookup_only);
}

template <typename T>
unsigned int NamedRef<T>::GetCheckSum() const
{
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "ValueRef::NamedRef");
    CheckSums::CheckSumCombine(retval, m_value_ref_name);
    TraceLogger() << "GetCheckSum(NamedRef<T>): " << typeid(*this).name() << " retval: " << retval;
    return retval;
}

template <typename T>
T NamedRef<T>::Eval(const ScriptingContext& context) const
{
    TraceLogger() << "NamedRef<" << typeid(T).name() << ">::Eval()";
    const ValueRef<T>* value_ref = ::GetValueRef<T>(m_value_ref_name);
    if (!value_ref) {
        ErrorLogger() << "NamedRef<" << typeid(T).name() << ">::Eval did not find " << m_value_ref_name;
        throw std::runtime_error(std::string("NamedValueLookup referenced unknown ValueRef<") + typeid(T).name() + "> named '" + m_value_ref_name + "'");
    }

    auto retval = value_ref->Eval(context);
    TraceLogger() << "NamedRef<" << typeid(T).name() << "> name: " << m_value_ref_name << "  retval: " << retval;
    return retval;
}

// trigger instantiations - was not necessary when NamedRef was living in ValueRefs.h/.cpp
// it would be better if the parser would trigger instantiation implicitly
template struct NamedRef<double>;
template struct NamedRef<int>;
// Enums
template struct NamedRef<PlanetEnvironment>;
template struct NamedRef<PlanetSize>;
template struct NamedRef<PlanetType>;
template struct NamedRef<StarType>;
template struct NamedRef<UniverseObjectType>;
template struct NamedRef<Visibility>;
} // ValueRef namespace
