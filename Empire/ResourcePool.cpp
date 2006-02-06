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
        for (std::vector<Planet*>::iterator it = first; it != last && 0.0<available_pool; ++it) {
            Planet* planet = *it;
            double receives = std::min(available_pool, PopEstimate(planet)*multiple - planet->AvailableFood());
            planet->SetAvailableFood(planet->AvailableFood() + receives);
            available_pool -= receives;
        }
    }

    bool temp_header_bool = RecordHeaderFile(ResourcePoolRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile: ResourcePool.cpp,v $", "$Revision$");
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
        // The design has changed, and due to the incremental growth of meters there should not be immediate
        // UI changes when planet updates occur (focus changes, etc.); therefore, these signals should not
        // exist.  However, I'm leaving this here since later we may want projections of output to be
        // displayed in the new SidePanel design, so this or something similar may be required or desired.
        //m_connections.push_back(GG::Connect(planet->ProdCenterChangedSignal, PlanetChangedFunctor<ResourcePool>(*this,planet->ID())));
    }
    std::sort(m_planets.begin(),m_planets.end(),SortFunc());

    PlanetChanged();
}

ResourcePool::SortFuncType ResourcePool::SortFunc() const
{
    return &Lower;
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

void MineralResourcePool::PlanetChanged()
{
    m_pool_production=0.0;
    m_needed_pool=0.0;

    // sum all minerals
    for(std::vector<Planet*>::iterator it = Planets().begin();it !=Planets().end();++it)
    {
        m_pool_production+=(*it)->MiningPoints();
        m_needed_pool+=(*it)->IndustryPoints();
    }

    // Note that m_stockpile is not updated; this should be done via a call the SetStockpile() by the owning empire's
    // production queue, at the point at which production uses minerals.  This is important because the amount of actual
    // production during a turn may be less than m_needed_pool, which is the sum of all industrial capacity.

    ChangedSignal();
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

void FoodResourcePool::PlanetChanged()
{
    m_pool_production=0.0;
    m_needed_pool=0.0;

    // sum all food
    for(std::vector<Planet*>::iterator it = Planets().begin();it !=Planets().end();++it)
    {
        Planet *planet=*it;
        planet->SetAvailableFood(0.0);
        m_pool_production+=planet->FarmingPoints();
        m_needed_pool+=PopEstimate(planet);
    }
    double available = m_pool_production + m_stockpile;

    // feed starving planets (limited by own food production)
    for(std::vector<Planet*>::iterator it = Planets().begin();it !=Planets().end() && 0.0<available;++it)
    {
        Planet *planet=*it;
        double receives = std::min(available,std::min(planet->FarmingPoints(),planet->PopPoints()) - planet->AvailableFood());
        planet->SetAvailableFood(planet->AvailableFood() + receives);
        available -= receives;
    }

    // feed starving planets
    for(std::vector<Planet*>::iterator it = Planets().begin();it !=Planets().end() && 0.0<available;++it)
    {
        Planet *planet=*it;
        double receives = std::min(available,planet->PopPoints() - planet->AvailableFood());
        planet->SetAvailableFood(planet->AvailableFood() + receives);
        available -= receives;
    }

    // feed for popgrown!!!!
    // use food production locally first
    for(std::vector<Planet*>::iterator it = Planets().begin();it !=Planets().end() && 0.0<available;++it)
    {
        Planet *planet=*it;
        double receives = std::min(available, std::min(planet->FarmingPoints(),PopEstimate(planet)) - planet->AvailableFood());
        planet->SetAvailableFood(planet->AvailableFood() + receives);
        available -= receives;
    }

    // second pass: give all planets up to 1 times the required minimum
    DistributeFood(Planets().begin(), Planets().end(), 1.0, available);

    // third pass: give all planets up to 2 times the required minimum
    DistributeFood(Planets().begin(), Planets().end(), 2.0, available);

    // fourth pass: give all planets up to 4 times the required minimum
    DistributeFood(Planets().begin(), Planets().end(), 4.0, available);

    m_stockpile=std::max(0.0, available);

    ChangedSignal();
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

void ResearchResourcePool::PlanetChanged()
{
    m_pool_production=0.0;

    // sum all research
    for(std::vector<Planet*>::iterator it = Planets().begin();it !=Planets().end();++it)
    {
        Planet *planet=*it;
        m_pool_production+=planet->ResearchPoints();
    }

    ChangedSignal();
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

void PopulationResourcePool::PlanetChanged()
{
    m_overall_pool=m_growth=0.0;

    for(std::vector<Planet*>::iterator it = Planets().begin();it !=Planets().end();++it)
    {
        Planet *planet=*it;
        m_overall_pool+=planet->PopPoints();
        m_growth+=planet->FuturePopGrowth();
    }

    ChangedSignal();
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

void IndustryResourcePool::PlanetChanged()
{
    m_pool_production=0.0;

    for(std::vector<Planet*>::iterator it = Planets().begin();it !=Planets().end();++it)
    {
        Planet *planet=*it;
        m_pool_production+=planet->IndustryPoints();
    }

    ChangedSignal();
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

void TradeResourcePool::PlanetChanged()
{
    m_pool_production=0.0;
    m_needed_pool=0.0;

    // sum all trade
    for(std::vector<Planet*>::iterator it = Planets().begin();it !=Planets().end();++it)
    {
        Planet *planet=*it;
        planet->SetAvailableTrade(0.0);
        m_pool_production+=planet->TradePoints();
        m_needed_pool+=planet->BuildingCosts();
    }
    double available = m_pool_production + m_stockpile;

    // first pass: give all planets required trade limited by local trade production
    for(std::vector<Planet*>::iterator it = Planets().begin();it !=Planets().end();++it)
    {
        Planet *planet=*it;
        planet->SetAvailableTrade(std::min(available,std::min(planet->TradePoints(),planet->BuildingCosts())));
        available-=planet->AvailableTrade();
    }

    // second pass: give all planets up to the required minimum to keep their buildings operating
    for(std::vector<Planet*>::iterator it = Planets().begin();it !=Planets().end();++it)
    {
        Planet *planet=*it;
        double receives = std::min(available,planet->BuildingCosts()-planet->AvailableTrade());
        planet->SetAvailableTrade(planet->AvailableTrade()+receives);
        available-=receives;
    }

    m_stockpile=available;

    ChangedSignal();
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
