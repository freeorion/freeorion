extends Control


var game_setup_dlg: Control
var pings: int = 0


# Called when the node enters the scene tree for the first time.
func _ready():
    game_setup_dlg = preload("res://GameSetup.tscn").instance()
    $Popup.add_child(game_setup_dlg)
    game_setup_dlg.connect("ok", self, "_on_GameSetupDlg_ok")
    game_setup_dlg.connect("cancel", self, "_on_GameSetupDlg_cancel")
    
    $Node.connect("ping", self, "_on_Node_ping")
    $Node.connect("auth_request", self, "_on_Node_auth_request")
    $Node.connect("empire_status", self, "_on_Node_empire_status")
    $Node.connect("start_game", self, "_on_Node_start_game")
    var systems: int = $Node.optionsDB._get_option_int("setup.star.count")
    print("Systems from optionsDB ", systems)


func _on_QuitBtn_pressed():
    get_tree().quit()


func _on_SinglePlayerBtn_pressed():
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


func _on_Node_ping(message: String):
    pings += 1
    print("Received message#", pings, " from C++ GDNative library code ", message)

func _on_Node_auth_request(player_name: String, _auth: String):
    $Node.networking._auth_response(player_name, "1")

func _on_Node_empire_status(status: int, about_empire_id: int):
    print("Received empire #", about_empire_id, " status ", status)

func _on_Node_start_game(is_new_game: bool):
    print("Received start game. New ", is_new_game)
    global.freeorion = $Node
    get_tree().change_scene("res://GalaxyMap.tscn")

func _on_MultiplayerBtn_pressed():
    var connected: bool = $Node.networking._is_connected()
    if connected:
        print("Already connected")
    else:
        connected = $Node.networking._connect_to_server("localhost", "GodotPlayer")
        if not connected:
            print("Cannot connect")
        else:
            pass
