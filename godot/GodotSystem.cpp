#include "GodotSystem.h"

#include "../universe/System.h"

using namespace godot;

void GodotSystem::_register_methods() {
    register_method("_get_starlanes_wormholes", &GodotSystem::get_starlanes_wormholes);
    register_property<GodotSystem, Spatial*>("spatial",
        &GodotSystem::set_spatial,
        &GodotSystem::get_spatial,
        nullptr);
    register_property<GodotSystem, int>("id",
        nullptr,
        &GodotSystem::get_id,
        0);
    register_property<GodotSystem, Vector3>("pos",
        nullptr,
        &GodotSystem::get_pos,
        Vector3());
    register_property<GodotSystem, String>("name",
        nullptr,
        &GodotSystem::get_name,
        String());
}

GodotSystem* GodotSystem::wrap(const std::shared_ptr<System>& impl) {
    GodotSystem* system = GodotSystem::_new();
    system->m_impl = impl;
    return system;
}

GodotSystem::GodotSystem() {
}

GodotSystem::~GodotSystem() {
}

void GodotSystem::_init() {
}

void GodotSystem::set_spatial(Spatial* spatial)
{ m_spatial = spatial; }

Spatial* GodotSystem::get_spatial() const
{ return m_spatial; }

Vector3 GodotSystem::get_pos() const
{ return Vector3(m_impl->X(), 0, m_impl->Y()); }

int GodotSystem::get_id() const
{ return m_impl->ID(); }

String GodotSystem::get_name() const
{ return String(m_impl->Name().c_str()); }

Dictionary GodotSystem::get_starlanes_wormholes() const {
    Dictionary starlanes_wormholes;
    for (const auto& sys : m_impl->StarlanesWormholes()) {
        starlanes_wormholes[sys.first] = sys.second;
    }
    return starlanes_wormholes;
}

