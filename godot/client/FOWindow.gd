tool


class_name FOWindow


extends NinePatchRect


export var Title: String = "Window Title" setget set_title

enum {NONE, RESIZE, DRAG, PINNED}
var state = NONE
var mouse_pos_offset: Vector2


# Called when the node enters the scene tree for the first time.
func _ready():
    pass # Replace with function body.


func _input(event):
    if event is InputEventMouseButton:
        if event.button_index != BUTTON_LEFT:
            return

        match state:
            NONE:
                if event.pressed:
                    var mouse_pos = get_global_mouse_position()
                    if $Title.get_global_rect().has_point(mouse_pos):
                        mouse_pos_offset = mouse_pos - get_global_rect().position
                        state = DRAG
                    elif $ResizeWidget.get_global_rect().has_point(mouse_pos):
                        mouse_pos_offset = mouse_pos - get_global_rect().end
                        state = RESIZE

            DRAG, RESIZE:
                if not event.pressed:
                    state = NONE

    elif event is InputEventMouseMotion:
        match state:
            DRAG:
                var pos = get_global_mouse_position() - mouse_pos_offset
                if pos.y < 0:
                    pos.y = 0
                set_position(pos)
                get_tree().set_input_as_handled()
            
            RESIZE:
                set_size(get_global_mouse_position() - mouse_pos_offset - get_global_rect().position)
                get_tree().set_input_as_handled()


func set_title(new_title):
    Title = new_title
    $Title.text = Title


func _on_PinWidget_toggled(button_pressed):
    if button_pressed:
        state = PINNED
    else:
        state = NONE
