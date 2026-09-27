extends Control

const ART_DIR := "res://assets/art/"


func _ready():
	GlobalFreeOrionNode.parsing_completed.connect(_on_freeorion_parsing_completed)

	GlobalFreeOrionNode.start_network_thread()
	GlobalFreeOrionNode.start_parsing_thread()

	$Version.text = GlobalFreeOrionNode.get_version()

	var date = Time.get_date_dict_from_system(false)
	var time = Time.get_time_dict_from_system(false)
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


func _on_freeorion_parsing_completed():
	if GlobalFreeOrionNode.options_get_bool("quickstart"):
		GlobalFreeOrionNode.new_single_player_game()
