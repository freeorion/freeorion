#include "Building.h"

#include "Effect.h"
#include "Condition.h"
#include "Planet.h"
#include "Predicates.h"
#include "Universe.h"
#include "Enums.h"
#include "ValueRef.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../util/OptionsDB.h"
#include "../util/Logger.h"
#include "../util/AppInterface.h"
#include "../util/GameRules.h"
#include "../util/CheckSums.h"
#include "../util/ScopedTimer.h"

#include <boost/filesystem/fstream.hpp>

namespace {
    void AddRules(GameRules& rules) {
        // makes all buildings cost 1 PP and take 1 turn to produce
        rules.Add<bool>("RULE_CHEAP_AND_FAST_BUILDING_PRODUCTION",
                        "RULE_CHEAP_AND_FAST_BUILDING_PRODUCTION_DESC",
                        "", false, true);
    }
    bool temp_bool = RegisterGameRules(&AddRules);
}

/////////////////////////////////////////////////
// Building                                    //
/////////////////////////////////////////////////
Building::Building(int empire_id, const std::string& building_type,
                   int produced_by_empire_id/* = ALL_EMPIRES*/) :
    m_building_type(building_type),
    m_produced_by_empire_id(produced_by_empire_id)
{
    UniverseObject::SetOwner(empire_id);
    const BuildingType* type = GetBuildingType(m_building_type);
    if (type)
        Rename(UserString(type->Name()));
    else
        Rename(UserString("ENC_BUILDING"));

    UniverseObject::Init();
}

Building* Building::Clone(int empire_id) const {
    Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(this->ID(), empire_id);

    if (!(vis >= VIS_BASIC_VISIBILITY && vis <= VIS_FULL_VISIBILITY))
        return nullptr;

    Building* retval = new Building();
    retval->Copy(shared_from_this(), empire_id);
    return retval;
}

void Building::Copy(std::shared_ptr<const UniverseObject> copied_object, int empire_id) {
    if (copied_object.get() == this)
        return;
    auto copied_building = std::dynamic_pointer_cast<const Building>(copied_object);
    if (!copied_building) {
        ErrorLogger() << "Building::Copy passed an object that wasn't a Building";
        return;
    }

    int copied_object_id = copied_object->ID();
    Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(copied_object_id, empire_id);
    auto visible_specials = GetUniverse().GetObjectVisibleSpecialsByEmpire(copied_object_id, empire_id);

    UniverseObject::Copy(copied_object, vis, visible_specials);

    if (vis >= VIS_BASIC_VISIBILITY) {
        this->m_planet_id =                 copied_building->m_planet_id;

        if (vis >= VIS_PARTIAL_VISIBILITY) {
            this->m_name =                      copied_building->m_name;

            this->m_building_type =             copied_building->m_building_type;
            this->m_produced_by_empire_id = copied_building->m_produced_by_empire_id;

            if (vis >= VIS_FULL_VISIBILITY) {
                this->m_ordered_scrapped =      copied_building->m_ordered_scrapped;
            }
        }
    }
}

bool Building::HostileToEmpire(int empire_id) const {
    if (OwnedBy(empire_id))
        return false;
    return empire_id == ALL_EMPIRES || Unowned() ||
           Empires().GetDiplomaticStatus(Owner(), empire_id) == DIPLO_WAR;
}

std::set<std::string> Building::Tags() const {
    const BuildingType* type = ::GetBuildingType(m_building_type);
    if (!type)
        return std::set<std::string>();
    return type->Tags();
}

bool Building::HasTag(const std::string& name) const {
    const BuildingType* type = GetBuildingType(m_building_type);

    return type && type->Tags().count(name);
}

UniverseObjectType Building::ObjectType() const
{ return OBJ_BUILDING; }

bool Building::ContainedBy(int object_id) const {
    return object_id != INVALID_OBJECT_ID
        && (    object_id == m_planet_id
            ||  object_id == this->SystemID());
}

std::string Building::Dump(unsigned short ntabs) const {
    std::stringstream os;
    os << UniverseObject::Dump(ntabs);
    os << " building type: " << m_building_type
       << " produced by empire id: " << m_produced_by_empire_id;
    return os.str();
}

std::shared_ptr<UniverseObject> Building::Accept(const UniverseObjectVisitor& visitor) const
{ return visitor.Visit(std::const_pointer_cast<Building>(std::static_pointer_cast<const Building>(shared_from_this()))); }

void Building::SetPlanetID(int planet_id) {
    if (planet_id != m_planet_id) {
        m_planet_id = planet_id;
        StateChangedSignal();
    }
}

void Building::ResetTargetMaxUnpairedMeters() {
    UniverseObject::ResetTargetMaxUnpairedMeters();

    //// give buildings base stealth slightly above 0, so that they can't be seen from a distance without high detection ability
    //if (Meter* stealth = GetMeter(METER_STEALTH))
    //    stealth->AddToCurrent(0.01f);
}

void Building::Reset() {
    UniverseObject::SetOwner(ALL_EMPIRES);
    m_ordered_scrapped = false;
}

void Building::SetOrderedScrapped(bool b) {
    bool initial_status = m_ordered_scrapped;
    if (b == initial_status) return;
    m_ordered_scrapped = b;
    StateChangedSignal();
}

/////////////////////////////////////////////////
// BuildingType                                //
/////////////////////////////////////////////////
BuildingType::BuildingType(const std::string& name,
                           const std::string& description,
                           CommonParams& common_params,
                           CaptureResult capture_result,
                           const std::string& icon) :
    m_name(name),
    m_description(description),
    m_production_cost(std::move(common_params.production_cost)),
    m_production_time(std::move(common_params.production_time)),
    m_producible(common_params.producible),
    m_capture_result(capture_result),
    m_tags(),
    m_production_meter_consumption(std::move(common_params.production_meter_consumption)),
    m_production_special_consumption(std::move(common_params.production_special_consumption)),
    m_location(std::move(common_params.location)),
    m_enqueue_location(std::move(common_params.enqueue_location)),
    m_effects(),
    m_icon(icon)
{
    for (auto&& effect : common_params.effects)
        m_effects.emplace_back(std::move(effect));
    Init();
    for (const std::string& tag : common_params.tags)
        m_tags.insert(boost::to_upper_copy<std::string>(tag));
}

BuildingType::~BuildingType()
{}

void BuildingType::Init() {
    if (m_production_cost)
        m_production_cost->SetTopLevelContent(m_name);
    if (m_production_time)
        m_production_time->SetTopLevelContent(m_name);
    if (m_location)
        m_location->SetTopLevelContent(m_name);
    if (m_enqueue_location)
        m_enqueue_location->SetTopLevelContent(m_name);
    for (auto& effect : m_effects) {
        effect->SetTopLevelContent(m_name);
    }
}

std::string BuildingType::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "BuildingType\n";
    retval += DumpIndent(ntabs+1) + "name = \"" + m_name + "\"\n";
    retval += DumpIndent(ntabs+1) + "description = \"" + m_description + "\"\n";
    if (m_production_cost)
        retval += DumpIndent(ntabs+1) + "buildcost = " + m_production_cost->Dump(ntabs+1) + "\n";
    if (m_production_time)
        retval += DumpIndent(ntabs+1) + "buildtime = " + m_production_time->Dump(ntabs+1) + "\n";
    retval += DumpIndent(ntabs+1) + (m_producible ? "Producible" : "Unproducible") + "\n";
    retval += DumpIndent(ntabs+1) + "captureresult = " +
        boost::lexical_cast<std::string>(m_capture_result) + "\n";

    if (!m_tags.empty()) {
        if (m_tags.size() == 1) {
            retval += DumpIndent(ntabs+1) + "tags = \"" + *m_tags.begin() + "\"\n";
        } else {
            retval += DumpIndent(ntabs+1) + "tags = [ ";
            for (const std::string& tag : m_tags)
               retval += "\"" + tag + "\" ";
            retval += " ]\n";
        }
    }

    if (m_location) {
        retval += DumpIndent(ntabs+1) + "location = \n";
        retval += m_location->Dump(ntabs+2);
    }
    if (m_enqueue_location) {
        retval += DumpIndent(ntabs+1) + "enqueue location = \n";
        retval += m_enqueue_location->Dump(ntabs+2);
    }

    if (m_effects.size() == 1) {
        retval += DumpIndent(ntabs+1) + "effectsgroups =\n";
        retval += m_effects[0]->Dump(ntabs+2);
    } else {
        retval += DumpIndent(ntabs+1) + "effectsgroups = [\n";
        for (auto& effect : m_effects)
            retval += effect->Dump(ntabs+2);
        retval += DumpIndent(ntabs+1) + "]\n";
    }
    retval += DumpIndent(ntabs+1) + "icon = \"" + m_icon + "\"\n";
    return retval;
}

bool BuildingType::ProductionCostTimeLocationInvariant() const {
    if (GetGameRules().Get<bool>("RULE_CHEAP_AND_FAST_BUILDING_PRODUCTION"))
        return true;
    if (m_production_cost && !(m_production_cost->TargetInvariant() && m_production_cost->SourceInvariant()))
        return false;
    if (m_production_time && !(m_production_time->TargetInvariant() && m_production_time->SourceInvariant()))
        return false;
    return true;
}

float BuildingType::ProductionCost(int empire_id, int location_id) const {
    if (GetGameRules().Get<bool>("RULE_CHEAP_AND_FAST_BUILDING_PRODUCTION") || !m_production_cost) {
        return 1.0f;
    } else {
        if (m_production_cost && m_production_cost->ConstantExpr())
            return m_production_cost->Eval();
        else if (m_production_cost->SourceInvariant() && m_production_cost->TargetInvariant())
            return m_production_cost->Eval();

        const auto arbitrary_large_number = 999999.9f;

        auto location = GetUniverseObject(location_id);
        if (!location && !m_production_cost->TargetInvariant())
            return arbitrary_large_number;

        auto source = Empires().GetSource(empire_id);
        if (!source && !m_production_cost->SourceInvariant())
            return arbitrary_large_number;

        ScriptingContext context(source, location);

        return m_production_cost->Eval(context);
    }
}

float BuildingType::PerTurnCost(int empire_id, int location_id) const
{ return ProductionCost(empire_id, location_id) / std::max(1, ProductionTime(empire_id, location_id)); }

int BuildingType::ProductionTime(int empire_id, int location_id) const {
    if (GetGameRules().Get<bool>("RULE_CHEAP_AND_FAST_BUILDING_PRODUCTION") || !m_production_time) {
        return 1;
    } else {
        if (m_production_time && m_production_time->ConstantExpr())
            return m_production_time->Eval();
        else if (m_production_time->SourceInvariant() && m_production_time->TargetInvariant())
            return m_production_time->Eval();

        const auto arbitrary_large_number = 9999;

        auto location = GetUniverseObject(location_id);
        if (!location && !m_production_time->TargetInvariant())
            return arbitrary_large_number;

        auto source = Empires().GetSource(empire_id);
        if (!source && !m_production_time->SourceInvariant())
            return arbitrary_large_number;

        ScriptingContext context(source, location);

        return m_production_time->Eval(context);
    }
}

bool BuildingType::ProductionLocation(int empire_id, int location_id) const {
    if (!m_location)
        return true;

    auto location = GetUniverseObject(location_id);
    if (!location)
        return false;

    auto source = Empires().GetSource(empire_id);
    if (!source)
        return false;

    return m_location->Eval(ScriptingContext(source), location);
}

bool BuildingType::EnqueueLocation(int empire_id, int location_id) const {
    if (!m_enqueue_location)
        return true;

    auto location = GetUniverseObject(location_id);
    if (!location)
        return false;

    auto source = Empires().GetSource(empire_id);
    if (!source)
        return false;

    return m_enqueue_location->Eval(ScriptingContext(source), location);
}

unsigned int BuildingType::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, m_name);
    CheckSums::CheckSumCombine(retval, m_description);
    CheckSums::CheckSumCombine(retval, m_production_cost);
    CheckSums::CheckSumCombine(retval, m_production_time);
    CheckSums::CheckSumCombine(retval, m_producible);
    CheckSums::CheckSumCombine(retval, m_capture_result);
    CheckSums::CheckSumCombine(retval, m_tags);
    CheckSums::CheckSumCombine(retval, m_production_meter_consumption);
    CheckSums::CheckSumCombine(retval, m_production_special_consumption);
    CheckSums::CheckSumCombine(retval, m_location);
    CheckSums::CheckSumCombine(retval, m_enqueue_location);
    CheckSums::CheckSumCombine(retval, m_effects);
    CheckSums::CheckSumCombine(retval, m_icon);

    return retval;
}

/////////////////////////////////////////////////
// BuildingTypeManager                         //
/////////////////////////////////////////////////
// static(s)
BuildingTypeManager* BuildingTypeManager::s_instance = nullptr;

BuildingTypeManager::BuildingTypeManager() :
    m_building_types()
{
    if (s_instance)
        throw std::runtime_error("Attempted to create more than one BuildingTypeManager.");

    // Only update the global pointer on sucessful construction.
    s_instance = this;
}

const BuildingType* BuildingTypeManager::GetBuildingType(const std::string& name) const {
    CheckPendingBuildingTypes();
    const auto& it = m_building_types.find(name);
    return it != m_building_types.end() ? it->second.get() : nullptr;
}

BuildingTypeManager::iterator BuildingTypeManager::begin() const {
    CheckPendingBuildingTypes();
    return m_building_types.begin();
}

BuildingTypeManager::iterator BuildingTypeManager::end() const {
    CheckPendingBuildingTypes();
    return m_building_types.end();
}

BuildingTypeManager& BuildingTypeManager::GetBuildingTypeManager() {
    static BuildingTypeManager manager;
    return manager;
}

unsigned int BuildingTypeManager::GetCheckSum() const {
    CheckPendingBuildingTypes();
    unsigned int retval{0};
    for (auto const& name_type_pair : m_building_types)
        CheckSums::CheckSumCombine(retval, name_type_pair);
    CheckSums::CheckSumCombine(retval, m_building_types.size());


    DebugLogger() << "BuildingTypeManager checksum: " << retval;
    return retval;
}

void BuildingTypeManager::SetBuildingTypes(Pending::Pending<BuildingTypeMap>&& pending_building_types)
{ m_pending_building_types = std::move(pending_building_types); }

void BuildingTypeManager::CheckPendingBuildingTypes() const
{ Pending::SwapPending(m_pending_building_types, m_building_types); }

///////////////////////////////////////////////////////////
// Free Functions                                        //
///////////////////////////////////////////////////////////
BuildingTypeManager& GetBuildingTypeManager()
{ return BuildingTypeManager::GetBuildingTypeManager(); }

const BuildingType* GetBuildingType(const std::string& name)
{ return GetBuildingTypeManager().GetBuildingType(name); }
