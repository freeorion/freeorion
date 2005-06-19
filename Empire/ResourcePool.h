// -*- C++ -*-
#ifndef _ResourcePool_h_
#define _ResourcePool_h_

#ifndef BOOST_SIGNAL_HPP
#include <boost/signal.hpp>
#endif

#ifndef _Universe_h_
#  include "../universe/Universe.h"
#endif

#ifndef _UniverseObject_h_
#  include "../universe/UniverseObject.h"
#endif

class Planet;
namespace GG {class XMLElement;}

template <class PoolT> 
class PlanetChangedFunctor
{
public:
    PlanetChangedFunctor(PoolT &pool, int planet_id);
    void operator()();

private:
    PoolT& m_pool;
    int    m_planet_id;
};

/**
* Base class for all resource pool.
*/
class ResourcePool
{
public:
    typedef bool (*SortFuncType)(const Planet*,const Planet*);///< type of function used to sort the planet vector

    ResourcePool();
    virtual ~ResourcePool();
    
    /** \name Signal Types */ //@{
    typedef boost::signal<void ()> ChangedSignalType;
    //@}

    const std::vector<Planet*>& Planets() const {return m_planets;} ///< returns the planet vector 
    virtual double Stockpile() const;
    virtual GG::XMLElement XMLEncode() const = 0;
    mutable ChangedSignalType ChangedSignal; ///< the changed signal object for this ResourcePool

    std::vector<Planet*>& Planets() {return m_planets;} ///< returns the planet vector 
    void                  SetPlanets(const Universe::ObjectVec &planet_vec);///< sets the planet vector 
    virtual void          SetStockpile(double d) {}

protected:
    virtual SortFuncType SortFunc() const; ///< used to order planet list
    virtual void PlanetChanged() = 0; ///< called when a planet of the planet vector has changed

private:
    std::vector<Planet*> m_planets; ///< list of planet which feed/consume the resource
    std::vector<boost::signals::connection > m_connections;///< connection list of planets

    friend class PlanetChangedFunctor<ResourcePool>;
};

/**
* Resource pool for minerals.
*/
class MineralResourcePool : public ResourcePool
{
public:
    MineralResourcePool();
    MineralResourcePool(const GG::XMLElement& elem);
    
    double Production() const {return m_pool_production;}
    double ExcessShortfall() const {return m_pool_production - m_needed_pool;}
    double Needed() const {return m_needed_pool;}
    virtual double Stockpile() const;
    virtual GG::XMLElement XMLEncode() const;

    virtual void SetStockpile(double d);

protected:
    virtual void PlanetChanged();

private:
    double m_pool_production,m_needed_pool,m_stockpile;
};

/**
* Resource pool for food.
*/
class FoodResourcePool : public ResourcePool
{
public:
    FoodResourcePool();
    FoodResourcePool(const GG::XMLElement& elem);

    double Production() const {return m_pool_production;}
    double ExcessShortfall() const {return m_pool_production - m_needed_pool;}
    double Needed() const {return m_needed_pool;}
    virtual double Stockpile() const;
    virtual GG::XMLElement XMLEncode() const;

    virtual void SetStockpile(double d);

protected:
    virtual SortFuncType SortFunc() const; 
    virtual void PlanetChanged();

private:
    double m_pool_production,m_needed_pool,m_stockpile;
};

/**
* Resource pool for research.
*/
class ResearchResourcePool : public ResourcePool
{
public:
    ResearchResourcePool();
    ResearchResourcePool(const GG::XMLElement& elem);

    double Production() const {return m_pool_production;}

    virtual GG::XMLElement XMLEncode() const;

protected:
    virtual void PlanetChanged();

private:
    double m_pool_production;
};

/**
* Resource pool for population.
*/
class PopulationResourcePool : public ResourcePool
{
public:
    PopulationResourcePool();
    PopulationResourcePool(const GG::XMLElement& elem);

    double Available() const {return m_overall_pool;}
    double Growth   () const {return m_growth;}

    virtual GG::XMLElement XMLEncode() const;

protected:
    virtual void PlanetChanged();

private:
    double m_overall_pool,m_growth;
};

/**
* Resource pool for industry.
*/
class IndustryResourcePool : public ResourcePool
{
public:
    IndustryResourcePool();
    IndustryResourcePool(const GG::XMLElement& elem);

    double Production() const {return m_pool_production;}

    virtual GG::XMLElement XMLEncode() const;

protected:
    virtual void PlanetChanged();

private:
    double m_pool_production;
};

/**
* Resource pool for trade.
*/
class TradeResourcePool : public ResourcePool
{
public:
    TradeResourcePool();
    TradeResourcePool(const GG::XMLElement& elem);

    double Production() const {return m_pool_production;}
    double ExcessShortfall() const {return m_pool_production - m_needed_pool;}
    double Needed   () const {return m_needed_pool;}
    virtual double Stockpile() const;
    virtual GG::XMLElement XMLEncode() const;

    virtual void SetStockpile(double d);

protected:
    virtual SortFuncType SortFunc() const; 
    virtual void PlanetChanged();

private:
    double m_pool_production,m_needed_pool,m_stockpile;
};


// template implementations
template <class PoolT> 
PlanetChangedFunctor<PoolT>::PlanetChangedFunctor(PoolT& pool, int planet_id) :
    m_pool(pool),
    m_planet_id(planet_id)
{}

template <class PoolT> 
void PlanetChangedFunctor<PoolT>::operator()()
{
    m_pool.OnPlanetChanged(m_planet_id);
}

inline std::pair<std::string, std::string> ResourcePoolRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _ResourcePool_h_
