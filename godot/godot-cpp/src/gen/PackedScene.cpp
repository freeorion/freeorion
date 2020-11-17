#include "PackedScene.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "SceneState.hpp"
#include "Node.hpp"


namespace godot {


PackedScene::___method_bindings PackedScene::___mb = {};

void PackedScene::___init_method_bindings() {
	___mb.mb__get_bundled_scene = godot::api->godot_method_bind_get_method("PackedScene", "_get_bundled_scene");
	___mb.mb__set_bundled_scene = godot::api->godot_method_bind_get_method("PackedScene", "_set_bundled_scene");
	___mb.mb_can_instance = godot::api->godot_method_bind_get_method("PackedScene", "can_instance");
	___mb.mb_get_state = godot::api->godot_method_bind_get_method("PackedScene", "get_state");
	___mb.mb_instance = godot::api->godot_method_bind_get_method("PackedScene", "instance");
	___mb.mb_pack = godot::api->godot_method_bind_get_method("PackedScene", "pack");
}

PackedScene *PackedScene::_new()
{
	return (PackedScene *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"PackedScene")());
}
Dictionary PackedScene::_get_bundled_scene() const {
	return ___godot_icall_Dictionary(___mb.mb__get_bundled_scene, (const Object *) this);
}

void PackedScene::_set_bundled_scene(const Dictionary arg0) {
	___godot_icall_void_Dictionary(___mb.mb__set_bundled_scene, (const Object *) this, arg0);
}

bool PackedScene::can_instance() const {
	return ___godot_icall_bool(___mb.mb_can_instance, (const Object *) this);
}

Ref<SceneState> PackedScene::get_state() {
	return Ref<SceneState>::__internal_constructor(___godot_icall_Object(___mb.mb_get_state, (const Object *) this));
}

Node *PackedScene::instance(const int64_t edit_state) const {
	return (Node *) ___godot_icall_Object_int(___mb.mb_instance, (const Object *) this, edit_state);
}

Error PackedScene::pack(const Node *path) {
	return (Error) ___godot_icall_int_Object(___mb.mb_pack, (const Object *) this, path);
}

}