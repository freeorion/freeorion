#include "ResourcePool.h"

#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"
#include "../universe/Planet.h"

#include "GGSignalsAndSlots.h"
#include "XMLDoc.h"

namespace {
    bool Greater(const Planet* elem1, const Planet* elem2)
    {
        return elem1->PopPoints() > elem2->PopPoints();
    }

    bool Lower(const Planet* elem1, const Planet* elem2)
    {
        return elem1->PopPoints() < elem2->PopPoints();
    }

    double PopEstimate(Planet* p)
    {
        return p->PopPoints() + std::min(p->FuturePopGrowthMax(), p->MaxPop() - p->PopPoints());
    }

    void DistributeFood(std::vector<Planet*>::iterator first, std::vector<Planet*>::iterator last, double multiple, double& available_pool)
    {
        for (std::vector<Planet*>::iterator it = first; it != last; ++it) {
            Planet* planet = *it;
            double receives = std::min(available_pool, PopEstimate(planet) - planet->AvailableFood());
            planet->SetAvailableFood(planet->AvailableFood() + receives);
            available_pool -= receives;
        }
    }

    bool temp_header_bool = RecordHeaderFile(ResourcePoolRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}

//////////////////////////////////////////////////
// ResourcePool
//////////////////////////////////////////////////
ResourcePool::ResourcePool()
{}

ResourcePool::~ResourcePool()
{
    for(unsigned int i=0;i<m_connections.size();i++)
        m_connections[i].disconnect();
    m_connections.clear();
    m_planets.clear();
}

double ResourcePool::Stockpile() const
{
    return 0.0;
}

void ResourcePool::SetPlanets(const Universe::ObjectVec &planet_vec)
{
    for(unsigned int i=0;i<m_connections.size();i++)
        m_connections[i].disconnect();
    m_connections.clear();
    m_planets.clear();

    for(unsigned int i=0;i<planet_vec.size();i++)
    {
        Planet *planet = universe_object_cast<Planet*>(planet_vec[i]);
        m_planets.push_back(planet);
        m_connections.push_back(GG::Connect(planet->ProdCenterChangedSignal(), PlanetChangedFunctor<ResourcePool>(*this,planet->ID())));
    }
    std::sort(m_planets.begin(),m_planets.end(),SortFunc());

    PlanetChanged(UniverseObject::INVALID_OBJECT_ID);
}

ResourcePool::SortFuncType ResourcePool::SortFunc() const
{
    return &Lower;
}

void ResourcePool::OnPlanetChanged(int m_planet_id)
{
    PlanetChanged(m_planet_id);
}

//////////////////////////////////////////////////
// MineralResourcePool
//////////////////////////////////////////////////
MineralResourcePool::MineralResourcePool() :
    ResourcePool(),
    m_stockpile(0.0)
{}

MineralResourcePool::MineralResourcePool(const GG::XMLElement& elem)
    : ResourcePool()
{
    if (elem.Tag() != "MineralResourcePool")
        throw std::invalid_argument("Attempted to construct a MineralResourcePool from an XMLElement that had a tag other than \"MineralResourcePool\"");

    m_stockpile = boost::lexical_cast<double>(elem.Child("m_stockpile").Text());
}

ResourcePool::SortFuncType MineralResourcePool::SortFunc() const
{
    return &Lower;
}

void MineralResourcePool::PlanetChanged(int m_planet_id)
{
    m_available_pool=m_stockpile;
    m_needed_pool=0.0;

    // sum all minerals
    for(std::vector<Planet*>::iterator it = Planets().begin();it !=Planets().end();++it)
    {
        Planet *planet=*it;
        planet->SetAvailableMinerals(0.0);
        m_available_pool+=planet->MiningPoints();
        m_needed_pool+=planet->IndustryPoints();
    }
    m_overall_pool = m_available_pool;

    // first run: give all planets required mineral limited by local mineral production
    for(std::vector<Planet*>::iterator it = Planets().begin();it !=Planets().end();++it)
    {
        Planet *planet=*it;
        planet->SetAvailableMinerals(std::min(m_available_pool,std::min(planet->MiningPoints(),planet->IndustryPoints())));
        m_available_pool-=planet->AvailableMinerals();
    }

    // second run: give all planets required mineral to build one unit or support max required minerals
    for(std::vector<Planet*>::iterator it = Planets().begin();it !=Planets().end();++it)
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
    for(std::vector<Planet*>::iterator it = Planets().begin();it !=Planets().end();++it)
    {
        Planet *planet=*it;
        if(planet->IndustryPoints()>planet->AvailableMinerals())
        {
            double receives = std::min(m_available_pool,planet->IndustryPoints()-planet->AvailableMinerals());
            planet->SetAvailableMinerals(planet->AvailableMinerals()+receives);
            m_available_pool-=receives;
        }
    }

    m_stockpile=std::max(0.0, m_available_pool);

    ChangedSignal()();
}

double MineralResourcePool::Stockpile() const
{
    return m_stockpile;
}

GG::XMLElement MineralResourcePool::XMLEncode() const
{
    GG::XMLElement retval("MineralResourcePool");
    retval.AppendChild(GG::XMLElement("m_stockpile", boost::lexical_cast<std::string>(m_stockpile)));
    return retval;
}

void MineralResourcePool::SetStockpile(double d)
{
    m_stockpile = d;
}

//////////////////////////////////////////////////
//FoodResourcePool
//////////////////////////////////////////////////
FoodResourcePool::FoodResourcePool() :
    ResourcePool(),
    m_stockpile(0.0)
{}

FoodResourcePool::FoodResourcePool(const GG::XMLElement& elem)
    : ResourcePool()
{
    if (elem.Tag() != "FoodResourcePool")
        throw std::invalid_argument("Attempted to construct a FoodResourcePool from an XMLElement that had a tag other than \"FoodResourcePool\"");

    m_stockpile = boost::lexical_cast<double>(elem.Child("m_stockpile").Text());
}

ResourcePool::SortFuncType FoodResourcePool::SortFunc() const
{
    return &Greater;
}

void FoodResourcePool::PlanetChanged(int m_planet_id)
{
    m_available_pool=m_stockpile;
    m_needed_pool=0.0;

    // sum all food
    for(std::vector<Planet*>::iterator it = Planets().begin();it !=Planets().end();++it)
    {
        Planet *planet=*it;
        planet->SetAvailableFood(0.0);
        m_available_pool+=planet->FarmingPoints();
        m_needed_pool+=PopEstimate(planet);
    }
    m_overall_pool = m_available_pool;

    // first pass: give all planets required food limited by local food production
    for(std::vector<Planet*>::iterator it = Planets().begin();it !=Planets().end();++it)
    {
        Planet *planet=*it;
        planet->SetAvailableFood(std::min(m_available_pool,std::min(planet->FarmingPoints(),PopEstimate(planet))));
        m_available_pool-=planet->AvailableFood();
    }

    // second pass: give all planets up to 1 times the required minimum
    DistributeFood(Planets().begin(), Planets().end(), 1.0, m_available_pool);

    // third pass: give all planets up to 2 times the required minimum
    DistributeFood(Planets().begin(), Planets().end(), 2.0, m_available_pool);

    // fourth pass: give all planets up to 4 times the required minimum
    DistributeFood(Planets().begin(), Planets().end(), 4.0, m_available_pool);

    m_stockpile=std::max(0.0, m_available_pool);

    ChangedSignal()();
}

double FoodResourcePool::Stockpile() const
{
    return m_stockpile;
}

GG::XMLElement FoodResourcePool::XMLEncode() const
{
    GG::XMLElement retval("FoodResourcePool");
    retval.AppendChild(GG::XMLElement("m_stockpile", boost::lexical_cast<std::string>(m_stockpile)));
    return retval;
}

void FoodResourcePool::SetStockpile(double d)
{
    m_stockpile = d;
}


//////////////////////////////////////////////////
//ResearchResourcePool
//////////////////////////////////////////////////
ResearchResourcePool::ResearchResourcePool() :
    ResourcePool()
{}

ResearchResourcePool::ResearchResourcePool(const GG::XMLElement& elem)
    : ResourcePool()
{
    if (elem.Tag() != "ResearchResourcePool")
        throw std::invalid_argument("Attempted to construct a ResearchResourcePool from an XMLElement that had a tag other than \"ResearchResourcePool\"");
}

void ResearchResourcePool::PlanetChanged(int m_planet_id)
{
    m_overall_pool=0.0;

    // sum all research
    for(std::vector<Planet*>::iterator it = Planets().begin();it !=Planets().end();++it)
    {
        Planet *planet=*it;
        m_overall_pool+=planet->ResearchPoints();
    }

    ChangedSignal()();
}

GG::XMLElement ResearchResourcePool::XMLEncode() const
{
    return GG::XMLElement("ResearchResourcePool");
}


//////////////////////////////////////////////////
//PopulationResourcePool
//////////////////////////////////////////////////
PopulationResourcePool::PopulationResourcePool() :
    ResourcePool()
{}

PopulationResourcePool::PopulationResourcePool(const GG::XMLElement& elem)
    : ResourcePool()
{
    if (elem.Tag() != "PopulationResourcePool")
        throw std::invalid_argument("Attempted to construct a PopulationResourcePool from an XMLElement that had a tag other than \"PopulationResourcePool\"");
}

void PopulationResourcePool::PlanetChanged(int m_planet_id)
{
    m_overall_pool=m_growth=0.0;

    for(std::vector<Planet*>::iterator it = Planets().begin();it !=Planets().end();++it)
    {
        Planet *planet=*it;
        m_overall_pool+=planet->PopPoints();
        m_growth+=planet->FuturePopGrowth();
    }

    ChangedSignal()();
}

GG::XMLElement PopulationResourcePool::XMLEncode() const
{
    return GG::XMLElement("PopulationResourcePool");
}


//////////////////////////////////////////////////
//IndustryResourcePool
//////////////////////////////////////////////////
IndustryResourcePool::IndustryResourcePool() :
    ResourcePool()
{}

IndustryResourcePool::IndustryResourcePool(const GG::XMLElement& elem)
    : ResourcePool()
{
    if (elem.Tag() != "IndustryResourcePool")
        throw std::invalid_argument("Attempted to construct a IndustryResourcePool from an XMLElement that had a tag other than \"IndustryResourcePool\"");
}

void IndustryResourcePool::PlanetChanged(int m_planet_id)
{
    m_overall_pool=0.0;

    for(std::vector<Planet*>::iterator it = Planets().begin();it !=Planets().end();++it)
    {
        Planet *planet=*it;
        m_overall_pool+=planet->IndustryPoints();
    }

    ChangedSignal()();
}

GG::XMLElement IndustryResourcePool::XMLEncode() const
{
    return GG::XMLElement("IndustryResourcePool");
}


//////////////////////////////////////////////////
//TradeResourcePool
//////////////////////////////////////////////////
TradeResourcePool::TradeResourcePool() :
    ResourcePool(),
    m_stockpile(0.0)
{}

TradeResourcePool::TradeResourcePool(const GG::XMLElement& elem)
    : ResourcePool()
{
    if (elem.Tag() != "TradeResourcePool")
        throw std::invalid_argument("Attempted to construct a TradeResourcePool from an XMLElement that had a tag other than \"TradeResourcePool\"");

    m_stockpile = boost::lexical_cast<double>(elem.Child("m_stockpile").Text());
}

ResourcePool::SortFuncType TradeResourcePool::SortFunc() const
{
    return &Greater;
}

void TradeResourcePool::PlanetChanged(int m_planet_id)
{
    m_available_pool=m_stockpile;
    m_needed_pool=0.0;

    // sum all trade
    for(std::vector<Planet*>::iterator it = Planets().begin();it !=Planets().end();++it)
    {
        Planet *planet=*it;
        planet->SetAvailableTrade(0.0);
        m_available_pool+=planet->TradePoints();
        m_needed_pool+=planet->BuildingCosts();
    }
    m_overall_pool = m_available_pool;

    // first pass: give all planets required trade limited by local trade production
    for(std::vector<Planet*>::iterator it = Planets().begin();it !=Planets().end();++it)
    {
        Planet *planet=*it;
        planet->SetAvailableTrade(std::min(m_available_pool,std::min(planet->TradePoints(),planet->BuildingCosts())));
        m_available_pool-=planet->AvailableTrade();
    }

    // second pass: give all planets up to the required minimum to keep their buildings operating
    for(std::vector<Planet*>::iterator it = Planets().begin();it !=Planets().end();++it)
    {
        Planet *planet=*it;
        double receives = std::min(m_available_pool,planet->BuildingCosts()-planet->AvailableTrade());
        planet->SetAvailableTrade(planet->AvailableTrade()+receives);
        m_available_pool-=receives;
    }

    m_stockpile=m_available_pool;

    ChangedSignal()();
}

double TradeResourcePool::Stockpile() const
{
    return m_stockpile;
}

GG::XMLElement TradeResourcePool::XMLEncode() const
{
    GG::XMLElement retval("TradeResourcePool");
    retval.AppendChild(GG::XMLElement("m_stockpile", boost::lexical_cast<std::string>(m_stockpile)));
    return retval;
}

void TradeResourcePool::SetStockpile(double d)
{
    m_stockpile = d;
}
