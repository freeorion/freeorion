extends Control

const ART_DIR := "res://assets/art/"


func _ready():
	GlobalFreeOrionNode.parsing_completed.connect(_on_freeorion_parsing_completed)
	GlobalFreeOrionNode.start_game.connect(_on_freeorion_start_game)

	GlobalFreeOrionNode.start_network_thread()
	GlobalFreeOrionNode.start_parsing_thread()

	var dpi := DisplayServer.screen_get_dpi()
	if dpi > 300:
		get_window().content_scale_factor = 1.75
	elif dpi > 200:
		get_window().content_scale_factor = 1.35
	else:
		get_window().content_scale_factor = 1.0

	var date := Time.get_date_dict_from_system(false)
	var time := Time.get_time_dict_from_system(false)
	var splash := "splash.png"
	var logo := "logo.png"
	if date.month == 4 and date.day == 1:
		splash = "splash0104.png"
		logo = "logo0104.png"
	elif date.month == 12 and date.day == 25:
		splash = "splash2512.png"
		logo = "logo2512.png"
	elif date.month == 10 and date.day == 31:
		splash = "splash3110.png"
		logo = "logo3110.png"
	elif time.second == 42:
		logo = "logo0104.png"
	$Splash.texture = load(ART_DIR + splash)
	$Logo.texture = load(ART_DIR + logo)

	var window := get_window()
	var margins := SafeArea.margins(window)
	var corner_margin := SafeArea.corner_inset(
		SafeArea.corner_radius(window, SafeArea.CORNER_BOTTOM_RIGHT)
	)
	$Version.offset_right = -maxf(margins.z, corner_margin)
	$Version.offset_bottom = -maxf(margins.w, corner_margin)
	$Version.text = GlobalFreeOrionNode.get_version()


func _on_freeorion_parsing_completed():
	if GlobalFreeOrionNode.options_get_bool("quickstart"):
		$Menu/VBoxContainer/MenuButtons/MarginContainer/VBoxContainer/QuickStart.disabled = true
		GlobalFreeOrionNode.new_single_player_game()
	else:
		$Menu/VBoxContainer/MenuButtons/MarginContainer/VBoxContainer/QuickStart.disabled = false
		$Menu/VBoxContainer/MenuButtons/MarginContainer/VBoxContainer/MultiPlayer.disabled = false


func _on_exit_game_pressed() -> void:
	get_tree().quit()


func _on_quick_start_pressed() -> void:
	$Menu/VBoxContainer/MenuButtons/MarginContainer/VBoxContainer/QuickStart.disabled = true
	GlobalFreeOrionNode.new_single_player_game()


func _on_freeorion_start_game(_is_new_game: bool) -> void:
	get_tree().change_scene_to_file("res://MapWnd.tscn")


func _on_multi_player_pressed() -> void:
	$Menu/VBoxContainer/MenuButtons/MarginContainer/VBoxContainer/MultiPlayer.disabled = true
	$Popup.add_child(preload("res://ServerConnectWnd.tscn").instantiate())
	$Popup.visible = true


func _on_credits_pressed() -> void:
	var credits_wnd := preload("res://CreditsWnd.tscn").instantiate()
	add_child(credits_wnd)
