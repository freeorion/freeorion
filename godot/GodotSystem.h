#ifndef _GODOT_SYSTEM_H_
#define _GODOT_SYSTEM_H_

#include <Godot.hpp>
#include <Object.hpp>
#include <String.hpp>
#include <Spatial.hpp>
#include <memory>

class System;

namespace godot {
    class GodotSystem : public Object {
        GODOT_CLASS(GodotSystem, Object)

    private:
        std::shared_ptr<System> m_impl;
        Spatial* m_spatial;
    public:
        static void _register_methods();

        static GodotSystem* wrap(const std::shared_ptr<System>& impl);

        GodotSystem();
        ~GodotSystem();

        void _init(); // our initializer called by Godot

        void set_spatial(Spatial* spatial);
        Spatial* get_spatial() const;

        int get_id() const;
        String get_name() const;
        Vector3 get_pos() const;
        Dictionary get_starlanes_wormholes() const;
    };
}


#endif

