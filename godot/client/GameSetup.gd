extends FOWindow


var galaxy_shape_textures = {}


signal ok
signal cancel


# Called when the node enters the scene tree for the first time.
func _ready():
    randomize()
    
    galaxy_shape_textures[global.GS_SPIRAL2] = preload("res://assets/image/gp_spiral2.png")
    galaxy_shape_textures[global.GS_SPIRAL3] = preload("res://assets/image/gp_spiral3.png")
    galaxy_shape_textures[global.GS_SPIRAL4] = preload("res://assets/image/gp_spiral4.png")
    galaxy_shape_textures[global.GS_CLUSTER] = preload("res://assets/image/gp_cluster.png")
    galaxy_shape_textures[global.GS_ELLIPTICAL] = preload("res://assets/image/gp_elliptical.png")
    galaxy_shape_textures[global.GS_DISC] = preload("res://assets/image/gp_disc.png")
    galaxy_shape_textures[global.GS_BOX] = preload("res://assets/image/gp_box.png")
    galaxy_shape_textures[global.GS_IRREGULAR] = preload("res://assets/image/gp_irregular.png")
    galaxy_shape_textures[global.GS_RING] = preload("res://assets/image/gp_ring.png")
    galaxy_shape_textures[global.GS_RANDOM] = preload("res://assets/image/gp_random.png")
    $ToplevelContainer/RightContainer/GalaxyShapeTex.texture = galaxy_shape_textures[global.GS_DISC]


func _on_CloseWidget_pressed():
    emit_signal("cancel")


func _on_OKBtn_pressed():
    #global.gs_seed = $ToplevelContainer/LeftContainer/Seed/LineEdit.text
    #global.gs_map_size = $ToplevelContainer/LeftContainer/Systems/SpinBox.value
    emit_signal("ok")


func _on_CancelBtn_pressed():
    emit_signal("cancel")


func _on_GalaxyShape_selected(index):
    var item_id = $ToplevelContainer/LeftContainer/GalaxyShape/OptionButton.get_item_id(index)
    $ToplevelContainer/RightContainer/GalaxyShapeTex.texture = galaxy_shape_textures[item_id]
