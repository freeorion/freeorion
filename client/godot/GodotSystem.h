#ifndef _GodotSystem_h_
#define _GodotSystem_h_

#include <Godot.hpp>
#include <Object.hpp>
#include <String.hpp>
#include <Spatial.hpp>
#include <memory>

class System;

class GodotSystem : public godot::Object {
    GODOT_CLASS(GodotSystem, godot::Object)

public:
    static void _register_methods();

    static GodotSystem* Wrap(const std::shared_ptr<System>& impl);

    GodotSystem();
    ~GodotSystem();

    void _init(); // our initializer called by Godot
       
private:
    std::shared_ptr<System> m_impl;
    godot::Spatial* m_spatial;

    void set_spatial(godot::Spatial* spatial);
    godot::Spatial* get_spatial() const;

    int get_id() const;
    godot::String get_name() const;
    godot::Vector3 get_pos() const;
    godot::Dictionary get_starlanes_wormholes() const;
};

#endif
