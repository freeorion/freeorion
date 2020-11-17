#include "GlobalConstants.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


GlobalConstants *GlobalConstants::_singleton = NULL;


GlobalConstants::GlobalConstants() {
	_owner = godot::api->godot_global_get_singleton((char *) "GlobalConstants");
}


GlobalConstants::___method_bindings GlobalConstants::___mb = {};

void GlobalConstants::___init_method_bindings() {
}

}