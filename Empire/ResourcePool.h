// -*- C++ -*-
#ifndef _ResourcePool_h_
#define _ResourcePool_h_

#ifndef _Ship_h_
#include "../universe/UniverseObject.h"
#endif

class Planet;
 template <class T> 
 class PlanetChangedFunctor
 {
   public:
       PlanetChangedFunctor(T &parent, int planet_id) : m_parent(parent), m_planet_id(planet_id) {}
       void operator()() {m_parent.PlanetChanged(m_planet_id);}
   private:
       T &m_parent;
       int m_planet_id;
 };

class MineralResourcePool
{
  public:
    MineralResourcePool();

    void SetPlanets(const Universe::ObjectVec &planet_vec);

  private:
    void Calc();
    void PlanetChanged(int m_planet_id);

    std::vector<Planet*> m_planets;
    std::vector<boost::signals::connection > m_connections;
    double m_overall_pool,m_available_pool;

    friend class PlanetChangedFunctor<MineralResourcePool>;
};

class FoodResourcePool
{
  public:
    FoodResourcePool();

    void SetPlanets(const Universe::ObjectVec &planet_vec);

  private:
    void Calc();
    void PlanetChanged(int m_planet_id);

    std::vector<Planet*> m_planets;
    std::vector<boost::signals::connection > m_connections;
    double m_overall_pool,m_available_pool;

    friend class PlanetChangedFunctor<FoodResourcePool>;
};
#endif
