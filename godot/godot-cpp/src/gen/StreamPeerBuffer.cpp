#include "StreamPeerBuffer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "StreamPeerBuffer.hpp"


namespace godot {


StreamPeerBuffer::___method_bindings StreamPeerBuffer::___mb = {};

void StreamPeerBuffer::___init_method_bindings() {
	___mb.mb_clear = godot::api->godot_method_bind_get_method("StreamPeerBuffer", "clear");
	___mb.mb_duplicate = godot::api->godot_method_bind_get_method("StreamPeerBuffer", "duplicate");
	___mb.mb_get_data_array = godot::api->godot_method_bind_get_method("StreamPeerBuffer", "get_data_array");
	___mb.mb_get_position = godot::api->godot_method_bind_get_method("StreamPeerBuffer", "get_position");
	___mb.mb_get_size = godot::api->godot_method_bind_get_method("StreamPeerBuffer", "get_size");
	___mb.mb_resize = godot::api->godot_method_bind_get_method("StreamPeerBuffer", "resize");
	___mb.mb_seek = godot::api->godot_method_bind_get_method("StreamPeerBuffer", "seek");
	___mb.mb_set_data_array = godot::api->godot_method_bind_get_method("StreamPeerBuffer", "set_data_array");
}

StreamPeerBuffer *StreamPeerBuffer::_new()
{
	return (StreamPeerBuffer *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"StreamPeerBuffer")());
}
void StreamPeerBuffer::clear() {
	___godot_icall_void(___mb.mb_clear, (const Object *) this);
}

Ref<StreamPeerBuffer> StreamPeerBuffer::duplicate() const {
	return Ref<StreamPeerBuffer>::__internal_constructor(___godot_icall_Object(___mb.mb_duplicate, (const Object *) this));
}

PoolByteArray StreamPeerBuffer::get_data_array() const {
	return ___godot_icall_PoolByteArray(___mb.mb_get_data_array, (const Object *) this);
}

int64_t StreamPeerBuffer::get_position() const {
	return ___godot_icall_int(___mb.mb_get_position, (const Object *) this);
}

int64_t StreamPeerBuffer::get_size() const {
	return ___godot_icall_int(___mb.mb_get_size, (const Object *) this);
}

void StreamPeerBuffer::resize(const int64_t size) {
	___godot_icall_void_int(___mb.mb_resize, (const Object *) this, size);
}

void StreamPeerBuffer::seek(const int64_t position) {
	___godot_icall_void_int(___mb.mb_seek, (const Object *) this, position);
}

void StreamPeerBuffer::set_data_array(const PoolByteArray data) {
	___godot_icall_void_PoolByteArray(___mb.mb_set_data_array, (const Object *) this, data);
}

}