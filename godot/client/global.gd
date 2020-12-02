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
        fleets = global.freeorion._get_fleets()

    func add_starlane(starlane: Starlane):
        if not ((starlane.source in systems.keys()) and (starlane.dest in systems.keys())):
            print("ERROR: Attempting to add starlane to non-existing systems")
            return

        starlanes.append(starlane)
        connect_points(starlane.source, starlane.dest)
