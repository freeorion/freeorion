#include "FreeOrionNode.h"
#include "GodotFleet.h"
#include "GodotSystem.h"

extern "C" void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options *o) {
    // ToDo: use future OS::get_native_handle instead
#if defined(FREEORION_ANDROID)
    for (unsigned int i = 0; i < o->api_struct->num_extensions; i++) {
        if (o->api_struct->extensions[i]->type == GDNATIVE_EXT_ANDROID) {
            FreeOrionNode::set_android_api_struct(reinterpret_cast<const godot_gdnative_ext_android_api_struct *>(o->api_struct->extensions[i]));
            break;
        }
    }
#endif

    godot::Godot::gdnative_init(o);
}

extern "C" void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options *o) {
    godot::Godot::gdnative_terminate(o);
}

extern "C" void GDN_EXPORT godot_nativescript_init(void *handle) {
    godot::Godot::nativescript_init(handle);

    godot::register_class<FreeOrionNode>();
    godot::register_class<GodotFleet>();
    godot::register_class<GodotSystem>();
}

