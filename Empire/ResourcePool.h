// -*- C++ -*-
#ifndef _ResourcePool_h_
#define _ResourcePool_h_

#ifndef BOOST_SIGNAL_HPP
#include <boost/signal.hpp>
#endif

#ifndef _UniverseObject_h_
#include "../universe/UniverseObject.h"
#endif

class Planet;
 template <class T> 
 class PlanetChangedFunctor : public boost::signals::trackable
 {
   public:
       PlanetChangedFunctor(T &parent, int planet_id) : m_parent(parent), m_planet_id(planet_id) {}
       void operator()() {m_parent.OnPlanetChanged(m_planet_id);}
   private:
       T &m_parent;
       int m_planet_id;
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

    ChangedSignalType& ChangedSignal() const {return m_changed_sig;} ///< returns the changed signal object for this ResourcePool

          std::vector<Planet*>& Planets()       {return m_planets;} ///< returns the planet vector 
    const std::vector<Planet*>& Planets() const {return m_planets;} ///< returns the planet vector 

    void SetPlanets(const Universe::ObjectVec &planet_vec);///< sets the planet vector 

  protected:
    virtual SortFuncType SortFunc() const; ///< used to order planet list
    virtual void PlanetChanged(int m_planet_id) = 0; ///< called when a planet of the planet vector has changed
  private:
    void OnPlanetChanged(int m_planet_id) {PlanetChanged(m_planet_id);}///< called through the PlanetChangedFunctor when a planet has changed

    std::vector<Planet*> m_planets; ///< list of planet which feed/consume the resource
    std::vector<boost::signals::connection > m_connections;///< connection list of planets

    friend class PlanetChangedFunctor<ResourcePool>;
    mutable ChangedSignalType m_changed_sig;
};

/**
* Resource pool for minerals.
*/
class MineralResourcePool : public ResourcePool
{
  public:
    MineralResourcePool();
    
    double Available() const {return m_overall_pool;} ///< amount of mineral which is produced by the planets
    double Spend    () const {return m_overall_pool-m_available_pool;}///< amount of mineral which is spend to planets to support production
    double Needed   () const {return m_needed_pool;}///< amount of mineral which is needed to support planet production

  protected:
    virtual SortFuncType SortFunc() const; 
    virtual void PlanetChanged(int m_planet_id);
  private:
    double m_overall_pool,m_available_pool,m_needed_pool;
};

/**
* Resource pool for food.
*/
class FoodResourcePool : public ResourcePool
{
  public:
    FoodResourcePool();

    double Available() const {return m_overall_pool;}
    double Spend    () const {return m_overall_pool-m_available_pool;}
    double Needed   () const {return m_needed_pool;}

  protected:
    virtual SortFuncType SortFunc() const; 
    virtual void PlanetChanged(int m_planet_id);
  private:
    double m_overall_pool,m_available_pool,m_needed_pool;
};

/**
* Resource pool for research.
*/
class ResearchResourcePool : public ResourcePool
{
  public:
    ResearchResourcePool();

    double Available() const {return m_overall_pool;}

  protected:
    virtual void PlanetChanged(int m_planet_id);
  private:
    double m_overall_pool;
};

/**
* Resource pool for population.
*/
class PopulationResourcePool : public ResourcePool
{
  public:
    PopulationResourcePool();

    double Available() const {return m_overall_pool;}
    double Growth   () const {return m_growth;}

  protected:
    virtual void PlanetChanged(int m_planet_id);
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

    double Available() const {return m_overall_pool;}

  protected:
    virtual void PlanetChanged(int m_planet_id);
  private:
    double m_overall_pool;
};
#endif
