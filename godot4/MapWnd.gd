extends Control


func _ready() -> void:
	GlobalFreeOrionNode.start_game.connect(_on_freeorion_start_game)
	GlobalFreeOrionNode.turn_update.connect(_on_freeorion_turn_update)
	$ToolBar/BtnTurn.set_click_sound(preload("res://assets/sound/turn_button_click.ogg"))
	_update_btn_turn()


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
