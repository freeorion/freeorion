extends Control


func _ready():
	GlobalFreeOrionNode.parsing_completed.connect(_on_freeorion_parsing_completed)

	GlobalFreeOrionNode.start_network_thread()
	GlobalFreeOrionNode.start_parsing_thread()

	$Version.text = GlobalFreeOrionNode.get_version()


func _on_freeorion_parsing_completed():
	if GlobalFreeOrionNode.options_get_bool("quickstart"):
		GlobalFreeOrionNode.new_single_player_game()
