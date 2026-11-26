class_name CUIIconButton
extends TextureButton

var _normal_icon: Texture2D
var _hover_icon: Texture2D


func _ready() -> void:
	_normal_icon = get_texture_normal()
	_hover_icon = get_texture_hover()
	mouse_entered.connect(_on_mouse_entered)
	pressed.connect(_on_pressed)


func set_active(active: bool) -> void:
	if _normal_icon == null or _hover_icon == null:
		return
	set_texture_normal(_hover_icon if active else _normal_icon)
	set_texture_hover(_normal_icon if active else _hover_icon)


func _on_mouse_entered() -> void:
	if not disabled:
		$Rollover.play()


func _on_pressed() -> void:
	$Click.play()
