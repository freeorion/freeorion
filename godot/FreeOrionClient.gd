extends Control

var game_setup_dlg: FOWindow
var multiplayer_setup_dlg: FOWindow
var auth_password_setup_dlg: FOWindow
var network_thread: Thread

onready var viewport = get_viewport()


# Called when the node enters the scene tree for the first time.
func _ready():
	game_setup_dlg = preload("res://GameSetup.tscn").instance()
	game_setup_dlg.connect("ok", self, "_on_GameSetupDlg_ok")
	game_setup_dlg.connect("cancel", self, "_on_GameSetupDlg_cancel")

	multiplayer_setup_dlg = preload("res://MultiplayerSetup.tscn").instance()
	multiplayer_setup_dlg.connect("ok", self, "_on_MultiplayerSetup_ok")
	multiplayer_setup_dlg.connect("cancel", self, "_on_MultiplayerSetup_cancel")

	auth_password_setup_dlg = preload("res://AuthPasswordSetup.tscn").instance()
	auth_password_setup_dlg.connect("ok", self, "_on_AuthSetupDlg_ok")
	auth_password_setup_dlg.connect("cancel", self, "_on_AuthSetupDlg_cancel")

	var scale = 1
	if OS.get_name() == "Android":
		scale = 1.4

	var minimum_size = Vector2(1600, 900) / scale

	var current_size = OS.get_window_size()

	var scale_factor = minimum_size.y / current_size.y
	var new_size = Vector2(current_size.x * scale_factor, minimum_size.y)

	if new_size.y < minimum_size.y:
		scale_factor = minimum_size.y / new_size.y
		new_size = Vector2(new_size.x * scale_factor, minimum_size.y)
	if new_size.x < minimum_size.x:
		scale_factor = minimum_size.x / new_size.x
		new_size = Vector2(minimum_size.x, new_size.y * scale_factor)

	viewport.set_size_override(true, new_size)

	network_thread = Thread.new()
	network_thread.start($FreeOrion, "network_thread")


func _exit_tree():
	network_thread.wait_to_finish()


func _on_SinglePlayerBtn_pressed():
	$Popup.add_child(game_setup_dlg)
	var pos_x = ($Popup.get_rect().size.x - game_setup_dlg.get_rect().size.x) / 2
	var pos_y = ($Popup.get_rect().size.y - game_setup_dlg.get_rect().size.y) / 2
	game_setup_dlg.set_position(Vector2(pos_x, pos_y))
	$Popup.popup()


func _on_QuickstartBtn_pressed():
	$FreeOrion.new_single_player_game()


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
	$FreeOrion.new_single_player_game()


func _on_GameSetupDlg_cancel():
	$Popup.hide()
	$Popup.remove_child(game_setup_dlg)


func _on_MultiplayerSetup_ok():
	$Popup.hide()
	$Popup.remove_child(multiplayer_setup_dlg)


func _on_MultiplayerSetup_cancel():
	$Popup.hide()
	$Popup.remove_child(multiplayer_setup_dlg)


func _on_AuthSetupDlg_ok():
	$Popup.hide()
	$Popup.remove_child(auth_password_setup_dlg)
	# ToDo: send password
	print("Send password from auth dialog")


func _on_AuthSetupDlg_cancel():
	$Popup.hide()
	$Popup.remove_child(auth_password_setup_dlg)


func _on_FreeOrion_auth_request(player_name, _auth):
	$Popup.add_child(auth_password_setup_dlg)
	var pos_x = ($Popup.get_rect().size.x - auth_password_setup_dlg.get_rect().size.x) / 2
	var pos_y = ($Popup.get_rect().size.y - auth_password_setup_dlg.get_rect().size.y) / 2
	auth_password_setup_dlg.set_position(Vector2(pos_x, pos_y))
	auth_password_setup_dlg.set_player_name(player_name)
	$Popup.popup()


func _on_FreeOrion_start_game(_is_new_game):
	global.freeorion = $FreeOrion
	get_tree().change_scene("res://GalaxyMap.tscn")
