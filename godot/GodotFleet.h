#ifndef _GODOT_FLEET_H_
#define _GODOT_FLEET_H_

#include <Godot.hpp>
#include <Object.hpp>
#include <String.hpp>
#include <Spatial.hpp>
#include <memory>

class Fleet;

namespace godot {
    class GodotFleet : public Object {
        GODOT_CLASS(GodotFleet, Object)

    private:
        std::shared_ptr<Fleet> m_impl;
        Spatial* m_spatial;
    public:
        static void _register_methods();

        static GodotFleet* wrap(const std::shared_ptr<Fleet>& impl);

        GodotFleet();
        ~GodotFleet();

        void _init(); // our initializer called by Godot

        void set_spatial(Spatial* spatial);
        Spatial* get_spatial() const;

        int get_id() const;
        String get_name() const;
        Vector3 get_pos() const;
        bool is_stationary() const;
    };
}


#endif

