extends Control


onready var viewport = get_viewport()

var game_setup_dlg: FOWindow
var multiplayer_setup_dlg: FOWindow
var auth_password_setup_dlg: FOWindow
var pings: int = 0
var network_thread: Thread


# Called when the node enters the scene tree for the first time.
func _ready():
    game_setup_dlg = preload("res://GameSetup.tscn").instance()
    game_setup_dlg.connect("ok", self, "_on_GameSetupDlg_ok")
    game_setup_dlg.connect("cancel", self, "_on_GameSetupDlg_cancel")

    multiplayer_setup_dlg = preload("res://MultiplayerSetup.tscn").instance()
    multiplayer_setup_dlg.connect("ok", self, "_on_MultiplayerSetupDlg_ok")
    multiplayer_setup_dlg.connect("cancel", self, "_on_MultiplayerSetupDlg_cancel")

    auth_password_setup_dlg = preload("res://AuthPasswordSetup.tscn").instance()
    auth_password_setup_dlg.connect("ok", self, "_on_AuthPasswordSetupDlg_ok")
    auth_password_setup_dlg.connect("cancel", self, "_on_AuthPasswordSetupDlg_cancel")

    $Node.connect("ping", self, "_on_Node_ping")
    $Node.connect("auth_request", self, "_on_Node_auth_request")
    $Node.connect("empire_status", self, "_on_Node_empire_status")
    $Node.connect("start_game", self, "_on_Node_start_game")
    $Node.connect("error", self, "_on_Node_error")
    var systems: int = $Node.optionsDB._get_option_int("setup.star.count")
    print("Systems from optionsDB ", systems)
    network_thread = Thread.new()
    network_thread.start($Node, "network_thread")
    
    var scale = 1
    if OS.get_name() == "Android":
        scale = 1.5
    
    var minimum_size = Vector2(1600, 900) / scale
    
    var current_size = OS.get_window_size()
    
    var scale_factor = minimum_size.y/current_size.y
    var new_size = Vector2(current_size.x*scale_factor, minimum_size.y)

    if new_size.y < minimum_size.y:
        scale_factor = minimum_size.y/new_size.y
        new_size = Vector2(new_size.x*scale_factor, minimum_size.y)
    if new_size.x < minimum_size.x:
        scale_factor = minimum_size.x/new_size.x
        new_size = Vector2(minimum_size.x, new_size.y*scale_factor)

    viewport.set_size_override(true, new_size)

func _exit_tree():
    network_thread.wait_to_finish()


func _on_QuitBtn_pressed():
    get_tree().quit()


func _on_SinglePlayerBtn_pressed():
    $Popup.add_child(game_setup_dlg)
    var pos_x = ($Popup.get_rect().size.x - game_setup_dlg.get_rect().size.x) / 2
    var pos_y = ($Popup.get_rect().size.y - game_setup_dlg.get_rect().size.y) / 2
    game_setup_dlg.set_position(Vector2(pos_x, pos_y))
    $Popup.popup()

func _on_QuickstartBtn_pressed():
    $Node._new_single_player_game(true)


func _on_GameSetupDlg_ok():
    $Popup.hide()
    $Node._new_single_player_game(false)

func _on_GameSetupDlg_cancel():
    $Popup.hide()


func _on_MultiplayerSetupDlg_ok():
    $Popup.hide()

    var connected: bool = $Node.networking._is_connected()
    if connected:
        print("Already connected")
    else:
        print("Connecting to ", multiplayer_setup_dlg.server_name)
        connected = $Node.networking._connect_to_server(multiplayer_setup_dlg.server_name, multiplayer_setup_dlg.player_name)
        if not connected:
            print("Cannot connect")
        else:
            pass

func _on_MultiplayerSetupDlg_cancel():
    $Popup.hide()


func _on_AuthPasswordSetupDlg_ok():
    $Popup.hide()
    $Node.networking._auth_response(auth_password_setup_dlg.player_name, auth_password_setup_dlg.password)

func _on_AuthPasswordSetupDlg_cancel():
    $Popup.hide()


func _on_Node_ping(message: String):
    pings += 1
    print("Received message#", pings, " from C++ GDNative library code ", message)

func _on_Node_auth_request(player_name: String, _auth: String):
    $Popup.add_child(auth_password_setup_dlg)
    var pos_x = ($Popup.get_rect().size.x - auth_password_setup_dlg.get_rect().size.x) / 2
    var pos_y = ($Popup.get_rect().size.y - auth_password_setup_dlg.get_rect().size.y) / 2
    auth_password_setup_dlg.set_position(Vector2(pos_x, pos_y))
    auth_password_setup_dlg.set_player_name(player_name)
    $Popup.popup()


func _on_Node_empire_status(status: int, about_empire_id: int):
    print("Received empire #", about_empire_id, " status ", status)

func _on_Node_start_game(is_new_game: bool):
    print("Received start game. New ", is_new_game)
    global.freeorion = $Node
    get_tree().change_scene("res://GalaxyMap.tscn")

func _on_Node_error(problem: String, fatal: bool):
    print("Received error: ", problem, " fatal: ", fatal)

func _on_MultiplayerBtn_pressed():
    $Popup.add_child(multiplayer_setup_dlg)
    var pos_x = ($Popup.get_rect().size.x - multiplayer_setup_dlg.get_rect().size.x) / 2
    var pos_y = ($Popup.get_rect().size.y - multiplayer_setup_dlg.get_rect().size.y) / 2
    multiplayer_setup_dlg.set_position(Vector2(pos_x, pos_y))
    $Popup.popup()
