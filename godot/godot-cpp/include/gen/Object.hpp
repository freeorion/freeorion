#ifndef GODOT_CPP_OBJECT_HPP
#define GODOT_CPP_OBJECT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/TagDB.hpp>

namespace godot {

class Object;
class Reference;

class Object : public _Wrapped {
public: enum { ___CLASS_IS_SCRIPT = 0, };

private:
	struct ___method_bindings {
		godot_method_bind *mb__get;
		godot_method_bind *mb__get_property_list;
		godot_method_bind *mb__init;
		godot_method_bind *mb__notification;
		godot_method_bind *mb__set;
		godot_method_bind *mb__to_string;
		godot_method_bind *mb_add_user_signal;
		godot_method_bind *mb_call;
		godot_method_bind *mb_call_deferred;
		godot_method_bind *mb_callv;
		godot_method_bind *mb_can_translate_messages;
		godot_method_bind *mb_connect;
		godot_method_bind *mb_disconnect;
		godot_method_bind *mb_emit_signal;
		godot_method_bind *mb_free;
		godot_method_bind *mb_get;
		godot_method_bind *mb_get_class;
		godot_method_bind *mb_get_incoming_connections;
		godot_method_bind *mb_get_indexed;
		godot_method_bind *mb_get_instance_id;
		godot_method_bind *mb_get_meta;
		godot_method_bind *mb_get_meta_list;
		godot_method_bind *mb_get_method_list;
		godot_method_bind *mb_get_property_list;
		godot_method_bind *mb_get_script;
		godot_method_bind *mb_get_signal_connection_list;
		godot_method_bind *mb_get_signal_list;
		godot_method_bind *mb_has_meta;
		godot_method_bind *mb_has_method;
		godot_method_bind *mb_has_user_signal;
		godot_method_bind *mb_is_blocking_signals;
		godot_method_bind *mb_is_class;
		godot_method_bind *mb_is_connected;
		godot_method_bind *mb_is_queued_for_deletion;
		godot_method_bind *mb_notification;
		godot_method_bind *mb_property_list_changed_notify;
		godot_method_bind *mb_remove_meta;
		godot_method_bind *mb_set;
		godot_method_bind *mb_set_block_signals;
		godot_method_bind *mb_set_deferred;
		godot_method_bind *mb_set_indexed;
		godot_method_bind *mb_set_message_translation;
		godot_method_bind *mb_set_meta;
		godot_method_bind *mb_set_script;
		godot_method_bind *mb_to_string;
		godot_method_bind *mb_tr;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Object"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum ConnectFlags {
		CONNECT_DEFERRED = 1,
		CONNECT_PERSIST = 2,
		CONNECT_ONESHOT = 4,
		CONNECT_REFERENCE_COUNTED = 8,
	};

	// constants
	const static int NOTIFICATION_POSTINITIALIZE = 0;
	const static int NOTIFICATION_PREDELETE = 1;


	static Object *_new();

	// methods
#ifndef GODOT_CPP_NO_OBJECT_CAST
	template<class T>
	static T *cast_to(const Object *obj);
#endif

	Variant _get(const String property);
	Array _get_property_list();
	void _init();
	void _notification(const int64_t what);
	bool _set(const String property, const Variant value);
	String _to_string();
	void add_user_signal(const String signal, const Array arguments = Array());
	Variant call(const String method, const Array& __var_args = Array());
	void call_deferred(const String method, const Array& __var_args = Array());
	Variant callv(const String method, const Array arg_array);
	bool can_translate_messages() const;
	Error connect(const String signal, const Object *target, const String method, const Array binds = Array(), const int64_t flags = 0);
	void disconnect(const String signal, const Object *target, const String method);
	void emit_signal(const String signal, const Array& __var_args = Array());
	void free();
	Variant get(const String property) const;
	String get_class() const;
	Array get_incoming_connections() const;
	Variant get_indexed(const NodePath property) const;
	int64_t get_instance_id() const;
	Variant get_meta(const String name) const;
	PoolStringArray get_meta_list() const;
	Array get_method_list() const;
	Array get_property_list() const;
	Reference *get_script() const;
	Array get_signal_connection_list(const String signal) const;
	Array get_signal_list() const;
	bool has_meta(const String name) const;
	bool has_method(const String method) const;
	bool has_user_signal(const String signal) const;
	bool is_blocking_signals() const;
	bool is_class(const String _class) const;
	bool is_connected(const String signal, const Object *target, const String method) const;
	bool is_queued_for_deletion() const;
	void notification(const int64_t what, const bool reversed = false);
	void property_list_changed_notify();
	void remove_meta(const String name);
	void set(const String property, const Variant value);
	void set_block_signals(const bool enable);
	void set_deferred(const String property, const Variant value);
	void set_indexed(const NodePath property, const Variant value);
	void set_message_translation(const bool enable);
	void set_meta(const String name, const Variant value);
	void set_script(const Reference *script);
	String to_string();
	String tr(const String message) const;
	template <class... Args> Variant call(const String method, Args... args){
		return call(method, Array::make(args...));
	}
	template <class... Args> void call_deferred(const String method, Args... args){
		return call_deferred(method, Array::make(args...));
	}
	template <class... Args> void emit_signal(const String signal, Args... args){
		return emit_signal(signal, Array::make(args...));
	}

};

}

#endif