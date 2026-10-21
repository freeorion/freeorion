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

	var safe_area := DisplayServer.get_display_safe_area()
	var window_size := DisplayServer.window_get_size()
	var phys_margin_right := float(window_size.x - safe_area.end.x)
	var phys_margin_bottom := float(window_size.y - safe_area.end.y)
	var scale_factor := get_window().content_scale_factor
	var logical_margin_x := phys_margin_right / scale_factor
	var logical_margin_y := phys_margin_bottom / scale_factor
	var phys_corner_margin := 0.0
	if Engine.has_singleton("FreeOrion"):
		var plugin := Engine.get_singleton("FreeOrion")
		var radii: Array = plugin.getRoundedCornerRadii()
		phys_corner_margin = float(radii[2])  # RoundedCorner.POSITION_BOTTOM_RIGHT
	var logical_corner_margin := phys_corner_margin / scale_factor
	if logical_corner_margin > 0:
		logical_corner_margin = logical_corner_margin * (1.0 - sqrt(2.0) / 2.0)
	var margin_x := maxf(logical_margin_x, logical_corner_margin)
	var margin_y := maxf(logical_margin_y, logical_corner_margin)
	$Version.offset_right = -margin_x
	$Version.offset_bottom = -margin_y
	$Version.text = GlobalFreeOrionNode.get_version()


func _on_freeorion_parsing_completed():
	if GlobalFreeOrionNode.options_get_bool("quickstart"):
		$Menu/VBoxContainer/MenuButtons/VBoxContainer/QuickStart.disabled = true
		GlobalFreeOrionNode.new_single_player_game()
	else:
		$Menu/VBoxContainer/MenuButtons/VBoxContainer/QuickStart.disabled = false


func _on_exit_game_pressed() -> void:
	get_tree().quit()


func _on_quick_start_pressed() -> void:
	$Menu/VBoxContainer/MenuButtons/VBoxContainer/QuickStart.disabled = true
	GlobalFreeOrionNode.new_single_player_game()


func _on_freeorion_start_game(_is_new_game: bool) -> void:
	get_tree().change_scene_to_file("res://MapWnd.tscn")
