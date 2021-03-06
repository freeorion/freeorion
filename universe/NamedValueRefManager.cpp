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

template <>
const ValueRef::ValueRef<int>* NamedValueRefManager::GetValueRef(const std::string& name,
                                                                 bool wait_for_named_value_focs_txt_parse) const
{
    if (wait_for_named_value_focs_txt_parse)
        CheckPendingNamedValueRefs();
    return GetValueRefImpl(m_value_refs_int, "int", name);
}

template <>
const ValueRef::ValueRef<double>* NamedValueRefManager::GetValueRef(const std::string& name,
                                                                    bool wait_for_named_value_focs_txt_parse) const
{
    if (wait_for_named_value_focs_txt_parse)
        CheckPendingNamedValueRefs();
    return GetValueRefImpl(m_value_refs_double, "double", name);
}

template <>
ValueRef::ValueRef<int>* NamedValueRefManager::GetMutableValueRef(const std::string& name,
                                                                  bool wait_for_named_value_focs_txt_parse)
{
    if (wait_for_named_value_focs_txt_parse)
        CheckPendingNamedValueRefs();
    return GetValueRefImpl(m_value_refs_int, "int", name);
}

template <>
ValueRef::ValueRef<double>* NamedValueRefManager::GetMutableValueRef(const std::string& name,
                                                                     bool wait_for_named_value_focs_txt_parse)
{
    if (wait_for_named_value_focs_txt_parse)
        CheckPendingNamedValueRefs();
    return GetValueRefImpl(m_value_refs_double, "double", name);
}


template const ValueRef::ValueRef<PlanetEnvironment>* NamedValueRefManager::GetValueRef(const std::string&, bool) const;
template const ValueRef::ValueRef<PlanetType>* NamedValueRefManager::GetValueRef(const std::string&, bool) const;
template const ValueRef::ValueRef<Visibility>* NamedValueRefManager::GetValueRef(const std::string&, bool) const;
template const ValueRef::ValueRef<PlanetSize>* NamedValueRefManager::GetValueRef(const std::string&, bool) const;
template const ValueRef::ValueRef<UniverseObjectType>* NamedValueRefManager::GetValueRef(const std::string&, bool) const;
template const ValueRef::ValueRef<StarType>* NamedValueRefManager::GetValueRef(const std::string&, bool) const;


const ValueRef::ValueRefBase* NamedValueRefManager::GetValueRefBase(const std::string& name) const {
    if (auto* drefp = GetValueRef<double>(name))
        return drefp;
    if (auto* irefp = GetValueRef<int>(name))
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
        if (container.count(valueref_name) > 0) {
            TraceLogger() << "Skip registration for already registered " << label << " valueref for " << valueref_name;
            TraceLogger() << "Number of registered " << label << " ValueRefs: " << container.size();
            return;
        }
        TraceLogger() << "RegisterValueRefImpl Check invariances for info. Then add the value ref in a thread safe way.";
        std::scoped_lock lock(mutex);
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

template <>
void NamedValueRefManager::RegisterValueRef(std::string&& valueref_name,
                                            std::unique_ptr<ValueRef::ValueRef<PlanetType>>&& vref)
{ RegisterValueRefImpl(m_value_refs, m_value_refs_mutex, "planettype", std::move(valueref_name), std::move(vref)); }

template <>
void NamedValueRefManager::RegisterValueRef(std::string&& valueref_name,
                                            std::unique_ptr<ValueRef::ValueRef<PlanetEnvironment>>&& vref)
{ RegisterValueRefImpl(m_value_refs, m_value_refs_mutex, "planetenvironement", std::move(valueref_name), std::move(vref)); }

template <>
void NamedValueRefManager::RegisterValueRef(std::string&& valueref_name,
                                            std::unique_ptr<ValueRef::ValueRef<int>>&& vref)
{ RegisterValueRefImpl(m_value_refs_int, m_value_refs_int_mutex, "int", std::move(valueref_name), std::move(vref)); }

template <>
void NamedValueRefManager::RegisterValueRef(std::string&& valueref_name,
                                            std::unique_ptr<ValueRef::ValueRef<double>>&& vref)
{
    RegisterValueRefImpl(m_value_refs_double, m_value_refs_double_mutex, "double",
                         std::move(valueref_name), std::move(vref));
}

NamedValueRefManager& GetNamedValueRefManager()
{ return NamedValueRefManager::GetNamedValueRefManager(); }

const ValueRef::ValueRefBase* GetValueRefBase(const std::string& name) {
    TraceLogger() << "NamedValueRefManager::GetValueRefBase look for registered valueref for \"" << name << '"';
    auto* vref = GetNamedValueRefManager().GetValueRefBase(name);
    if (vref)
        return vref;
    InfoLogger() << "NamedValueRefManager::GetValueRefBase could not find registered valueref for \"" << name << '"';
    return nullptr;
}


// trigger instantiations
template const ValueRef::ValueRef<int>* GetValueRef(const std::string& name, const bool wait_for_named_value_focs_txt_parse);
template const ValueRef::ValueRef<double>* GetValueRef(const std::string& name, const bool wait_for_named_value_focs_txt_parse);
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
    // trigger instantiations
    template struct NamedRef<double>;
    template struct NamedRef<int>;
    template struct NamedRef<PlanetEnvironment>;
    template struct NamedRef<PlanetSize>;
    template struct NamedRef<PlanetType>;
    template struct NamedRef<StarType>;
    template struct NamedRef<UniverseObjectType>;
    template struct NamedRef<Visibility>;
}
