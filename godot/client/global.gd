extends Node


const LETTER_UPPER = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
const LETTER_LOWER = "abcdefghijklmnopqrstuvwxyz"
const LETTER_DIGITS = "1234567890"

const GO_NONE = 0
const GO_LOW = 1
const GO_MEDIUM = 2
const GO_HIGH = 3
const GO_RANDOM = 4

const GS_SPIRAL2 = 0
const GS_SPIRAL3 = 1
const GS_SPIRAL4 = 2
const GS_CLUSTER = 3
const GS_ELLIPTICAL = 4
const GS_DISC = 5
const GS_BOX = 6
const GS_IRREGULAR = 7
const GS_RING = 8
const GS_RANDOM = 9

const GA_YOUNG = 0
const GA_MATURE = 1
const GA_ANCIENT = 2
const GA_RANDOM = 3

const AIA_BEGINNER = 0
const AIA_TURTLE = 1
const AIA_CAUTIOUS = 2
const AIA_TYPICAL = 3
const AIA_AGGRESSIVE = 4
const AIA_MANIACAL = 5

const SP_HUMAN = 0
const SP_LAENFA = 1
const SP_SCYLIOR = 2
const SP_EGASSEM = 3
const SP_TRITH = 4

const MIN_SYS_DIST = 1
const MAX_STARLANE_LENGTH = 15


var galaxy: Galaxy
var starfield: Spatial
var freeorion: Node


class Starlane:
    var source: int
    var dest: int
    
    func _init(a_source, a_dest):
        source = a_source
        dest = a_dest
        if not valid():
            print("WARNING: Created starlane where source and destination system are the same")
    
    func valid():
        return (source != dest)

class Fleet:
    var id: int
    var pos: Vector3
    var current_sys: Object
    var dest_sys: Object
    var dist_travelled: float
    var spatial: Spatial
    
    func _init(a_id: int, at_sys: Object):
        id = a_id
        current_sys = at_sys
        pos = current_sys.pos
        dest_sys = null
        dist_travelled = 0.0
    
    func set_transit(dest: Object, dist: float):
        dest_sys = dest
        dist_travelled = dist
        update_position()
    
    func set_stationary():
        set_transit(null, 0)
    
    func is_stationary():
        return dist_travelled == 0
    
    func update_position():
        if not spatial:
            return
        if dist_travelled:
            var vec_to_dest = (dest_sys.pos - current_sys.pos).normalized()
            pos = current_sys.pos + vec_to_dest * dist_travelled
            spatial.look_at_from_position(current_sys.pos, dest_sys.pos, Vector3(0, 1, 0))
            spatial.translation = pos
        else:
            pos = current_sys.pos
            spatial.translation = current_sys.pos + Vector3(0, 0.5, 0)


class Galaxy extends AStar:
    var systems = {}
    var starlanes = []
    var fleets = {}

    func _init():
        systems = global.freeorion._get_systems()

    func add_starlane(starlane: Starlane):
        if not ((starlane.source in systems.keys()) and (starlane.dest in systems.keys())):
            print("ERROR: Attempting to add starlane to non-existing systems")
            return
        
        var ssys: Object = systems[starlane.source]
        var ssys_linked_sys = ssys.get_linked_systems()
        var dsys: Object = systems[starlane.dest]
        var dsys_linked_sys = dsys.get_linked_systems()
        
        if (ssys.id in dsys_linked_sys) and (not dsys.id in ssys_linked_sys) or (dsys.id in ssys_linked_sys) and (not ssys.id in dsys_linked_sys):
            print("ERROR: Found corrupted starlane lists when attempting to add starlane to system")
            return
        
        ssys.add_starlane(starlane)
        dsys.add_starlane(starlane)
        starlanes.append(starlane)
        connect_points(starlane.source, starlane.dest)
    
    func get_all_sys_connected_to(sys_id, connected_sys_list: Array = []):
        if sys_id in connected_sys_list:
            return
        
        connected_sys_list.append(sys_id)
        var this_sys: Object = systems[sys_id]
        
        for ssid in this_sys.get_linked_systems():
            if ssid in connected_sys_list:
                continue
            get_all_sys_connected_to(ssid, connected_sys_list)
        
        return connected_sys_list
    
    func get_islands():
        var islands = []
        var already_assigned_sys = []
        for ssid in systems.keys():
            if ssid in already_assigned_sys:
                continue
            var island = get_all_sys_connected_to(ssid)
            islands.append(island)
            already_assigned_sys += island
        return islands

    func generate_starlanes():
        for ss in systems.values():
            set_point_disabled(ss.id, true)
            var dest = get_closest_point(ss.pos, false)
            set_point_disabled(ss.id, false)
            if (dest >= 0) and (not dest in ss.get_linked_systems()):
                add_starlane(Starlane.new(ss.id, dest))
        
        var islands = get_islands()
        var last_amount_of_islands: int = 0
        while len(islands) != last_amount_of_islands:
            for island in islands:
                for ssid in island:
                    set_point_disabled(ssid, true)
                
                var dist_map = {}
                var dist_list = []
                for ssid in island:
                    var ss: Object = systems[ssid]
                    var closest_island = get_closest_point(ss.pos)
                    if closest_island >= 0:
                        var dist = ss.pos.distance_to(get_point_position(closest_island))
                        if dist in dist_map.keys():
                            dist_map[dist].append(ss)
                        else:
                            dist_map[dist] = [ss]
                        dist_list.append(dist)
                
                if dist_list:
                    dist_list.sort()
                    for ss in dist_map[dist_list[0]]:
                        add_starlane(Starlane.new(ss.id, get_closest_point(ss.pos)))

                for ssid in island:
                    set_point_disabled(ssid, false)
            
            last_amount_of_islands = len(islands)
            islands = get_islands()
        
        for ss in systems.values():
            var linked_sys = ss.get_linked_systems()
            if len(linked_sys) == 1:
                set_point_disabled(ss.id, true)
                set_point_disabled(linked_sys[0], true)
                var closest_neighbor = get_closest_point(ss.pos)
                add_starlane(Starlane.new(ss.id, closest_neighbor))
                set_point_disabled(ss.id, false)
                set_point_disabled(linked_sys[0], false)
