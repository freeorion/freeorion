#include "WebSocketPeer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


WebSocketPeer::___method_bindings WebSocketPeer::___mb = {};

void WebSocketPeer::___init_method_bindings() {
	___mb.mb_close = godot::api->godot_method_bind_get_method("WebSocketPeer", "close");
	___mb.mb_get_connected_host = godot::api->godot_method_bind_get_method("WebSocketPeer", "get_connected_host");
	___mb.mb_get_connected_port = godot::api->godot_method_bind_get_method("WebSocketPeer", "get_connected_port");
	___mb.mb_get_write_mode = godot::api->godot_method_bind_get_method("WebSocketPeer", "get_write_mode");
	___mb.mb_is_connected_to_host = godot::api->godot_method_bind_get_method("WebSocketPeer", "is_connected_to_host");
	___mb.mb_set_no_delay = godot::api->godot_method_bind_get_method("WebSocketPeer", "set_no_delay");
	___mb.mb_set_write_mode = godot::api->godot_method_bind_get_method("WebSocketPeer", "set_write_mode");
	___mb.mb_was_string_packet = godot::api->godot_method_bind_get_method("WebSocketPeer", "was_string_packet");
}

WebSocketPeer *WebSocketPeer::_new()
{
	return (WebSocketPeer *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"WebSocketPeer")());
}
void WebSocketPeer::close(const int64_t code, const String reason) {
	___godot_icall_void_int_String(___mb.mb_close, (const Object *) this, code, reason);
}

String WebSocketPeer::get_connected_host() const {
	return ___godot_icall_String(___mb.mb_get_connected_host, (const Object *) this);
}

int64_t WebSocketPeer::get_connected_port() const {
	return ___godot_icall_int(___mb.mb_get_connected_port, (const Object *) this);
}

WebSocketPeer::WriteMode WebSocketPeer::get_write_mode() const {
	return (WebSocketPeer::WriteMode) ___godot_icall_int(___mb.mb_get_write_mode, (const Object *) this);
}

bool WebSocketPeer::is_connected_to_host() const {
	return ___godot_icall_bool(___mb.mb_is_connected_to_host, (const Object *) this);
}

void WebSocketPeer::set_no_delay(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_no_delay, (const Object *) this, enabled);
}

void WebSocketPeer::set_write_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_write_mode, (const Object *) this, mode);
}

bool WebSocketPeer::was_string_packet() const {
	return ___godot_icall_bool(___mb.mb_was_string_packet, (const Object *) this);
}

}