extends Control

var game_setup_dlg: FOWindow
var multiplayer_setup_dlg: FOWindow
var auth_password_setup_dlg: FOWindow
var error_dlg: FOWindow

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

	error_dlg = preload("res://ErrorWindow.tscn").instance()
	error_dlg.connect("ok", self, "_on_ErrorDlg_ok")

	global.chat_window = preload("res://ChatWindow.tscn").instance()
	add_child(global.chat_window)
	global.chat_window.hide()

	FreeOrionNode.start_network_thread()

	FreeOrionNode.connect("auth_request", self, "_on_FreeOrion_auth_request", [], CONNECT_DEFERRED)
	FreeOrionNode.connect("start_game", self, "_on_FreeOrion_start_game", [], CONNECT_DEFERRED)
	FreeOrionNode.connect(
		"chat_message", global.chat_window, "_on_FreeOrion_chat_message", [], CONNECT_DEFERRED
	)
	FreeOrionNode.connect("chat_message", self, "_on_FreeOrion_chat_message", [], CONNECT_DEFERRED)
	FreeOrionNode.connect("error", self, "_on_FreeOrion_error", [], CONNECT_DEFERRED)

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


func _on_SinglePlayerBtn_pressed():
	$Popup.add_child(game_setup_dlg)
	var pos_x = ($Popup.get_rect().size.x - game_setup_dlg.get_rect().size.x) / 2
	var pos_y = ($Popup.get_rect().size.y - game_setup_dlg.get_rect().size.y) / 2
	game_setup_dlg.set_position(Vector2(pos_x, pos_y))
	$Popup.popup()


func _on_QuickstartBtn_pressed():
	FreeOrionNode.new_single_player_game()


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
	FreeOrionNode.new_single_player_game()


func _on_GameSetupDlg_cancel():
	$Popup.hide()
	$Popup.remove_child(game_setup_dlg)


func _on_MultiplayerSetup_ok():
	$Popup.hide()
	$Popup.remove_child(multiplayer_setup_dlg)
	FreeOrionNode.connect_to_server(multiplayer_setup_dlg.server_name)
	if FreeOrionNode.is_server_connected():
		FreeOrionNode.join_game(
			multiplayer_setup_dlg.player_name, multiplayer_setup_dlg.client_type
		)


func _on_MultiplayerSetup_cancel():
	$Popup.hide()
	$Popup.remove_child(multiplayer_setup_dlg)


func _on_AuthSetupDlg_ok():
	$Popup.hide()
	$Popup.remove_child(auth_password_setup_dlg)
	FreeOrionNode.auth_response(
		auth_password_setup_dlg.player_name, auth_password_setup_dlg.password
	)


func _on_AuthSetupDlg_cancel():
	$Popup.hide()
	$Popup.remove_child(auth_password_setup_dlg)


func _on_ErrorDlg_ok():
	$Popup.hide()
	$Popup.remove_child(error_dlg)


func _on_FreeOrion_auth_request(player_name, _auth):
	$Popup.add_child(auth_password_setup_dlg)
	var pos_x = ($Popup.get_rect().size.x - auth_password_setup_dlg.get_rect().size.x) / 2
	var pos_y = ($Popup.get_rect().size.y - auth_password_setup_dlg.get_rect().size.y) / 2
	auth_password_setup_dlg.set_position(Vector2(pos_x, pos_y))
	auth_password_setup_dlg.set_player_name(player_name)
	$Popup.popup()


func _on_FreeOrion_error(problem: String, _fatal: bool):
	$Popup.add_child(error_dlg)
	error_dlg.set_error_text(problem)
	$Popup.popup()


func _on_FreeOrion_start_game(_is_new_game):
	remove_child(global.chat_window)
	get_tree().change_scene("res://GalaxyMap.tscn")


func _on_FreeOrion_chat_message(_text: String, _player_name: String, _text_color: Color, _pm: bool):
	global.chat_window.show()
