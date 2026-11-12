extends Control

var _turn_base_left := 0.0
var _turn_base_top := 0.0
var _turn_base_width := 0.0


func _ready() -> void:
	var btn: Button = $ToolBar/BtnTurn
	_turn_base_left = btn.offset_left
	_turn_base_top = btn.offset_top
	_turn_base_width = btn.offset_right - btn.offset_left

	GlobalFreeOrionNode.start_game.connect(_on_freeorion_start_game)
	GlobalFreeOrionNode.turn_update.connect(_on_freeorion_turn_update)
	btn.set_click_sound(preload("res://assets/sound/turn_button_click.ogg"))
	_update_btn_turn()
	_apply_safe_area()
	get_viewport().size_changed.connect(_apply_safe_area)


func _on_freeorion_start_game(_is_new_game: bool) -> void:
	_update_btn_turn()


func _on_freeorion_turn_update() -> void:
	_update_btn_turn()


func _on_btn_turn_pressed() -> void:
	GlobalFreeOrionNode.start_turn()
	_update_btn_turn()


func _update_btn_turn() -> void:
	var turn: int = GlobalFreeOrionNode.get_current_turn()
	if turn < 1:
		$ToolBar/BtnTurn.disabled = true
		return
	$ToolBar/BtnTurn.disabled = false
	$ToolBar/BtnTurn.text = tr("MAP_BTN_TURN_UPDATE").replace("%1%", str(turn))


# The toolbar is drawn under the Android status bar and the rounded screen
# corner, so the Turn button is moved in until it clears both.  The scene
# offsets are remembered so the shift is recomputed from them on every resize.
func _apply_safe_area() -> void:
	if OS.get_name() != "Android":
		return
	var btn: Button = $ToolBar/BtnTurn
	var shift := maxf(_safe_area_inset() - _turn_base_left, 0.0)
	btn.offset_left = _turn_base_left + shift
	btn.offset_right = btn.offset_left + _turn_base_width


func _safe_area_inset() -> float:
	var window := get_window()
	return maxf(
		SafeArea.margins(window).x,
		SafeArea.corner_clearance(
			SafeArea.corner_radius(window, SafeArea.CORNER_TOP_LEFT), _turn_base_top
		)
	)
