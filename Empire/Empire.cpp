#include "Empire.h"

#include "../universe/Building.h"
#include "../universe/Fleet.h"
#include "../util/MultiplayerCommon.h"
#include "../universe/Predicates.h"
#include "../universe/Planet.h"
#include "ResourcePool.h"
#include "../universe/ShipDesign.h"
#include "../universe/System.h"
#include "../universe/Universe.h"
#include "../util/AppInterface.h"
#include <algorithm>

#include <boost/lexical_cast.hpp>

#include <iostream>

using std::find;
using boost::lexical_cast;


namespace {
    const double EPSILON = 1.0e-5;

    void UpdateTechQueue(double RPs, const std::map<std::string, double>& research_status, ResearchQueue::QueueType& queue, double& total_RPs_spent, int& projects_in_progress)
    {
        total_RPs_spent = 0.0;
        projects_in_progress = 0;
        for (ResearchQueue::iterator it = queue.begin(); it != queue.end(); ++it) {
            const Tech* tech = it->get<0>();
            std::map<std::string, double>::const_iterator progress_it = research_status.find(tech->Name());
            double progress = progress_it == research_status.end() ? 0.0 : progress_it->second;
            double RPs_needed = tech->ResearchCost() * tech->ResearchTurns() - progress;
            double RPs_to_spend = std::min(RPs_needed, tech->ResearchCost());
            if (total_RPs_spent + RPs_to_spend <= RPs - EPSILON) {
                it->get<1>() = RPs_to_spend;
                total_RPs_spent += it->get<1>();
                ++projects_in_progress;
            } else if (total_RPs_spent < RPs - EPSILON) {
                it->get<1>() = RPs - total_RPs_spent;
                total_RPs_spent += it->get<1>();
                ++projects_in_progress;
            } else {
                it->get<1>() = 0.0;
            }
        }
    }

    void UpdateProdQueue(Empire* empire, double PPs, const std::vector<double>& production_status, ProductionQueue::QueueType& queue, double& total_PPs_spent, int& projects_in_progress)
    {
        std::cout << "UpdateProdQueue()\n";
        assert(production_status.size() == queue.size());
        total_PPs_spent = 0.0;
        projects_in_progress = 0;
        int i = 0;
        for (ProductionQueue::iterator it = queue.begin(); it != queue.end(); ++it, ++i) {
            double item_cost;
            int build_turns;
            boost::tie(item_cost, build_turns) = empire->ProductionCostAndTime(it->item.build_type, it->item.name);
            double progress = production_status[i];
            double PPs_needed = item_cost * build_turns * it->remaining - progress;
            double PPs_to_spend = std::min(PPs_needed, item_cost);
            std::cout << "build item " << it->remaining << " X " << it->item.build_type << " " << it->item.name
                      << " (" << item_cost << "PP / " << build_turns << " turns) " << progress << "/" << (item_cost * build_turns * it->remaining) << ":\n";
            if (total_PPs_spent + PPs_to_spend <= PPs - EPSILON) {
                it->spending = PPs_to_spend;
                total_PPs_spent += it->spending;
                ++projects_in_progress;
                std::cout << "    unit completed, spending=" << it->spending << "\n";
            } else if (total_PPs_spent < PPs - EPSILON) {
                it->spending = PPs - total_PPs_spent;
                total_PPs_spent += it->spending;
                ++projects_in_progress;
                std::cout << "    unit in progress, spending=" << it->spending << "\n";
            } else {
                it->spending = 0.0;
                std::cout << "    NOT in progress, spending=0.0\n";
            }
        }
        std::cout << "\n" << std::endl;
    }

    bool temp_header_bool = RecordHeaderFile(EmpireRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}


////////////////////////////////////////
// ResearchQueue                      //
////////////////////////////////////////
ResearchQueue::ResearchQueue() :
    m_projects_in_progress(0),
    m_total_RPs_spent(0.0)
{}

ResearchQueue::ResearchQueue(const GG::XMLElement& elem) :
    m_projects_in_progress(0),
    m_total_RPs_spent(0.0)
{
    if (elem.Tag() != "ResearchQueue")
        throw std::invalid_argument("Attempted to construct a ResearchQueue from an XMLElement that had a tag other than \"ResearchQueue\"");

    // note that this leaves the queue elements incompletely specified, and does not find values for m_projects_in_progress or m_total_RPs_spent.
    // the owner of this object must call Update() after this object is constructed
    for (GG::XMLElement::const_child_iterator it = elem.Child("m_queue").child_begin(); it != elem.Child("m_queue").child_end(); ++it) {
        push_back(GetTech(it->Tag()));
    }
}

bool ResearchQueue::InQueue(const Tech* tech) const
{
    return find(tech) != end();
}

int ResearchQueue::ProjectsInProgress() const
{
    return m_projects_in_progress;
}

double ResearchQueue::TotalRPsSpent() const
{
    return m_total_RPs_spent;
}

bool ResearchQueue::empty() const
{
    return !m_queue.size();
}

unsigned int ResearchQueue::size() const
{
    return m_queue.size();
}

ResearchQueue::const_iterator ResearchQueue::begin() const
{
    return m_queue.begin();
}

ResearchQueue::const_iterator ResearchQueue::end() const
{
    return m_queue.end();
}

ResearchQueue::const_iterator ResearchQueue::find(const Tech* tech) const
{
    for (const_iterator it = begin(); it != end(); ++it) {
        if (it->get<0>() == tech)
            return it;
    }
    return end();
}

ResearchQueue::const_iterator ResearchQueue::UnderfundedProject() const
{
    for (const_iterator it = begin(); it != end(); ++it) {
        if (it->get<1>() && it->get<1>() < it->get<0>()->ResearchCost())
            return it;
    }
    return end();
}

GG::XMLElement ResearchQueue::XMLEncode() const
{
    GG::XMLElement retval("ResearchQueue");
    retval.AppendChild("m_queue");
    for (unsigned int i = 0; i < m_queue.size(); ++i) {
        retval.LastChild().AppendChild(m_queue[i].get<0>()->Name());
    }
    return retval;
}

void ResearchQueue::Update(double RPs, const std::map<std::string, double>& research_status)
{
    UpdateTechQueue(RPs, research_status, m_queue, m_total_RPs_spent, m_projects_in_progress);

    if (EPSILON < RPs) {
        // simulate future turns in order to determine when the techs in the queue will be finished
        int turns = 1;
        QueueType sim_queue = m_queue;
        std::map<std::string, double> sim_research_status = research_status;
        std::map<const Tech*, int> simulation_results;
        while (!sim_queue.empty()) {
            double total_RPs_spent = 0.0;
            int projects_in_progress = 0;
            UpdateTechQueue(RPs, sim_research_status, sim_queue, total_RPs_spent, projects_in_progress);
            for (unsigned int i = 0; i < sim_queue.size(); ++i) {
                const Tech* tech = sim_queue[i].get<0>();
                double& status = sim_research_status[tech->Name()];
                status += sim_queue[i].get<1>();
                if (tech->ResearchCost() * tech->ResearchTurns() - EPSILON <= status) {
                    simulation_results[tech] = turns;
                    sim_queue.erase(sim_queue.begin() + i--);
                }
            }
            ++turns;
        }
        for (unsigned int i = 0; i < m_queue.size(); ++i) {
            m_queue[i].get<2>() = simulation_results[m_queue[i].get<0>()];
        }
    } else {
        // since there are so few RPs, indicate that the number of turns left is indeterminate by providing a number < 0
        for (unsigned int i = 0; i < m_queue.size(); ++i) {
            m_queue[i].get<2>() = -1;
        }
    }
}

void ResearchQueue::push_back(const Tech* tech)
{
    m_queue.push_back(Element(tech, 0.0, -1));
}

void ResearchQueue::insert(iterator it, const Tech* tech)
{
    m_queue.insert(it, Element(tech, 0.0, -1));
}

void ResearchQueue::erase(iterator it)
{
    assert(it != end());
    m_queue.erase(it);
}

ResearchQueue::iterator ResearchQueue::find(const Tech* tech)
{
    for (iterator it = begin(); it != end(); ++it) {
        if (it->get<0>() == tech)
            return it;
    }
    return end();
}

ResearchQueue::iterator ResearchQueue::begin()
{
    return m_queue.begin();
}

ResearchQueue::iterator ResearchQueue::end()
{
    return m_queue.end();
}

ResearchQueue::iterator ResearchQueue::UnderfundedProject()
{
    for (iterator it = begin(); it != end(); ++it) {
        if (it->get<2>() && it->get<2>() < it->get<0>()->ResearchCost())
            return it;
    }
    return end();
}


////////////////////////////////////////
// ProductionQueue                    //
////////////////////////////////////////

// ProductionQueue::ProductionItem
ProductionQueue::ProductionItem::ProductionItem()
{}

ProductionQueue::ProductionItem::ProductionItem(BuildType build_type_, std::string name_) :
    build_type(build_type_),
    name(name_)
{}

ProductionQueue::ProductionItem::ProductionItem(const GG::XMLElement& elem)
{
    if (elem.Tag() != "ProductionItem")
        throw std::invalid_argument("Attempted to construct a ProductionQueue::ProductionItem from an XMLElement that had a tag other than \"ProductionItem\"");

    build_type = boost::lexical_cast<BuildType>(elem.Child("build_type").Text());
    name = elem.Child("name").Text();
}

GG::XMLElement ProductionQueue::ProductionItem::XMLEncode() const
{
    GG::XMLElement retval("ProductionItem");
    retval.AppendChild(GG::XMLElement("build_type", boost::lexical_cast<std::string>(build_type)));
    retval.AppendChild(GG::XMLElement("name", name));
    return retval;
}

// ProductionQueue::Elemnt
ProductionQueue::Element::Element() :
    ordered(0),
    remaining(0),
    location(UniverseObject::INVALID_OBJECT_ID),
    spending(0.0),
    turns_left_to_next_item(-1),
    turns_left_to_completion(-1)
{}

ProductionQueue::Element::Element(ProductionItem item_, int ordered_, int remaining_, int location_) :
    item(item_),
    ordered(ordered_),
    remaining(remaining_),
    location(location_),
    spending(0.0),
    turns_left_to_next_item(-1),
    turns_left_to_completion(-1)
{}

ProductionQueue::Element::Element(BuildType build_type, std::string name, int ordered_, int remaining_, int location_) :
    item(build_type, name),
    ordered(ordered_),
    remaining(remaining_),
    location(location_),
    spending(0.0),
    turns_left_to_next_item(-1),
    turns_left_to_completion(-1)
{}

ProductionQueue::Element::Element(const GG::XMLElement& elem) :
    spending(0.0),
    turns_left_to_next_item(-1),
    turns_left_to_completion(-1)
{
    if (elem.Tag() != "Element")
        throw std::invalid_argument("Attempted to construct a ProductionQueue::Element from an XMLElement that had a tag other than \"Element\"");

    // note that this leaves the queue element incompletely specified; see ProductionQueue::ProductionQueue() for details
    item = ProductionItem(elem.Child("item").Child("ProductionItem"));
    ordered = boost::lexical_cast<int>(elem.Child("ordered").Text());
    remaining = boost::lexical_cast<int>(elem.Child("remaining").Text());
    location = boost::lexical_cast<int>(elem.Child("location").Text());
}

GG::XMLElement ProductionQueue::Element::XMLEncode() const
{
    GG::XMLElement retval("Element");
    retval.AppendChild(GG::XMLElement("item", item.XMLEncode()));
    retval.AppendChild(GG::XMLElement("ordered", boost::lexical_cast<std::string>(ordered)));
    retval.AppendChild(GG::XMLElement("remaining", boost::lexical_cast<std::string>(remaining)));
    retval.AppendChild(GG::XMLElement("location", boost::lexical_cast<std::string>(location)));
    return retval;
}


// ProductionQueue
ProductionQueue::ProductionQueue() :
    m_projects_in_progress(0),
    m_total_PPs_spent(0.0)
{}

ProductionQueue::ProductionQueue(const GG::XMLElement& elem) :
    m_projects_in_progress(0),
    m_total_PPs_spent(0.0)
{
    if (elem.Tag() != "ProductionQueue")
        throw std::invalid_argument("Attempted to construct a ProductionQueue from an XMLElement that had a tag other than \"ProductionQueue\"");

    // note that this leaves the queue elements incompletely specified, and does not find values for m_projects_in_progress or m_total_PPs_spent.
    // the owner of this object must call Update() after this object is constructed
    for (GG::XMLElement::const_child_iterator it = elem.Child("m_queue").child_begin(); it != elem.Child("m_queue").child_end(); ++it) {
        m_queue.push_back(Element(*it));
    }
}

int ProductionQueue::ProjectsInProgress() const
{
    return m_projects_in_progress;
}

double ProductionQueue::TotalPPsSpent() const
{
    return m_total_PPs_spent;
}

bool ProductionQueue::empty() const
{
    return !m_queue.size();
}

unsigned int ProductionQueue::size() const
{
    return m_queue.size();
}

ProductionQueue::const_iterator ProductionQueue::begin() const
{
    return m_queue.begin();
}

ProductionQueue::const_iterator ProductionQueue::end() const
{
    return m_queue.end();
}

ProductionQueue::const_iterator ProductionQueue::find(int i) const
{
    return (0 <= i && i < static_cast<int>(size())) ? (begin() + i) : end();
}

const ProductionQueue::Element& ProductionQueue::operator[](int i) const
{
    assert(0 <= i && i < static_cast<int>(m_queue.size()));
    return m_queue[i];
}

ProductionQueue::const_iterator ProductionQueue::UnderfundedProject(const Empire* empire) const
{
    for (const_iterator it = begin(); it != end(); ++it) {
        double item_cost;
        int build_turns;
        boost::tie(item_cost, build_turns) = empire->ProductionCostAndTime(it->item.build_type, it->item.name);
        if (it->spending && it->spending < item_cost)
            return it;
    }
    return end();
}

GG::XMLElement ProductionQueue::XMLEncode() const
{
    GG::XMLElement retval("ProductionQueue");
    retval.AppendChild("m_queue");
    for (unsigned int i = 0; i < m_queue.size(); ++i) {
        retval.LastChild().AppendChild(m_queue[i].XMLEncode());
    }
    return retval;
}

void ProductionQueue::Update(Empire* empire, double PPs, const std::vector<double>& production_status)
{
    std::cout << "Update()\nTurn 0:\n";
    UpdateProdQueue(empire, PPs, production_status, m_queue, m_total_PPs_spent, m_projects_in_progress);

    if (EPSILON < PPs) {
        // simulate future turns in order to determine when the techs in the queue will be finished
        int turns = 1;
        QueueType sim_queue = m_queue;
        std::vector<double> sim_production_status = production_status;
        std::vector<int> simulation_results(sim_production_status.size(), -1);
        std::vector<int> sim_queue_original_indices(sim_production_status.size());
        for (unsigned int i = 0; i < sim_queue_original_indices.size(); ++i) {
            sim_queue_original_indices[i] = i;
        }
        while (!sim_queue.empty()) {
            double total_PPs_spent = 0.0;
            int projects_in_progress = 0;
            std::cout << "Turn " << turns << "\n";
            UpdateProdQueue(empire, PPs, sim_production_status, sim_queue, total_PPs_spent, projects_in_progress);
            for (unsigned int i = 0; i < sim_queue.size(); ++i) {
                double item_cost;
                int build_turns;
                boost::tie(item_cost, build_turns) = empire->ProductionCostAndTime(sim_queue[i].item.build_type, sim_queue[i].item.name);
                double& status = sim_production_status[i];
                status += sim_queue[i].spending;
                if (item_cost * build_turns - EPSILON <= status) {
                    sim_production_status[i] -= item_cost * build_turns;
                    if (sim_queue[i].remaining == m_queue[sim_queue_original_indices[i]].remaining) {
                        m_queue[sim_queue_original_indices[i]].turns_left_to_next_item = turns;
                    }
                    if (!--sim_queue[i].remaining) {
                        m_queue[sim_queue_original_indices[i]].turns_left_to_completion = turns;
                        sim_queue.erase(sim_queue.begin() + i);
                        sim_production_status.erase(sim_production_status.begin() + i);
                        sim_queue_original_indices.erase(sim_queue_original_indices.begin() + i--);
                    }
                }
            }
            ++turns;
        }
        std::cout << std::endl;
    } else {
        // since there are so few PPs, indicate that the number of turns left is indeterminate by providing a number < 0
        for (unsigned int i = 0; i < m_queue.size(); ++i) {
            m_queue[i].turns_left_to_next_item = -1;
            m_queue[i].turns_left_to_completion = -1;
        }
    }
}

void ProductionQueue::push_back(const Element& element)
{
    m_queue.push_back(element);
}

void ProductionQueue::insert(iterator it, const Element& element)
{
    m_queue.insert(it, element);
}

void ProductionQueue::erase(int i)
{
    assert(i <= static_cast<int>(size()));
    m_queue.erase(begin() + i);
}

void ProductionQueue::erase(iterator it)
{
    assert(it != end());
    m_queue.erase(it);
}

ProductionQueue::iterator ProductionQueue::begin()
{
    return m_queue.begin();
}

ProductionQueue::iterator ProductionQueue::end()
{
    return m_queue.end();
}

ProductionQueue::iterator ProductionQueue::find(int i)
{
    return (0 <= i && i < static_cast<int>(size())) ? (begin() + i) : end();
}

ProductionQueue::Element& ProductionQueue::operator[](int i)
{
    assert(0 <= i && i < static_cast<int>(m_queue.size()));
    return m_queue[i];
}

ProductionQueue::iterator ProductionQueue::UnderfundedProject(const Empire* empire)
{
    for (iterator it = begin(); it != end(); ++it) {
        double item_cost;
        int build_turns;
        boost::tie(item_cost, build_turns) = empire->ProductionCostAndTime(it->item.build_type, it->item.name);
        if (it->spending && it->spending < item_cost)
            return it;
    }
    return end();
}


////////////////////////////////////////
// class Empire                       //
////////////////////////////////////////
Empire::Empire(const std::string& name, const std::string& player_name, int ID, const GG::Clr& color, int homeworld_id) :
    m_id(ID),
    m_name(name),
    m_player_name(player_name),
    m_color(color), 
    m_homeworld_id(homeworld_id), 
    m_mineral_resource_pool(),
    m_food_resource_pool(),
    m_research_resource_pool(),
    m_population_resource_pool(),
    m_industry_resource_pool(),
    m_trade_resource_pool()
{}

Empire::Empire(const GG::XMLElement& elem) :
    m_research_queue(elem.Child("m_research_queue").Child("ResearchQueue")),
    m_production_queue(elem.Child("m_production_queue").Child("ProductionQueue")),
    m_mineral_resource_pool(elem.Child("m_mineral_resource_pool").Child("MineralResourcePool")),
    m_food_resource_pool(elem.Child("m_food_resource_pool").Child("FoodResourcePool")),
    m_research_resource_pool(elem.Child("m_research_resource_pool").Child("ResearchResourcePool")),
    m_population_resource_pool(elem.Child("m_population_resource_pool").Child("PopulationResourcePool")),
    m_industry_resource_pool(elem.Child("m_industry_resource_pool").Child("IndustryResourcePool")),
    m_trade_resource_pool(elem.Child("m_trade_resource_pool").Child("TradeResourcePool"))
{
    if (elem.Tag() != "Empire")
        throw std::invalid_argument("Attempted to construct a Empire from an XMLElement that had a tag other than \"Empire\"");

    using GG::XMLElement;

    m_id = lexical_cast<int>(elem.Child("m_id").Text());
    m_name = elem.Child("m_name").Text();
    m_player_name = elem.Child("m_player_name").Text();
    m_color = XMLToClr(elem.Child("m_color").Child("GG::Clr"));
    m_homeworld_id = elem.Child("m_homeworld_id").Text().empty() ? 
        UniverseObject::INVALID_OBJECT_ID :
        lexical_cast<int>(elem.Child("m_homeworld_id").Text());

    const XMLElement& sitreps_elem = elem.Child("m_sitrep_entries");
    for (int i = 0; i < sitreps_elem.NumChildren(); ++i) {
        AddSitRepEntry(new SitRepEntry(sitreps_elem.Child(i)));
    }

    const XMLElement& designs_elem = elem.Child("m_ship_designs");
    for (int i = 0; i < designs_elem.NumChildren(); ++i) {
        ShipDesign ship_design(designs_elem.Child(i).Child(0));
        m_ship_designs[ship_design.name] = ship_design;
    }

    m_explored_systems = GG::ContainerFromString<std::set<int> >(elem.Child("m_explored_systems").Text());

    const XMLElement& techs_elem = elem.Child("m_techs");
    for (int i = 0; i < techs_elem.NumChildren(); ++i) {
        m_techs.insert(techs_elem.Child(i).Text());
    }

    const XMLElement& research_status_elem = elem.Child("m_research_status");
    for (int i = 0; i < research_status_elem.NumChildren(); ++i) {
        m_research_status[research_status_elem.Child(i).Tag()] = lexical_cast<double>(research_status_elem.Child(i).Text());
    }

    m_production_status = GG::ContainerFromString<std::vector<double> >(elem.Child("m_production_status").Text());

    const XMLElement& building_types_elem = elem.Child("m_building_types");
    for (int i = 0; i < building_types_elem.NumChildren(); ++i) {
        m_building_types.insert(building_types_elem.Child(i).Text());
    }

    UpdateResourcePool();
    m_research_queue.Update(m_research_resource_pool.Production(), m_research_status);
    m_production_queue.Update(this, ProductionPoints(), m_production_status);
}

Empire::~Empire()
{
    for (std::map<std::string, BuildingType*>::const_iterator it = m_modified_building_types.begin(); it != m_modified_building_types.end(); ++it) {
        delete it->second;
    }
    ClearSitRep();
}

/** Misc Accessors */
const std::string& Empire::Name() const
{
    return m_name;
}

const std::string& Empire::PlayerName() const
{
    return m_player_name;
}

int Empire::EmpireID() const
{
    return m_id;
}

const GG::Clr& Empire::Color() const
{
    return m_color;
}

int Empire::HomeworldID() const
{
    return m_homeworld_id;
}

int Empire::CapitolID() const
{
    // TODO: come up with a system for changing (moving) the capitol from the homeworld to somewhere else
    return m_homeworld_id;
}

const ShipDesign* Empire::GetShipDesign(const std::string& name) const
{
    Empire::ShipDesignItr it = m_ship_designs.find(name);
    return (it == m_ship_designs.end()) ? 0 : &it->second;
}

bool Empire::ResearchableTech(const std::string& name) const
{
    const Tech* tech = GetTech(name);
    if (!tech)
        return false;
    const std::set<std::string>& prereqs = tech->Prerequisites();
    for (std::set<std::string>::const_iterator it = prereqs.begin(); it != prereqs.end(); ++it) {
        if (m_techs.find(*it) == m_techs.end())
            return false;
    }
    return true;
}

const ResearchQueue& Empire::GetResearchQueue() const
{
    return m_research_queue;
}

double Empire::ResearchStatus(const std::string& name) const
{
    std::map<std::string, double>::const_iterator it = m_research_status.find(name);
    return (it == m_research_status.end()) ? -1.0 : it->second;
}

const std::set<std::string>& Empire::AvailableTechs() const
{
    return m_techs;
}

bool Empire::TechAvailable(const std::string& name) const
{
    Empire::TechItr item = m_techs.find(name);
    return item != m_techs.end();
}

const std::set<std::string>& Empire::AvailableBuildingTypes() const
{
    return m_building_types;
}

bool Empire::BuildingTypeAvailable(const std::string& name) const
{
    Empire::BuildingTypeItr item = m_building_types.find(name);
    return item != m_building_types.end();
}

const BuildingType* Empire::GetBuildingType(const std::string& name) const
{
    std::map<std::string, BuildingType*>::const_iterator it = m_modified_building_types.find(name);
    if (it != m_modified_building_types.end()) {
        return it->second;
    } else {
        return ::GetBuildingType(name);
    }
}

const ProductionQueue& Empire::GetProductionQueue() const
{
    return m_production_queue;
}

double Empire::ProductionStatus(int i) const
{
    return (0 <= i && i < static_cast<int>(m_production_status.size())) ? m_production_status[i] : -1.0;
}

std::pair<double, int> Empire::ProductionCostAndTime(BuildType build_type, std::string name) const
{
    switch (build_type) {
    case BT_BUILDING: {
        const BuildingType* building_type = GetBuildingType(name);
        if (!building_type)
            break;
        return std::make_pair(building_type->BuildCost(), building_type->BuildTime());
    }
    case BT_SHIP: {
        const ShipDesign* ship_design = GetShipDesign(name);
        if (!ship_design)
            break;
        return std::make_pair(ship_design->cost, 5); // v0.3 only
    }
    case BT_ORBITAL:
        return std::make_pair(20.0, 10); // v0.3 only
    default:
        break;
    }
    return std::make_pair(-1.0, -1);
}

bool Empire::HasExploredSystem(int ID) const
{
    Empire::SystemIDItr item = find(ExploredBegin(), ExploredEnd(), ID);
    return (item != ExploredEnd());
}

bool Empire::BuildableItem(BuildType build_type, std::string name) const
{
    return ProductionCostAndTime(build_type, name) != std::make_pair(-1.0, -1);
}

int Empire::NumSitRepEntries() const
{
    return m_sitrep_entries.size();
}


/* *************************************
    (const) Iterators over our various lists
***************************************/
Empire::TechItr Empire::TechBegin() const
{
    return m_techs.begin();
}
Empire::TechItr Empire::TechEnd() const
{
    return m_techs.end();
}

Empire::TechItr Empire::BuildingTypeBegin() const
{
    return m_building_types.begin();
}
Empire::TechItr Empire::BuildingTypeEnd() const
{
    return m_building_types.end();
}

Empire::SystemIDItr Empire::ExploredBegin()  const
{
    return m_explored_systems.begin();
}
Empire::SystemIDItr Empire::ExploredEnd() const
{
    return m_explored_systems.end();
}

Empire::ShipDesignItr Empire::ShipDesignBegin() const
{
    return m_ship_designs.begin();
}
Empire::ShipDesignItr Empire::ShipDesignEnd() const
{
    return m_ship_designs.end();
}

Empire::SitRepItr Empire::SitRepBegin() const
{
    return m_sitrep_entries.begin();
}
Empire::SitRepItr Empire::SitRepEnd() const
{
    return m_sitrep_entries.end();
}

double Empire::ProductionPoints() const
{
    return std::min(m_industry_resource_pool.Production(), m_mineral_resource_pool.Stockpile());
}

void Empire::PlaceTechInQueue(const Tech* tech, int pos/* = -1*/)
{
    if (!ResearchableTech(tech->Name()) || m_techs.find(tech->Name()) != m_techs.end())
        return;
    ResearchQueue::iterator it = m_research_queue.find(tech);
    if (pos < 0 || static_cast<int>(m_research_queue.size()) <= pos) {
        if (it != m_research_queue.end())
            m_research_queue.erase(it);
        m_research_queue.push_back(tech);
    } else {
        if (it < m_research_queue.begin() + pos)
            --pos;
        if (it != m_research_queue.end())
            m_research_queue.erase(it);
        m_research_queue.insert(m_research_queue.begin() + pos, tech);
    }
    m_research_queue.Update(m_research_resource_pool.Production(), m_research_status);
}

void Empire::RemoveTechFromQueue(const Tech* tech)
{
    ResearchQueue::iterator it = m_research_queue.find(tech);
    if (it != m_research_queue.end()) {
        m_research_queue.erase(it);
        m_research_queue.Update(m_research_resource_pool.Production(), m_research_status);
    }
}

void Empire::PlaceBuildInQueue(BuildType build_type, const std::string& name, int number, int location, int pos/* = -1*/)
{
    if (!BuildableItem(build_type, name))
        return;
    ProductionQueue::Element build(build_type, name, number, number, location);
    if (pos < 0 || static_cast<int>(m_production_queue.size()) <= pos) {
        m_production_queue.push_back(build);
        m_production_status.push_back(0.0);
    } else {
        m_production_queue.insert(m_production_queue.begin() + pos, build);
        m_production_status.insert(m_production_status.begin() + pos, 0.0);
    }
    m_production_queue.Update(this, ProductionPoints(), m_production_status);
}

void Empire::MoveBuildWithinQueue(int index, int new_index)
{
    if (index < new_index)
        --new_index;
    if (0 <= index && index < static_cast<int>(m_production_queue.size()) &&
        0 <= new_index) {
        ProductionQueue::Element build = m_production_queue[index];
        double status = m_production_status[index];
        m_production_queue.erase(index);
        m_production_status.erase(m_production_status.begin() + index);
        m_production_queue.insert(m_production_queue.begin() + new_index, build);
        m_production_status.insert(m_production_status.begin() + new_index, status);
        m_production_queue.Update(this, ProductionPoints(), m_production_status);
    }
}

void Empire::RemoveBuildFromQueue(int index)
{
    if (0 <= index && index < static_cast<int>(m_production_queue.size())) {
        m_production_queue.erase(index);
        m_production_status.erase(m_production_status.begin() + index);
        m_production_queue.Update(this, ProductionPoints(), m_production_status);
    }
}

void Empire::AddTech(const std::string& name)
{
    m_techs.insert(name);
}

void Empire::UnlockItem(const Tech::ItemSpec& item)
{
    // TODO: handle other types (such as ship components) as they are implemented
    if (item.type == UIT_BUILDING) {
        AddBuildingType(item.name);
    }
}

void Empire::AddBuildingType(const std::string& name)
{
    m_building_types.insert(name);
}

void Empire::RefineBuildingType(const std::string& name, const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects)
{
    BuildingType*& building = m_modified_building_types[name];
    if (!building)
        building = new BuildingType(*::GetBuildingType(name));
    building->AddEffects(effects);
}

void Empire::ClearRefinements()
{
    for (std::map<std::string, BuildingType*>::iterator it = m_modified_building_types.begin(); it != m_modified_building_types.end(); ++it) {
        delete it->second;
    }
    m_modified_building_types.clear();
}

void Empire::AddExploredSystem(int ID)
{
    m_explored_systems.insert(ID);
}

void Empire::AddShipDesign(const ShipDesign& design)
{
   m_ship_designs[design.name] = design;
}

void Empire::AddSitRepEntry(SitRepEntry* entry)
{
    m_sitrep_entries.push_back(entry);
}


/*************************************************
    Methods to remove items from our various lists
**************************************************/
void Empire::RemoveTech(const std::string& name)
{
    m_techs.erase(name);
}

void Empire::LockItem(const Tech::ItemSpec& item)
{
    // TODO: handle other types (such as ship components) as they are implemented
    if (item.type == UIT_BUILDING) {
        RemoveBuildingType(item.name);
    }
}

void Empire::RemoveBuildingType(const std::string& name)
{
    m_building_types.erase(name);
}

void Empire::ClearSitRep()
{
    for (SitRepItr it = m_sitrep_entries.begin(); it != m_sitrep_entries.end(); ++it)
        delete *it;
    m_sitrep_entries.clear();
}

GG::XMLElement Empire::XMLEncode() const
{
    using GG::XMLElement;
    using boost::lexical_cast;

    XMLElement retval("Empire");
    retval.AppendChild(XMLElement("m_id", lexical_cast<std::string>(m_id)));
    retval.AppendChild(XMLElement("m_name", m_name));
    retval.AppendChild(XMLElement("m_player_name", m_player_name));
    retval.AppendChild(XMLElement("m_color", ClrToXML(m_color)));
    retval.AppendChild(XMLElement("m_homeworld_id", lexical_cast<std::string>(m_homeworld_id)));

    retval.AppendChild(XMLElement("m_sitrep_entries"));
    for (SitRepItr it = SitRepBegin(); it != SitRepEnd(); ++it) {
       retval.LastChild().AppendChild((*it)->XMLEncode());
    }

    retval.AppendChild(XMLElement("m_ship_designs"));
    int i = 0;
    for (ShipDesignItr it = ShipDesignBegin(); it != ShipDesignEnd(); ++it) {
        retval.LastChild().AppendChild(XMLElement("design" + lexical_cast<std::string>(i++), it->second.XMLEncode()));
    }

    retval.AppendChild(XMLElement("m_explored_systems", GG::StringFromContainer<std::set<int> >(m_explored_systems)));
    retval.AppendChild(XMLElement("m_techs"));
    i = 0;
    for (TechItr it = TechBegin(); it != TechEnd(); ++it) {
        retval.LastChild().AppendChild(XMLElement("tech" + lexical_cast<std::string>(i++), *it));
    }

    retval.AppendChild(XMLElement("m_research_queue", m_research_queue.XMLEncode()));
    retval.AppendChild(XMLElement("m_research_status"));
    for (std::map<std::string, double>::const_iterator it = m_research_status.begin(); it != m_research_status.end(); ++it) {
        retval.LastChild().AppendChild(XMLElement(it->first, lexical_cast<std::string>(it->second)));
    }

    retval.AppendChild(XMLElement("m_production_queue", m_production_queue.XMLEncode()));
    retval.AppendChild(XMLElement("m_production_status", GG::StringFromContainer(m_production_status)));

    retval.AppendChild(XMLElement("m_building_types"));
    i = 0;
    for (BuildingTypeItr it = BuildingTypeBegin(); it != BuildingTypeEnd(); ++it) {
        retval.LastChild().AppendChild(XMLElement("building_type" + lexical_cast<std::string>(i++), *it));
    }

    retval.AppendChild(XMLElement("m_mineral_resource_pool", m_mineral_resource_pool.XMLEncode()));
    retval.AppendChild(XMLElement("m_food_resource_pool", m_food_resource_pool.XMLEncode()));
    retval.AppendChild(XMLElement("m_research_resource_pool", m_research_resource_pool.XMLEncode()));
    retval.AppendChild(XMLElement("m_industry_resource_pool", m_industry_resource_pool.XMLEncode()));
    retval.AppendChild(XMLElement("m_population_resource_pool", m_population_resource_pool.XMLEncode()));
    retval.AppendChild(XMLElement("m_trade_resource_pool", m_trade_resource_pool.XMLEncode()));

    return retval;
}

GG::XMLElement Empire::XMLEncode(const Empire& viewer) const
{
    // same empire --->  call other version
    if (viewer.EmpireID() == this->EmpireID())
    {
        return XMLEncode();
    }
    
    using GG::XMLElement;
    using boost::lexical_cast;

    XMLElement retval("Empire");
    retval.AppendChild(XMLElement("m_id", lexical_cast<std::string>(m_id)));
    retval.AppendChild(XMLElement("m_name", m_name));
    retval.AppendChild(XMLElement("m_player_name", m_player_name));
    retval.AppendChild(XMLElement("m_color", ClrToXML(m_color)));

    // leave these in, but unpopulated or default-populated
    retval.AppendChild(XMLElement("m_homeworld_id"));
    retval.AppendChild(XMLElement("m_sitrep_entries"));
    retval.AppendChild(XMLElement("m_ship_designs"));
    retval.AppendChild(XMLElement("m_explored_systems"));
    retval.AppendChild(XMLElement("m_techs"));
    retval.AppendChild(XMLElement("m_research_queue", ResearchQueue().XMLEncode()));
    retval.AppendChild(XMLElement("m_research_status"));
    retval.AppendChild(XMLElement("m_production_queue", ProductionQueue().XMLEncode()));
    retval.AppendChild(XMLElement("m_production_status"));
    retval.AppendChild(XMLElement("m_building_types"));
    retval.AppendChild(XMLElement("m_mineral_resource_pool", MineralResourcePool().XMLEncode()));
    retval.AppendChild(XMLElement("m_food_resource_pool", FoodResourcePool().XMLEncode()));
    retval.AppendChild(XMLElement("m_research_resource_pool", ResearchResourcePool().XMLEncode()));
    retval.AppendChild(XMLElement("m_industry_resource_pool", IndustryResourcePool().XMLEncode()));
    retval.AppendChild(XMLElement("m_population_resource_pool", PopulationResourcePool().XMLEncode()));
    retval.AppendChild(XMLElement("m_trade_resource_pool", TradeResourcePool().XMLEncode()));

    return retval;
}

void Empire::CheckResearchProgress()
{
    m_research_queue.Update(m_research_resource_pool.Production(), m_research_status);
    std::vector<const Tech*> to_erase;
    for (ResearchQueue::iterator it = m_research_queue.begin(); it != m_research_queue.end(); ++it) {
        const Tech* tech = it->get<0>();
        double& status = m_research_status[tech->Name()];
        status += it->get<1>();
        if (tech->ResearchCost() * tech->ResearchTurns() - EPSILON <= status) {
            m_techs.insert(tech->Name());
            const std::vector<Tech::ItemSpec>& unlocked_items = tech->UnlockedItems();
            for (unsigned int i = 0; i < unlocked_items.size(); ++i) {
                UnlockItem(unlocked_items[i]);
            }
            AddSitRepEntry(CreateTechResearchedSitRep(tech->Name()));
            // TODO: create unlocked item sitreps?
            m_research_status.erase(tech->Name());
            to_erase.push_back(tech);
        }
    }

    for (std::vector<const Tech*>::iterator it = to_erase.begin(); it != to_erase.end(); ++it) {
        ResearchQueue::iterator temp_it = m_research_queue.find(*it);
        if (temp_it != m_research_queue.end())
            m_research_queue.erase(temp_it);
    }
}

void Empire::CheckProductionProgress()
{
    m_production_queue.Update(this, ProductionPoints(), m_production_status);
    std::vector<int> to_erase;
    for (unsigned int i = 0; i < m_production_queue.size(); ++i) {
        double item_cost;
        int build_turns;
        boost::tie(item_cost, build_turns) = ProductionCostAndTime(m_production_queue[i].item.build_type, m_production_queue[i].item.name);
        double& status = m_production_status[i];
        status += m_production_queue[i].spending;
        if (item_cost * build_turns - EPSILON <= status) {
            m_production_status[i] -= item_cost * build_turns;
            switch (m_production_queue[i].item.build_type) {
            case BT_BUILDING: {
                Universe& universe = GetUniverse();
                Planet* planet = universe.Object<Planet>(m_production_queue[i].location);
                assert(planet);
                Building* building = new Building(m_id, m_production_queue[i].item.name, planet->ID());
                int building_id = universe.Insert(building);
                planet->AddBuilding(building->ID());
                SitRepEntry *entry = CreateBuildingBuiltSitRep(building_id, planet->ID());
                AddSitRepEntry(entry);
                break;
            }

            case BT_SHIP: {
                Universe& universe = GetUniverse();
                UniverseObject* build_location = universe.Object(m_production_queue[i].location);
                System* system = universe_object_cast<System*>(build_location);
                if (!system && build_location)
                    system = build_location->GetSystem();
                // TODO: account for shipyards and/or other ship production sites that are in interstellar space, if needed
                assert(system);

                // create new fleet with new ship
                Fleet* fleet = new Fleet("", system->X(), system->Y(), m_id);
                int fleet_id = universe.Insert(fleet);
                // TODO: What is the mechanism for determining new fleet name?
                std::string fleet_name("New fleet ");
                fleet_name += boost::lexical_cast<std::string>(fleet_id);
                fleet->Rename(fleet_name);
                system->Insert(fleet);
  
                // add ship
                const ShipDesign* ship_design = GetShipDesign(m_production_queue[i].item.name);
                Ship *ship = new Ship(m_id, m_production_queue[i].item.name);
                int ship_id = universe.Insert(ship);
                std::string ship_name(ship_design->name);
                ship_name += boost::lexical_cast<std::string>(ship_id);
                ship->Rename(ship_name);
                fleet->AddShip(ship_id);

                // add sitrep
                SitRepEntry *entry = CreateShipBuiltSitRep(ship_id, system->ID());
                AddSitRepEntry(entry);
                break;
            }

            case BT_ORBITAL: {
                // v0.3 only
                Planet* planet = GetUniverse().Object<Planet>(m_production_queue[i].location);
                assert(planet);
                planet->AdjustDefBases(1);
                AddSitRepEntry(CreateBaseBuiltSitRep(planet->SystemID(), planet->ID()));
                break;
            }

            default:
                break;
            }

            if (!--m_production_queue[i].remaining)
                to_erase.push_back(i);
        }
    }

    for (std::vector<int>::iterator it = to_erase.begin(); it != to_erase.end(); ++it) {
        m_production_status.erase(m_production_status.begin() + *it);
        m_production_queue.erase(*it);
    }

    m_mineral_resource_pool.SetStockpile(m_mineral_resource_pool.Stockpile() + (m_mineral_resource_pool.Production() - m_production_queue.TotalPPsSpent()));
}

void Empire::SetColor(const GG::Clr& color)
{
    m_color = color;
}

void Empire::SetName(const std::string& name)
{
    m_name = name;
}

void Empire::SetPlayerName(const std::string& player_name)
{
    m_player_name = player_name;
}

void Empire::UpdateResourcePool()
{
    m_mineral_resource_pool.SetPlanets(GetUniverse().FindObjects(OwnedVisitor<Planet>(m_id)));
    m_food_resource_pool.SetPlanets(GetUniverse().FindObjects(OwnedVisitor<Planet>(m_id)));
    m_research_resource_pool.SetPlanets(GetUniverse().FindObjects(OwnedVisitor<Planet>(m_id)));
    m_industry_resource_pool.SetPlanets(GetUniverse().FindObjects(OwnedVisitor<Planet>(m_id)));
    m_population_resource_pool.SetPlanets(GetUniverse().FindObjects(OwnedVisitor<Planet>(m_id)));
    m_industry_resource_pool.SetPlanets(GetUniverse().FindObjects(OwnedVisitor<Planet>(m_id)));
    m_trade_resource_pool.SetPlanets(GetUniverse().FindObjects(OwnedVisitor<Planet>(m_id)));
}

void Empire::UpdateResearchQueue()
{
    m_research_queue.Update(m_research_resource_pool.Production(), m_research_status);
}

void Empire::UpdateProductionQueue()
{
    m_production_queue.Update(this, m_industry_resource_pool.Production(), m_production_status);
}
