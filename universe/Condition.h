// -*- C++ -*-
#ifndef _Condition_h_
#define _Condition_h_

#include "Meter.h"
#include "Planet.h"
#include "System.h"
#include "ValueRef.h"

class UniverseObject;

/** this namespace holds ConditionBase and its subclasses; these classes represent predicates about UniverseObjects used
    by, for instance, the Effect system. */
namespace Condition {
    struct ConditionBase;
    struct All;
    struct EmpireAffiliation;
    struct Self;
    struct Type;
    struct Building;
    struct HasSpecial;
    struct Contains;
    struct PlanetEnvironment;
    struct PlanetSize;
    struct FocusType;
    struct StarType;
    struct Chance;
    struct MeterValue;
    struct StockpileValue;
    struct VisibleToEmpire;
    struct WithinDistance;
    struct WithinStarlaneJumps;
    struct EffectTarget;
    struct And;
    struct Or;
    struct Not;
    GG::XMLObjectFactory<ConditionBase> ConditionFactory(); ///< an XML factory that creates the right subclass of ConditionBase from a given XML element
}

struct Condition::ConditionBase
{
    typedef std::set<UniverseObject*> ObjectSet;

    enum SearchDomain {
        NON_TARGETS, ///< The Condition will only examine items in the nontarget set; those that match the Condition will be inserted into the target set.
        TARGETS      ///< The Condition will only examine items in the target set; those that do not match the Condition will be inserted into the nontarget set.
    };

    ConditionBase();
    virtual ~ConditionBase();
    virtual void Eval(const UniverseObject* source, ObjectSet& targets, ObjectSet& non_targets, SearchDomain search_domain = NON_TARGETS) const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
};

struct Condition::All : Condition::ConditionBase
{
    All();
    All(const GG::XMLElement& elem);
    virtual void Eval(const UniverseObject* source, ObjectSet& targets, ObjectSet& non_targets, SearchDomain search_domain = NON_TARGETS) const;
};

struct Condition::EmpireAffiliation : Condition::ConditionBase
{
    EmpireAffiliation(const ValueRef::ValueRefBase<int>* empire_id, EmpireAffiliationType affiliation, bool exclusive);
    EmpireAffiliation(const GG::XMLElement& elem);
    virtual ~EmpireAffiliation();

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    const ValueRef::ValueRefBase<int>* m_empire_id;
    EmpireAffiliationType              m_affiliation;
    bool                               m_exclusive;
};

struct Condition::Self : Condition::ConditionBase
{
    Self();
    Self(const GG::XMLElement& elem);

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
};

struct Condition::Type : Condition::ConditionBase
{
    Type(const ValueRef::ValueRefBase<UniverseObjectType>* type);
    Type(const GG::XMLElement& elem);

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    const ValueRef::ValueRefBase<UniverseObjectType>* m_type;
};

struct Condition::Building : Condition::ConditionBase
{
    Building(const std::string& name);
    Building(const GG::XMLElement& elem);

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    std::string m_name;
};

/* accepts "All" */
struct Condition::HasSpecial : Condition::ConditionBase
{
    HasSpecial(const std::string& name);
    HasSpecial(const GG::XMLElement& elem);

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    std::string m_name;
};

struct Condition::Contains : Condition::ConditionBase
{
    Contains(const ConditionBase* condition);
    Contains(const GG::XMLElement& elem);

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    const ConditionBase* m_condition;
};

struct Condition::PlanetEnvironment : Condition::ConditionBase
{
    PlanetEnvironment(const std::vector<const ValueRef::ValueRefBase< ::PlanetEnvironment>*>& environments);
    PlanetEnvironment(const GG::XMLElement& elem);
    virtual ~PlanetEnvironment();

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    std::vector<const ValueRef::ValueRefBase< ::PlanetEnvironment>*> m_environments;
};

struct Condition::PlanetSize : Condition::ConditionBase
{
    PlanetSize(const std::vector<const ValueRef::ValueRefBase< ::PlanetSize>*>& sizes);
    PlanetSize(const GG::XMLElement& elem);
    virtual ~PlanetSize();

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    std::vector<const ValueRef::ValueRefBase< ::PlanetSize>*> m_sizes;
};

struct Condition::FocusType : Condition::ConditionBase
{
    FocusType(const std::vector<const ValueRef::ValueRefBase< ::FocusType>*>& foci, bool primary);
    FocusType(const GG::XMLElement& elem);
    virtual ~FocusType();

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    std::vector<const ValueRef::ValueRefBase< ::FocusType>*> m_foci;
    bool m_primary;
};

struct Condition::StarType : Condition::ConditionBase
{
    StarType(const std::vector<const ValueRef::ValueRefBase< ::StarType>*>& types);
    StarType(const GG::XMLElement& elem);
    virtual ~StarType();

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    std::vector<const ValueRef::ValueRefBase< ::StarType>*> m_types;
};

struct Condition::Chance : Condition::ConditionBase
{
    Chance(const ValueRef::ValueRefBase<double>* chance);
    Chance(const GG::XMLElement& elem);
    virtual ~Chance();

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    const ValueRef::ValueRefBase<double>* m_chance;
};

struct Condition::MeterValue : Condition::ConditionBase
{
    MeterValue(MeterType meter, const ValueRef::ValueRefBase<double>* low, const ValueRef::ValueRefBase<double>* high, bool max_meter);
    MeterValue(const GG::XMLElement& elem);
    virtual ~MeterValue();

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    MeterType                             m_meter;
    const ValueRef::ValueRefBase<double>* m_low;
    const ValueRef::ValueRefBase<double>* m_high;
    bool                                  m_max_meter;
};

/*struct Condition::StockpileValue : Condition::ConditionBase
{
    StockpileValue(StockpileType stockpile, const ValueRef::ValueRefBase<double>* low, const ValueRef::ValueRefBase<double>* high);
    StockpileValue(const GG::XMLElement& elem);
    virtual ~StockpileValue();

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    StockpileType m_stockpile;
    const ValueRef::ValueRefBase<double>* m_low;
    const ValueRef::ValueRefBase<double>* m_high;
};*/

struct Condition::VisibleToEmpire : Condition::ConditionBase
{
    VisibleToEmpire(const std::vector<const ValueRef::ValueRefBase<int>*>& empire_ids);
    VisibleToEmpire(const GG::XMLElement& elem);
    virtual ~VisibleToEmpire();

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    std::vector<const ValueRef::ValueRefBase<int>*> m_empire_ids;
};

struct Condition::WithinDistance : Condition::ConditionBase
{
    WithinDistance(const ValueRef::ValueRefBase<double>* distance, const ConditionBase* condition);
    WithinDistance(const GG::XMLElement& elem);
    virtual ~WithinDistance();
    virtual void Eval(const UniverseObject* source, ObjectSet& targets, ObjectSet& non_targets, SearchDomain search_domain = NON_TARGETS) const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    const ValueRef::ValueRefBase<double>* m_distance;
    const ConditionBase*                  m_condition;
};

struct Condition::WithinStarlaneJumps : Condition::ConditionBase
{
    WithinStarlaneJumps(const ValueRef::ValueRefBase<int>* jumps, const ConditionBase* condition);
    WithinStarlaneJumps(const GG::XMLElement& elem);
    virtual ~WithinStarlaneJumps();
    virtual void Eval(const UniverseObject* source, ObjectSet& targets, ObjectSet& non_targets, SearchDomain search_domain = NON_TARGETS) const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    const ValueRef::ValueRefBase<int>* m_jumps;
    const ConditionBase*               m_condition;
};

struct Condition::EffectTarget : Condition::ConditionBase
{
    EffectTarget();
    EffectTarget(const GG::XMLElement& elem);

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
};

struct Condition::And : Condition::ConditionBase
{
    And(const std::vector<const ConditionBase*>& operands);
    And(const GG::XMLElement& elem);
    virtual ~And();
    virtual void Eval(const UniverseObject* source, ObjectSet& targets, ObjectSet& non_targets, SearchDomain search_domain = NON_TARGETS) const;

private:
    std::vector<const ConditionBase*> m_operands;
};

struct Condition::Or : Condition::ConditionBase
{
    Or(const std::vector<const ConditionBase*>& operands);
    Or(const GG::XMLElement& elem);
    virtual ~Or();
    virtual void Eval(const UniverseObject* source, ObjectSet& targets, ObjectSet& non_targets, SearchDomain search_domain = NON_TARGETS) const;

private:
    std::vector<const ConditionBase*> m_operands;
};

struct Condition::Not : Condition::ConditionBase
{
    Not(const ConditionBase* operand);
    Not(const GG::XMLElement& elem);
    virtual ~Not();
    virtual void Eval(const UniverseObject* source, ObjectSet& targets, ObjectSet& non_targets, SearchDomain search_domain = NON_TARGETS) const;

private:
    const ConditionBase* m_operand;
};

#endif // _Condition_h_
