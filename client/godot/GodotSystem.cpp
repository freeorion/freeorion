#include "GodotSystem.h"

#include "../../universe/System.h"

void GodotSystem::_register_methods() {
    register_method("get_starlanes_wormholes", &GodotSystem::get_starlanes_wormholes);
    godot::register_property<GodotSystem, godot::Spatial*>("spatial",
        &GodotSystem::set_spatial,
        &GodotSystem::get_spatial,
        nullptr);
    godot::register_property<GodotSystem, int>("id",
        nullptr,
        &GodotSystem::get_id,
        0);
    godot::register_property<GodotSystem, godot::Vector3>("pos",
        nullptr,
        &GodotSystem::get_pos,
        godot::Vector3());
    godot::register_property<GodotSystem, godot::String>("name",
        nullptr,
        &GodotSystem::get_name,
        godot::String());
}

GodotSystem* GodotSystem::Wrap(const std::shared_ptr<System>& impl) {
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

void GodotSystem::set_spatial(godot::Spatial* spatial)
{ m_spatial = spatial; }

godot::Spatial* GodotSystem::get_spatial() const
{ return m_spatial; }

godot::Vector3 GodotSystem::get_pos() const
{ return godot::Vector3(m_impl->X(), 0, m_impl->Y()); }

int GodotSystem::get_id() const
{ return m_impl->ID(); }

godot::String GodotSystem::get_name() const
{ return godot::String(m_impl->Name().c_str()); }

godot::Dictionary GodotSystem::get_starlanes_wormholes() const {
    godot::Dictionary starlanes_wormholes;
    for (const auto& sys : m_impl->StarlanesWormholes()) {
        starlanes_wormholes[sys.first] = sys.second;
    }
    return starlanes_wormholes;
}
