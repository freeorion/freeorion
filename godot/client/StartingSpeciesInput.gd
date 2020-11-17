extends OptionButton


# Called when the node enters the scene tree for the first time.
func _ready():
    clear()
#    add_icon_item(global.species_textures[global.SP_EGASSEM], "Egassem", global.SP_EGASSEM)
    add_icon_item(preload("res://assets/image/species/egassem.png"), "Egassem", global.SP_EGASSEM)
    add_icon_item(preload("res://assets/image/species/human.png"), "Human", global.SP_HUMAN)
    add_icon_item(preload("res://assets/image/species/laenfa.png"), "Laenfa", global.SP_LAENFA)
    add_icon_item(preload("res://assets/image/species/scylior.png"), "Scylior", global.SP_SCYLIOR)
    add_icon_item(preload("res://assets/image/species/trith.png"), "Trith", global.SP_TRITH)
    select(get_item_index(global.SP_HUMAN))
