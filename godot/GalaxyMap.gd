extends Spatial


# Called when the node enters the scene tree for the first time.
func _ready():
	# ToDo: catch window resize
	var toolbox_size: Vector2 = $ToolBoxPanelContainer.get_size()
	toolbox_size.x = OS.get_window_size().x
	$ToolBoxPanelContainer.set_size(toolbox_size)
