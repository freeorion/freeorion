extends Spatial


var ig = ImmediateGeometry.new()
var galaxy = global.Galaxy.new()
var fleet_icon = preload("res://FleetIcon.tscn")


# Called when the node enters the scene tree for the first time.
func _ready():
    global.starfield = self
    var star_scene = preload("res://Star.tscn")

    global.galaxy = galaxy
    
    for ss in galaxy.systems.values():
        var star = star_scene.instance()
        ss.spatial = star
        star.translate(ss.pos)
        add_child(star)

    for fleet in galaxy.fleets.values():
        print("Init fleet: ", fleet.id)
        fleet.spatial = fleet_icon.instance()
        fleet.spatial.fleet = fleet
        add_child(fleet.spatial)

    ig.material_override = load("res://resources/materials/starlane_material.tres")
    add_child(ig)


func _process(delta):
    ig.clear()
    ig.begin(Mesh.PRIMITIVE_LINES)
    ig.set_color(Color(1, 1, 1))
    for starlane in galaxy.starlanes:
        ig.add_vertex(galaxy.get_point_position(starlane.source))
        ig.add_vertex(galaxy.get_point_position(starlane.dest))
    ig.end()


func _on_Star_input_event(camera, event, click_position, click_normal, shape_idx):
    if not event is InputEventMouseButton:
        return
    if not event.button_index == BUTTON_LEFT:
        return
    
    var ssid = global.galaxy.get_closest_point(click_position)
    if ssid < 0:
        return
    
    $StarSystemSelectionMarker.translation = global.galaxy.systems[ssid].pos
    $StarSystemSelectionMarker.show()
