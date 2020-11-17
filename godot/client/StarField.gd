extends Spatial


var ig = ImmediateGeometry.new()
var galaxy = global.Galaxy.new()
var fleet_icon = preload("res://FleetIcon.tscn")


func place_fleets(num: int):
    var sys_list: Array = global.galaxy.systems.values().duplicate()
    sys_list.shuffle()
    
    for id in range(num):
        var ss: Object = sys_list.pop_front()
        var fleet = global.Fleet.new(id, ss)
        fleet.spatial = fleet_icon.instance()
        fleet.spatial.fleet = fleet
        add_child(fleet.spatial)
        global.galaxy.fleets[fleet.id] = fleet
        
        if randf() < 0.5:
            var neighbors: Array = ss.get_linked_systems()
            neighbors.shuffle()
            var dest_sys: Object = global.galaxy.systems[neighbors[0]]
            var dist: float = ss.pos.distance_to(dest_sys.pos)
            fleet.set_transit(dest_sys, dist * randf())
        else:
            fleet.set_stationary()


# Called when the node enters the scene tree for the first time.
func _ready():
    global.starfield = self
    var star_scene = preload("res://Star.tscn")

    #galaxy.generate_starlanes()
    global.galaxy = galaxy
    
    for ss in galaxy.systems.values():
        var star = star_scene.instance()
        ss.spatial = star
        star.translate(ss.pos)
        add_child(star)
    
    #place_fleets(round(global.gs_map_size / 10))
    
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
