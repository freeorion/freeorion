extends OptionButton


# Called when the node enters the scene tree for the first time.
func _ready():
	clear()
#    add_icon_item(global.species_textures[global.SP_EGASSEM], "Egassem", global.SP_EGASSEM)
	add_icon_item(
		preload("res://assets/art/icons/species/egassem.png"), "Egassem", global.SP_EGASSEM
	)
	add_icon_item(preload("res://assets/art/icons/species/human.png"), "Human", global.SP_HUMAN)
	add_icon_item(preload("res://assets/art/icons/species/laenfa.png"), "Laenfa", global.SP_LAENFA)
	add_icon_item(
		preload("res://assets/art/icons/species/scylior.png"), "Scylior", global.SP_SCYLIOR
	)
	add_icon_item(preload("res://assets/art/icons/species/trith.png"), "Trith", global.SP_TRITH)
	select(get_item_index(global.SP_HUMAN))
