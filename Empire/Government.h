#ifndef _Government_h_
#define _Government_h_

#include "../Universe/ValueRefFwd.h"

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
    Policy(const std::string& name, const std::string& description,
           const std::string& short_description, const std::string& category,
           std::unique_ptr<ValueRef::ValueRefBase<double>>&& adoption_cost,
           std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects,
           const std::string& graphic);
    ~Policy();

    /** \name Accessors */ //@{
    const std::string&  Name() const                { return m_name; }              //!< returns name of this tech
    const std::string&  Description() const         { return m_description; }       //!< Returns the text description of this tech
    const std::string&  ShortDescription() const    { return m_short_description; } //!< Returns the single-line short text description of this tech
    std::string         Dump(unsigned short ntabs = 0) const;                       //!< Returns a text representation of this object
    const std::string&  Category() const            { return m_category; }          //!< retursn the name of the category to which this tech belongs
    float               AdoptionCost(int empire_id) const;                          //!< returns the total research cost in RPs required to research this tech

    /** returns the effects that are applied to the discovering empire's capital
      * when this tech is researched; not all techs have effects, in which case
      * this returns 0 */
    const std::vector<std::shared_ptr<Effect::EffectsGroup>>& Effects() const
    { return m_effects; }

    const std::string&  Graphic() const       { return m_graphic; }         //!< returns the name of the grapic file for this tech

    /** Returns a number, calculated from the contained data, which should be
      * different for different contained data, and must be the same for
      * the same contained data, and must be the same on different platforms
      * and executions of the program and the function. Useful to verify that
      * the parsed content is consistent without sending it all between
      * clients and server. */
    unsigned int        GetCheckSum() const;
    //@}

private:
    Policy(const Policy&);                  // disabled
    const Policy& operator=(const Policy&); // disabled
    void Init();

    std::string                                         m_name = "";
    std::string                                         m_description = "";
    std::string                                         m_short_description = "";
    std::string                                         m_category = "";
    std::unique_ptr<ValueRef::ValueRefBase<double>>     m_adoption_cost = nullptr;
    std::vector<std::shared_ptr<Effect::EffectsGroup>>  m_effects;
    std::string                                         m_graphic = "";

    friend class PolicyManager;
};

/** Keeps track of policies that can be chosen by empires. */
class FO_COMMON_API PolicyManager {
public:
    using PoliciesTypeMap = std::map<std::string, std::unique_ptr<Policy>>;

    /** \name Structors */ //@{
    PolicyManager();
    ~PolicyManager();
    //@}

    /** \name Accessors */ //@{
    /** returns the policy with the name \a name; you should use the free function GetPolicy() instead */
    const Policy*                   GetPolicy(const std::string& name) const;
    std::vector<std::string>        PolicyNames() const;
    unsigned int                    GetCheckSum() const;
    //@}

    /** Sets types to the value of \p future. */
    void SetPolicies(Pending::Pending<PoliciesTypeMap>&& future);

private:
    /** Assigns any m_pending_types to m_specials. */
    void CheckPendingPolicies() const;

    /** Future types being parsed by parser.  mutable so that it can
        be assigned to m_species_types when completed.*/
    mutable boost::optional<Pending::Pending<PoliciesTypeMap>> m_pending_types = boost::none;

    mutable PoliciesTypeMap m_policies;
};

/** returns the singleton policy manager */
FO_COMMON_API PolicyManager& GetPolicyManager();

/** returns a pointer to the policy with the name \a name, or 0 if no such tech exists */
FO_COMMON_API const Policy* GetPolicy(const std::string& name);


#endif // _Government_h_
