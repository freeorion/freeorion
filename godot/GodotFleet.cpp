#include "GodotFleet.h"

#include "../util/AppInterface.h"
#include "../universe/Fleet.h"
#include "../universe/System.h"

using namespace godot;

void GodotFleet::_register_methods() {
    register_method("is_stationary", &GodotFleet::is_stationary);
    register_property<GodotFleet, Spatial*>("spatial",
        &GodotFleet::set_spatial,
        &GodotFleet::get_spatial,
        nullptr);
    register_property<GodotFleet, int>("id",
        nullptr,
        &GodotFleet::get_id,
        0);
    register_property<GodotFleet, Vector3>("pos",
        nullptr,
        &GodotFleet::get_pos,
        Vector3());
    register_property<GodotFleet, String>("name",
        nullptr,
        &GodotFleet::get_name,
        String());
}

GodotFleet* GodotFleet::wrap(const std::shared_ptr<Fleet>& impl) {
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

void GodotFleet::set_spatial(Spatial* spatial) {
    m_spatial = spatial;
    if (m_spatial != nullptr) {
        if (!is_stationary()) {
            int prev_id = m_impl->PreviousSystemID();
            int next_id = m_impl->NextSystemID();
            auto prev = Objects().get<System>(prev_id);
            auto next = Objects().get<System>(next_id);
            if (prev && next) {
                m_spatial->look_at_from_position(
                    Vector3(prev->X(), 0, prev->Y()),
                    Vector3(next->X(), 0, next->Y()),
                    Vector3(0, 1, 0));
            }
        }
        Vector3 translation(m_impl->X(), 0.5, m_impl->Y());
        m_spatial->set_translation(translation);
    }
}

Spatial* GodotFleet::get_spatial() const
{ return m_spatial; }

Vector3 GodotFleet::get_pos() const
{ return Vector3(m_impl->X(), 0, m_impl->Y()); }

int GodotFleet::get_id() const
{ return m_impl->ID(); }

String GodotFleet::get_name() const
{ return String(m_impl->Name().c_str()); }

bool GodotFleet::is_stationary() const {
    return (m_impl->FinalDestinationID() == INVALID_OBJECT_ID
            || m_impl->SystemID() != INVALID_OBJECT_ID);
}

