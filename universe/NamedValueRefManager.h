#ifndef _ValueRefManager_h_
#define _ValueRefManager_h_

#include <map>
#include "ValueRef.h"

namespace ValueRef {
    
/** The NamedRef class. Looks up a named ValueRef from the NamedValueRefManager
  */
template <typename T>
struct FO_COMMON_API NamedRef final : public ValueRef<T>
{
    NamedRef(std::string value_ref_name, bool is_only_lookup = false);

    bool RootCandidateInvariant() const override;
    bool LocalCandidateInvariant() const override;
    bool TargetInvariant() const override;
    bool SourceInvariant() const override;
    bool SimpleIncrement() const override;
    bool ConstantExpr() const override;

    bool operator==(const ValueRef<T>& rhs) const override;
    T  Eval(const ScriptingContext& context) const override;

    std::string Description() const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;

    const ValueRef<T>* GetValueRef() const;
    unsigned int GetCheckSum() const override;

private:
    //! initialises invariants from registered valueref, waits a bit for registration, use on first access
    bool NamedRefInitInvariants();

    std::string m_value_ref_name;           //! registered name of value ref
    bool m_invariants_initialized = false;  //! true if the invariants were initialized from the referenced vale ref
    bool m_is_lookup_only;                  //! true if created by a *Lookup in FOCS
};
}

//! Holds all FreeOrion named ValueRef%s.  ValueRef%s may be looked up by name.
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
    using entry_type        = std::pair<key_type, value_type>;
    using int_entry_type    = std::pair<key_type, int_value_type>;
    using double_entry_type = std::pair<key_type, double_value_type>;
    using any_container_type = std::map<key_type, std::reference_wrapper<ValueRef::ValueRefBase>>;
    using any_entry_type     = std::pair<key_type, std::reference_wrapper<ValueRef::ValueRefBase>>;

    using iterator = container_type::const_iterator;

    //! Returns the ValueRef with the name @p name or nullptr if there is nov ValueRef with such a name or of the wrong type
    //! use the free function GetValueRef(...) instead, mainly to save some typing.
    template <typename T>
    auto GetValueRef(const std::string& name, const bool wait_for_named_value_focs_txt_parse = false) -> ValueRef::ValueRef<T>* const;

    //! Returns the ValueRef with the name @p name; you should use the
    //! free function GetValueRef(...) instead, mainly to save some typing.
    auto GetValueRefBase(const std::string& name) const -> ValueRef::ValueRefBase* const;

    /** returns a map with all named value refs */
    auto GetItems() const -> any_container_type;

    // Singleton
    NamedValueRefManager&  operator=(NamedValueRefManager const&) = delete; // no copy via assignment

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
    FO_COMMON_API void SetNamedValueRefParse(Pending::Pending<NamedValueRefParseMap>&& future);

    //! Register the @p value_ref under the evaluated @p name.
    template <typename T>
    void RegisterValueRef(std::string&& name, std::unique_ptr<ValueRef::ValueRef<T>>&& vref);

private:
    NamedValueRefManager();

    //! Waits for parsing of named_value_refs.focs.txt to finish
    void CheckPendingNamedValueRefs() const;

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

//! Returns the singleton manager for named value refs
FO_COMMON_API auto GetNamedValueRefManager() -> NamedValueRefManager&;

//! Returns the ValueRef object registered with the given
//! @p name.  If no such ValueRef exists, nullptr is returned instead.
FO_COMMON_API auto GetValueRefBase(const std::string& name) -> ValueRef::ValueRefBase* const;

//! Returns the ValueRef object registered with the given
//! @p name in the registry matching the given type T.  If no such ValueRef exists, nullptr is returned instead.
template <typename T>
FO_COMMON_API auto GetValueRef(const std::string& name, const bool wait_for_named_value_focs_txt_parse = false) -> ValueRef::ValueRef<T>* const;

//! Register and take possesion of the ValueRef object @p vref under the given @p name.
template <typename T>
FO_COMMON_API void RegisterValueRef(std::string name, std::unique_ptr<ValueRef::ValueRef<T>>&& vref);

#endif // _ValueRefManager_h_
