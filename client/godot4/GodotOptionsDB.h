#ifndef _GodotOptionsDB_h_
#define _GodotOptionsDB_h_

#include <godot_cpp/classes/object.hpp>

class GodotOptionsDB : public godot::Object {
    GDCLASS(GodotOptionsDB, Object)
protected:
    static void _bind_methods();
};

#endif

