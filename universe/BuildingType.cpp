#include "BuildingType.h"

#include <boost/algorithm/string/case_conv.hpp>
#include "../util/CheckSums.h"
#include "../util/GameRules.h"
#include "../util/Logger.h"
#include "../Empire/EmpireManager.h"
#include "Condition.h"
#include "Effect.h"
#include "ValueRef.h"


namespace {
    void AddRules(GameRules& rules) {
        // makes all buildings cost 1 PP and take 1 turn to produce
        rules.Add<bool>("RULE_CHEAP_AND_FAST_BUILDING_PRODUCTION",
                        "RULE_CHEAP_AND_FAST_BUILDING_PRODUCTION_DESC",
                        "", false, true);
    }
    bool temp_bool = RegisterGameRules(&AddRules);
}


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
    m_production_meter_consumption(std::move(common_params.production_meter_consumption)),
    m_production_special_consumption(std::move(common_params.production_special_consumption)),
    m_location(std::move(common_params.location)),
    m_enqueue_location(std::move(common_params.enqueue_location)),
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
    for (auto& effect : m_effects)
        effect->SetTopLevelContent(m_name);
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
    // if rule is active, then scripted costs and times are ignored and actual costs are invariant
    if (GetGameRules().Get<bool>("RULE_CHEAP_AND_FAST_BUILDING_PRODUCTION"))
        return true;

    // if cost or time are specified and not invariant, result is non-invariance
    if (m_production_cost && !(m_production_cost->TargetInvariant() && m_production_cost->SourceInvariant()))
        return false;
    if (m_production_time && !(m_production_time->TargetInvariant() && m_production_time->SourceInvariant()))
        return false;
    // if both cost and time are not specified, result is invariance
    return true;
}

float BuildingType::ProductionCost(int empire_id, int location_id) const {
    if (GetGameRules().Get<bool>("RULE_CHEAP_AND_FAST_BUILDING_PRODUCTION") || !m_production_cost)
        return 1.0f;

    if (m_production_cost->ConstantExpr())
        return m_production_cost->Eval();
    else if (m_production_cost->SourceInvariant() && m_production_cost->TargetInvariant())
        return m_production_cost->Eval();

    const auto ARBITRARY_LARGE_COST = 999999.9f;

    auto location = Objects().get(location_id);
    if (!location && !m_production_cost->TargetInvariant())
        return ARBITRARY_LARGE_COST;

    auto source = Empires().GetSource(empire_id);
    if (!source && !m_production_cost->SourceInvariant())
        return ARBITRARY_LARGE_COST;

    ScriptingContext context(source, location);

    return m_production_cost->Eval(context);
}

float BuildingType::PerTurnCost(int empire_id, int location_id) const
{ return ProductionCost(empire_id, location_id) / std::max(1, ProductionTime(empire_id, location_id)); }

int BuildingType::ProductionTime(int empire_id, int location_id) const {
    if (GetGameRules().Get<bool>("RULE_CHEAP_AND_FAST_BUILDING_PRODUCTION") || !m_production_time)
        return 1;

    if (m_production_time->ConstantExpr())
        return m_production_time->Eval();
    else if (m_production_time->SourceInvariant() && m_production_time->TargetInvariant())
        return m_production_time->Eval();

    const int ARBITRARY_LARGE_TURNS = 9999;

    auto location = Objects().get(location_id);
    if (!location && !m_production_time->TargetInvariant())
        return ARBITRARY_LARGE_TURNS;

    auto source = Empires().GetSource(empire_id);
    if (!source && !m_production_time->SourceInvariant())
        return ARBITRARY_LARGE_TURNS;

    ScriptingContext context(source, location);

    return m_production_time->Eval(context);
}

bool BuildingType::ProductionLocation(int empire_id, int location_id) const {
    if (!m_location)
        return true;

    auto location = Objects().get(location_id);
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

    auto location = Objects().get(location_id);
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


BuildingTypeManager* BuildingTypeManager::s_instance = nullptr;

BuildingTypeManager::BuildingTypeManager() {
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


BuildingTypeManager& GetBuildingTypeManager()
{ return BuildingTypeManager::GetBuildingTypeManager(); }

const BuildingType* GetBuildingType(const std::string& name)
{ return GetBuildingTypeManager().GetBuildingType(name); }
