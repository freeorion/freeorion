extends Spatial


# Called when the node enters the scene tree for the first time.
func _ready():
	# ToDo: catch window resize
	var toolbox_size: Vector2 = $ToolBoxPanelContainer.get_size()
	toolbox_size.x = OS.get_window_size().x
	$ToolBoxPanelContainer.set_size(toolbox_size)

	var camera_init: Vector3 = Vector3(0, 0, 0)

	for ss in global.galaxy.systems.values():
		ss.spatial.connect("input_event", $StarField, "_on_Star_input_event")
		ss.spatial.connect("input_event", $Starnames, "_on_Star_input_event")
		camera_init = ss.pos

	for fleet in global.galaxy.fleets.values():
		fleet.spatial.connect("clicked", self, "_on_Fleet_clicked")
		camera_init = fleet.pos

	$GalaxyMapCamera.look_at_galaxy(camera_init)


func _on_Fleet_clicked(fleet):
	$FleetWindow.set_fleet(fleet)
	$FleetWindow.show()
