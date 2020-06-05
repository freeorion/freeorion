#ifndef _Government_h_
#define _Government_h_

#include "../universe/ValueRef.h"
#include "../util/Export.h"
#include "../util/Pending.h"

#include <boost/optional/optional.hpp>

#include <memory>
#include <string>
#include <map>
#include <set>


namespace Effect {
    class EffectsGroup;
}

class FO_COMMON_API Policy {
public:
    Policy(std::string name, std::string description,
           std::string short_description, std::string category,
           std::unique_ptr<ValueRef::ValueRef<double>>&& adoption_cost,
           std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects,
           std::string graphic);

    /** \name Accessors */ //@{
    const std::string&  Name() const                { return m_name; }
    const std::string&  Description() const         { return m_description; }
    const std::string&  ShortDescription() const    { return m_short_description; }
    std::string         Dump(unsigned short ntabs = 0) const;
    const std::string&  Category() const            { return m_category; }
    float               AdoptionCost(int empire_id) const;

    //! returns the effects that are applied to the discovering empire's capital
    //! when this policy is adopted.
    const std::vector<std::shared_ptr<Effect::EffectsGroup>>& Effects() const
    { return m_effects; }

    const std::string&  Graphic() const       { return m_graphic; }

    //! Returns a number, calculated from the contained data, which should be
    //! different for different contained data, and must be the same for
    //! the same contained data, and must be the same on different platforms
    //! and executions of the program and the function. Useful to verify that
    //! the parsed content is consistent without sending it all between
    //! clients and server. */
    unsigned int        GetCheckSum() const;
    //@}

private:
    Policy(const Policy&);                  // disabled
    const Policy& operator=(const Policy&); // disabled
    void Init();

    std::string                                         m_name;
    std::string                                         m_description;
    std::string                                         m_short_description;
    std::string                                         m_category;
    std::unique_ptr<ValueRef::ValueRef<double>>         m_adoption_cost;
    std::vector<std::shared_ptr<Effect::EffectsGroup>>  m_effects;
    std::string                                         m_graphic;

    friend class PolicyManager;
};

//! Keeps track of policies that can be chosen by empires.
class FO_COMMON_API PolicyManager {
public:
    using PoliciesTypeMap = std::map<std::string, std::unique_ptr<Policy>>;
    using iterator = PoliciesTypeMap::const_iterator;

    /** \name Structors */ //@{
    PolicyManager();
    ~PolicyManager();
    //@}

    /** \name Accessors */ //@{
    //! returns the policy with the name \a name; you should use the free
    //! function GetPolicy() instead
    const Policy*               GetPolicy(const std::string& name) const;
    std::vector<std::string>    PolicyNames() const;
    //! returns list of names of policies in specified category
    std::vector<std::string>    PolicyNames(const std::string& name) const;
    std::set<std::string>       PolicyCategories() const;
    unsigned int                GetCheckSum() const;

    iterator begin() const; //! iterator to the first policy
    iterator end() const;   //! iterator to the last + 1th policy
    //@}

    //! sets types to the value of \p future
    void SetPolicies(Pending::Pending<PoliciesTypeMap>&& future);

private:
    void CheckPendingPolicies() const;  //! Assigns any m_pending_types to m_specials.

    //! Future types being parsed by parser.  mutable so that it can
    //! be assigned to m_species_types when completed.
    mutable boost::optional<Pending::Pending<PoliciesTypeMap>> m_pending_types = boost::none;

    mutable PoliciesTypeMap m_policies;
};

FO_COMMON_API PolicyManager& GetPolicyManager();
FO_COMMON_API const Policy* GetPolicy(const std::string& name);


#endif // _Government_h_
