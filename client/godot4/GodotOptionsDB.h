#ifndef _GodotOptionsDB_h_
#define _GodotOptionsDB_h_

#include <godot_cpp/classes/object.hpp>

class GodotOptionsDB : public godot::Object {
    GDCLASS(GodotOptionsDB, Object)
protected:
    static void _bind_methods();
private:
    bool get_bool(godot::String option) const;

    godot::String get_string(godot::String option) const;

    godot::PackedStringArray find_options(godot::String prefix, bool allow_unrecognized = false) const;

    bool option_exists(godot::String option) const;

    void add_string(godot::String option, godot::String description, godot::String value);

    void set_string(godot::String option, godot::String value);

    bool commit(bool only_if_dirty = true, bool only_non_default = true);
};

#endif

