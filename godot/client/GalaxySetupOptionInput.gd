tool


extends OptionButton


export var WithNoneOption: bool = false


# Called when the node enters the scene tree for the first time.
func _ready():
    clear()
    if WithNoneOption:
        add_item("None", global.GO_NONE)
    add_item("Low", global.GO_LOW)
    add_item("Medium", global.GO_MEDIUM)
    add_item("High", global.GO_HIGH)
    add_item("Random", global.GO_RANDOM)
    select(get_item_index(global.GO_MEDIUM))
