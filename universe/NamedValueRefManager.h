#ifndef _ValueRefManager_h_
#define _ValueRefManager_h_

#include <map>
#include <thread>
#include "ValueRef.h"
#include "../util/CheckSums.h"

[[nodiscard]] FO_COMMON_API const std::string& UserString(const std::string& str);

class NamedValueRefManager;
[[nodiscard]] FO_COMMON_API auto GetNamedValueRefManager() -> NamedValueRefManager&;

template <typename T>
[[nodiscard]] FO_COMMON_API const ValueRef::ValueRef<T>* GetValueRef(
    std::string_view name, bool wait_for_named_value_focs_txt_parse = false);

namespace ValueRef {

/** The NamedRef class. Looks up a named ValueRef from the NamedValueRefManager */
template <typename T>
struct FO_COMMON_API NamedRef final : public ValueRef<T>
{
    explicit NamedRef(std::string value_ref_name, bool is_only_lookup = false) :
        m_value_ref_name(std::move(value_ref_name)),
        m_is_lookup_only(is_only_lookup)
    {
        TraceLogger() << "ctor(NamedRef<T>): " << typeid(*this).name() << "  value_ref_name: "
                      << m_value_ref_name << "  is_lookup_only: " << m_is_lookup_only;
    }

    [[nodiscard]] bool RootCandidateInvariant() const override
    { return NamedRefInitInvariants() ? m_root_candidate_invariant_local : false; }

    [[nodiscard]] bool LocalCandidateInvariant() const override
    { return NamedRefInitInvariants() ? m_local_candidate_invariant_local : false; }

    [[nodiscard]] bool TargetInvariant() const override
    { return NamedRefInitInvariants() ? m_target_invariant_local : false; }

    [[nodiscard]] bool SourceInvariant() const override
    { return NamedRefInitInvariants() ? m_source_invariant_local : false; }

    [[nodiscard]] bool SimpleIncrement() const override
    { return NamedRefInitInvariants() ? GetValueRef()->SimpleIncrement() : false; }

    [[nodiscard]] bool ConstantExpr() const override
    { return NamedRefInitInvariants() ? GetValueRef()->ConstantExpr() : false; }

    [[nodiscard]] bool operator==(const ValueRef<T>& rhs) const override {
        if (&rhs == this)
            return true;
        if (typeid(rhs) != typeid(*this))
            return false;
        const NamedRef<T>& rhs_ = static_cast<const NamedRef<T>&>(rhs);
        return (m_value_ref_name == rhs_.m_value_ref_name);
    }

    [[nodiscard]] T Eval(const ScriptingContext& context) const override {
        TraceLogger() << "NamedRef<" << typeid(T).name() << ">::Eval()";
        auto ref = GetValueRef();
        if (!ref) {
            ErrorLogger() << "NamedRef<" << typeid(T).name() << ">::Eval did not find " << m_value_ref_name;
            throw std::runtime_error(std::string("NamedValueLookup referenced unknown ValueRef<") + typeid(T).name() + "> named '" + m_value_ref_name + "'");
        }

        auto retval = ref->Eval(context);
        TraceLogger() << "NamedRef<" << typeid(T).name() << "> name: " << m_value_ref_name << "  retval: " << retval;
        return retval;
    }

    [[nodiscard]] std::string Description() const override {
        auto ref = GetValueRef();
        return ref ? ref->Description() : UserString("NAMED_REF_UNKNOWN");
    }

    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override {
        std::string retval = "Named";
        if constexpr (std::is_same_v<T, int>)
            retval += "Integer";
        else if constexpr (std::is_same_v<T, double>)
            retval += "Real";
        else
            retval += "Generic";

        if (m_is_lookup_only)
            retval += "Lookup";
        retval += " name = \"" + m_value_ref_name + "\"";
        if (!m_is_lookup_only) {
            auto ref = GetValueRef();
            retval += " value = " + (ref ? ref->Dump() : " (NAMED_REF_UNKNOWN)");
        }
        return retval;
    }

    void SetTopLevelContent(const std::string& content_name) override;

    [[nodiscard]] const ValueRef<T>* GetValueRef() const {
        TraceLogger() << "NamedRef<T>::GetValueRef() look for registered valueref for \"" << m_value_ref_name << '"';
        return ::GetValueRef<T>(m_value_ref_name, m_is_lookup_only);
    }

    [[nodiscard]] uint32_t GetCheckSum() const override {
        uint32_t retval{0};
        CheckSums::CheckSumCombine(retval, "ValueRef::NamedRef");
        CheckSums::CheckSumCombine(retval, m_value_ref_name);
        TraceLogger() << "GetCheckSum(NamedRef<T>): " << typeid(*this).name() << " retval: " << retval;
        return retval;
    }

    [[nodiscard]] std::unique_ptr<ValueRef<T>> Clone() const override
    { return std::make_unique<NamedRef<T>>(m_value_ref_name, m_is_lookup_only); }

private:
    //! ensures invariants are initialized from registered valueref
    bool NamedRefInitInvariants() const {
        {
            std::scoped_lock invariants_lock(m_invariants_mutex);
            if (m_invariants_initialized)
                return true;
        }

        auto* vref = GetValueRef();
        if (!vref && !m_is_lookup_only) {
            ErrorLogger() << "NamedRef<T>::NamedRefInitInvariants() Trying to use invariants without existing value ref (which should exist in this case)";
            return false;
        } else if (!vref) {
            DebugLogger() << "NamedRef<T>::NamedRefInitInvariants() could not find value ref, will sleep a bit and retry.";
        }

        static constexpr int MAX_TRIES = 5;
        for (int try_num = 1; try_num <= MAX_TRIES; ++try_num) {
            if (vref) {
                std::scoped_lock invariants_lock(m_invariants_mutex);

                // initialize invariants and return
                m_root_candidate_invariant_local = vref->RootCandidateInvariant();
                m_local_candidate_invariant_local = vref->LocalCandidateInvariant();
                m_target_invariant_local = vref->TargetInvariant();
                m_source_invariant_local = vref->SourceInvariant();
                m_invariants_initialized = true;
                return true;

            } else if (try_num == MAX_TRIES) {
                ErrorLogger() << "NamedRef<T>::NamedRefInitInvariants() still could not find value ref after trying "
                              << try_num << " times. Giving up.";

            } else {
                // wait a while for parsing...
                int msecs_count = 200 * try_num;
                std::chrono::milliseconds msecs(msecs_count);
                TraceLogger() << "NamedRef<T>::NamedRefInitInvariants() after try " << try_num
                              << " sleeping for " << msecs_count << " ms before retry.";
                std::this_thread::sleep_for(msecs);

                // try again to get value ref
                vref = GetValueRef();
            }
        }

        WarnLogger() << "NamedRef<T>::NamedRefInitInvariants() Trying to use invariants in a Lookup value ref without existing value ref. "
                     << "Falling back to non-invariance will prevent performance optimisations. This may be a parse race condition.";
        return false;
    }

    const std::string  m_value_ref_name;                 //! registered name of value ref

    mutable bool       m_invariants_initialized = false; //! true if the invariants were initialized from the referenced vale ref
    mutable bool       m_root_candidate_invariant_local = false;
    mutable bool       m_local_candidate_invariant_local = false;
    mutable bool       m_target_invariant_local = false;
    mutable bool       m_source_invariant_local = false;
    mutable std::mutex m_invariants_mutex;

    const bool         m_is_lookup_only;                 //! true if created by a *Lookup in FOCS
};

}

//! Holds all FreeOrion named ValueRef%s.  ValueRefs may be looked up by name.
class FO_COMMON_API NamedValueRefManager {
public:
    //using container_type = std::map<const std::string, const std::unique_ptr<ValueRef::ValueRefBase>>;
    using key_type = const std::string;
    using value_type = std::unique_ptr<ValueRef::ValueRefBase>;
    using int_value_type = std::unique_ptr<ValueRef::ValueRef<int>>;
    using double_value_type = std::unique_ptr<ValueRef::ValueRef<double>>;
    using container_type = std::map<key_type, value_type, std::less<>>;
    using int_container_type = std::map<key_type, int_value_type, std::less<>>;
    using double_container_type = std::map<key_type, double_value_type, std::less<>>;
    using entry_type = std::pair<key_type, value_type>;
    using int_entry_type = std::pair<key_type, int_value_type>;
    using double_entry_type = std::pair<key_type, double_value_type>;
    using any_container_type = std::map<key_type, std::reference_wrapper<ValueRef::ValueRefBase>, std::less<>>;
    using any_entry_type = std::pair<key_type, std::reference_wrapper<ValueRef::ValueRefBase>>;

    using iterator = container_type::const_iterator;

    //! Returns the ValueRef with the name @p name or nullptr if there is no
    //! ValueRef with such a name or of the wrong type use the free function
    //! GetValueRef(...) instead, mainly to save some typing.
    template <typename T>
    const ValueRef::ValueRef<T>* GetValueRef(std::string_view name,
                                             bool wait_for_named_value_focs_txt_parse = false) const
    {
        if (wait_for_named_value_focs_txt_parse)
            CheckPendingNamedValueRefs();
        return dynamic_cast<ValueRef::ValueRef<T>*>(GetValueRefImpl(m_value_refs, "generic", name));
    }

    //! Returns the ValueRef with the name @p name; you should use the
    //! free function GetValueRef(...) instead, mainly to save some typing.
    auto GetValueRefBase(std::string_view name) const -> const ValueRef::ValueRefBase*;

    /** returns a map with all named value refs */
    auto GetItems() const -> any_container_type;

    // Singleton
    NamedValueRefManager& operator=(NamedValueRefManager const&) = delete; // no copy via assignment
    NamedValueRefManager(NamedValueRefManager const&) = delete;            // no copies via construction
    ~NamedValueRefManager() = default;

    //! Returns the instance of this singleton class; you should use the free
    //! function GetNamedValueRefManager() instead
    static NamedValueRefManager& GetNamedValueRefManager();

    //! Returns a number, calculated from the contained data, which should be
    //! different for different contained data, and must be the same for
    //! the same contained data, and must be the same on different platforms
    //! and executions of the program and the function. Useful to verify that
    //! the parsed content is consistent without sending it all between
    //! clients and server.
    auto GetCheckSum() const -> uint32_t;

    using NamedValueRefParseMap = std::map<std::string, std::unique_ptr<ValueRef::ValueRefBase>, std::less<>>;
    //! This sets the asynchronous parse, so we can block on that
    //! when a function needs to access a registry
    FO_COMMON_API void SetNamedValueRefParse(Pending::Pending<NamedValueRefParseMap>&& future)
    { m_pending_named_value_refs_focs_txt = std::move(future); }

    //! Register the @p value_ref under the evaluated @p name.
    template <typename T>
    void RegisterValueRef(std::string&& name, std::unique_ptr<ValueRef::ValueRef<T>>&& vref);

private:
    NamedValueRefManager();

#ifdef __clang__
    template <typename T>
    friend struct ValueRef::NamedRef; // for SetTopLevelContent
#else
    template <typename T>
    friend void ValueRef::NamedRef<T>::SetTopLevelContent(const std::string& content_name);
#endif

    // getter of mutable ValueRef<T>* that can be modified within SetTopLevelContext functions
    template <typename T>
    ValueRef::ValueRef<T>* GetMutableValueRef(std::string_view name,
                                              bool wait_for_named_value_focs_txt_parse = false)
    {
        if (wait_for_named_value_focs_txt_parse)
            CheckPendingNamedValueRefs();
        return dynamic_cast<ValueRef::ValueRef<T>*>(GetValueRefImpl(m_value_refs, "generic", name));
    }

    template <typename V>
    V* GetValueRefImpl(const std::map<NamedValueRefManager::key_type, std::unique_ptr<V>, std::less<>>& registry,
                       std::string_view label, std::string_view name) const
    {
        //TraceLogger() << "NamedValueRefManager::GetValueRef look for registered (" << label << ") valueref for \"" << name << '"';
        //TraceLogger() << "Number of registered (" << label << ") ValueRefs: " << registry.size();
        const auto it = registry.find(name);
        if (it != registry.end())
            return it->second.get();
        DebugLogger() << "NamedValueRefManager::GetValueRef found no registered (" << label << ") valueref for \"" << name
                      << "\". This is may be due to looking in the wrong registry (which can be OK)"
                      << ".  This should not happen if looking in the right registry.";
        return nullptr;
    }

    //! Waits for parsing of named_value_refs.focs.txt to finish
    void CheckPendingNamedValueRefs() const {
        if (!m_pending_named_value_refs_focs_txt)
            return;
        // we block on the asynchronous parse
        // throw away the result, the parser already registered the values
        WaitForPending(m_pending_named_value_refs_focs_txt, /*do not care about result*/true);
    }

    mutable boost::optional<Pending::Pending<NamedValueRefParseMap>> m_pending_named_value_refs_focs_txt = boost::none;

    //! Map of ValueRef%s identified by a name and mutexes for those to allow asynchronous registration
    double_container_type m_value_refs_double; // int value refs
    std::mutex            m_value_refs_double_mutex;
    int_container_type    m_value_refs_int; // int value refs
    std::mutex            m_value_refs_int_mutex;
    container_type        m_value_refs; // everything else
    std::mutex            m_value_refs_mutex;

    // The s_instance creation is lazily triggered via a function local.
    // There is exactly one for all translation units.
    static NamedValueRefManager* s_instance;
};


// Template Implementations
///////////////////////////////////////////////////////////
// NamedValueRefManager                                  //
///////////////////////////////////////////////////////////
template<>
FO_COMMON_API void NamedValueRefManager::RegisterValueRef(
    std::string&& name, std::unique_ptr<ValueRef::ValueRef<PlanetType>>&& vref);

template<>
FO_COMMON_API void NamedValueRefManager::RegisterValueRef(
    std::string&& name, std::unique_ptr<ValueRef::ValueRef<PlanetEnvironment>>&& vref);

template<>
[[nodiscard]] FO_COMMON_API const ValueRef::ValueRef<int>* NamedValueRefManager::GetValueRef(
    std::string_view, bool) const;

template<>
[[nodiscard]] FO_COMMON_API const ValueRef::ValueRef<double>* NamedValueRefManager::GetValueRef(
    std::string_view, bool) const;

template<>
[[nodiscard]] ValueRef::ValueRef<int>* NamedValueRefManager::GetMutableValueRef(std::string_view, bool);

template<>
[[nodiscard]] ValueRef::ValueRef<double>* NamedValueRefManager::GetMutableValueRef(std::string_view, bool);


///////////////////////////////////////////////////////////
// NamedRef                                              //
///////////////////////////////////////////////////////////
template <typename T>
void ::ValueRef::NamedRef<T>::SetTopLevelContent(const std::string& content_name)
{
    if (m_is_lookup_only) {
        TraceLogger() << "Ignored call of SetTopLevelContent(" << content_name
                      << ") on a Lookup NamedRef for value ref " << m_value_ref_name;
        return;
    }
    // only supposed to work for named-in-the-middle-case, SetTopLevelContent checks that
    if (auto ref = GetNamedValueRefManager().GetMutableValueRef<T>(m_value_ref_name, m_is_lookup_only)) {
        ref->SetTopLevelContent(content_name);
    } else {
        const char* named_ref_kind = ( content_name == "THERE_IS_NO_TOP_LEVEL_CONTENT" ? "top-level" : "named-in-the-middle" );
        ErrorLogger() << "Unexpected call of SetTopLevelContent(" << content_name
                      << ") on a " << named_ref_kind
                      << " NamedRef - unexpected because no value ref " << m_value_ref_name
                      << " registered yet. Should not happen";
    }
}


//! Returns the ValueRef object registered with the given
//! @p name.  If no such ValueRef exists, nullptr is returned instead.
[[nodiscard]] FO_COMMON_API const ValueRef::ValueRefBase* GetValueRefBase(std::string_view name);

//! Returns the ValueRef object registered with the given
//! @p name in the registry matching the given type T.  If no such ValueRef exists, nullptr is returned instead.
template <typename T>
[[nodiscard]] FO_COMMON_API const ValueRef::ValueRef<T>* GetValueRef(
    std::string_view name, bool wait_for_named_value_focs_txt_parse)
{ return GetNamedValueRefManager().GetValueRef<T>(name, wait_for_named_value_focs_txt_parse); }

//! Register and take possesion of the ValueRef object @p vref under the given @p name.
template <typename T>
FO_COMMON_API void RegisterValueRef(
    std::string name, std::unique_ptr<ValueRef::ValueRef<T>>&& vref)
{ return GetNamedValueRefManager().RegisterValueRef<T>(std::move(name), std::move(vref)); }

#endif // _ValueRefManager_h_
