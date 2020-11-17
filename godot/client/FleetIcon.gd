extends KinematicBody


var fleet: global.Fleet


signal clicked(fleet)


func _on_FleetIcon_input_event(camera, event, click_position, click_normal, shape_idx):
    if not event is InputEventMouseButton:
        return
    if not event.button_index == BUTTON_LEFT:
        return
    emit_signal("clicked", fleet)
