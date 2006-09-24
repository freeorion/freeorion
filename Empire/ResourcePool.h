// -*- C++ -*-
#ifndef _ResourcePool_h_
#define _ResourcePool_h_

#include "../universe/Enums.h"
#include "../universe/Universe.h"
#include "../universe/UniverseObject.h"

class XMLElement;
class ResourceCenter;
class PopCenter;

//! The ResourcePool class keeps track of an empire's stockpile and production of 
/** a particular resource (food, minerals, trade, and even research or industry). */
class ResourcePool
{
public:
    /** \name Signal Types */ //@{
    typedef boost::signal<void ()> ChangedSignalType;
    //@}

    /** \name Structors */ //@{
    ResourcePool(ResourceType type);
    ResourcePool(const XMLElement& elem);
    ~ResourcePool();
    //@}

    /** \name Accessors */ //@{
    const std::vector<ResourceCenter*>& ResourceCenters() const {return m_resource_centers;} ///< returns the ResourceCenter vector

    double Stockpile() const;       ///< returns current stockpiled amount of resource
    double MaxStockpile() const;    ///< returns maximum allowed stockpile of resource
    double Production() const;      ///< returns amount of resource being produced by ResourceCenters
    double Available() const;       ///< returns amount of resource immediately available = production + stockpile
    //@}

    /** \name Mutators */ //@{
    XMLElement XMLEncode() const;

    mutable ChangedSignalType ChangedSignal;    ///< emitted after updating production, or called externally to indicate that stockpile and change need to be refreshed

    void SetResourceCenters(const std::vector<ResourceCenter*>& resource_center_vec);///< sets the ResourceCenter vector 

    void SetStockpile(double d);    ///< sets current sockpiled amount of resource
    void SetMaxStockpile(double d); ///< sets maximum allowed stockpile of resource
    //@}

    void Update();  ///< recalculates total resource production

private:
    std::vector<ResourceCenter*> m_resource_centers;        ///< list of ResourceCenters: produce resources

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);

    double m_stockpile;         ///< current stockpiled amount of resource
    double m_max_stockpile;     ///< maximum allowed stockpile of resource

    double m_production;        ///< amount of resource being produced by empire

    ResourceType m_type;        ///< what kind of resource does this pool hold?
};

//! The PopulationPool class keeps track of an empire's total population
/**
  */
class PopulationPool
{
public:
    /** \name Signal Types */ //@{
    typedef boost::signal<void ()> ChangedSignalType;
    //@}

    /** \name Structors */ //@{
    PopulationPool();
    PopulationPool(const XMLElement& elem);
    ~PopulationPool();
    //@}

    /** \name Accessors */ //@{
    const std::vector<PopCenter*>& PopCenters() const {return m_pop_centers;} ///< returns the PopCenter vector

    double Population() const;  ///< returns current total population
    double Growth() const;      ///< returns predicted growth for next turn    
    //@}
    
    /** \name Mutators */ //@{
    XMLElement XMLEncode() const;

    mutable ChangedSignalType ChangedSignal;    ///< emitted after updating population and growth numbers
    
    void SetPopCenters(const std::vector<PopCenter*>& pop_center_vec);  ///< sets the PopCenter vector 
    //@}

    void Update();  ///< recalculates total population and growth

private:
    std::vector<PopCenter*> m_pop_centers;   ///< list of PopCenters that contribute to empire total population pool

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);

    double m_population;        ///< total population of all PopCenters in pool
    double m_growth;            ///< total predicted population growth for next turn for all PopCenters in pool
};

// template implementations

template <class Archive>
void ResourcePool::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_resource_centers)
        & BOOST_SERIALIZATION_NVP(m_stockpile)
        & BOOST_SERIALIZATION_NVP(m_max_stockpile)
        & BOOST_SERIALIZATION_NVP(m_production)
        & BOOST_SERIALIZATION_NVP(m_type);
}

template <class Archive>
void PopulationPool::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_pop_centers)
	& BOOST_SERIALIZATION_NVP(m_population)
	& BOOST_SERIALIZATION_NVP(m_growth);
}
#endif // _ResourcePool_h_
