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
const ValueRef::ValueRef<int>* NamedValueRefManager::GetValueRef(std::string_view name,
                                                                 bool wait_for_named_value_focs_txt_parse) const
{
    if (wait_for_named_value_focs_txt_parse)
        CheckPendingNamedValueRefs();
    return GetValueRefImpl(m_value_refs_int, "int", name);
}

template <>
const ValueRef::ValueRef<double>* NamedValueRefManager::GetValueRef(std::string_view name,
                                                                    bool wait_for_named_value_focs_txt_parse) const
{
    if (wait_for_named_value_focs_txt_parse)
        CheckPendingNamedValueRefs();
    return GetValueRefImpl(m_value_refs_double, "double", name);
}

template <>
ValueRef::ValueRef<int>* NamedValueRefManager::GetMutableValueRef(std::string_view name,
                                                                  bool wait_for_named_value_focs_txt_parse)
{
    if (wait_for_named_value_focs_txt_parse)
        CheckPendingNamedValueRefs();
    return GetValueRefImpl(m_value_refs_int, "int", name);
}

template <>
ValueRef::ValueRef<double>* NamedValueRefManager::GetMutableValueRef(std::string_view name,
                                                                     bool wait_for_named_value_focs_txt_parse)
{
    if (wait_for_named_value_focs_txt_parse)
        CheckPendingNamedValueRefs();
    return GetValueRefImpl(m_value_refs_double, "double", name);
}


template const ValueRef::ValueRef<PlanetEnvironment>* NamedValueRefManager::GetValueRef(std::string_view, bool) const;
template const ValueRef::ValueRef<PlanetType>* NamedValueRefManager::GetValueRef(std::string_view, bool) const;
template const ValueRef::ValueRef<Visibility>* NamedValueRefManager::GetValueRef(std::string_view, bool) const;
template const ValueRef::ValueRef<PlanetSize>* NamedValueRefManager::GetValueRef(std::string_view, bool) const;
template const ValueRef::ValueRef<UniverseObjectType>* NamedValueRefManager::GetValueRef(std::string_view, bool) const;
template const ValueRef::ValueRef<StarType>* NamedValueRefManager::GetValueRef(std::string_view, bool) const;


const ValueRef::ValueRefBase* NamedValueRefManager::GetValueRefBase(std::string_view name) const {
    if (auto* drefp = GetValueRef<double>(name))
        return drefp;
    if (auto* irefp = GetValueRef<int>(name)) {
        DebugLogger() << "NamedValueRefManager::GetValueRefBase found registered (int) valueref for \"" << name << "\" "
                      << "(After trying (double) registry)";
        return irefp;
    }
    CheckPendingNamedValueRefs();
    const auto it = m_value_refs.find(name);
    if (it != m_value_refs.end()) {
        DebugLogger() << "NamedValueRefManager::GetValueRefBase found no registered (generic) valueref for \"" << name << "\" "
                      << "(After trying (int|double) registries.";
        return it->second.get();
    }
    ErrorLogger() << "NamedValueRefManager::GetValueRefBase found no registered (double|int|generic) valueref for \"" << name << "\". "
                  << "This should not happen once \"#3225 Refactor initialisation of invariants in value refs to happen after parsing\" is implemented";
    return nullptr;
}

NamedValueRefManager& NamedValueRefManager::GetNamedValueRefManager() {
    //TraceLogger() << "NamedValueRefManager::GetNamedValueRefManager starts (check the thread)";
    static NamedValueRefManager manager; // function local
    //TraceLogger() << "NamedValueRefManager::GetNamedValueRefManager at " << &manager;
    return manager;
}

uint32_t NamedValueRefManager::GetCheckSum() const {
    CheckPendingNamedValueRefs();
    uint32_t retval{0};
    for (auto const& name_type_pair : m_value_refs)
        CheckSums::CheckSumCombine(retval, name_type_pair);
    DebugLogger() << "NamedValueRefManager partial checksum: " << retval;
    for (auto const& name_type_pair : m_value_refs_int)
        CheckSums::CheckSumCombine(retval, name_type_pair);
    DebugLogger() << "NamedValueRefManager second partial checksum: " << retval;
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
        std::scoped_lock lock(mutex);
        if (container.contains(valueref_name)) {
            TraceLogger() << "Skip registration for already registered " << label << " valueref for " << valueref_name;
            TraceLogger() << "Number of registered " << label << " ValueRefs: " << container.size();
            return;
        }
        TraceLogger() << "RegisterValueRefImpl Check invariances for info. Then add the value ref in a thread safe way.";

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
                                            std::unique_ptr<ValueRef::ValueRef<std::string>>&& vref)
{ RegisterValueRefImpl(m_value_refs, m_value_refs_mutex, "string", std::move(valueref_name), std::move(vref)); }

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

const ValueRef::ValueRefBase* GetValueRefBase(std::string_view name) {
    //TraceLogger() << "NamedValueRefManager::GetValueRefBase look for registered valueref for \"" << name << '"';
    if (auto* vref = GetNamedValueRefManager().GetValueRefBase(name))
        return vref;
    InfoLogger() << "NamedValueRefManager::GetValueRefBase could not find registered valueref for \"" << name << '"';
    return nullptr;
}


// trigger instantiations
template const ValueRef::ValueRef<int>* GetValueRef(std::string_view name, const bool wait_for_named_value_focs_txt_parse);
template const ValueRef::ValueRef<double>* GetValueRef(std::string_view name, const bool wait_for_named_value_focs_txt_parse);
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
    template struct NamedRef<std::string>;
    template struct NamedRef<PlanetEnvironment>;
    template struct NamedRef<PlanetSize>;
    template struct NamedRef<PlanetType>;
    template struct NamedRef<StarType>;
    template struct NamedRef<UniverseObjectType>;
    template struct NamedRef<Visibility>;
}
