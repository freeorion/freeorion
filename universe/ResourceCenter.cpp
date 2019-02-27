#include "ResourceCenter.h"

#include "../util/Directories.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../Empire/Empire.h"
#include "Fleet.h"
#include "Planet.h"
#include "ShipDesign.h"
#include "System.h"
#include "Building.h"
#include "Enums.h"

#include <stdexcept>

namespace {
    static const std::string EMPTY_STRING;
}

ResourceCenter::ResourceCenter() :
    m_focus(),
    m_last_turn_focus_changed(INVALID_GAME_TURN),
    m_focus_turn_initial(),
    m_last_turn_focus_changed_turn_initial(INVALID_GAME_TURN)
{}

ResourceCenter::~ResourceCenter()
{}

ResourceCenter::ResourceCenter(const ResourceCenter& rhs) :
    m_focus(rhs.m_focus),
    m_last_turn_focus_changed(rhs.m_last_turn_focus_changed),
    m_focus_turn_initial(rhs.m_focus_turn_initial),
    m_last_turn_focus_changed_turn_initial(rhs.m_last_turn_focus_changed_turn_initial)
{}

void ResourceCenter::Copy(std::shared_ptr<const ResourceCenter> copied_object, Visibility vis) {
    if (copied_object.get() == this)
        return;
    if (!copied_object) {
        ErrorLogger() << "ResourceCenter::Copy passed a null object";
        return;
    }

    if (vis >= VIS_PARTIAL_VISIBILITY) {
        this->m_focus = copied_object->m_focus;
        this->m_last_turn_focus_changed = copied_object->m_last_turn_focus_changed;
        this->m_focus_turn_initial = copied_object->m_focus_turn_initial;
        this->m_last_turn_focus_changed_turn_initial = copied_object->m_last_turn_focus_changed_turn_initial;
    }
}

void ResourceCenter::Copy(std::shared_ptr<const ResourceCenter> copied_object)
{ Copy(copied_object, VIS_FULL_VISIBILITY); }

void ResourceCenter::Init() {
    //DebugLogger() << "ResourceCenter::Init";
    AddMeter(METER_INDUSTRY);
    AddMeter(METER_RESEARCH);
    AddMeter(METER_TRADE);
    AddMeter(METER_CONSTRUCTION);
    AddMeter(METER_TARGET_INDUSTRY);
    AddMeter(METER_TARGET_RESEARCH);
    AddMeter(METER_TARGET_TRADE);
    AddMeter(METER_TARGET_CONSTRUCTION);
    m_focus.clear();
    m_last_turn_focus_changed = INVALID_GAME_TURN;
    m_focus_turn_initial.clear();
    m_last_turn_focus_changed_turn_initial = INVALID_GAME_TURN;
}

const std::string& ResourceCenter::Focus() const
{ return m_focus; }

int ResourceCenter::TurnsSinceFocusChange() const {
    if (m_last_turn_focus_changed == INVALID_GAME_TURN)
        return 0;
    int current_turn = CurrentTurn();
    if (current_turn == INVALID_GAME_TURN)
        return 0;
    return current_turn - m_last_turn_focus_changed;
}

std::vector<std::string> ResourceCenter::AvailableFoci() const
{ return std::vector<std::string>(); }

const std::string& ResourceCenter::FocusIcon(const std::string& focus_name) const
{ return EMPTY_STRING; }

std::string ResourceCenter::Dump(unsigned short ntabs) const {
    std::stringstream os;
    os << "ResourceCenter focus: " << m_focus << " last changed on turn: " << m_last_turn_focus_changed;
    return os.str();
}

void ResourceCenter::SetFocus(const std::string& focus) {
    if (focus == m_focus)
        return;
    if (focus.empty()) {
        ClearFocus();
        return;
    }
    auto avail_foci = AvailableFoci();
    auto foci_it = std::find(avail_foci.begin(), avail_foci.end(), focus);
    if (foci_it != avail_foci.end()) {
        m_focus = focus;
        if (m_focus == m_focus_turn_initial)
            m_last_turn_focus_changed = m_last_turn_focus_changed_turn_initial;
        else
            m_last_turn_focus_changed = CurrentTurn();
        ResourceCenterChangedSignal();
        return;
    }
    ErrorLogger() << "ResourceCenter::SetFocus Exploiter!-- unavailable focus " << focus << " attempted to be set for object w/ dump string: " << Dump();
}

void ResourceCenter::ClearFocus() {
    m_focus.clear();
    m_last_turn_focus_changed = CurrentTurn();
    ResourceCenterChangedSignal();
}

void ResourceCenter::UpdateFocusHistory() {
    DebugLogger() << "ResourceCenter::UpdateFocusHistory: focus: " << m_focus
                  << "  initial focus: " << m_focus_turn_initial
                  << "  turns since change initial: " << m_last_turn_focus_changed_turn_initial;
    if (m_focus != m_focus_turn_initial) {
        m_focus_turn_initial = m_focus;
        m_last_turn_focus_changed_turn_initial = m_last_turn_focus_changed;
    }
}

void ResourceCenter::ResourceCenterResetTargetMaxUnpairedMeters() {
    GetMeter(METER_TARGET_INDUSTRY)->ResetCurrent();
    GetMeter(METER_TARGET_RESEARCH)->ResetCurrent();
    GetMeter(METER_TARGET_TRADE)->ResetCurrent();
    GetMeter(METER_TARGET_CONSTRUCTION)->ResetCurrent();
}

void ResourceCenter::ResourceCenterClampMeters() {
    GetMeter(METER_TARGET_INDUSTRY)->ClampCurrentToRange();
    GetMeter(METER_TARGET_RESEARCH)->ClampCurrentToRange();
    GetMeter(METER_TARGET_TRADE)->ClampCurrentToRange();
    GetMeter(METER_TARGET_CONSTRUCTION)->ClampCurrentToRange();

    GetMeter(METER_INDUSTRY)->ClampCurrentToRange();
    GetMeter(METER_RESEARCH)->ClampCurrentToRange();
    GetMeter(METER_TRADE)->ClampCurrentToRange();
    GetMeter(METER_CONSTRUCTION)->ClampCurrentToRange();
}

void ResourceCenter::Reset() {
    m_focus.clear();
    m_last_turn_focus_changed = INVALID_GAME_TURN;

    GetMeter(METER_INDUSTRY)->Reset();
    GetMeter(METER_RESEARCH)->Reset();
    GetMeter(METER_TRADE)->Reset();
    GetMeter(METER_CONSTRUCTION)->Reset();

    GetMeter(METER_TARGET_INDUSTRY)->Reset();
    GetMeter(METER_TARGET_RESEARCH)->Reset();
    GetMeter(METER_TARGET_TRADE)->Reset();
    GetMeter(METER_TARGET_CONSTRUCTION)->Reset();
}
