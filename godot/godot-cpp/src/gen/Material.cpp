#include "Material.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Material.hpp"


namespace godot {


Material::___method_bindings Material::___mb = {};

void Material::___init_method_bindings() {
	___mb.mb_get_next_pass = godot::api->godot_method_bind_get_method("Material", "get_next_pass");
	___mb.mb_get_render_priority = godot::api->godot_method_bind_get_method("Material", "get_render_priority");
	___mb.mb_set_next_pass = godot::api->godot_method_bind_get_method("Material", "set_next_pass");
	___mb.mb_set_render_priority = godot::api->godot_method_bind_get_method("Material", "set_render_priority");
}

Ref<Material> Material::get_next_pass() const {
	return Ref<Material>::__internal_constructor(___godot_icall_Object(___mb.mb_get_next_pass, (const Object *) this));
}

int64_t Material::get_render_priority() const {
	return ___godot_icall_int(___mb.mb_get_render_priority, (const Object *) this);
}

void Material::set_next_pass(const Ref<Material> next_pass) {
	___godot_icall_void_Object(___mb.mb_set_next_pass, (const Object *) this, next_pass.ptr());
}

void Material::set_render_priority(const int64_t priority) {
	___godot_icall_void_int(___mb.mb_set_render_priority, (const Object *) this, priority);
}

}