extends Spatial


# Called when the node enters the scene tree for the first time.
func _ready():
    for ss in global.galaxy.systems.values():
        ss.spatial.connect("input_event", $StarField, "_on_Star_input_event")
        ss.spatial.connect("input_event", $Starnames, "_on_Star_input_event")
    
    for fleet in global.galaxy.fleets.values():
        fleet.spatial.connect("clicked", self, "_on_Fleet_clicked")


func _on_Fleet_clicked(fleet):
    $FleetWindow.set_fleet(fleet)
    $FleetWindow.show()
