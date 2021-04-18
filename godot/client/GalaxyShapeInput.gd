extends OptionButton


# Declare member variables here. Examples:
# var a = 2
# var b = "text"


# Called when the node enters the scene tree for the first time.
func _ready():
    clear()
    add_item("Spiral, 2-arm", global.GS_SPIRAL2)
    add_item("Spiral, 3-arm", global.GS_SPIRAL3)
    add_item("Spiral, 4-arm", global.GS_SPIRAL4)
    add_item("Cluster", global.GS_CLUSTER)
    add_item("Elliptical", global.GS_ELLIPTICAL)
    add_item("Disc", global.GS_DISC)
    add_item("Box", global.GS_BOX)
    add_item("Irregular", global.GS_IRREGULAR)
    add_item("Ring", global.GS_RING)
    add_item("Random", global.GS_RANDOM)
    select(get_item_index(global.GS_DISC))
