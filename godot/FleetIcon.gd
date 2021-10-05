extends KinematicBody

signal clicked(fleet)

var fleet: Object


func _on_FleetIcon_input_event(_camera, event, _click_position, _click_normal, _shape_idx):
	if not event is InputEventMouseButton:
		return
	if not event.button_index == BUTTON_LEFT:
		return
	emit_signal("clicked", fleet)
