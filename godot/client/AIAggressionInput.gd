extends OptionButton


# Called when the node enters the scene tree for the first time.
func _ready():
    clear()
    add_item("Beginner Mode", global.AIA_BEGINNER)
    add_item("Turtle", global.AIA_TURTLE)
    add_item("Cautious", global.AIA_CAUTIOUS)
    add_item("Typical", global.AIA_TYPICAL)
    add_item("Aggressive", global.AIA_AGGRESSIVE)
    add_item("Maniacal", global.AIA_MANIACAL)
    select(get_item_index(global.AIA_MANIACAL))
