extends Control


func _ready():
	GlobalFreeOrionNode.start_network_thread()
	GlobalFreeOrionNode.start_parsing_thread()

	$Version.text = GlobalFreeOrionNode.get_version()
