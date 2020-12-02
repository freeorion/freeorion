#include "gdfreeorion.h"
#include "OptionsDB.h"
#include "GodotNetworking.h"
#include "GodotSystem.h"
#include "GodotFleet.h"

extern "C" void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options *o) {
    godot::Godot::gdnative_init(o);
}

extern "C" void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options *o) {
    godot::Godot::gdnative_terminate(o);
}

extern "C" void GDN_EXPORT godot_nativescript_init(void *handle) {
    godot::Godot::nativescript_init(handle);

    godot::register_class<godot::GDFreeOrion>();
    godot::register_class<godot::OptionsDB>();
    godot::register_class<godot::GodotNetworking>();
    godot::register_class<godot::GodotSystem>();
    godot::register_class<godot::GodotFleet>();
}
