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
    struct PlanetSize;
    struct PlanetType;
    struct PlanetEnvironment;
    struct FocusType;
    struct StarType;
    struct Chance;
    struct MeterValue;
    struct EmpireStockpileValue;
    struct OwnerHasTech;
    struct VisibleToEmpire;
    struct WithinDistance;
    struct WithinStarlaneJumps;
    struct EffectTarget;
    struct And;
    struct Or;
    struct Not;
	struct Turn;
	struct NumberOf;
	struct ContainedBy;
}

/** The base class for all Conditions. */
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
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
};

/** Matches all objects if the current game turn is >= \a low and < \a high. */
struct Condition::Turn : Condition::ConditionBase
{
    Turn(const ValueRef::ValueRefBase<int>* low, const ValueRef::ValueRefBase<int>* high);
    virtual ~Turn();
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    const ValueRef::ValueRefBase<int>* m_low;
    const ValueRef::ValueRefBase<int>* m_high;
};

/** Matches a randomly selected \a number of objects that match Condition \a condition, or as many objects
    as match the condition if the number of objects is less than the number requested. */
struct Condition::NumberOf : Condition::ConditionBase
{
    NumberOf(const ValueRef::ValueRefBase<int>* number, const ConditionBase* condition);
    virtual ~NumberOf();
    virtual void Eval(const UniverseObject* source, ObjectSet& targets, ObjectSet& non_targets, SearchDomain search_domain = NON_TARGETS) const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    const ValueRef::ValueRefBase<int>* m_number;
	const ConditionBase*               m_condition;
};

/** Matches all objects. */
struct Condition::All : Condition::ConditionBase
{
    All();
    virtual void Eval(const UniverseObject* source, ObjectSet& targets, ObjectSet& non_targets, SearchDomain search_domain = NON_TARGETS) const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;
};

/** Matches all objects that are owned (if \a exclusive == false) or only owned (if \a exclusive == true) by an empire that has
    affilitation type \a affilitation with Empire \a empire_id. */
struct Condition::EmpireAffiliation : Condition::ConditionBase
{
    EmpireAffiliation(const ValueRef::ValueRefBase<int>* empire_id, EmpireAffiliationType affiliation, bool exclusive);
    virtual ~EmpireAffiliation();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    const ValueRef::ValueRefBase<int>* m_empire_id;
    EmpireAffiliationType              m_affiliation;
    bool                               m_exclusive;
};

/** Matches the source object only. */
struct Condition::Self : Condition::ConditionBase
{
    Self();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
};

/** Matches all objects that are of UniverseObjectType \a type. */
struct Condition::Type : Condition::ConditionBase
{
    Type(const ValueRef::ValueRefBase<UniverseObjectType>* type);
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    const ValueRef::ValueRefBase<UniverseObjectType>* m_type;
};

/** Matches all Building objects of the sort specified by \a name. */
struct Condition::Building : Condition::ConditionBase
{
    Building(const std::string& name);
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    std::string m_name;
};

/** Matches all objects that have an attached Special of the sort specified by \a name.  Passing "All" for
    \a name will match all objects with attached Specials. */
struct Condition::HasSpecial : Condition::ConditionBase
{
    HasSpecial(const std::string& name);
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    std::string m_name;
};

/** Matches all objects that contain an object that matches Condition \a condition.  Container objects are Systems,
    Planets (which contain Buildings), and Fleets (which contain Ships). */
struct Condition::Contains : Condition::ConditionBase
{
    Contains(const ConditionBase* condition);
    virtual void Eval(const UniverseObject* source, ObjectSet& targets, ObjectSet& non_targets, SearchDomain search_domain = NON_TARGETS) const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    const ConditionBase* m_condition;
};

/** Matches all objects that are contained by an object that matches Condition \a condition.  Container objects
    are Systems, Planets (which contain Buildings), and Fleets (which contain Ships). */
struct Condition::ContainedBy : Condition::ConditionBase
{
    ContainedBy(const ConditionBase* condition);
    virtual void Eval(const UniverseObject* source, ObjectSet& targets, ObjectSet& non_targets, SearchDomain search_domain = NON_TARGETS) const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    const ConditionBase* m_condition;
};

/** Matches all Planet objects that have one of the PlanetTypes in \a types.  Note that all
    Building objects which are on matching planets are also matched. */
struct Condition::PlanetType : Condition::ConditionBase
{
    PlanetType(const std::vector<const ValueRef::ValueRefBase< ::PlanetType>*>& types);
    virtual ~PlanetType();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    std::vector<const ValueRef::ValueRefBase< ::PlanetType>*> m_types;
};

/** Matches all Planet objects that have one of the PlanetSizes in \a sizes.  Note that all
    Building objects which are on matching planets are also matched. */
struct Condition::PlanetSize : Condition::ConditionBase
{
    PlanetSize(const std::vector<const ValueRef::ValueRefBase< ::PlanetSize>*>& sizes);
    virtual ~PlanetSize();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    std::vector<const ValueRef::ValueRefBase< ::PlanetSize>*> m_sizes;
};

/** Matches all Planet objects that have one of the PlanetEnvironments in \a environments.  Note that all
    Building objects which are on matching planets are also matched. */
struct Condition::PlanetEnvironment : Condition::ConditionBase
{
    PlanetEnvironment(const std::vector<const ValueRef::ValueRefBase< ::PlanetEnvironment>*>& environments);
    virtual ~PlanetEnvironment();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    std::vector<const ValueRef::ValueRefBase< ::PlanetEnvironment>*> m_environments;
};

/** Matches all ProdCenter objects that have one of the FocusTypes in \a foci. */
struct Condition::FocusType : Condition::ConditionBase
{
    FocusType(const std::vector<const ValueRef::ValueRefBase< ::FocusType>*>& foci, bool primary);
    virtual ~FocusType();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    std::vector<const ValueRef::ValueRefBase< ::FocusType>*> m_foci;
    bool m_primary;
};

/** Matches all System objects that have one of the StarTypes in \a types.  Note that all objects
    in matching Systems are also matched (Ships, Fleets, Buildings, Planets, etc.). */
struct Condition::StarType : Condition::ConditionBase
{
    StarType(const std::vector<const ValueRef::ValueRefBase< ::StarType>*>& types);
    virtual ~StarType();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    std::vector<const ValueRef::ValueRefBase< ::StarType>*> m_types;
};

/** Matches a given object with a linearly distributed probability of \a chance. */
struct Condition::Chance : Condition::ConditionBase
{
    Chance(const ValueRef::ValueRefBase<double>* chance);
    virtual ~Chance();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    const ValueRef::ValueRefBase<double>* m_chance;
};

/** Matches all objects that have a meter of type \a meter, and whose current value (if \a max_meter == false) or
    maximum value (if \a max_meter == true) of that meter is >= \a low and < \a high. */
struct Condition::MeterValue : Condition::ConditionBase
{
    MeterValue(MeterType meter, const ValueRef::ValueRefBase<double>* low, const ValueRef::ValueRefBase<double>* high, bool max_meter);
    virtual ~MeterValue();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    MeterType                             m_meter;
    const ValueRef::ValueRefBase<double>* m_low;
    const ValueRef::ValueRefBase<double>* m_high;
    bool                                  m_max_meter;
};

/** Matches all objects with exactly one owner, whose owner's stockpile of \a stockpile is between \a low
    and \a high, inclusive. */
struct Condition::EmpireStockpileValue : Condition::ConditionBase
{
    EmpireStockpileValue(StockpileType stockpile, const ValueRef::ValueRefBase<double>* low, const ValueRef::ValueRefBase<double>* high);
    virtual ~EmpireStockpileValue();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    StockpileType m_stockpile;
    const ValueRef::ValueRefBase<double>* m_low;
    const ValueRef::ValueRefBase<double>* m_high;
};

/** Matches all objects that have a single owner who has tech \a tech_name. */
struct Condition::OwnerHasTech : Condition::ConditionBase
{
    OwnerHasTech(const std::string& name);
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    std::string m_name;
};

/** Matches all objects that are visible to at least one Empire in \a empire_ids. */
struct Condition::VisibleToEmpire : Condition::ConditionBase
{
    VisibleToEmpire(const std::vector<const ValueRef::ValueRefBase<int>*>& empire_ids);
    virtual ~VisibleToEmpire();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    std::vector<const ValueRef::ValueRefBase<int>*> m_empire_ids;
};

/** Matches all objects that are within \a distance units of at least one object that meets \a condition.
    Warning: this Condition can slow things down considerably if overused.  It is best to use Conditions
    that yield relatively few matches. */
struct Condition::WithinDistance : Condition::ConditionBase
{
    WithinDistance(const ValueRef::ValueRefBase<double>* distance, const ConditionBase* condition);
    virtual ~WithinDistance();
    virtual void Eval(const UniverseObject* source, ObjectSet& targets, ObjectSet& non_targets, SearchDomain search_domain = NON_TARGETS) const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    const ValueRef::ValueRefBase<double>* m_distance;
    const ConditionBase*                  m_condition;
};

/** Matches all objects that are within \a jumps starlane jumps of at least one object that meets \a condition.
    Warning: this Condition can slow things down considerably if overused.  It is best to use Conditions
    that yield relatively few matches. */
struct Condition::WithinStarlaneJumps : Condition::ConditionBase
{
    WithinStarlaneJumps(const ValueRef::ValueRefBase<int>* jumps, const ConditionBase* condition);
    virtual ~WithinStarlaneJumps();
    virtual void Eval(const UniverseObject* source, ObjectSet& targets, ObjectSet& non_targets, SearchDomain search_domain = NON_TARGETS) const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    const ValueRef::ValueRefBase<int>* m_jumps;
    const ConditionBase*               m_condition;
};

struct Condition::EffectTarget : Condition::ConditionBase
{
    EffectTarget();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
};

/** Matches all objects that match every Condition in \a operands. */
struct Condition::And : Condition::ConditionBase
{
    And(const std::vector<const ConditionBase*>& operands);
    virtual ~And();
    virtual void Eval(const UniverseObject* source, ObjectSet& targets, ObjectSet& non_targets, SearchDomain search_domain = NON_TARGETS) const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    std::vector<const ConditionBase*> m_operands;
};

/** Matches all objects that match at least one Condition in \a operands. */
struct Condition::Or : Condition::ConditionBase
{
    Or(const std::vector<const ConditionBase*>& operands);
    virtual ~Or();
    virtual void Eval(const UniverseObject* source, ObjectSet& targets, ObjectSet& non_targets, SearchDomain search_domain = NON_TARGETS) const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    std::vector<const ConditionBase*> m_operands;
};

/** Matches all objects that do not match the Condition \a operand. */
struct Condition::Not : Condition::ConditionBase
{
    Not(const ConditionBase* operand);
    virtual ~Not();
    virtual void Eval(const UniverseObject* source, ObjectSet& targets, ObjectSet& non_targets, SearchDomain search_domain = NON_TARGETS) const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    const ConditionBase* m_operand;
};

#endif // _Condition_h_
