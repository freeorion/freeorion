extends Control

var game_setup_dlg: FOWindow
var multiplayer_setup_dlg: FOWindow
var auth_password_setup_dlg: FOWindow


# Called when the node enters the scene tree for the first time.
func _ready():
	game_setup_dlg = preload("res://GameSetup.tscn").instance()
	game_setup_dlg.connect("ok", self, "_on_GameSetupDlg_ok")
	game_setup_dlg.connect("cancel", self, "_on_GameSetupDlg_cancel")

	multiplayer_setup_dlg = preload("res://MultiplayerSetup.tscn").instance()
	multiplayer_setup_dlg.connect("ok", self, "_on_MultiplayerSetup_ok")
	multiplayer_setup_dlg.connect("cancel", self, "_on_MultiplayerSetup_cancel")


func _on_SinglePlayerBtn_pressed():
	$Popup.add_child(game_setup_dlg)
	var pos_x = ($Popup.get_rect().size.x - game_setup_dlg.get_rect().size.x) / 2
	var pos_y = ($Popup.get_rect().size.y - game_setup_dlg.get_rect().size.y) / 2
	game_setup_dlg.set_position(Vector2(pos_x, pos_y))
	$Popup.popup()


func _on_QuickstartBtn_pressed():
	pass  # Replace with function body.


func _on_MultiplayerBtn_pressed():
	$Popup.add_child(multiplayer_setup_dlg)
	var pos_x = ($Popup.get_rect().size.x - multiplayer_setup_dlg.get_rect().size.x) / 2
	var pos_y = ($Popup.get_rect().size.y - multiplayer_setup_dlg.get_rect().size.y) / 2
	multiplayer_setup_dlg.set_position(Vector2(pos_x, pos_y))
	$Popup.popup()


func _on_QuitBtn_pressed():
	get_tree().quit()


func _on_GameSetupDlg_ok():
	$Popup.hide()
	$Popup.remove_child(game_setup_dlg)


func _on_GameSetupDlg_cancel():
	$Popup.hide()
	$Popup.remove_child(game_setup_dlg)


func _on_MultiplayerSetup_ok():
	$Popup.hide()
	$Popup.remove_child(multiplayer_setup_dlg)


func _on_MultiplayerSetup_cancel():
	$Popup.hide()
	$Popup.remove_child(multiplayer_setup_dlg)
