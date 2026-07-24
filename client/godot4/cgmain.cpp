#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

#include "FreeOrionNode.h"

void initialize_freeorion_module(godot::ModuleInitializationLevel p_level) {
	if (p_level != godot::MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

    GDREGISTER_CLASS(FreeOrionNode);
}

void uninitialize_freeorion_module(godot::ModuleInitializationLevel p_level) {
	if (p_level != godot::MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}

extern "C" GDExtensionBool GDE_EXPORT freeorion_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address,
                                                             const GDExtensionClassLibraryPtr p_library,
                                                             GDExtensionInitialization *r_initialization)
{
	godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

	init_obj.register_initializer(initialize_freeorion_module);
	init_obj.register_terminator(uninitialize_freeorion_module);
	init_obj.set_minimum_library_initialization_level(godot::MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}
