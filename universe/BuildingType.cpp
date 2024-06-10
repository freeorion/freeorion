#include "BuildingType.h"

#include <boost/algorithm/string/case_conv.hpp>
#include "../util/CheckSums.h"
#include "../util/GameRules.h"
#include "../util/GameRuleRanks.h"
#include "../util/Logger.h"
#include "../util/ScopedTimer.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "Condition.h"
#include "Effect.h"
#include "ValueRef.h"
#include <numeric>

#define CHECK_COND_VREF_MEMBER(m_ptr) if (m_ptr == rhs.m_ptr) {            \
                                          /* check next member */          \
                                      } else if (!m_ptr || !rhs.m_ptr) {   \
                                          return false;                    \
                                      } else if (*m_ptr != *(rhs.m_ptr)) { \
                                          return false;                    \
                                      }

namespace {
    #define UserStringNop(key) key

    void AddRules(GameRules& rules) {
        // makes all buildings cost 1 PP and take 1 turn to produce
        rules.Add<bool>(UserStringNop("RULE_CHEAP_AND_FAST_BUILDING_PRODUCTION"),
                        UserStringNop("RULE_CHEAP_AND_FAST_BUILDING_PRODUCTION_DESC"),
                        GameRuleCategories::GameRuleCategory::TEST,
                        false, true,
                        GameRuleRanks::RULE_CHEAP_AND_FAST_BUILDING_PRODUCTION_RANK);
    }
    bool temp_bool = RegisterGameRules(&AddRules);
}


BuildingType::BuildingType(std::string&& name, std::string&& description,
                           CommonParams&& common_params, CaptureResult capture_result,
                           std::string&& icon) :
    m_name(name), // intentional copy so name is usable later in member initializers
    m_description(std::move(description)),
    m_production_cost([](auto&& pc, const auto& name) {
        pc->SetTopLevelContent(name);
        return std::move(pc);
    }(std::move(common_params.production_cost), name)),
    m_production_time([](auto&& pt, const auto& name) {
        pt->SetTopLevelContent(name);
        return std::move(pt);
    }(std::move(common_params.production_time), name)),
    m_producible(common_params.producible),
    m_capture_result(capture_result),
    m_tags_concatenated([](auto& tags) {
        // ensure tags are all upper-case
        std::for_each(tags.begin(), tags.end(),
                      [](auto& t) { boost::to_upper<std::string>(t); });

        // allocate storage for concatenated tags
        std::size_t params_sz = std::transform_reduce(tags.begin(), tags.end(), 0u, std::plus{},
                                                      [](const auto& tag) { return tag.size(); });
        std::string retval;
        retval.reserve(params_sz);

        // concatenate tags
        std::for_each(tags.begin(), tags.end(), [&retval](const auto& t) { retval.append(t); });
        return retval;
    }(common_params.tags)),
    m_tags([](const auto& tags, const std::string_view tags_concatenated) {
        std::vector<std::string_view> retval;
        std::size_t next_idx = 0;
        retval.reserve(tags.size());

        // store views into concatenated tags string
        std::for_each(tags.begin(), tags.end(), [&next_idx, &retval, tags_concatenated](const auto& t) {
            retval.push_back(tags_concatenated.substr(next_idx, t.size()));
            next_idx += t.size();
        });
        return retval;
    }(common_params.tags, m_tags_concatenated)),
    m_production_meter_consumption(std::move(common_params.production_meter_consumption)),
    m_production_special_consumption(std::move(common_params.production_special_consumption)),
    m_location([](auto&& l, const auto& name) {
        l->SetTopLevelContent(name);
        return std::move(l);
    }(std::move(common_params.location), name)),
    m_enqueue_location([](auto&& el, const auto& name) {
        el->SetTopLevelContent(name);
        return std::move(el);
    }(std::move(common_params.enqueue_location), name)),
    m_effects([](auto&& effects, const auto& name) {
        std::vector<Effect::EffectsGroup> retval;
        retval.reserve(effects.size());
        for (auto& e : effects) {
            e->SetTopLevelContent(name);
            retval.push_back(std::move(*e));
        }
        return retval;
    }(std::move(common_params.effects), name)),
    m_icon(std::move(icon))
{}

BuildingType::~BuildingType() = default;

bool BuildingType::operator==(const BuildingType& rhs) const {
    if (&rhs == this)
        return true;

    if (m_name != rhs.m_name ||
        m_description != rhs.m_description ||
        m_producible != rhs.m_producible ||
        m_capture_result != rhs.m_capture_result ||
        m_tags != rhs.m_tags ||
        m_icon != rhs.m_icon)
    { return false; }

    CHECK_COND_VREF_MEMBER(m_production_cost)
    CHECK_COND_VREF_MEMBER(m_production_time)
    CHECK_COND_VREF_MEMBER(m_location)
    CHECK_COND_VREF_MEMBER(m_enqueue_location)

    if (m_effects != rhs.m_effects)
        return false;
    if (m_production_meter_consumption.size() != rhs.m_production_meter_consumption.size())
        return false;

    try {
        for (auto& [meter_type, my_refs_cond_pair] : m_production_meter_consumption) {
            auto& [my_ref, my_cond] = my_refs_cond_pair;
            const auto& rhs_refs_cond_pair{rhs.m_production_meter_consumption.at(meter_type)};
            auto& [rhs_ref, rhs_cond] = rhs_refs_cond_pair;

            if (!my_ref && !rhs_ref && !my_cond && !rhs_cond)
                continue;
            if ((my_ref && !rhs_ref) || (!my_ref && rhs_ref))
                return false;
            if (*my_ref != *rhs_ref)
                return false;
            if ((my_cond && !rhs_cond) || (!my_cond && rhs_cond))
                return false;
            if (*my_cond != *rhs_cond)
                return false;
        }
    } catch (...) {
        return false;
    }

    if (m_production_meter_consumption.size() != rhs.m_production_meter_consumption.size())
        return false;
    try {
        for (auto& [meter_type, my_refs_cond_pair] : m_production_special_consumption) {
            auto& [my_ref, my_cond] = my_refs_cond_pair;
            const auto& rhs_refs_cond_pair{rhs.m_production_special_consumption.at(meter_type)};
            auto& [rhs_ref, rhs_cond] = rhs_refs_cond_pair;

            if (!my_ref && !rhs_ref && !my_cond && !rhs_cond)
                continue;
            if ((my_ref && !rhs_ref) || (!my_ref && rhs_ref))
                return false;
            if (*my_ref != *rhs_ref)
                return false;
            if ((my_cond && !rhs_cond) || (!my_cond && rhs_cond))
                return false;
            if (*my_cond != *rhs_cond)
                return false;
        }
    } catch (...) {
        return false;
    }

    return true;
}

std::string BuildingType::Dump(uint8_t ntabs) const {
    std::string retval = DumpIndent(ntabs) + "BuildingType\n";
    retval += DumpIndent(ntabs+1) + "name = \"" + m_name + "\"\n";
    retval += DumpIndent(ntabs+1) + "description = \"" + m_description + "\"\n";
    if (m_production_cost)
        retval += DumpIndent(ntabs+1) + "buildcost = " + m_production_cost->Dump(ntabs+1) + "\n";
    if (m_production_time)
        retval += DumpIndent(ntabs+1) + "buildtime = " + m_production_time->Dump(ntabs+1) + "\n";
    retval += DumpIndent(ntabs+1) + (m_producible ? "Producible" : "Unproducible") + "\n";
    retval += DumpIndent(ntabs+1) + "captureresult = ";
    retval.append(to_string(m_capture_result)).append("\n");

    if (!m_tags.empty()) {
        if (m_tags.size() == 1) {
            retval.append(DumpIndent(ntabs+1)).append("tags = \"").append(m_tags.front()).append("\"\n");
        } else {
            retval += DumpIndent(ntabs+1) + "tags = [ ";
            for (const auto& tag : m_tags)
               retval.append("\"").append(tag).append("\" ");
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
        retval += m_effects.front().Dump(ntabs+2);
    } else {
        retval += DumpIndent(ntabs+1) + "effectsgroups = [\n";
        for (auto& effect : m_effects)
            retval += effect.Dump(ntabs+2);
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

float BuildingType::ProductionCost(int empire_id, int location_id,
                                   const ScriptingContext& context) const
{
    if (GetGameRules().Get<bool>("RULE_CHEAP_AND_FAST_BUILDING_PRODUCTION") || !m_production_cost)
        return 1.0f;

    ScopedTimer timer("BuildingType::ProductionCost: " + m_name);

    if (m_production_cost->ConstantExpr())
        return m_production_cost->Eval();
    else if (m_production_cost->SourceInvariant() && m_production_cost->TargetInvariant())
        return m_production_cost->Eval();

    const auto ARBITRARY_LARGE_COST = 999999.9f;

    auto location = context.ContextObjects().getRaw(location_id);
    if (!location && !m_production_cost->TargetInvariant())
        return ARBITRARY_LARGE_COST;

    auto empire = context.GetEmpire(empire_id);
    auto source = empire ? empire->Source(context.ContextObjects()) : nullptr;

    if (!source && !m_production_cost->SourceInvariant())
        return ARBITRARY_LARGE_COST;

    // cost uses target object to represent the location where something is
    // being produced, and target object is normally mutable, but will not
    // actually be modified by evaluating the cost ValueRef
    UniverseObject* target = const_cast<UniverseObject*>(location);
    const ScriptingContext local_context{context, ScriptingContext::Source{}, source.get(),
                                         ScriptingContext::Target{}, target,
                                         INVALID_DESIGN_ID, 1};

    return m_production_cost->Eval(local_context);
}

float BuildingType::PerTurnCost(int empire_id, int location_id, const ScriptingContext& context) const {
    return ProductionCost(empire_id, location_id, context) /
        std::max(1, ProductionTime(empire_id, location_id, context));
}

int BuildingType::ProductionTime(int empire_id, int location_id, const ScriptingContext& context) const {
    if (GetGameRules().Get<bool>("RULE_CHEAP_AND_FAST_BUILDING_PRODUCTION") || !m_production_time)
        return 1;

    ScopedTimer timer("BuildingType::ProductionTime: " + m_name, true, std::chrono::milliseconds(20));

    if (m_production_time->ConstantExpr())
        return m_production_time->Eval();
    else if (m_production_time->SourceInvariant() && m_production_time->TargetInvariant())
        return m_production_time->Eval();

    static constexpr int ARBITRARY_LARGE_TURNS = 9999;

    auto location = context.ContextObjects().getRaw(location_id);
    if (!location && !m_production_time->TargetInvariant())
        return ARBITRARY_LARGE_TURNS;

    auto empire = context.GetEmpire(empire_id);
    auto source = empire ? empire->Source(context.ContextObjects()) : nullptr;

    if (!source && !m_production_time->SourceInvariant())
        return ARBITRARY_LARGE_TURNS;

    // cost uses target object to represent the location where something is
    // being produced, and target object is normally mutable, but will not
    // actually be modified by evaluating the cost ValueRef
    UniverseObject* target = const_cast<UniverseObject*>(location);
    const ScriptingContext local_context{context, ScriptingContext::Source{}, source.get(),
                                         ScriptingContext::Target{}, target, INVALID_DESIGN_ID, 1};

    return m_production_time->Eval(local_context);
}

bool BuildingType::ProductionLocation(int empire_id, int location_id, const ScriptingContext& context) const {
    if (!m_location)
        return true;

    const auto* location = context.ContextObjects().getRaw(location_id);
    if (!location)
        return false;

    auto empire = context.GetEmpire(empire_id);
    auto source = empire ? empire->Source(context.ContextObjects()) : nullptr;
    if (!source)
        return false;

    UniverseObject* target = const_cast<UniverseObject*>(location); // ... hopefully OK, as evaluating condition should not modify the object
    const ScriptingContext local_context{context, ScriptingContext::Source{}, source.get(),
                                         ScriptingContext::Target{}, target, INVALID_DESIGN_ID, 1};

    return m_location->EvalOne(local_context, location);
}

bool BuildingType::EnqueueLocation(int empire_id, int location_id, const ScriptingContext& context) const {
    if (!m_enqueue_location)
        return true;

    auto location = context.ContextObjects().getRaw(location_id);
    if (!location)
        return false;

    auto empire = context.GetEmpire(empire_id);
    auto source = empire ? empire->Source(context.ContextObjects()) : nullptr;
    if (!source)
        return false;

    UniverseObject* target = const_cast<UniverseObject*>(location); // ... hopefully OK, as evaluating condition should not modify the object
    const ScriptingContext local_context{context, ScriptingContext::Source{}, source.get(),
                                         ScriptingContext::Target{}, target, INVALID_DESIGN_ID, 1};

    return m_enqueue_location->EvalOne(local_context, location);
}

uint32_t BuildingType::GetCheckSum() const {
    uint32_t retval{0};

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

const BuildingType* BuildingTypeManager::GetBuildingType(std::string_view name) const {
    CheckPendingBuildingTypes();
    const auto it = m_building_types.find(name);
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

uint32_t BuildingTypeManager::GetCheckSum() const {
    CheckPendingBuildingTypes();
    uint32_t retval{0};
    for (auto const& name_type_pair : m_building_types)
        CheckSums::CheckSumCombine(retval, name_type_pair);
    CheckSums::CheckSumCombine(retval, m_building_types.size());


    DebugLogger() << "BuildingTypeManager checksum: " << retval;
    return retval;
}

void BuildingTypeManager::SetBuildingTypes(Pending::Pending<container_type>&& pending_building_types)
{ m_pending_building_types = std::move(pending_building_types); }

void BuildingTypeManager::CheckPendingBuildingTypes() const
{ Pending::SwapPending(m_pending_building_types, m_building_types); }

BuildingTypeManager& GetBuildingTypeManager()
{ return BuildingTypeManager::GetBuildingTypeManager(); }

const BuildingType* GetBuildingType(std::string_view name)
{ return GetBuildingTypeManager().GetBuildingType(name); }
