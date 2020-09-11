#include "NamedValueRefManager.h"
#include "ValueRefs.h"

#include <functional>
#include <iomanip>
#include <iterator>
#include <mutex>
#include <unordered_map>
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

    // Only update the global pointer on sucessful construction.
    InfoLogger() << "NamedValueRefManager::NameValueRefManager constructs singleton " << this;
    s_instance = this;
}

template <typename V>
V* const NamedValueRefManager::GetValueRefImpl(std::map<NamedValueRefManager::key_type, std::unique_ptr<V>>& registry, const std::string& label, const std::string& name) /*const*/ {
    DebugLogger() << "NamedValueRefManager::GetValueRef look for registered " << label << " valueref for \"" << name << '"';
    TraceLogger() << "Number of registered " << label << " ValueRefs: " << registry.size();
    const auto it = registry.find(name);
    if (it != registry.end())
        return it->second.get();
    ErrorLogger() << "NamedValueRefManager::GetValueRef found no registered " << label << " valueref for \"" << name << '"';
    return nullptr;
}

// default implementation - queries the untyped registry
// will return nullptr if no such entry in the generic registry exists or if it has a different type than requested
template <typename T>
ValueRef::ValueRef<T>* const NamedValueRefManager::GetValueRef(const std::string& name) /*const*/ {
    return dynamic_cast<ValueRef::ValueRef<T>*>(this->GetValueRefImpl(m_value_refs, "generic", name));
}

// int specialisation - queries the ValueRef<int> registry
template <>
ValueRef::ValueRef<int>* const NamedValueRefManager::GetValueRef(const std::string& name) /*const*/ {
    return this->GetValueRefImpl(m_value_refs_int, "int", name);
}

// double specialisation - queries the ValueRef<double> registry
template <>
ValueRef::ValueRef<double>* const NamedValueRefManager::GetValueRef(const std::string& name) /*const*/ {
    return this->GetValueRefImpl(m_value_refs_double, "double", name);
}

ValueRef::ValueRefBase* const NamedValueRefManager::GetValueRefBase(const std::string& name) const {
    /* TODO straighten out const shtuff */
    auto* drefp = const_cast<NamedValueRefManager*>(this)->GetValueRef<double>(name);
    //if (auto* drefp = const_cast<NamedValueRefManager*>(this)->GetValueRef<double>(name)) // TODO C++17
    if (drefp)
        return drefp;
    auto* irefp = const_cast<NamedValueRefManager*>(this)->GetValueRef<int>(name);
    //if (auto* irefp = const_cast<NamedValueRefManager*>(this)->GetValueRef<int>(name)) // TODO C++17
    if (irefp)
        return irefp;
    const auto it = m_value_refs.find(name);
    return it != m_value_refs.end() ? it->second.get() : nullptr;
}

NamedValueRefManager& NamedValueRefManager::GetNamedValueRefManager() {
    DebugLogger() << "NamedValueRefManager::GetNamedValueRefManager starts (check the thread)";
    
    static NamedValueRefManager manager; // function local 
    InfoLogger() << "NamedValueRefManager::GetNamedValueRefManager at " << &manager;
    return manager;
}


unsigned int NamedValueRefManager::GetCheckSum() const {
    unsigned int retval{0};
    for (auto const& name_type_pair : m_value_refs)
        CheckSums::CheckSumCombine(retval, name_type_pair);
    for (auto const& name_type_pair : m_value_refs_int)
        CheckSums::CheckSumCombine(retval, name_type_pair);
    for (auto const& name_type_pair : m_value_refs_double)
        CheckSums::CheckSumCombine(retval, name_type_pair);
    //CheckSums::CheckSumCombine(retval, m_value_refs.size()); // why also add size?
    
    DebugLogger() << "NamedValueRefManager checksum: " << retval;
    return retval;
}


NamedValueRefManager::any_container_type  NamedValueRefManager::GetItems() const {
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

template <typename R, typename VR>
void NamedValueRefManager::RegisterValueRefImpl(R& container, std::mutex& mutex, const std::string& label, std::string&& valueref_name, std::unique_ptr<VR>&& vref) {
    InfoLogger() << "Register " << label << " valueref for " << valueref_name << ": " << vref->Description();
    if (container.count(valueref_name)>0) {
        DebugLogger() << "Skip registration for already registered " << label << " valueref for " << valueref_name;
        DebugLogger() << "Number of registered " << label << " ValueRefs: " << m_value_refs.size();
        return;
    }
    const std::lock_guard<std::mutex> lock(mutex);
    if (!(vref->RootCandidateInvariant() && vref->LocalCandidateInvariant() && vref->TargetInvariant() && vref->SourceInvariant()))
            ErrorLogger() << "Currently only invariant value refs can be named. " << valueref_name;
    container.emplace(std::move(valueref_name), std::move(vref));
    DebugLogger() << "Number of registered " << label << " ValueRefs: " << container.size();
}

template <typename T>
void NamedValueRefManager::RegisterValueRef(std::string&& valueref_name, std::unique_ptr<ValueRef::ValueRef<T>>&& vref) {
    this->RegisterValueRefImpl(m_value_refs, m_value_refs_mutex, "generic", std::move(valueref_name), std::move(vref));
}

// specialisation for registering to the ValueRef<int> registry
template <>
void NamedValueRefManager::RegisterValueRef(std::string&& valueref_name, std::unique_ptr<ValueRef::ValueRef<int>>&& vref) {
    this->RegisterValueRefImpl(m_value_refs_int, m_value_refs_int_mutex, "int", std::move(valueref_name), std::move(vref));
}

// specialisation for registering to the ValueRef<double> registry
template <>
void NamedValueRefManager::RegisterValueRef(std::string&& valueref_name, std::unique_ptr<ValueRef::ValueRef<double>>&& vref) {
    this->RegisterValueRefImpl(m_value_refs_double, m_value_refs_double_mutex, "double", std::move(valueref_name), std::move(vref));
}

NamedValueRefManager& GetNamedValueRefManager()
{ return NamedValueRefManager::GetNamedValueRefManager(); }

ValueRef::ValueRefBase* const GetValueRefBase(const std::string& name)
{
    DebugLogger() << "NamedValueRefManager::GetValueRefBase look for registered valueref for \"" << name << '"';
    auto* vref = GetNamedValueRefManager().GetValueRefBase(name);
    if (vref)
        return vref;
    InfoLogger() << "NamedValueRefManager::GetValueRefBase could not find registered valueref for \"" << name << '"';
    return nullptr;
}

template <typename T>
ValueRef::ValueRef<T>* const GetValueRef(const std::string& name)
{ return GetNamedValueRefManager().GetValueRef<T>(name); }

template <typename T>
void RegisterValueRef(std::string name, std::unique_ptr<ValueRef::ValueRef<T>>&& vref)
{ return GetNamedValueRefManager().RegisterValueRef<T>(std::move(name), std::move(vref)); }

// trigger instantiations
template ValueRef::ValueRef<int>*    const GetValueRef(const std::string& name);
template ValueRef::ValueRef<double>* const GetValueRef(const std::string& name);
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
NamedRef<T>::NamedRef(std::string value_ref_name) :
    m_value_ref_name(value_ref_name)
{
    DebugLogger() << "ctor(NamedRef<T>): " << typeid(*this).name() << " value_ref_name: " << m_value_ref_name;
    // not invariant value refs are not supported currently as we do not need those yet
    if (auto ref = GetValueRef()) {
        this->m_root_candidate_invariant = ref->RootCandidateInvariant();
        this->m_local_candidate_invariant = ref->LocalCandidateInvariant();
        this->m_target_invariant = ref->TargetInvariant();
        this->m_source_invariant = ref->SourceInvariant();;
        if (!(this->m_root_candidate_invariant && this->m_local_candidate_invariant
              && this->m_target_invariant && this->m_source_invariant))
            ErrorLogger() << "Currently only invariant value refs can be named. " << m_value_ref_name;
    } else {
        this->m_root_candidate_invariant = true;
        this->m_local_candidate_invariant = true;
        this->m_target_invariant = true;
        this->m_source_invariant = true;
    }
}

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
    // only supposed to work for named-in-the-middle-case
    if ( GetValueRef() )
        const_cast<ValueRef<T>*>(GetValueRef())->SetTopLevelContent(content_name);
    else
        ErrorLogger() << "Unexpected call of SetTopLevelContent on a NamedRef - unexpected because no value ref registered yet. This should not happen";
}

template <typename T>
const ValueRef<T>* NamedRef<T>::GetValueRef() const
{
    DebugLogger() << "NamedRef<T>::GetValueRef() look for registered valueref for \"" << m_value_ref_name << '"';
    return ::GetValueRef<T>(m_value_ref_name);
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
    DebugLogger() << "NamedRef<" << typeid(T).name() << ">::Eval()";
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
template class NamedRef<double>;
template class NamedRef<int>;
// Enums
template class NamedRef<PlanetEnvironment>;
template class NamedRef<PlanetSize>;
template class NamedRef<PlanetType>;
template class NamedRef<StarType>;
template class NamedRef<UniverseObjectType>;
template class NamedRef<Visibility>;
} // ValueRef namespace
