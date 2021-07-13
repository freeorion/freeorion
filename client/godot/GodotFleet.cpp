#include "GodotFleet.h"

#include "../../util/AppInterface.h"
#include "../../universe/Fleet.h"
#include "../../universe/System.h"

void GodotFleet::_register_methods() {
    register_method("is_stationary", &GodotFleet::is_stationary);
    godot::register_property<GodotFleet, godot::Spatial*>("spatial",
        &GodotFleet::set_spatial,
        &GodotFleet::get_spatial,
        nullptr);
    godot::register_property<GodotFleet, int>("id",
        nullptr,
        &GodotFleet::get_id,
        0);
    godot::register_property<GodotFleet, godot::Vector3>("pos",
        nullptr,
        &GodotFleet::get_pos,
        godot::Vector3());
    godot::register_property<GodotFleet, godot::String>("name",
        nullptr,
        &GodotFleet::get_name,
        godot::String());
}

GodotFleet* GodotFleet::Wrap(const std::shared_ptr<Fleet>& impl) {
    GodotFleet* fleet = GodotFleet::_new();
    fleet->m_impl = impl;
    return fleet;
}

GodotFleet::GodotFleet() {
}

GodotFleet::~GodotFleet() {
}

void GodotFleet::_init() {
}

void GodotFleet::set_spatial(godot::Spatial* spatial) {
    m_spatial = spatial;
    if (m_spatial != nullptr) {
        if (!is_stationary()) {
            int prev_id = m_impl->PreviousSystemID();
            int next_id = m_impl->NextSystemID();
            auto prev = Objects().get<System>(prev_id);
            auto next = Objects().get<System>(next_id);
            if (prev && next) {
                m_spatial->look_at_from_position(
                    godot::Vector3(prev->X(), 0, prev->Y()),
                    godot::Vector3(next->X(), 0, next->Y()),
                    godot::Vector3(0, 1, 0));
            }
        }
        godot::Vector3 translation(m_impl->X(), 0.5, m_impl->Y());
        m_spatial->set_translation(translation);
    }
}

godot::Spatial* GodotFleet::get_spatial() const
{ return m_spatial; }

godot::Vector3 GodotFleet::get_pos() const
{ return godot::Vector3(m_impl->X(), 0, m_impl->Y()); }

int GodotFleet::get_id() const
{ return m_impl->ID(); }

godot::String GodotFleet::get_name() const
{ return godot::String(m_impl->Name().c_str()); }

bool GodotFleet::is_stationary() const {
    return (m_impl->FinalDestinationID() == INVALID_OBJECT_ID
            || m_impl->SystemID() != INVALID_OBJECT_ID);
}
