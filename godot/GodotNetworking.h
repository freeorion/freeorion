#ifndef _GODOT_NETWORKING_H_
#define _GODOT_NETWORKING_H_

#include <Godot.hpp>
#include <Object.hpp>

namespace godot {
    class GodotNetworking : public Object {
        GODOT_CLASS(GodotNetworking, Object)

    private:
    public:
        static void _register_methods();

        GodotNetworking();
        ~GodotNetworking();

        void _init(); // our initializer called by Godot

        bool _is_connected() const;
        bool _connect_to_server(String dest, String player_name);
        void _auth_response(String player_name, String password);
    };
}

#endif

