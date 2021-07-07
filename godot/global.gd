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

const CLIENT_TYPE_AI_PLAYER = 0
const CLIENT_TYPE_HUMAN_PLAYER = 1
const CLIENT_TYPE_HUMAN_OBSERVER = 2
const CLIENT_TYPE_HUMAN_MODERATOR = 3

var freeorion: Node
