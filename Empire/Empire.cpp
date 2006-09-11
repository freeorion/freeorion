#include "Empire.h"

#include "../universe/Building.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../util/MultiplayerCommon.h"
#include "../universe/Predicates.h"
#include "../universe/Planet.h"
#include "ResourcePool.h"
#include "../universe/ShipDesign.h"
#include "../universe/System.h"
#include "../universe/Universe.h"
#include "../util/AppInterface.h"
#include "../universe/Enums.h"
#include "../universe/UniverseObject.h"
#include "EmpireManager.h"

#include <algorithm>

#include <boost/lexical_cast.hpp>


using std::find;
using boost::lexical_cast;

namespace {
    const double EPSILON = 1.0e-5;

    void SetTechQueueElementSpending(double RPs, const std::map<std::string, double>& research_status, ResearchQueue::QueueType& queue, double& total_RPs_spent, int& projects_in_progress)
    {
        total_RPs_spent = 0.0;
        projects_in_progress = 0;
        for (ResearchQueue::iterator it = queue.begin(); it != queue.end(); ++it) {
            const Tech* tech = it->tech;
            std::map<std::string, double>::const_iterator progress_it = research_status.find(tech->Name());
            double progress = progress_it == research_status.end() ? 0.0 : progress_it->second;
            double RPs_needed = tech->ResearchCost() * tech->ResearchTurns() - progress;
            double RPs_to_spend = std::min(RPs_needed, tech->ResearchCost());
            if (total_RPs_spent + RPs_to_spend <= RPs - EPSILON) {
                it->spending = RPs_to_spend;
                total_RPs_spent += it->spending;
                ++projects_in_progress;
            } else if (total_RPs_spent < RPs - EPSILON) {
                it->spending = RPs - total_RPs_spent;
                total_RPs_spent += it->spending;
                ++projects_in_progress;
            } else {
                it->spending = 0.0;
            }
        }
    }
    
    // sets the .spending, value for each Element in the queue.  Only sets nonzero funding to
    // an Element if its ProductionItem is buildable this turn.  Also determines total number
    // of spent PP (returning by reference in total_PPs_spent)
    void SetProdQueueElementSpending(Empire* empire, double PPs, const std::vector<double>& production_status, ProductionQueue::QueueType& queue, double& total_PPs_spent, int& projects_in_progress)
    {
        assert(production_status.size() == queue.size());
        total_PPs_spent = 0.0;
        projects_in_progress = 0;
        int i = 0;

        for (ProductionQueue::iterator it = queue.begin(); it != queue.end(); ++it, ++i) {
            // get details on what is being built...
            BuildType build_type = it->item.build_type;
            std::string name = it->item.name;
            int location = it->location;
            bool buildable = empire->BuildableItem(build_type, name, location);
                        
            if (buildable) {
                //Logger().debugStream() << "SetProdQueueElementSpending: ..Item is buildable";
                double item_cost;
                int build_turns;
                boost::tie(item_cost, build_turns) = empire->ProductionCostAndTime(build_type, name);
                double progress = production_status[i];
                double PPs_needed = item_cost * build_turns * it->remaining - progress;
                double PPs_to_spend = std::min(PPs_needed, item_cost);
                if (total_PPs_spent + PPs_to_spend <= PPs - EPSILON) {
                    it->spending = PPs_to_spend;
                    total_PPs_spent += it->spending;
                    ++projects_in_progress;
                } else if (total_PPs_spent < PPs - EPSILON) {
                    it->spending = PPs - total_PPs_spent;
                    total_PPs_spent += it->spending;
                    ++projects_in_progress;
                } else {
                    it->spending = 0.0;
                }
                //Logger().debugStream() << "SetProdQueueElementSpending: ....spending: " << it->spending;
            } else {
                // item can't be produced at its location this turn
                it->spending = 0.0;
                //Logger().debugStream() << "SetProdQueueElementSpending: ..Item is not buildable.";
            }
        }
    }
}


////////////////////////////////////////
// ResearchQueue::Element             //
////////////////////////////////////////
ResearchQueue::Element::Element() :
    tech(0),
    spending(0.0),
    turns_left(0)
{}

ResearchQueue::Element::Element(const Tech* tech_, double spending_, int turns_left_) :
    tech(tech_),
    spending(spending_),
    turns_left(turns_left_)
{}


////////////////////////////////////////
// ResearchQueue                      //
////////////////////////////////////////
ResearchQueue::ResearchQueue() :
    m_projects_in_progress(0),
    m_total_RPs_spent(0.0)
{}

ResearchQueue::ResearchQueue(const XMLElement& elem) :
    m_projects_in_progress(0),
    m_total_RPs_spent(0.0)
{
    if (elem.Tag() != "ResearchQueue")
        throw std::invalid_argument("Attempted to construct a ResearchQueue from an XMLElement that had a tag other than \"ResearchQueue\"");

    // note that this leaves the queue elements incompletely specified, and does not find values for m_projects_in_progress or m_total_RPs_spent.
    // the owner of this object must call Update() after this object is constructed
    for (XMLElement::const_child_iterator it = elem.Child("m_queue").child_begin(); it != elem.Child("m_queue").child_end(); ++it) {
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
        if (it->tech == tech)
            return it;
    }
    return end();
}

ResearchQueue::const_iterator ResearchQueue::UnderfundedProject() const
{
    for (const_iterator it = begin(); it != end(); ++it) {
        if (it->spending && it->spending < it->tech->ResearchCost() && 1 < it->turns_left)
            return it;
    }
    return end();
}

XMLElement ResearchQueue::XMLEncode() const
{
    XMLElement retval("ResearchQueue");
    retval.AppendChild("m_queue");
    for (unsigned int i = 0; i < m_queue.size(); ++i) {
        retval.LastChild().AppendChild(m_queue[i].tech->Name());
    }
    return retval;
}

void ResearchQueue::Update(double RPs, const std::map<std::string, double>& research_status)
{
    SetTechQueueElementSpending(RPs, research_status, m_queue, m_total_RPs_spent, m_projects_in_progress);

    if (m_queue.empty()) return;    // nothing more to do...
    const int TOO_MANY_TURNS = 500; // stop counting turns to completion after this long, to prevent seemingly endless loops

    if (EPSILON < RPs) {
        // simulate future turns in order to determine when the techs in the queue will be finished
        int turns = 1;
        QueueType sim_queue = m_queue;
        std::map<std::string, double> sim_research_status = research_status;

        std::map<const Tech*, int> simulation_results;
        // initialize simulation_results with -1 for all techs, so that any techs that aren't
        // finished in simulation by turn TOO_MANY_TURNS will be left marked as never to be finished
        for (unsigned int i = 0; i < sim_queue.size(); ++i)
            simulation_results[m_queue[i].tech] = -1;

        while (!sim_queue.empty() && turns < TOO_MANY_TURNS) {
            double total_RPs_spent = 0.0;
            int projects_in_progress = 0;
            SetTechQueueElementSpending(RPs, sim_research_status, sim_queue, total_RPs_spent, projects_in_progress);
            for (unsigned int i = 0; i < sim_queue.size(); ++i) {
                const Tech* tech = sim_queue[i].tech;
                double& status = sim_research_status[tech->Name()];
                status += sim_queue[i].spending;
                if (tech->ResearchCost() * tech->ResearchTurns() - EPSILON <= status) {
                    m_queue[i].turns_left = simulation_results[m_queue[i].tech];
                    simulation_results[tech] = turns;
                    sim_queue.erase(sim_queue.begin() + i--);
                }
            }
            ++turns;
        }
        // return results
        for (unsigned int i = 0; i < m_queue.size(); ++i) {
            m_queue[i].turns_left = simulation_results[m_queue[i].tech];
        }
    } else {
        // since there are so few RPs, indicate that the number of turns left is indeterminate by providing a number < 0
        for (unsigned int i = 0; i < m_queue.size(); ++i) {
            m_queue[i].turns_left = -1;
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
        if (it->tech == tech)
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
        if (it->spending && it->spending < it->tech->ResearchCost() && 1 < it->turns_left)
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

ProductionQueue::ProductionItem::ProductionItem(const XMLElement& elem)
{
    if (elem.Tag() != "ProductionItem")
        throw std::invalid_argument("Attempted to construct a ProductionQueue::ProductionItem from an XMLElement that had a tag other than \"ProductionItem\"");

    build_type = boost::lexical_cast<BuildType>(elem.Child("build_type").Text());
    name = elem.Child("name").Text();
}

XMLElement ProductionQueue::ProductionItem::XMLEncode() const
{
    XMLElement retval("ProductionItem");
    retval.AppendChild(XMLElement("build_type", boost::lexical_cast<std::string>(build_type)));
    retval.AppendChild(XMLElement("name", name));
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

ProductionQueue::Element::Element(const XMLElement& elem) :
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

XMLElement ProductionQueue::Element::XMLEncode() const
{
    XMLElement retval("Element");
    retval.AppendChild(XMLElement("item", item.XMLEncode()));
    retval.AppendChild(XMLElement("ordered", boost::lexical_cast<std::string>(ordered)));
    retval.AppendChild(XMLElement("remaining", boost::lexical_cast<std::string>(remaining)));
    retval.AppendChild(XMLElement("location", boost::lexical_cast<std::string>(location)));
    return retval;
}


// ProductionQueue
ProductionQueue::ProductionQueue() :
    m_projects_in_progress(0),
    m_total_PPs_spent(0.0)
{}

ProductionQueue::ProductionQueue(const XMLElement& elem) :
    m_projects_in_progress(0),
    m_total_PPs_spent(0.0)
{
    if (elem.Tag() != "ProductionQueue")
        throw std::invalid_argument("Attempted to construct a ProductionQueue from an XMLElement that had a tag other than \"ProductionQueue\"");

    // note that this leaves the queue elements incompletely specified, and does not find values for m_projects_in_progress or m_total_PPs_spent.
    // the owner of this object must call Update() after this object is constructed
    for (XMLElement::const_child_iterator it = elem.Child("m_queue").child_begin(); it != elem.Child("m_queue").child_end(); ++it) {
        m_queue.push_back(Element(it->Child(0)));
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
        if (it->spending && it->spending < item_cost && 1 < it->turns_left_to_next_item)
            return it;
    }
    return end();
}

XMLElement ProductionQueue::XMLEncode() const
{
    XMLElement retval("ProductionQueue");
    retval.AppendChild("m_queue");
    for (unsigned int i = 0; i < m_queue.size(); ++i) {
        retval.LastChild().AppendChild(XMLElement("Element" + boost::lexical_cast<std::string>(i), m_queue[i].XMLEncode()));
    }
    return retval;
}

void ProductionQueue::Update(Empire* empire, double PPs, const std::vector<double>& production_status)
{
    SetProdQueueElementSpending(empire, PPs, production_status, m_queue, m_total_PPs_spent, m_projects_in_progress);

    if (m_queue.empty()) return;   // nothing more to do...
    const int TOO_MANY_TURNS = 500; // stop counting turns to completion after this long, to prevent seemingly endless loops
    
    if (EPSILON < PPs) {
        //Logger().debugStream() << "ProductionQueue::Update: Simulating future turns of production queue";
        // simulate future turns in order to determine when the builditems in the queue will be finished
        int turns = 1;
        QueueType sim_queue = m_queue;
        std::vector<double> sim_production_status = production_status;
        std::vector<int> simulation_results(sim_production_status.size(), -1);
        std::vector<int> sim_queue_original_indices(sim_production_status.size());
        for (unsigned int i = 0; i < sim_queue_original_indices.size(); ++i) {
            sim_queue_original_indices[i] = i;
        }
        
        // remove from simulated queue any items that can't be built due to not meeting their location conditions
        // might be better to re-check buildability each turn, but this would require creating a simulated universe
        // into which simulated completed buildings could be inserted, as well as spoofing the current turn, or
        // otherwise faking the results for evaluating arbitrary location conditions for the simulated universe.
        // this would also be inaccurate anyway due to player choices or random chance, so for simplicity, it is
        // assume that building location conditions evaluated at the present turn apply indefinitely
        for (unsigned int i = 0; i < sim_queue.size(); ++i) {
            BuildType build_type = sim_queue[i].item.build_type;
            std::string name = sim_queue[i].item.name;
            int location = sim_queue[i].location;
            if (empire->BuildableItem(build_type, name, location)) continue;
            
            // remove unbuildable items from the simulated queue, since they'll never finish...            
            m_queue[sim_queue_original_indices[i]].turns_left_to_completion = -1;   // turns left is indeterminate for this item
            sim_queue.erase(sim_queue.begin() + i);
            sim_production_status.erase(sim_production_status.begin() + i);
            sim_queue_original_indices.erase(sim_queue_original_indices.begin() + i--);
        }
        
        // cycle through items on queue, adding up their allotted PP until each is finished and removed from queue
        // until everything on queue has been finished, in order to calculate expected completion times
        while (!sim_queue.empty() && turns < TOO_MANY_TURNS) {
            double total_PPs_spent = 0.0;
            int projects_in_progress = 0;

            //Logger().debugStream() << "ProductionQueue::Update: Calling SetProdQueueElementSpending for simulated queue";
            SetProdQueueElementSpending(empire, PPs, sim_production_status, sim_queue, total_PPs_spent, projects_in_progress);
            
            // cycle through items on queue, apply one turn's PP towards items, remove items that are done
            for (unsigned int i = 0; i < sim_queue.size(); ++i) {
                BuildType build_type = sim_queue[i].item.build_type;
                std::string name = sim_queue[i].item.name;
                double item_cost;
                int build_turns;
                boost::tie(item_cost, build_turns) = empire->ProductionCostAndTime(build_type, name);
                
                double& status = sim_production_status[i];
                status += sim_queue[i].spending;
                
                if (item_cost * build_turns - EPSILON <= status) {
                    sim_production_status[i] -= item_cost * build_turns;    // might have spillover to next item in order, so don't set to exactly 0
                    if (sim_queue[i].remaining == m_queue[sim_queue_original_indices[i]].remaining) {
                        m_queue[sim_queue_original_indices[i]].turns_left_to_next_item = turns;
                    }
                    if (!--sim_queue[i].remaining) {
                        //Logger().debugStream() << "    ITEM COMPLETE!  REMOVING";
                        m_queue[sim_queue_original_indices[i]].turns_left_to_completion = turns;
                        sim_queue.erase(sim_queue.begin() + i);
                        sim_production_status.erase(sim_production_status.begin() + i);
                        sim_queue_original_indices.erase(sim_queue_original_indices.begin() + i--);
                    }
                }
            }            
            ++turns;            
        }   // loop while (!sim_queue.empty() && turns < TOO_MANY_TURNS)
        
        // mark rest of items on simulated queue (if any) as never to be finished
        for (unsigned int i = 0; i < sim_queue.size(); ++i) {
            if (sim_queue[i].remaining == m_queue[sim_queue_original_indices[i]].remaining)
                m_queue[sim_queue_original_indices[i]].turns_left_to_next_item = -1;
            m_queue[sim_queue_original_indices[i]].turns_left_to_completion = -1;
        }
        
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

ProductionQueue::iterator ProductionQueue::erase(iterator it)
{
    assert(it != end());
    return m_queue.erase(it);
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
        if (it->spending && it->spending < item_cost && 1 < it->turns_left_to_next_item)
            return it;
    }
    return end();
}


////////////////////////////////////////
// class Empire                       //
////////////////////////////////////////
Empire::Empire() :
    m_id(-1),
    m_homeworld_id(-1),
    m_mineral_resource_pool(RE_MINERALS),
    m_food_resource_pool(RE_FOOD),
    m_research_resource_pool(RE_RESEARCH),
    m_population_pool(),
    m_industry_resource_pool(RE_INDUSTRY),
    m_trade_resource_pool(RE_TRADE)

{}

Empire::Empire(const std::string& name, const std::string& player_name, int ID, const GG::Clr& color, int homeworld_id) :
    m_id(ID),
    m_name(name),
    m_player_name(player_name),
    m_color(color), 
    m_homeworld_id(homeworld_id), 
    m_mineral_resource_pool(RE_MINERALS),
    m_food_resource_pool(RE_FOOD),
    m_research_resource_pool(RE_RESEARCH),
    m_population_pool(),
    m_industry_resource_pool(RE_INDUSTRY),
    m_trade_resource_pool(RE_TRADE)
{}

Empire::Empire(const XMLElement& elem) :
    m_research_queue(elem.Child("m_research_queue").Child("ResearchQueue")),
    m_production_queue(elem.Child("m_production_queue").Child("ProductionQueue")),
    m_mineral_resource_pool(elem.Child("m_mineral_resource_pool").Child("ResourcePool")),
    m_food_resource_pool(elem.Child("m_food_resource_pool").Child("ResourcePool")),
    m_research_resource_pool(elem.Child("m_research_resource_pool").Child("ResourcePool")),
    m_population_pool(elem.Child("m_population_resource_pool").Child("PopulationPool")),
    m_industry_resource_pool(elem.Child("m_industry_resource_pool").Child("ResourcePool")),
    m_trade_resource_pool(elem.Child("m_trade_resource_pool").Child("ResourcePool"))
{
    if (elem.Tag() != "Empire")
        throw std::invalid_argument("Attempted to construct a Empire from an XMLElement that had a tag other than \"Empire\"");

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

    m_explored_systems = ContainerFromString<std::set<int> >(elem.Child("m_explored_systems").Text());

    const XMLElement& techs_elem = elem.Child("m_techs");
    for (int i = 0; i < techs_elem.NumChildren(); ++i) {
        m_techs.insert(techs_elem.Child(i).Text());
    }

    const XMLElement& research_status_elem = elem.Child("m_research_status");
    for (int i = 0; i < research_status_elem.NumChildren(); ++i) {
        m_research_status[research_status_elem.Child(i).Tag()] = lexical_cast<double>(research_status_elem.Child(i).Text());
    }

    m_production_status = ContainerFromString<std::vector<double> >(elem.Child("m_production_status").Text());

    const XMLElement& building_types_elem = elem.Child("m_building_types");
    for (int i = 0; i < building_types_elem.NumChildren(); ++i) {
        m_building_types.insert(building_types_elem.Child(i).Text());
    }
}

Empire::~Empire()
{
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

bool Empire::BuildableItem(BuildType build_type, std::string name, int location) const
{
    if (ProductionCostAndTime(build_type, name) != std::make_pair(-1.0, -1)) {
        UniverseObject* build_location = GetUniverse().Object(location);

        if (build_type == BT_BUILDING) {
            const BuildingType* building_type = GetBuildingType(name);
            if (!building_type) return false;
            return building_type->ProductionLocation(m_id, location);

        } else {
            return build_location && build_location->Owners().size() == 1 &&
                *build_location->Owners().begin() == m_id;
        }
        // TODO: require ships to be built at shipyards
    } else {
        return false;
    }
}

int Empire::NumSitRepEntries() const
{
    return m_sitrep_entries.size();
}


/**************************************
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
    return std::min(m_industry_resource_pool.Available(), m_mineral_resource_pool.Available());
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
    m_research_queue.Update(m_research_resource_pool.Available(), m_research_status);
}

void Empire::RemoveTechFromQueue(const Tech* tech)
{
    ResearchQueue::iterator it = m_research_queue.find(tech);
    if (it != m_research_queue.end()) {
        m_research_queue.erase(it);
        m_research_queue.Update(m_research_resource_pool.Available(), m_research_status);
    }
}

void Empire::PlaceBuildInQueue(BuildType build_type, const std::string& name, int number, int location, int pos/* = -1*/)
{
    if (!BuildableItem(build_type, name, location))
        Logger().debugStream() << "Empire::PlaceBuildInQueue() : Placed a non-buildable item in queue...";

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

void Empire::SetBuildQuantity(int index, int quantity)
{
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index)
        throw std::runtime_error("Empire::SetBuildQuantity() : Attempted to adjust the quantity of items to be built in a nonexistent production queue item.");
    if (quantity < 1)
        throw std::runtime_error("Empire::SetBuildQuantity() : Attempted to set the quantity of a build run to a value less than zero.");
    if (m_production_queue[index].item.build_type == BT_BUILDING && 1 < quantity)
        throw std::runtime_error("Empire::SetBuildQuantity() : Attempted to build more than one instance of a building in the same build run.");
    int original_quantity = m_production_queue[index].remaining;
    m_production_queue[index].remaining = quantity;
    m_production_queue[index].ordered += quantity - original_quantity;
    m_production_queue.Update(this, ProductionPoints(), m_production_status);
}

void Empire::MoveBuildWithinQueue(int index, int new_index)
{
    if (index < new_index)
        --new_index;
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index ||
        new_index < 0 || static_cast<int>(m_production_queue.size()) <= new_index)
        throw std::runtime_error("Empire::MoveBuildWithinQueue() : Attempted to move a production queue item to or from an invalid index.");
    ProductionQueue::Element build = m_production_queue[index];
    double status = m_production_status[index];
    m_production_queue.erase(index);
    m_production_status.erase(m_production_status.begin() + index);
    m_production_queue.insert(m_production_queue.begin() + new_index, build);
    m_production_status.insert(m_production_status.begin() + new_index, status);
    m_production_queue.Update(this, ProductionPoints(), m_production_status);
}

void Empire::RemoveBuildFromQueue(int index)
{
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index)
        throw std::runtime_error("Empire::RemoveBuildFromQueue() : Attempted to delete a production queue item with an invalid index.");
    m_production_queue.erase(index);
    m_production_status.erase(m_production_status.begin() + index);
    m_production_queue.Update(this, ProductionPoints(), m_production_status);
}

void Empire::ConquerBuildsAtLocation(int location_id) {
    if (location_id == UniverseObject::INVALID_OBJECT_ID)
        throw std::invalid_argument("Empire::ConquerBuildsAtLocationFromEmpire: tried to conquer build items located at an invalid location");
    
    Logger().debugStream() << "Empire::ConquerBuildsAtLocationFromEmpire: conquering items located at " << location_id << " to empire " << m_id;
    /** Processes Builditems on queues of empires other than this empire, at the location with id \a location_id and,
        as appropriate, adds them to the build queue of \a this empire, deletes them, or leaves them on the build 
        queue of their current empire */

    for (EmpireManager::iterator emp_it = Empires().begin(); emp_it != Empires().end(); ++emp_it) {
        int from_empire_id = emp_it->first;
        if (from_empire_id == m_id) continue;    // skip this empire; can't capture one's own builditems

        Empire* from_empire = emp_it->second;
        ProductionQueue& queue = from_empire->m_production_queue;
        std::vector<double>& status = from_empire->m_production_status;

        int i = 0;
        for (ProductionQueue::iterator queue_it = queue.begin(); queue_it != queue.end(); ) {
            ProductionQueue::Element elem = *queue_it;
            if (elem.location != location_id) continue; // skip projects with wrong location
            
            ProductionQueue::ProductionItem item = elem.item;
            
            if (item.build_type == BT_BUILDING) {
                std::string name = item.name;
                const BuildingType* type = GetBuildingType(name);
                if (!type)
                    throw std::invalid_argument("Empire::ConquerBuildsAtLocationFromEmpire: ProductionQueue item had an invalid BuildingType name");
                
                CaptureResult result = type->GetCaptureResult(from_empire_id, m_id, location_id, true);
                
                if (result == DESTROY) {
                    // item removed from current queue, NOT added to conquerer's queue
                    queue_it = queue.erase(queue_it);
                    status.erase(status.begin() + i);
                    
                } else if (result == CAPTURE) {
                    // item removed from current queue, added to conquerer's queue
                    ProductionQueue::Element build(item, elem.ordered, elem.remaining, location_id);
                    m_production_queue.push_back(build);

                    m_production_status.push_back(status[i]);

                    queue_it = queue.erase(queue_it);
                    status.erase(status.begin() + i);
                    
                } else if (result == INVALID_CAPTURE_RESULT) {
                    throw std::invalid_argument("Empire::ConquerBuildsAtLocationFromEmpire: BuildingType had an invalid CaptureResult");
                } else {
                    ++queue_it;
                    ++i;
                }
                // otherwise do nothing: item left on current queue, conquerer gets nothing
            } else {
                ++queue_it;
                ++i;
            }

            // TODO: other types of build item...
        }
    }
}

void Empire::AddTech(const std::string& name)
{
    m_techs.insert(name);
}

void Empire::UnlockItem(const ItemSpec& item)
{
    // TODO: handle other types (such as ship components) as they are implemented
    if (item.type == UIT_BUILDING)
        AddBuildingType(item.name);
}

void Empire::AddBuildingType(const std::string& name)
{
    m_building_types.insert(name);
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

void Empire::LockItem(const ItemSpec& item)
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

XMLElement Empire::XMLEncode(int empire_id/* = ALL_EMPIRES*/) const
{
    using boost::lexical_cast;

    XMLElement retval("Empire");
    retval.AppendChild(XMLElement("m_id", lexical_cast<std::string>(m_id)));
    retval.AppendChild(XMLElement("m_name", m_name));
    retval.AppendChild(XMLElement("m_player_name", m_player_name));
    retval.AppendChild(XMLElement("m_color", ClrToXML(m_color)));

    if (empire_id == m_id || empire_id == ALL_EMPIRES) {
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

        retval.AppendChild(XMLElement("m_explored_systems", StringFromContainer<std::set<int> >(m_explored_systems)));
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
        retval.AppendChild(XMLElement("m_production_status", StringFromContainer(m_production_status)));

        retval.AppendChild(XMLElement("m_building_types"));
        i = 0;
        for (BuildingTypeItr it = BuildingTypeBegin(); it != BuildingTypeEnd(); ++it) {
            retval.LastChild().AppendChild(XMLElement("building_type" + lexical_cast<std::string>(i++), *it));
        }

        retval.AppendChild(XMLElement("m_mineral_resource_pool", m_mineral_resource_pool.XMLEncode()));
        retval.AppendChild(XMLElement("m_food_resource_pool", m_food_resource_pool.XMLEncode()));
        retval.AppendChild(XMLElement("m_research_resource_pool", m_research_resource_pool.XMLEncode()));
        retval.AppendChild(XMLElement("m_industry_resource_pool", m_industry_resource_pool.XMLEncode()));
        retval.AppendChild(XMLElement("m_population_resource_pool", m_population_pool.XMLEncode()));
        retval.AppendChild(XMLElement("m_trade_resource_pool", m_trade_resource_pool.XMLEncode()));
    } else {
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
        retval.AppendChild(XMLElement("m_mineral_resource_pool", ResourcePool(RE_MINERALS).XMLEncode()));
        retval.AppendChild(XMLElement("m_food_resource_pool", ResourcePool(RE_FOOD).XMLEncode()));
        retval.AppendChild(XMLElement("m_research_resource_pool", ResourcePool(RE_RESEARCH).XMLEncode()));
        retval.AppendChild(XMLElement("m_industry_resource_pool", ResourcePool(RE_INDUSTRY).XMLEncode()));
        retval.AppendChild(XMLElement("m_population_resource_pool", PopulationPool().XMLEncode()));
        retval.AppendChild(XMLElement("m_trade_resource_pool", ResourcePool(RE_TRADE).XMLEncode()));
    }
    return retval;
}

void Empire::CheckResearchProgress()
{
    // following commented line should be redundant, as previous call to UpdateResourcePools should have generated necessary info
    // m_research_queue.Update(m_research_resource_pool.Available(), m_research_status);
    std::vector<const Tech*> to_erase;
    for (ResearchQueue::iterator it = m_research_queue.begin(); it != m_research_queue.end(); ++it) {
        const Tech* tech = it->tech;
        double& status = m_research_status[tech->Name()];
        status += it->spending;
        if (tech->ResearchCost() * tech->ResearchTurns() - EPSILON <= status) {
            m_techs.insert(tech->Name());
            const std::vector<ItemSpec>& unlocked_items = tech->UnlockedItems();
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
    // can uncomment following line when / if research stockpiling is enabled...
    // m_research_resource_pool.SetStockpile(m_industry_resource_pool.Available() - m_research_queue.TotalRPsSpent());
}

void Empire::CheckProductionProgress()
{
    // following commented line should be redundant, as previous call to UpdateResourcePools should have generated necessary info
    // m_production_queue.Update(this, ProductionPoints(), m_production_status);
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
                planet->AddBuilding(building_id);
                SitRepEntry *entry = CreateBuildingBuiltSitRep(m_production_queue[i].item.name, planet->ID());
                AddSitRepEntry(entry);
                //Logger().debugStream() << "New Building created on turn: " << building->CreationTurn();
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
                Logger().debugStream() << "New Fleet created on turn: " << fleet->CreationTurn();
  
                // add ship
                const ShipDesign* ship_design = GetShipDesign(m_production_queue[i].item.name);
                Ship *ship = new Ship(m_id, m_production_queue[i].item.name);
                int ship_id = universe.Insert(ship);
                std::string ship_name(ship_design->name);
                ship_name += boost::lexical_cast<std::string>(ship_id);
                ship->Rename(ship_name);
                fleet->AddShip(ship_id);
                Logger().debugStream() << "New Ship created on turn: " << ship->CreationTurn();

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

    for (std::vector<int>::reverse_iterator it = to_erase.rbegin(); it != to_erase.rend(); ++it) {
        m_production_status.erase(m_production_status.begin() + *it);
        m_production_queue.erase(*it);
    }

    m_mineral_resource_pool.SetStockpile(m_mineral_resource_pool.Available() - m_production_queue.TotalPPsSpent());
    // can uncomment following line when / if industry stockpiling is allowed...
    // m_industry_resource_pool.SetStockpile(m_industry_resource_pool.Available() - m_production_queue.TotalPPsSpent());
}

void Empire::CheckTradeSocialProgress()
{
    m_trade_resource_pool.SetStockpile(m_trade_resource_pool.Available() - m_maintenance_total_cost);
}

/** Updates food stockpile.  Growth actually occurs in PopGrowthProductionResearchPhase() of objects
  */
void Empire::CheckGrowthFoodProgress()
{
    m_food_resource_pool.SetStockpile(m_food_resource_pool.Available() - m_food_total_distributed);
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
    Universe::ObjectVec object_vec = GetUniverse().FindObjects(OwnedVisitor<UniverseObject>(m_id));
    std::vector<ResourceCenter*> res_vec;
    std::vector<PopCenter*> pop_vec;
    // determine if each object owned by this empire is a ResourceCenter and/or PopCenter (could be one, neither or both)
    for (unsigned int i = 0; i < object_vec.size(); ++i)
    {
        if (ResourceCenter* rc = dynamic_cast<ResourceCenter*>(object_vec[i]))
	        res_vec.push_back(rc);
	    if (PopCenter* pc = dynamic_cast<PopCenter*>(object_vec[i]))
	        pop_vec.push_back(pc);
    }

    m_mineral_resource_pool.SetResourceCenters(res_vec);
    m_food_resource_pool.SetResourceCenters(res_vec);
    m_research_resource_pool.SetResourceCenters(res_vec);
    m_industry_resource_pool.SetResourceCenters(res_vec);
    m_trade_resource_pool.SetResourceCenters(res_vec);

    m_population_pool.SetPopCenters(pop_vec);

    UpdateResearchQueue();
    UpdateProductionQueue();
    UpdateTradeSpending();
    UpdateFoodDistribution();
    UpdatePopulationGrowth();
}

void Empire::UpdateResearchQueue()
{
    m_research_resource_pool.Update();
    m_research_queue.Update(m_research_resource_pool.Available(), m_research_status);
    m_research_resource_pool.ChangedSignal();
}

void Empire::UpdateProductionQueue()
{
    m_mineral_resource_pool.Update();
    m_industry_resource_pool.Update();
    m_production_queue.Update(this, ProductionPoints(), m_production_status);
    m_mineral_resource_pool.ChangedSignal();
    m_industry_resource_pool.ChangedSignal();
}

void Empire::UpdateTradeSpending()
{
    m_trade_resource_pool.Update();

    // TODO: Replace with call to some other subsystem, similar to the Update...Queue functions
    m_maintenance_total_cost = 0.0;

    Universe::ObjectVec buildings = GetUniverse().FindObjects(OwnedVisitor<Building>(m_id));
    for (Universe::ObjectVec::const_iterator it = buildings.begin(); it != buildings.end(); ++it)
    {
        Building *building = universe_object_cast<Building*>(*it);
        if (!building) continue;
        if (building->Operating())
            m_maintenance_total_cost += GetBuildingType(building->BuildingTypeName())->MaintenanceCost();
    }
    m_trade_resource_pool.ChangedSignal();
}

void Empire::UpdateFoodDistribution()
{
    m_food_resource_pool.Update();

    Logger().debugStream() << "Empire::UpdateFoodDistribution for empire " << m_id;

    double available_food = GetFoodResPool().Available();
    m_food_total_distributed = 0.0;

    Logger().debugStream() << "Empire::UpdateFoodDistribution: total available_food = " << available_food;
    
    std::vector<PopCenter*> pop_centers = GetPopulationPool().PopCenters(); //GetUniverse().FindObjects(OwnedVisitor<PopCenter>(m_id));
    std::vector<PopCenter*>::iterator pop_it;
    std::vector<ResourceCenter*> resource_centers = GetFoodResPool().ResourceCenters(); //GetUniverse().FindObjects(OwnedVisitor<ResourceCenter>(m_id));
    std::vector<ResourceCenter*>::iterator res_it;

    Logger().debugStream() << "Empire::UpdateFoodDistribution: pop_centers.size() = " << pop_centers.size();

    // compile map of food production of ResourceCenters, indexed by center's id
    std::map<int, double> fp_map;
    std::map<int, double>::iterator fp_map_it;    
    for (res_it = resource_centers.begin(); res_it != resource_centers.end(); ++res_it)
    {
        ResourceCenter *center = *res_it;
        UniverseObject *obj = dynamic_cast<UniverseObject*>(center);    // can't use universe_object_cast<UniverseObject*> because ResourceCenter is not derived from UniverseObject
        if (!obj) continue; // apparently wasn't a valid object... might want to throw an error in this situation instead...
        fp_map[obj->ID()] = center->FarmingPoints();
    }

    // first pass: give food to PopCenters that produce food, limited by their food need and their food production
    for (pop_it = pop_centers.begin(); pop_it != pop_centers.end() && available_food > 0.0; ++pop_it)
    {
        PopCenter *center = *pop_it;
        double need = center->PopPoints();  // basic need is current population - prevents starvation

        Logger().debugStream() << "Empire::UpdateFoodDistribution: PopCenter needs: " << need;

        UniverseObject *obj = dynamic_cast<UniverseObject*>(center);    // can't use universe_object_cast<UniverseObject*> because ResourceCenter is not derived from UniverseObject
        if (!obj)
        {
            // apparently wasn't a valid object... might want to throw an error in this situation instead...
            center->SetAvailableFood(0.0);
            continue; 
        }
        
        // determine if, and if so how much, food this center produces locally
        double food_prod = 0.0;
        fp_map_it = fp_map.find(obj->ID());
        if (fp_map_it != fp_map.end())
            food_prod = fp_map_it->second;

        Logger().debugStream() << "Empire::UpdateFoodDistribution: PopCenter produces: " << food_prod;

        // allocate food to this PopCenter, deduct from pool, add to total food distribution tally
        double allocation = std::min(available_food, std::min(need, food_prod));

        Logger().debugStream() << "Empire::UpdateFoodDistribution: PopCenter allocated: " << allocation;

        center->SetAvailableFood(allocation);
        m_food_total_distributed += allocation;
        available_food -= allocation;
    }

    Logger().debugStream() << "Empire::UpdateFoodDistribution: m_food_total_distributed: " << m_food_total_distributed;

    // second pass: give food to PopCenters limited by their food need only: prevent starvation if possible
    for (pop_it = pop_centers.begin(); pop_it != pop_centers.end() && available_food > 0.0; ++pop_it)
    {
        PopCenter *center = *pop_it;
        double need = center->PopPoints();
        double has = center->AvailableFood();
        double addition = std::min(need - has, available_food);

        center->SetAvailableFood(center->AvailableFood() + addition);
        available_food -= addition;
        m_food_total_distributed += addition;
    }

    /* third pass: give food to PopCenters limited by their twice their basic food need (the most a planet
       can consume on one turn) or their local production if it is less than twice the basic need, but more
       than they already have.  (Don't take any food away if production is less than already allocated.) */
    for (pop_it = pop_centers.begin(); pop_it != pop_centers.end() && available_food > 0.0; ++pop_it)
    {
        PopCenter *center = *pop_it;
        double basic_need = center->PopPoints();
        double full_need = 2*basic_need;
        double has = center->AvailableFood();

        UniverseObject *obj = dynamic_cast<UniverseObject*>(center);

        double food_prod = 0.0;
        fp_map_it = fp_map.find(obj->ID());
        if (fp_map_it != fp_map.end())
            food_prod = fp_map_it->second;

        double addition = 0.0;
        if (food_prod > has)
            addition = std::min(available_food, std::min(full_need - has, food_prod - has));

        center->SetAvailableFood(has + addition);
        available_food -= addition;
        m_food_total_distributed += addition;
    }

    // fourth pass: give food to PopCenters limited by twice their food need only: allow full growth rate    
    for (pop_it = pop_centers.begin(); pop_it != pop_centers.end() && available_food > 0.0; ++pop_it)
    {
        PopCenter *center = *pop_it;
        double basic_need = center->PopPoints();
        double full_need = 2*basic_need;
        double has = center->AvailableFood();
        double addition = std::min(full_need - has, available_food);

        center->SetAvailableFood(has + addition);
        available_food -= addition;
        m_food_total_distributed += addition;
    }
    // after changing food distribution, population growth predictions may need to be redone
    // by calling UpdatePopulationGrowth()  

    m_food_resource_pool.ChangedSignal();
}

/** Has m_population_pool recalculate all PopCenters' and empire's total expected population growth
  * Assumes UpdateFoodDistribution() has been called to determine food allocations to each planet (which
  * are a factor in the growth prediction calculation).
  */
void Empire::UpdatePopulationGrowth()
{
    m_population_pool.Update();
}