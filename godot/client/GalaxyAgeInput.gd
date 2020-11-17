extends OptionButton


# Called when the node enters the scene tree for the first time.
func _ready():
    clear()
    add_item("Young", global.GA_YOUNG)
    add_item("Mature", global.GA_MATURE)
    add_item("Ancient", global.GA_ANCIENT)
    add_item("Random", global.GA_RANDOM)
    select(get_item_index(global.GA_MATURE))
