#ifndef _GodotFleet_h_
#define _GodotFleet_h_

#include <Godot.hpp>
#include <Object.hpp>
#include <String.hpp>
#include <Spatial.hpp>
#include <memory>

class Fleet;

class GodotFleet : public godot::Object {
    GODOT_CLASS(GodotFleet, godot::Object)

public:
    static void _register_methods();

    static GodotFleet* Wrap(const std::shared_ptr<Fleet>& impl);

    GodotFleet();
    ~GodotFleet();

    void _init(); // our initializer called by Godot

private:
    std::shared_ptr<Fleet> m_impl;
    godot::Spatial* m_spatial;

    void set_spatial(godot::Spatial* spatial);
    godot::Spatial* get_spatial() const;

    int get_id() const;
    godot::String get_name() const;
    godot::Vector3 get_pos() const;
    bool is_stationary() const;
};

#endif

