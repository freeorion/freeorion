#include "Shape2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Shape2D.hpp"


namespace godot {


Shape2D::___method_bindings Shape2D::___mb = {};

void Shape2D::___init_method_bindings() {
	___mb.mb_collide = godot::api->godot_method_bind_get_method("Shape2D", "collide");
	___mb.mb_collide_and_get_contacts = godot::api->godot_method_bind_get_method("Shape2D", "collide_and_get_contacts");
	___mb.mb_collide_with_motion = godot::api->godot_method_bind_get_method("Shape2D", "collide_with_motion");
	___mb.mb_collide_with_motion_and_get_contacts = godot::api->godot_method_bind_get_method("Shape2D", "collide_with_motion_and_get_contacts");
	___mb.mb_get_custom_solver_bias = godot::api->godot_method_bind_get_method("Shape2D", "get_custom_solver_bias");
	___mb.mb_set_custom_solver_bias = godot::api->godot_method_bind_get_method("Shape2D", "set_custom_solver_bias");
}

bool Shape2D::collide(const Transform2D local_xform, const Ref<Shape2D> with_shape, const Transform2D shape_xform) {
	return ___godot_icall_bool_Transform2D_Object_Transform2D(___mb.mb_collide, (const Object *) this, local_xform, with_shape.ptr(), shape_xform);
}

Array Shape2D::collide_and_get_contacts(const Transform2D local_xform, const Ref<Shape2D> with_shape, const Transform2D shape_xform) {
	return ___godot_icall_Array_Transform2D_Object_Transform2D(___mb.mb_collide_and_get_contacts, (const Object *) this, local_xform, with_shape.ptr(), shape_xform);
}

bool Shape2D::collide_with_motion(const Transform2D local_xform, const Vector2 local_motion, const Ref<Shape2D> with_shape, const Transform2D shape_xform, const Vector2 shape_motion) {
	return ___godot_icall_bool_Transform2D_Vector2_Object_Transform2D_Vector2(___mb.mb_collide_with_motion, (const Object *) this, local_xform, local_motion, with_shape.ptr(), shape_xform, shape_motion);
}

Array Shape2D::collide_with_motion_and_get_contacts(const Transform2D local_xform, const Vector2 local_motion, const Ref<Shape2D> with_shape, const Transform2D shape_xform, const Vector2 shape_motion) {
	return ___godot_icall_Array_Transform2D_Vector2_Object_Transform2D_Vector2(___mb.mb_collide_with_motion_and_get_contacts, (const Object *) this, local_xform, local_motion, with_shape.ptr(), shape_xform, shape_motion);
}

real_t Shape2D::get_custom_solver_bias() const {
	return ___godot_icall_float(___mb.mb_get_custom_solver_bias, (const Object *) this);
}

void Shape2D::set_custom_solver_bias(const real_t bias) {
	___godot_icall_void_float(___mb.mb_set_custom_solver_bias, (const Object *) this, bias);
}

}