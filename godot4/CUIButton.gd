class_name CUIButton
extends Button


func _ready() -> void:
	mouse_entered.connect(_on_mouse_entered)
	pressed.connect(_on_pressed)


func _on_mouse_entered() -> void:
	if not disabled:
		$Rollover.play()


func _on_pressed() -> void:
	$Click.play()
