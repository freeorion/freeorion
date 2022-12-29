#include "ResourceCenter.h"

#include <stdexcept>
#include "Building.h"
#include "Fleet.h"
#include "Planet.h"
#include "System.h"
#include "../Empire/Empire.h"
#include "../util/AppInterface.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"


namespace {
    static const std::string EMPTY_STRING;
}

ResourceCenter::ResourceCenter() :
    m_last_turn_focus_changed(INVALID_GAME_TURN),
    m_last_turn_focus_changed_turn_initial(INVALID_GAME_TURN)
{}

ResourceCenter::~ResourceCenter() = default;

ResourceCenter::ResourceCenter(const ResourceCenter& rhs) :
    m_focus(rhs.m_focus),
    m_last_turn_focus_changed(rhs.m_last_turn_focus_changed),
    m_focus_turn_initial(rhs.m_focus_turn_initial),
    m_last_turn_focus_changed_turn_initial(rhs.m_last_turn_focus_changed_turn_initial)
{}

void ResourceCenter::Copy(const ResourceCenter& copied_object, Visibility vis) {
    if (&copied_object == this)
        return;

    if (vis >= Visibility::VIS_PARTIAL_VISIBILITY) {
        this->m_focus = copied_object.m_focus;
        this->m_last_turn_focus_changed = copied_object.m_last_turn_focus_changed;
        this->m_focus_turn_initial = copied_object.m_focus_turn_initial;
        this->m_last_turn_focus_changed_turn_initial = copied_object.m_last_turn_focus_changed_turn_initial;
    }
}

void ResourceCenter::Copy(const ResourceCenter& copied_object)
{ Copy(copied_object, Visibility::VIS_FULL_VISIBILITY); }

void ResourceCenter::Init() {
    //DebugLogger() << "ResourceCenter::Init";
    AddMeter(MeterType::METER_INDUSTRY);
    AddMeter(MeterType::METER_RESEARCH);
    AddMeter(MeterType::METER_INFLUENCE);
    AddMeter(MeterType::METER_CONSTRUCTION);
    AddMeter(MeterType::METER_TARGET_INDUSTRY);
    AddMeter(MeterType::METER_TARGET_RESEARCH);
    AddMeter(MeterType::METER_TARGET_INFLUENCE);
    AddMeter(MeterType::METER_TARGET_CONSTRUCTION);
    m_focus.clear();
    m_last_turn_focus_changed = INVALID_GAME_TURN;
    m_focus_turn_initial.clear();
    m_last_turn_focus_changed_turn_initial = INVALID_GAME_TURN;
}

int ResourceCenter::TurnsSinceFocusChange(int current_turn) const {
    if (m_last_turn_focus_changed == INVALID_GAME_TURN)
        return 0;
    if (current_turn == INVALID_GAME_TURN)
        return 0;
    return current_turn - m_last_turn_focus_changed;
}

const std::string& ResourceCenter::FocusIcon(std::string_view, const ScriptingContext&) const
{ return EMPTY_STRING; }

std::string ResourceCenter::Dump(uint8_t ntabs) const {
    return std::string{"ResourceCenter focus: "}.append(m_focus)
        .append(" last changed on turn: ").append(std::to_string(m_last_turn_focus_changed));
}

void ResourceCenter::SetFocus(std::string focus, const ScriptingContext& context) {
    if (focus == m_focus)
        return;
    if (focus.empty()) {
        ClearFocus(context.current_turn);
        return;
    }
    if (!FocusAvailable(focus, context)) {
        ErrorLogger() << "ResourceCenter::SetFocus Exploiter!-- unavailable focus " << focus
                      << " attempted to be set for object w/ dump string: " << Dump();
        return;
    }

    m_focus = std::move(focus);
    if (m_focus == m_focus_turn_initial)
        m_last_turn_focus_changed = m_last_turn_focus_changed_turn_initial;
    else
        m_last_turn_focus_changed = context.current_turn;
    ResourceCenterChangedSignal();
}

void ResourceCenter::ClearFocus(int current_turn) {
    m_focus.clear();
    m_last_turn_focus_changed = current_turn;
    ResourceCenterChangedSignal();
}

void ResourceCenter::UpdateFocusHistory() {
    TraceLogger() << "ResourceCenter::UpdateFocusHistory: focus: " << m_focus
                  << "  initial focus: " << m_focus_turn_initial
                  << "  turns since change initial: " << m_last_turn_focus_changed_turn_initial;
    if (m_focus != m_focus_turn_initial) {
        m_focus_turn_initial = m_focus;
        m_last_turn_focus_changed_turn_initial = m_last_turn_focus_changed;
    }
}

void ResourceCenter::ResourceCenterResetTargetMaxUnpairedMeters() {
    GetMeter(MeterType::METER_TARGET_INDUSTRY)->ResetCurrent();
    GetMeter(MeterType::METER_TARGET_RESEARCH)->ResetCurrent();
    GetMeter(MeterType::METER_TARGET_INFLUENCE)->ResetCurrent();
    GetMeter(MeterType::METER_TARGET_CONSTRUCTION)->ResetCurrent();
}

void ResourceCenter::ResourceCenterClampMeters() {
    GetMeter(MeterType::METER_TARGET_INDUSTRY)->ClampCurrentToRange();
    GetMeter(MeterType::METER_TARGET_RESEARCH)->ClampCurrentToRange();
    //GetMeter(MeterType::METER_TARGET_INFLUENCE)->ClampCurrentToRange(-Meter::LARGE_VALUE, Meter::LARGE_VALUE);
    GetMeter(MeterType::METER_TARGET_CONSTRUCTION)->ClampCurrentToRange();

    GetMeter(MeterType::METER_INDUSTRY)->ClampCurrentToRange();
    GetMeter(MeterType::METER_RESEARCH)->ClampCurrentToRange();
    //GetMeter(MeterType::METER_INFLUENCE)->ClampCurrentToRange(-Meter::LARGE_VALUE, Meter::LARGE_VALUE);
    GetMeter(MeterType::METER_CONSTRUCTION)->ClampCurrentToRange();
}

void ResourceCenter::Reset(ObjectMap&) {
    m_focus.clear();
    m_last_turn_focus_changed = INVALID_GAME_TURN;

    GetMeter(MeterType::METER_INDUSTRY)->Reset();
    GetMeter(MeterType::METER_RESEARCH)->Reset();
    GetMeter(MeterType::METER_INFLUENCE)->Reset();
    GetMeter(MeterType::METER_CONSTRUCTION)->Reset();

    GetMeter(MeterType::METER_TARGET_INDUSTRY)->Reset();
    GetMeter(MeterType::METER_TARGET_RESEARCH)->Reset();
    GetMeter(MeterType::METER_TARGET_INFLUENCE)->Reset();
    GetMeter(MeterType::METER_TARGET_CONSTRUCTION)->Reset();
}
