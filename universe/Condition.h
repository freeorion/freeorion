// -*- C++ -*-
#ifndef _Condition_h_
#define _Condition_h_

#include <set>

#include "ValueRef.h"

class UniverseObject;


/** this namespace holds ConditionBase and its subclasses; these classes represent predicates about UniverseObjects used
    by, for instance, the Effect system. */
namespace Condition {
    typedef std::set<UniverseObject*> ObjectSet;

    enum SearchDomain {
        NON_TARGETS, ///< The Condition will only examine items in the nontarget set; those that match the Condition will be inserted into the target set.
        TARGETS      ///< The Condition will only examine items in the target set; those that do not match the Condition will be inserted into the nontarget set.
    };

    struct ConditionBase;
    struct All;
    struct EmpireAffiliation;
    struct Self;
    struct Homeworld;
    struct Capitol;
    struct Type;
    struct Building;
    struct HasSpecial;
    struct Contains;
    struct PlanetSize;
    struct PlanetType;
    struct PlanetEnvironment;
    struct FocusType;
    struct StarType;
    struct DesignHasHull;
    struct DesignHasPart;
    struct Chance;
    struct MeterValue;
    struct EmpireStockpileValue;
    struct OwnerHasTech;
    struct VisibleToEmpire;
    struct WithinDistance;
    struct WithinStarlaneJumps;
    struct ExploredByEmpire;
    struct Stationary;
    struct And;
    struct Or;
    struct Not;
    struct Turn;
    struct NumberOf;
    struct ContainedBy;
    struct Number;
}

/** The base class for all Conditions. */
struct Condition::ConditionBase
{
    ConditionBase();
    virtual ~ConditionBase();
    virtual void Eval(const UniverseObject* source, Condition::ObjectSet& targets, Condition::ObjectSet& non_targets, SearchDomain search_domain = NON_TARGETS) const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects if the number of objects that match Condition
  * \a condition is is >= \a low and < \a high.  Matched objects may
  * or may not themselves match the condition. */
struct Condition::Number : Condition::ConditionBase
{
    Number(const ValueRef::ValueRefBase<int>* low, const ValueRef::ValueRefBase<int>* high, const ConditionBase* condition);
    virtual ~Number();
    virtual void Eval(const UniverseObject* source, Condition::ObjectSet& targets, Condition::ObjectSet& non_targets, SearchDomain search_domain = NON_TARGETS) const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    const ValueRef::ValueRefBase<int>* m_low;
    const ValueRef::ValueRefBase<int>* m_high;
    const ConditionBase*               m_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
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

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches a randomly selected \a number of objects that match Condition \a condition, or as many objects
    as match the condition if the number of objects is less than the number requested. */
struct Condition::NumberOf : Condition::ConditionBase
{
    NumberOf(const ValueRef::ValueRefBase<int>* number, const ConditionBase* condition);
    virtual ~NumberOf();
    virtual void Eval(const UniverseObject* source, Condition::ObjectSet& targets, Condition::ObjectSet& non_targets, SearchDomain search_domain = NON_TARGETS) const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    const ValueRef::ValueRefBase<int>* m_number;
    const ConditionBase*               m_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects. */
struct Condition::All : Condition::ConditionBase
{
    All();
    virtual void Eval(const UniverseObject* source, Condition::ObjectSet& targets, Condition::ObjectSet& non_targets, SearchDomain search_domain = NON_TARGETS) const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that are owned (if \a exclusive == false) or only owned
  * (if \a exclusive == true) by an empire that has affilitation type
  * \a affilitation with Empire \a empire_id. */
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

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches the source object only. */
struct Condition::Self : Condition::ConditionBase
{
    Self();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches planets that are a homeworld. */
struct Condition::Homeworld : Condition::ConditionBase
{
    Homeworld();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches planets that are an empire's capitol. */
struct Condition::Capitol : Condition::ConditionBase
{
    Capitol();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
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

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
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

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that have an attached Special of the sort specified by
  * \a name.  Passing "All" for \a name will match all objects with attached
  * Specials. */
struct Condition::HasSpecial : Condition::ConditionBase
{
    HasSpecial(const std::string& name);
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    std::string m_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that contain an object that matches Condition
  * \a condition.  Container objects are Systems, Planets (which contain
  * Buildings), and Fleets (which contain Ships). */
struct Condition::Contains : Condition::ConditionBase
{
    Contains(const ConditionBase* condition);
    virtual void Eval(const UniverseObject* source, Condition::ObjectSet& targets, Condition::ObjectSet& non_targets, SearchDomain search_domain = NON_TARGETS) const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    const ConditionBase* m_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that are contained by an object that matches Condition
  * \a condition.  Container objects are Systems, Planets (which contain
  * Buildings), and Fleets (which contain Ships). */
struct Condition::ContainedBy : Condition::ConditionBase
{
    ContainedBy(const ConditionBase* condition);
    virtual void Eval(const UniverseObject* source, Condition::ObjectSet& targets, Condition::ObjectSet& non_targets, SearchDomain search_domain = NON_TARGETS) const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    const ConditionBase* m_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all Planet objects that have one of the PlanetTypes in \a types.
  * Note that all Building objects which are on matching planets are also
  * matched. */
struct Condition::PlanetType : Condition::ConditionBase
{
    PlanetType(const std::vector<const ValueRef::ValueRefBase< ::PlanetType>*>& types);
    virtual ~PlanetType();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    std::vector<const ValueRef::ValueRefBase< ::PlanetType>*> m_types;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all Planet objects that have one of the PlanetSizes in \a sizes.
  * Note that all Building objects which are on matching planets are also
  * matched. */
struct Condition::PlanetSize : Condition::ConditionBase
{
    PlanetSize(const std::vector<const ValueRef::ValueRefBase< ::PlanetSize>*>& sizes);
    virtual ~PlanetSize();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    std::vector<const ValueRef::ValueRefBase< ::PlanetSize>*> m_sizes;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
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

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
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

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
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

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all ships whose ShipDesign has the hull specified by \a name. */
struct Condition::DesignHasHull : Condition::ConditionBase
{
    DesignHasHull(const std::string& name);
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    std::string     m_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all ships whose ShipDesign has >= \a low and < \a high of the ship
  * part specified by \a name. */
struct Condition::DesignHasPart : Condition::ConditionBase
{
    DesignHasPart(const ValueRef::ValueRefBase<int>* low, const ValueRef::ValueRefBase<int>* high, const std::string& name);
    virtual ~DesignHasPart();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    const ValueRef::ValueRefBase<int>*  m_low;
    const ValueRef::ValueRefBase<int>*  m_high;
    std::string                         m_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
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

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
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

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects with exactly one owner, whose owner's stockpile of \a stockpile is between \a low
    and \a high, inclusive. */
struct Condition::EmpireStockpileValue : Condition::ConditionBase
{
    EmpireStockpileValue(ResourceType stockpile, const ValueRef::ValueRefBase<double>* low, const ValueRef::ValueRefBase<double>* high);
    virtual ~EmpireStockpileValue();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    ResourceType m_stockpile;
    const ValueRef::ValueRefBase<double>* m_low;
    const ValueRef::ValueRefBase<double>* m_high;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
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

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
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

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that are within \a distance units of at least one object that meets \a condition.
    Warning: this Condition can slow things down considerably if overused.  It is best to use Conditions
    that yield relatively few matches. */
struct Condition::WithinDistance : Condition::ConditionBase
{
    WithinDistance(const ValueRef::ValueRefBase<double>* distance, const ConditionBase* condition);
    virtual ~WithinDistance();
    virtual void Eval(const UniverseObject* source, Condition::ObjectSet& targets, Condition::ObjectSet& non_targets, SearchDomain search_domain = NON_TARGETS) const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    const ValueRef::ValueRefBase<double>* m_distance;
    const ConditionBase*                  m_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that are within \a jumps starlane jumps of at least one object that meets \a condition.
    Warning: this Condition can slow things down considerably if overused.  It is best to use Conditions
    that yield relatively few matches. */
struct Condition::WithinStarlaneJumps : Condition::ConditionBase
{
    WithinStarlaneJumps(const ValueRef::ValueRefBase<int>* jumps, const ConditionBase* condition);
    virtual ~WithinStarlaneJumps();
    virtual void Eval(const UniverseObject* source, Condition::ObjectSet& targets, Condition::ObjectSet& non_targets, SearchDomain search_domain = NON_TARGETS) const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    const ValueRef::ValueRefBase<int>* m_jumps;
    const ConditionBase*               m_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches systems that have been explored by at least one Empire in \a empire_ids. */
struct Condition::ExploredByEmpire : Condition::ConditionBase
{
    ExploredByEmpire(const std::vector<const ValueRef::ValueRefBase<int>*>& empire_ids);
    virtual ~ExploredByEmpire();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;
    std::vector<const ValueRef::ValueRefBase<int>*> m_empire_ids;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches objects that are moving. ... What does that mean?  Departing this
  * turn, or were located somewhere else last turn...? */
struct Condition::Stationary : Condition::ConditionBase
{
    Stationary();
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    virtual bool Match(const UniverseObject* source, const UniverseObject* target) const;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that match every Condition in \a operands. */
struct Condition::And : Condition::ConditionBase
{
    And(const std::vector<const ConditionBase*>& operands);
    virtual ~And();
    virtual void Eval(const UniverseObject* source, Condition::ObjectSet& targets, Condition::ObjectSet& non_targets, SearchDomain search_domain = NON_TARGETS) const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    std::vector<const ConditionBase*> m_operands;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that match at least one Condition in \a operands. */
struct Condition::Or : Condition::ConditionBase
{
    Or(const std::vector<const ConditionBase*>& operands);
    virtual ~Or();
    virtual void Eval(const UniverseObject* source, Condition::ObjectSet& targets, Condition::ObjectSet& non_targets, SearchDomain search_domain = NON_TARGETS) const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    std::vector<const ConditionBase*> m_operands;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Matches all objects that do not match the Condition \a operand. */
struct Condition::Not : Condition::ConditionBase
{
    Not(const ConditionBase* operand);
    virtual ~Not();
    virtual void Eval(const UniverseObject* source, Condition::ObjectSet& targets, Condition::ObjectSet& non_targets, SearchDomain search_domain = NON_TARGETS) const;
    virtual std::string Description(bool negated = false) const;
    virtual std::string Dump() const;

private:
    const ConditionBase* m_operand;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// template implementations
template <class Archive>
void Condition::ConditionBase::serialize(Archive& ar, const unsigned int version)
{}

template <class Archive>
void Condition::Number::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_low)
        & BOOST_SERIALIZATION_NVP(m_high)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <class Archive>
void Condition::Turn::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_low)
        & BOOST_SERIALIZATION_NVP(m_high);
}

template <class Archive>
void Condition::NumberOf::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_number)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <class Archive>
void Condition::All::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase);
}

template <class Archive>
void Condition::EmpireAffiliation::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_affiliation)
        & BOOST_SERIALIZATION_NVP(m_exclusive);
}

template <class Archive>
void Condition::Self::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase);
}

template <class Archive>
void Condition::Homeworld::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase);
}

template <class Archive>
void Condition::Capitol::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase);
}

template <class Archive>
void Condition::Type::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_type);
}

template <class Archive>
void Condition::Building::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <class Archive>
void Condition::HasSpecial::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <class Archive>
void Condition::Contains::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <class Archive>
void Condition::ContainedBy::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <class Archive>
void Condition::PlanetType::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_types);
}

template <class Archive>
void Condition::PlanetSize::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_sizes);
}

template <class Archive>
void Condition::PlanetEnvironment::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_environments);
}

template <class Archive>
void Condition::FocusType::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_foci)
        & BOOST_SERIALIZATION_NVP(m_primary);
}

template <class Archive>
void Condition::StarType::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_types);
}

template <class Archive>
void Condition::DesignHasHull::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <class Archive>
void Condition::DesignHasPart::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_low)
        & BOOST_SERIALIZATION_NVP(m_high)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <class Archive>
void Condition::Chance::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_chance);
}

template <class Archive>
void Condition::MeterValue::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_meter)
        & BOOST_SERIALIZATION_NVP(m_low)
        & BOOST_SERIALIZATION_NVP(m_high)
        & BOOST_SERIALIZATION_NVP(m_max_meter);
}

template <class Archive>
void Condition::EmpireStockpileValue::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_low)
        & BOOST_SERIALIZATION_NVP(m_high);
}

template <class Archive>
void Condition::OwnerHasTech::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <class Archive>
void Condition::VisibleToEmpire::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_empire_ids);
}

template <class Archive>
void Condition::WithinDistance::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_distance)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <class Archive>
void Condition::WithinStarlaneJumps::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_jumps)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <class Archive>
void Condition::ExploredByEmpire::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_empire_ids);
}

template <class Archive>
void Condition::Stationary::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase);
}

template <class Archive>
void Condition::And::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_operands);
}

template <class Archive>
void Condition::Or::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_operands);
}

template <class Archive>
void Condition::Not::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase)
        & BOOST_SERIALIZATION_NVP(m_operand);
}

#endif // _Condition_h_
