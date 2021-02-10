#ifndef _ValueRefManager_h_
#define _ValueRefManager_h_

#include <map>
#include "ValueRef.h"

FO_COMMON_API const std::string& UserString(const std::string& str);

namespace CheckSums {
    FO_COMMON_API void CheckSumCombine(unsigned int& sum, const char* s);
    FO_COMMON_API void CheckSumCombine(unsigned int& sum, const std::string& c);
}

class NamedValueRefManager;
FO_COMMON_API auto GetNamedValueRefManager() -> NamedValueRefManager&;

template <typename T>
FO_COMMON_API ValueRef::ValueRef<T>* const GetValueRef(const std::string& name,
                                                       const bool wait_for_named_value_focs_txt_parse = false);

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

    bool RootCandidateInvariant() const override
    { return (const_cast<NamedRef<T>*>(this))->NamedRefInitInvariants() ? ValueRefBase::m_root_candidate_invariant : false; }

    bool LocalCandidateInvariant() const override
    { return (const_cast<NamedRef<T>*>(this))->NamedRefInitInvariants() ? ValueRefBase::m_local_candidate_invariant : false; }

    bool TargetInvariant() const override
    { return (const_cast<NamedRef<T>*>(this))->NamedRefInitInvariants() ? ValueRefBase::m_target_invariant : false; }

    bool SourceInvariant() const override
    { return (const_cast<NamedRef<T>*>(this))->NamedRefInitInvariants() ? ValueRefBase::m_source_invariant : false; }

    bool SimpleIncrement() const override
    { return (const_cast<NamedRef<T>*>(this))->NamedRefInitInvariants() ? GetValueRef()->SimpleIncrement() : false; }

    bool ConstantExpr() const override
    { return (const_cast<NamedRef<T>*>(this))->NamedRefInitInvariants() ? GetValueRef()->ConstantExpr() : false; }

    bool operator==(const ValueRef<T>& rhs) const override {
        if (&rhs == this)
            return true;
        if (typeid(rhs) != typeid(*this))
            return false;
        const NamedRef<T>& rhs_ = static_cast<const NamedRef<T>&>(rhs);
        return (m_value_ref_name == rhs_.m_value_ref_name);
    }

    T Eval(const ScriptingContext& context) const override {
        TraceLogger() << "NamedRef<" << typeid(T).name() << ">::Eval()";
        auto value_ref = ::GetValueRef<T>(m_value_ref_name);
        if (!value_ref) {
            ErrorLogger() << "NamedRef<" << typeid(T).name() << ">::Eval did not find " << m_value_ref_name;
            throw std::runtime_error(std::string("NamedValueLookup referenced unknown ValueRef<") + typeid(T).name() + "> named '" + m_value_ref_name + "'");
        }

        auto retval = value_ref->Eval(context);
        TraceLogger() << "NamedRef<" << typeid(T).name() << "> name: " << m_value_ref_name << "  retval: " << retval;
        return retval;
    }

    std::string Description() const override
    { return GetValueRef() ? GetValueRef()->Description() : UserString("NAMED_REF_UNKNOWN"); }

    std::string Dump(unsigned short ntabs = 0) const override
    { return GetValueRef() ? GetValueRef()->Dump() : "NAMED_REF_UNKNOWN"; }

    void SetTopLevelContent(const std::string& content_name) override {
        if (m_is_lookup_only) {
            TraceLogger() << "Ignored call of SetTopLevelContent(" << content_name
                          << ") on a Lookup NamedRef for value ref " << m_value_ref_name;
            return;
        }
        // only supposed to work for named-in-the-middle-case, SetTopLevelContent checks that
        if (GetValueRef()) {
            const_cast<ValueRef<T>*>(GetValueRef())->SetTopLevelContent(content_name);
        } else {
            const char* named_ref_kind = ( content_name == "THERE_IS_NO_TOP_LEVEL_CONTENT" ? "top-level" : "named-in-the-middle" );
            ErrorLogger() << "Unexpected call of SetTopLevelContent(" << content_name
                          << ") on a " << named_ref_kind
                          << " NamedRef - unexpected because no value ref " << m_value_ref_name
                          << " registered yet. Should not happen";
        }
    }

    const ValueRef<T>* GetValueRef() const {
        TraceLogger() << "NamedRef<T>::GetValueRef() look for registered valueref for \"" << m_value_ref_name << '"';
        return ::GetValueRef<T>(m_value_ref_name, m_is_lookup_only);
    }

    unsigned int GetCheckSum() const override {
        unsigned int retval{0};
        CheckSums::CheckSumCombine(retval, "ValueRef::NamedRef");
        CheckSums::CheckSumCombine(retval, m_value_ref_name);
        TraceLogger() << "GetCheckSum(NamedRef<T>): " << typeid(*this).name() << " retval: " << retval;
        return retval;
    }


private:
    //! initialises invariants from registered valueref, waits a bit for registration, use on first access
    bool NamedRefInitInvariants() {
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

    const std::string m_value_ref_name;                 //! registered name of value ref
    bool              m_invariants_initialized = false; //! true if the invariants were initialized from the referenced vale ref
    const bool        m_is_lookup_only;                 //! true if created by a *Lookup in FOCS
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
    using container_type = std::map<key_type, value_type>;
    using int_container_type = std::map<key_type, int_value_type>;
    using double_container_type = std::map<key_type, double_value_type>;
    using entry_type = std::pair<key_type, value_type>;
    using int_entry_type = std::pair<key_type, int_value_type>;
    using double_entry_type = std::pair<key_type, double_value_type>;
    using any_container_type = std::map<key_type, std::reference_wrapper<ValueRef::ValueRefBase>>;
    using any_entry_type = std::pair<key_type, std::reference_wrapper<ValueRef::ValueRefBase>>;

    using iterator = container_type::const_iterator;

    //! Returns the ValueRef with the name @p name or nullptr if there is no
    //! ValueRef with such a name or of the wrong type use the free function
    //!  GetValueRef(...) instead, mainly to save some typing.
    template <typename T>
    ValueRef::ValueRef<T>* const GetValueRef(const std::string& name,
                                             const bool wait_for_named_value_focs_txt_parse = false)
    {
        if (wait_for_named_value_focs_txt_parse)
            CheckPendingNamedValueRefs();
        return dynamic_cast<ValueRef::ValueRef<T>*>(GetValueRefImpl(m_value_refs, "generic", name));
    }

    //! Returns the ValueRef with the name @p name; you should use the
    //! free function GetValueRef(...) instead, mainly to save some typing.
    auto GetValueRefBase(const std::string& name) const -> ValueRef::ValueRefBase* const;

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
    auto GetCheckSum() const -> unsigned int;

    using NamedValueRefParseMap = std::map<std::string, std::unique_ptr<ValueRef::ValueRefBase>>;
    //! This sets the asynchronous parse, so we can block on that
    //! when a function needs to access a registry
    FO_COMMON_API void SetNamedValueRefParse(Pending::Pending<NamedValueRefParseMap>&& future)
    { m_pending_named_value_refs_focs_txt = std::move(future); }

    //! Register the @p value_ref under the evaluated @p name.
    template <typename T>
    void RegisterValueRef(std::string&& name, std::unique_ptr<ValueRef::ValueRef<T>>&& vref);

private:
    NamedValueRefManager();

    template <typename V>
    V* const GetValueRefImpl(std::map<NamedValueRefManager::key_type, std::unique_ptr<V>>& registry,
                             const std::string& label, const std::string& name)
    {
        TraceLogger() << "NamedValueRefManager::GetValueRef look for registered " << label << " valueref for \"" << name << '"';
        TraceLogger() << "Number of registered " << label << " ValueRefs: " << registry.size();
        const auto it = registry.find(name);
        if (it != registry.end())
            return it->second.get();
        WarnLogger() << "NamedValueRefManager::GetValueRef found no registered " << label << " valueref for \"" << name << "\". This should not happen once \"#3225 Refactor initialisation of invariants in value refs to happen after parsing\" is implemented";
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


template<>
FO_COMMON_API void NamedValueRefManager::RegisterValueRef(std::string&& name, std::unique_ptr<ValueRef::ValueRef<PlanetType>>&& vref);

template<>
FO_COMMON_API void NamedValueRefManager::RegisterValueRef(std::string&& name, std::unique_ptr<ValueRef::ValueRef<PlanetEnvironment>>&& vref);

template<>
FO_COMMON_API ValueRef::ValueRef<int>* const NamedValueRefManager::GetValueRef(const std::string&, const bool);

template<>
FO_COMMON_API ValueRef::ValueRef<double>* const NamedValueRefManager::GetValueRef(const std::string&, const bool);


//! Returns the ValueRef object registered with the given
//! @p name.  If no such ValueRef exists, nullptr is returned instead.
FO_COMMON_API auto GetValueRefBase(const std::string& name) -> ValueRef::ValueRefBase* const;

//! Returns the ValueRef object registered with the given
//! @p name in the registry matching the given type T.  If no such ValueRef exists, nullptr is returned instead.
template <typename T>
FO_COMMON_API ValueRef::ValueRef<T>* const GetValueRef(const std::string& name,
                                                       const bool wait_for_named_value_focs_txt_parse)
{ return GetNamedValueRefManager().GetValueRef<T>(name, wait_for_named_value_focs_txt_parse); }

//! Register and take possesion of the ValueRef object @p vref under the given @p name.
template <typename T>
FO_COMMON_API void RegisterValueRef(std::string name, std::unique_ptr<ValueRef::ValueRef<T>>&& vref)
{ return GetNamedValueRefManager().RegisterValueRef<T>(std::move(name), std::move(vref)); }

#endif // _ValueRefManager_h_
