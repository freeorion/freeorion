#include "ResourcePool.h"

#include "GGSignalsAndSlots.h"

#include "../util/AppInterface.h"
#include "../universe/Planet.h"

struct FindByPlanetID
{
    FindByPlanetID(int planet = -1) : planet_id(planet) {}
    bool operator()(const Planet* planet) const {return planet->ID()==planet_id;}
    const int planet_id;
};

bool greater(const Planet* elem1,const Planet* elem2 )
{
  return elem1->PopPoints() > elem2->PopPoints();
}

bool lower(const Planet* elem1,const Planet* elem2 )
{
  return elem1->PopPoints() < elem2->PopPoints();
}


MineralResourcePool::MineralResourcePool()
{}

void MineralResourcePool::SetPlanets(const Universe::ObjectVec &planet_vec)
{
  for(unsigned int i=0;i<m_connections.size();i++)
    m_connections[i].disconnect();

  m_connections.clear();
  m_planets.clear();

  for(unsigned int i=0;i<planet_vec.size();i++)
  {
    Planet *planet = dynamic_cast<Planet*>(planet_vec[i]);
    m_planets.push_back(planet);
    m_connections.push_back(GG::Connect(planet->ProdCenterChangedSignal(), PlanetChangedFunctor<MineralResourcePool>(*this,planet->ID())));
  }

  std::sort(m_planets.begin(),m_planets.end(),lower);

  Calc();
}

void MineralResourcePool::PlanetChanged(int m_planet_id)
{
  Calc();
}

void MineralResourcePool::Calc()
{
  m_available_pool=0.0;

  // sum all minerals
  for(std::vector<Planet*>::iterator it = m_planets.begin();it !=m_planets.end();++it)
  {
    Planet *planet=*it;
    planet->SetAvailableMinerals(0.0);
    m_available_pool+=planet->MiningPoints();
  }
  m_overall_pool = m_available_pool;

  // first run: give all planets required mineral limited by local mineral production
  for(std::vector<Planet*>::iterator it = m_planets.begin();it !=m_planets.end();++it)
  {
    Planet *planet=*it;
    planet->SetAvailableMinerals(std::min(m_available_pool,std::min(planet->MiningPoints(),planet->IndustryPoints())));
    m_available_pool-=planet->AvailableMinerals();
  }

  // second run: give all planets required mineral to build one unit or support max required minerals
  for(std::vector<Planet*>::iterator it = m_planets.begin();it !=m_planets.end();++it)
  {
    Planet *planet=*it;
    double complete_one_item_cost = (planet->ItemBuildCost()-planet->Rollover())-planet->ProductionPoints();
    if(complete_one_item_cost>0.0 && planet->IndustryPoints()>planet->AvailableMinerals())
    {
      double receives = std::min(m_available_pool,std::min(complete_one_item_cost,planet->IndustryPoints()-planet->AvailableMinerals()));
      planet->SetAvailableMinerals(planet->AvailableMinerals()+receives);
      m_available_pool-=receives;
    }
  }

  // third run: give all planets required mineral up to the maximum needed
  for(std::vector<Planet*>::iterator it = m_planets.begin();it !=m_planets.end();++it)
  {
    Planet *planet=*it;
    if(planet->IndustryPoints()>planet->AvailableMinerals())
    {
      double receives = std::min(m_available_pool,planet->IndustryPoints()-planet->AvailableMinerals());
      planet->SetAvailableMinerals(planet->AvailableMinerals()+receives);
      m_available_pool-=receives;
    }
  }
}

//FoodResourcePool
FoodResourcePool::FoodResourcePool()
{}

void FoodResourcePool::SetPlanets(const Universe::ObjectVec &planet_vec)
{
  for(unsigned int i=0;i<m_connections.size();i++)
    m_connections[i].disconnect();

  m_connections.clear();
  m_planets.clear();

  for(unsigned int i=0;i<planet_vec.size();i++)
  {
    Planet *planet = dynamic_cast<Planet*>(planet_vec[i]);
    m_planets.push_back(planet);
    m_connections.push_back(GG::Connect(planet->ProdCenterChangedSignal(), PlanetChangedFunctor<FoodResourcePool>(*this,planet->ID())));
  }

  std::sort(m_planets.begin(),m_planets.end(),greater);

  Calc();
}

void FoodResourcePool::PlanetChanged(int m_planet_id)
{
  Calc();
}

void FoodResourcePool::Calc()
{
  m_available_pool=0.0;

  // sum all foods
  for(std::vector<Planet*>::iterator it = m_planets.begin();it !=m_planets.end();++it)
  {
    Planet *planet=*it;
    planet->SetAvailableFood(0.0);
    m_available_pool+=planet->FarmingPoints();
  }
  m_overall_pool = m_available_pool;

  // first run: give all planets required food limited by local food production
  for(std::vector<Planet*>::iterator it = m_planets.begin();it !=m_planets.end();++it)
  {
    Planet *planet=*it;
    planet->SetAvailableFood(std::min(m_available_pool,std::min(planet->FarmingPoints(),planet->PopPoints()+planet->FuturePopGrowthMax())));
    m_available_pool-=planet->AvailableFood();
  }

  // second run: give all planets required food to maximum which is needed
  for(std::vector<Planet*>::iterator it = m_planets.begin();it !=m_planets.end();++it)
  {
    Planet *planet=*it;
    if(planet->PopPoints()>planet->AvailableFood())
    {
      double receives = std::min(m_available_pool,planet->PopPoints()+planet->FuturePopGrowthMax()-planet->AvailableFood());
      planet->SetAvailableFood(planet->AvailableFood()+receives);
      m_available_pool-=receives;
    }
  }
}

