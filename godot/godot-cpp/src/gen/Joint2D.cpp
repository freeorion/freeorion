#include "Joint2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


Joint2D::___method_bindings Joint2D::___mb = {};

void Joint2D::___init_method_bindings() {
	___mb.mb_get_bias = godot::api->godot_method_bind_get_method("Joint2D", "get_bias");
	___mb.mb_get_exclude_nodes_from_collision = godot::api->godot_method_bind_get_method("Joint2D", "get_exclude_nodes_from_collision");
	___mb.mb_get_node_a = godot::api->godot_method_bind_get_method("Joint2D", "get_node_a");
	___mb.mb_get_node_b = godot::api->godot_method_bind_get_method("Joint2D", "get_node_b");
	___mb.mb_set_bias = godot::api->godot_method_bind_get_method("Joint2D", "set_bias");
	___mb.mb_set_exclude_nodes_from_collision = godot::api->godot_method_bind_get_method("Joint2D", "set_exclude_nodes_from_collision");
	___mb.mb_set_node_a = godot::api->godot_method_bind_get_method("Joint2D", "set_node_a");
	___mb.mb_set_node_b = godot::api->godot_method_bind_get_method("Joint2D", "set_node_b");
}

real_t Joint2D::get_bias() const {
	return ___godot_icall_float(___mb.mb_get_bias, (const Object *) this);
}

bool Joint2D::get_exclude_nodes_from_collision() const {
	return ___godot_icall_bool(___mb.mb_get_exclude_nodes_from_collision, (const Object *) this);
}

NodePath Joint2D::get_node_a() const {
	return ___godot_icall_NodePath(___mb.mb_get_node_a, (const Object *) this);
}

NodePath Joint2D::get_node_b() const {
	return ___godot_icall_NodePath(___mb.mb_get_node_b, (const Object *) this);
}

void Joint2D::set_bias(const real_t bias) {
	___godot_icall_void_float(___mb.mb_set_bias, (const Object *) this, bias);
}

void Joint2D::set_exclude_nodes_from_collision(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_exclude_nodes_from_collision, (const Object *) this, enable);
}

void Joint2D::set_node_a(const NodePath node) {
	___godot_icall_void_NodePath(___mb.mb_set_node_a, (const Object *) this, node);
}

void Joint2D::set_node_b(const NodePath node) {
	___godot_icall_void_NodePath(___mb.mb_set_node_b, (const Object *) this, node);
}

}