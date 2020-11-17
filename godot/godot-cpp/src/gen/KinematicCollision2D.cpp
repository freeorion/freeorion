#include "KinematicCollision2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"


namespace godot {


KinematicCollision2D::___method_bindings KinematicCollision2D::___mb = {};

void KinematicCollision2D::___init_method_bindings() {
	___mb.mb_get_collider = godot::api->godot_method_bind_get_method("KinematicCollision2D", "get_collider");
	___mb.mb_get_collider_id = godot::api->godot_method_bind_get_method("KinematicCollision2D", "get_collider_id");
	___mb.mb_get_collider_metadata = godot::api->godot_method_bind_get_method("KinematicCollision2D", "get_collider_metadata");
	___mb.mb_get_collider_shape = godot::api->godot_method_bind_get_method("KinematicCollision2D", "get_collider_shape");
	___mb.mb_get_collider_shape_index = godot::api->godot_method_bind_get_method("KinematicCollision2D", "get_collider_shape_index");
	___mb.mb_get_collider_velocity = godot::api->godot_method_bind_get_method("KinematicCollision2D", "get_collider_velocity");
	___mb.mb_get_local_shape = godot::api->godot_method_bind_get_method("KinematicCollision2D", "get_local_shape");
	___mb.mb_get_normal = godot::api->godot_method_bind_get_method("KinematicCollision2D", "get_normal");
	___mb.mb_get_position = godot::api->godot_method_bind_get_method("KinematicCollision2D", "get_position");
	___mb.mb_get_remainder = godot::api->godot_method_bind_get_method("KinematicCollision2D", "get_remainder");
	___mb.mb_get_travel = godot::api->godot_method_bind_get_method("KinematicCollision2D", "get_travel");
}

KinematicCollision2D *KinematicCollision2D::_new()
{
	return (KinematicCollision2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"KinematicCollision2D")());
}
Object *KinematicCollision2D::get_collider() const {
	return (Object *) ___godot_icall_Object(___mb.mb_get_collider, (const Object *) this);
}

int64_t KinematicCollision2D::get_collider_id() const {
	return ___godot_icall_int(___mb.mb_get_collider_id, (const Object *) this);
}

Variant KinematicCollision2D::get_collider_metadata() const {
	return ___godot_icall_Variant(___mb.mb_get_collider_metadata, (const Object *) this);
}

Object *KinematicCollision2D::get_collider_shape() const {
	return (Object *) ___godot_icall_Object(___mb.mb_get_collider_shape, (const Object *) this);
}

int64_t KinematicCollision2D::get_collider_shape_index() const {
	return ___godot_icall_int(___mb.mb_get_collider_shape_index, (const Object *) this);
}

Vector2 KinematicCollision2D::get_collider_velocity() const {
	return ___godot_icall_Vector2(___mb.mb_get_collider_velocity, (const Object *) this);
}

Object *KinematicCollision2D::get_local_shape() const {
	return (Object *) ___godot_icall_Object(___mb.mb_get_local_shape, (const Object *) this);
}

Vector2 KinematicCollision2D::get_normal() const {
	return ___godot_icall_Vector2(___mb.mb_get_normal, (const Object *) this);
}

Vector2 KinematicCollision2D::get_position() const {
	return ___godot_icall_Vector2(___mb.mb_get_position, (const Object *) this);
}

Vector2 KinematicCollision2D::get_remainder() const {
	return ___godot_icall_Vector2(___mb.mb_get_remainder, (const Object *) this);
}

Vector2 KinematicCollision2D::get_travel() const {
	return ___godot_icall_Vector2(___mb.mb_get_travel, (const Object *) this);
}

}