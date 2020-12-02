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
        for sys in systems.values():
            add_point(sys.id, sys.pos)
        for sys in systems.values():
            var starlanes_wormholes: Dictionary = sys._get_starlanes_wormholes()
            for id in starlanes_wormholes.keys():
                if !starlanes_wormholes[id] && sys.id > id:
                    add_starlane(Starlane.new(sys.id, id))

    func add_starlane(starlane: Starlane):
        if not ((starlane.source in systems.keys()) and (starlane.dest in systems.keys())):
            print("ERROR: Attempting to add starlane to non-existing systems")
            return

        starlanes.append(starlane)
        connect_points(starlane.source, starlane.dest)
