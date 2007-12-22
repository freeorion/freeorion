#include "ShipDesign.h"

#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../universe/ParserUtil.h"
#include "../util/OptionsDB.h"

#include <fstream>

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

namespace {
    struct store_part_type_impl {
        template <class T1, class T2>
        struct result {typedef void type;};
        template <class T>
        void operator()(std::map<std::string, PartType*>& part_types, const T& part_type) const {
            if (part_types.find(part_type->Name()) != part_types.end()) {
                std::string error_str = "ERROR: More than one ship part in ship_parts.txt has the name " + part_type->Name();
                throw std::runtime_error(error_str.c_str());
            }
            part_types[part_type->Name()] = part_type;
        }
    };

    const phoenix::function<store_part_type_impl> store_part_type_;
    
    struct store_hull_type_impl {
        template <class T1, class T2>
        struct result {typedef void type;};
        template <class T>
        void operator()(std::map<std::string, HullType*>& hull_types, const T& hull_type) const {
            if (hull_types.find(hull_type->Name()) != hull_types.end()) {
                std::string error_str = "ERROR: More than one ship hull in ship_hulls.txt has the name " + hull_type->Name();
                throw std::runtime_error(error_str.c_str());
            }
            hull_types[hull_type->Name()] = hull_type;
        }
    };

    const phoenix::function<store_hull_type_impl> store_hull_type_;
}

////////////////////////////////////////////////
// Free Functions                             //
////////////////////////////////////////////////
const PartTypeManager& GetPartTypeManager() {
    return PartTypeManager::GetPartTypeManager();
}

const PartType* GetPartType(const std::string& name) {
    return GetPartTypeManager().GetPartType(name);
}

const HullTypeManager& GetHullTypeManager() {
    return HullTypeManager::GetHullTypeManager();
}

const HullType* GetHullType(const std::string& name) {
    return GetHullTypeManager().GetHullType(name);
}

const ShipDesign* GetShipDesign(int ship_design_id)
{
    return GetUniverse().GetShipDesign(ship_design_id);
}


/////////////////////////////////////
// PartTypeManager                 //
/////////////////////////////////////
// static
PartTypeManager* PartTypeManager::s_instance = 0;

PartTypeManager::PartTypeManager() {
    if (s_instance)
        throw std::runtime_error("Attempted to create more than one PartTypeManager.");
    s_instance = this;

    std::string settings_dir = GetOptionsDB().Get<std::string>("settings-dir");
    if (!settings_dir.empty() && settings_dir[settings_dir.size() - 1] != '/')
        settings_dir += '/';
    std::string filename = settings_dir + "ship_parts.txt";
    std::ifstream ifs(filename.c_str());
    
    std::string input;
    std::getline(ifs, input, '\0');
    ifs.close();
    using namespace boost::spirit;
    using namespace phoenix;
    parse_info<const char*> result =
        parse(input.c_str(),
              as_lower_d[*part_p[store_part_type_(var(m_parts), arg1)]],
              skip_p);
    if (!result.full)
        ReportError(std::cerr, input.c_str(), result);

    std::string cycle_str = FindFirstDependencyCycle();
    if (!cycle_str.empty())
        throw std::runtime_error(cycle_str.c_str());
}

const PartType* PartTypeManager::GetPartType(const std::string& name) const {
    std::map<std::string, PartType*>::const_iterator it = m_parts.find(name);
    return it != m_parts.end() ? it->second : 0;
}

std::string PartTypeManager::FindFirstDependencyCycle() {
    return "";

    //assert(!m_parts.empty());

    //std::set<std::string> checked_parts; // the list of parts that are not part of any cycle
    //for (std::map<std::string, PartType*>::iterator it = m_parts.begin(); it != m_parts.end(); ++it) {
    //    if (checked_parts.find(it->first) != checked_parts.end())
    //        continue;   // part already checked, so skip

    //    std::vector<std::string> stack;
    //    stack.push_back(*it);
    //    while (!stack.empty()) {
    //        // Examine the part on top of the stack.  If the part has no upgrades, or if its
    //        // upgrade has already been checked, pop it off the stack and mark it as checked;
    //        // otherwise, push its unchecked upgrade onto the stack.
    //        const PartType* current_part = stack.back();
    //        unsigned int starting_stack_size = stack.size();
    //        const std::string upgrade_name = current_part->Upgrade();

    //        if (upgrade_name != "") {
    //            const PartType* upgrade = GetPartType(upgrade_name);
    //            if (checked_parts.find(upgrade_name) == checked_parts.end()) {
    //                // since this is not a checked upgrade, see if it is already in the stack somewhere; if so, we have a cycle
    //                std::vector<const PartType*>::reverse_iterator stack_duplicate_it =
    //                    std::find(stack.rbegin(), stack.rend(), upgrade);
    //                if (stack_duplicate_it != stack.rend()) {
    //                    std::stringstream stream;
    //                    std::string upgrade_name = upgrade->Name();
    //                    stream << "ERROR: Ship PartType dependency cycle found in ship_parts.txt (A <-- B means A is an upgrade of B): \""
    //                           << upgrade_name << "\"";
    //                    for (std::vector<const Tech*>::reverse_iterator stack_it = stack.rbegin();
    //                         stack_it != stack_duplicate_it;
    //                         ++stack_it) {
    //                        if ((*stack_it)->Upgrade() != "") {
    //                            upgrade_name = (*stack_it)->Name();
    //                            stream << " <-- \"" << upgrade_name << "\"";
    //                        }
    //                    }
    //                    stream << " <-- \"" << upgrade->Name() << "\" ... ";
    //                    return stream.str();
    //                } else {
    //                    stack.push_back(upgrade);
    //                }
    //            }
    //        }
    //        if (starting_stack_size == stack.size()) {
    //            stack.pop_back();
    //            checked_parts.insert(upgrade);
    //        }
    //    }
    //}
    //return "";
}

const PartTypeManager& PartTypeManager::GetPartTypeManager() {
    static PartTypeManager manager;
    return manager;
}

PartTypeManager::iterator PartTypeManager::begin() const {
    return m_parts.begin();
}

PartTypeManager::iterator PartTypeManager::end() const {
    return m_parts.end();
}


////////////////////////////////////////////////
// PartType
////////////////////////////////////////////////
PartType::PartType() :
    m_name("invalid part type"),
    m_description("indescribable"),
    m_class(INVALID_SHIP_PART_CLASS),
    m_upgrade(""),
    m_mass(1.0),
    m_power(1.0),
    m_range(1.0),
    m_effects(),
    m_graphic(""),
    m_battle_animation("")
{}

PartType::PartType(std::string name, std::string description, ShipPartClass part_class, std::string upgrade,
                   double mass, double power, double range, /*std::vector<boost::shared_ptr<const Effect::EffectsGroup> > effects, */
                   std::string graphic) :
    m_name(name),
    m_description(description),
    m_class(part_class),
    m_upgrade(upgrade),
    m_mass(mass),
    m_power(power),
    m_range(range),
    m_effects(),
    m_graphic(graphic),
    m_battle_animation("")
{}

std::string PartType::Name() const {
    return m_name;
}
    
std::string PartType::Upgrade() const {
    return m_upgrade;
}


////////////////////////////////////////////////
// HullType
////////////////////////////////////////////////
HullType::HullType() :
    m_name("generic hull type"),
    m_description("indescribable"),
    m_mass(1.0),
    m_speed(1.0),
    m_number_slots(5),
    m_effects(),
    m_graphic("")
{}

HullType::HullType(std::string name, std::string description, double mass, double speed, int num_slots,
                   std::string graphic) :
    m_name(name),
    m_description(description),
    m_mass(mass),
    m_speed(speed),
    m_number_slots(num_slots),
    m_effects(),
    m_graphic(graphic)
{}

std::string HullType::Name() const {
    return m_name;
}


/////////////////////////////////////
// HullTypeManager                 //
/////////////////////////////////////
// static
HullTypeManager* HullTypeManager::s_instance = 0;

HullTypeManager::HullTypeManager() {
    if (s_instance)
        throw std::runtime_error("Attempted to create more than one HullTypeManager.");
    s_instance = this;

    std::string settings_dir = GetOptionsDB().Get<std::string>("settings-dir");
    if (!settings_dir.empty() && settings_dir[settings_dir.size() - 1] != '/')
        settings_dir += '/';
    std::string filename = settings_dir + "ship_hulls.txt";
    std::ifstream ifs(filename.c_str());
    
    std::string input;
    std::getline(ifs, input, '\0');
    ifs.close();
    using namespace boost::spirit;
    using namespace phoenix;
    parse_info<const char*> result =
        parse(input.c_str(),
              as_lower_d[*hull_p[store_hull_type_(var(m_hulls), arg1)]],
              skip_p);
    if (!result.full)
        ReportError(std::cerr, input.c_str(), result);
}

const HullType* HullTypeManager::GetHullType(const std::string& name) const {
    std::map<std::string, HullType*>::const_iterator it = m_hulls.find(name);
    return it != m_hulls.end() ? it->second : 0;
}

const HullTypeManager& HullTypeManager::GetHullTypeManager()
{
    static HullTypeManager manager;
    return manager;
}

HullTypeManager::iterator HullTypeManager::begin() const {
    return m_hulls.begin();
}

HullTypeManager::iterator HullTypeManager::end() const {
    return m_hulls.end();
}


////////////////////////////////////////////////
// ShipDesign
////////////////////////////////////////////////
ShipDesign::ShipDesign() :
    m_id(UniverseObject::INVALID_OBJECT_ID),
    m_name(""),
    m_designed_by_empire_id(-1),
    m_designed_on_turn(UniverseObject::INVALID_OBJECT_AGE),
    m_hull(""),
    m_parts(),
    m_graphic(""),
    m_3D_model("")
{}

ShipDesign::ShipDesign(std::string name, int designed_by_empire_id, int designed_on_turn, 
                       std::string hull, std::vector<std::string> parts, std::string graphic,
                       std::string model) :
    m_id(UniverseObject::INVALID_OBJECT_ID),
    m_name(name),
    m_designed_by_empire_id(designed_by_empire_id),
    m_designed_on_turn(designed_on_turn),
    m_hull(hull),
    m_parts(parts),
    m_graphic(graphic),
    m_3D_model(model)
{}

std::string ShipDesign::Name() const
{
    return m_name;
}
    
int ShipDesign::DesignedByEmpire() const
{
    return m_designed_by_empire_id;
}

void ShipDesign::SetID(int id)
{
    m_id = id;
}

void ShipDesign::Rename(const std::string& name)
{
    m_name = name;
}

std::string ShipDesign::Graphic() const
{
    if (m_name == "Scout")
        return "misc/scout1.png";
    if (m_name == "Colony Ship")
        return "misc/colony1.png";
    if (m_name == "Mark I")
        return "misc/mark1.png";
    if (m_name == "Mark II")
        return "misc/mark2.png";
    if (m_name == "Mark III")
        return "misc/mark3.png";
    if (m_name == "Mark IV")
        return "misc/mark4.png";
    return "";
}

std::string ShipDesign::Description() const
{
    if (m_name == "Scout")
        return "Small and cheap unarmed vessel designed for recon and exploration.";
    if (m_name == "Colony Ship")
        return "Huge unarmed vessel capable of delivering millions of citizens safely to new colony sites.";
    if (m_name == "Mark I")
        return "Affordable armed patrol frigate.";
    if (m_name == "Mark II")
        return "Cruiser with storng defensive and offensive capabilities.";
    if (m_name == "Mark III")
        return "Advanced cruiser with heavy weaponry and armor to do the dirty work.";
    if (m_name == "Mark IV")
        return "Massive state-of-art warship armed and protected with the latest technolgy. Priced accordingly.";
    return "A nonspecific ship";
}

double ShipDesign::Defense() const
{
    if (m_name == "Mark I")
        return 1.0;
    if (m_name == "Mark II")
        return 2.0;
    if (m_name == "Mark III")
        return 3.0;
    if (m_name == "Mark IV")
        return 5.0;
    return 1.0;
}

double ShipDesign::Speed() const
{
    if (m_name == "Scout")
        return 80.0;
    if (m_name == "Colony Ship")
        return 50.0;
    if (m_name == "Mark I")
        return 50.0;
    if (m_name == "Mark II")
        return 40.0;
    if (m_name == "Mark III")
        return 30.0;
    if (m_name == "Mark IV")
        return 25.0;
    return 50.0;
}

double ShipDesign::Attack() const
{
    if (m_name == "Mark I")
        return 2.0;
    if (m_name == "Mark II")
        return 5.0;
    if (m_name == "Mark III")
        return 10.0;
    if (m_name == "Mark IV")
        return 15.0;
    return 0.0;
}

bool ShipDesign::Colonize() const
{
    if (m_name == "Colony Ship")
        return true;    
    return false;
}

double ShipDesign::Cost() const
{
    if (m_name == "Scout")
        return 10.0;
    if (m_name == "Colony Ship")
        return 50.0;
    if (m_name == "Mark I")
        return 20.0;
    if (m_name == "Mark II")
        return 40.0;
    if (m_name == "Mark III")
        return 60.0;
    if (m_name == "Mark IV")
        return 80.0;
    return 0;
}
