#include "PackedDataContainer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


PackedDataContainer::___method_bindings PackedDataContainer::___mb = {};

void PackedDataContainer::___init_method_bindings() {
	___mb.mb__get_data = godot::api->godot_method_bind_get_method("PackedDataContainer", "_get_data");
	___mb.mb__iter_get = godot::api->godot_method_bind_get_method("PackedDataContainer", "_iter_get");
	___mb.mb__iter_init = godot::api->godot_method_bind_get_method("PackedDataContainer", "_iter_init");
	___mb.mb__iter_next = godot::api->godot_method_bind_get_method("PackedDataContainer", "_iter_next");
	___mb.mb__set_data = godot::api->godot_method_bind_get_method("PackedDataContainer", "_set_data");
	___mb.mb_pack = godot::api->godot_method_bind_get_method("PackedDataContainer", "pack");
	___mb.mb_size = godot::api->godot_method_bind_get_method("PackedDataContainer", "size");
}

PackedDataContainer *PackedDataContainer::_new()
{
	return (PackedDataContainer *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"PackedDataContainer")());
}
PoolByteArray PackedDataContainer::_get_data() const {
	return ___godot_icall_PoolByteArray(___mb.mb__get_data, (const Object *) this);
}

Variant PackedDataContainer::_iter_get(const Variant arg0) {
	return ___godot_icall_Variant_Variant(___mb.mb__iter_get, (const Object *) this, arg0);
}

Variant PackedDataContainer::_iter_init(const Array arg0) {
	return ___godot_icall_Variant_Array(___mb.mb__iter_init, (const Object *) this, arg0);
}

Variant PackedDataContainer::_iter_next(const Array arg0) {
	return ___godot_icall_Variant_Array(___mb.mb__iter_next, (const Object *) this, arg0);
}

void PackedDataContainer::_set_data(const PoolByteArray arg0) {
	___godot_icall_void_PoolByteArray(___mb.mb__set_data, (const Object *) this, arg0);
}

Error PackedDataContainer::pack(const Variant value) {
	return (Error) ___godot_icall_int_Variant(___mb.mb_pack, (const Object *) this, value);
}

int64_t PackedDataContainer::size() const {
	return ___godot_icall_int(___mb.mb_size, (const Object *) this);
}

}