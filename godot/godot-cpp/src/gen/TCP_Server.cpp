#include "TCP_Server.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "StreamPeerTCP.hpp"


namespace godot {


TCP_Server::___method_bindings TCP_Server::___mb = {};

void TCP_Server::___init_method_bindings() {
	___mb.mb_is_connection_available = godot::api->godot_method_bind_get_method("TCP_Server", "is_connection_available");
	___mb.mb_is_listening = godot::api->godot_method_bind_get_method("TCP_Server", "is_listening");
	___mb.mb_listen = godot::api->godot_method_bind_get_method("TCP_Server", "listen");
	___mb.mb_stop = godot::api->godot_method_bind_get_method("TCP_Server", "stop");
	___mb.mb_take_connection = godot::api->godot_method_bind_get_method("TCP_Server", "take_connection");
}

TCP_Server *TCP_Server::_new()
{
	return (TCP_Server *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"TCP_Server")());
}
bool TCP_Server::is_connection_available() const {
	return ___godot_icall_bool(___mb.mb_is_connection_available, (const Object *) this);
}

bool TCP_Server::is_listening() const {
	return ___godot_icall_bool(___mb.mb_is_listening, (const Object *) this);
}

Error TCP_Server::listen(const int64_t port, const String bind_address) {
	return (Error) ___godot_icall_int_int_String(___mb.mb_listen, (const Object *) this, port, bind_address);
}

void TCP_Server::stop() {
	___godot_icall_void(___mb.mb_stop, (const Object *) this);
}

Ref<StreamPeerTCP> TCP_Server::take_connection() {
	return Ref<StreamPeerTCP>::__internal_constructor(___godot_icall_Object(___mb.mb_take_connection, (const Object *) this));
}

}