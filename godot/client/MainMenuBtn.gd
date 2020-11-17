tool


class_name MainMenuBtn


extends Button


export var Title = "Title" setget set_title


# Called when the node enters the scene tree for the first time.
func _ready():
    $Label.text = Title


func set_title(new_title):
    Title = new_title
    $Label.text = Title
