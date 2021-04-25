extends OptionButton


func _ready():
    clear()
    add_item("Player", global.CLIENT_TYPE_HUMAN_PLAYER)
    add_item("Moderator", global.CLIENT_TYPE_HUMAN_MODERATOR)
    add_item("Observer", global.CLIENT_TYPE_HUMAN_OBSERVER)
    select(get_item_index(global.CLIENT_TYPE_HUMAN_PLAYER))

