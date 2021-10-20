extends Spatial


# Called when the node enters the scene tree for the first time.
func _ready():
	var camera_init: Vector3 = Vector3(0, 0, 0)

	for ss in global.galaxy.systems.values():
		ss.spatial.connect("input_event", $StarField, "_on_Star_input_event")
		ss.spatial.connect("input_event", $Starnames, "_on_Star_input_event")
		camera_init = ss.pos

	for fleet in global.galaxy.fleets.values():
		fleet.spatial.connect("clicked", self, "_on_Fleet_clicked")
		camera_init = fleet.pos

	$GalaxyMapCamera.look_at_galaxy(camera_init)

	add_child(global.chat_window)
	global.chat_window.hide()

	FreeOrionNode.connect("chat_message", self, "_on_FreeOrion_chat_message", [], CONNECT_DEFERRED)


func _exit_tree():
	remove_child(global.chat_window)


func _on_Fleet_clicked(fleet):
	$FleetWindow.set_fleet(fleet)
	$FleetWindow.show()


func _on_FreeOrion_chat_message(_text: String, _player_name: String, _text_color: Color, _pm: bool):
	global.chat_window.show()


func _on_MessagesButton_toggled(button_pressed):
	print("Test ", button_pressed)
	if button_pressed:
		global.chat_window.show()
	else:
		global.chat_window.hide()
