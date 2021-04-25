extends Node2D


const SCALE_1_AT = 50.0
const NAME_FONT_SIZE = 4
const COLOR_NORMAL = Color(1, 1, 1)
const COLOR_SELECTED = Color(0, 0, 1)


var font_robo = preload("res://resources/fonts/Roboto-Regular.tres")
var fonts = {}

var star_selected: int = -1


# Called when the node enters the scene tree for the first time.
func _ready():
    for ss in global.galaxy.systems.values():
        fonts[ss.id] = font_robo.duplicate()


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
    update()


func _draw():
    var cam = get_viewport().get_camera()
    var cpos = cam.translation

    for ss in global.galaxy.systems.values():
        if cam.is_position_behind(ss.pos):
            continue
        
        var dec_scale = SCALE_1_AT / (cpos.distance_to(ss.pos))
        if dec_scale < 1:
            continue
        
        var pos_2d = cam.unproject_position(ss.pos)
        var font: Font = fonts[ss.id]
        var color = COLOR_NORMAL
        font.size = NAME_FONT_SIZE * dec_scale
        if star_selected == ss.id:
            color = COLOR_SELECTED
        var off_x = -len(ss.name)
        draw_string(font, pos_2d + (Vector2(off_x, 7) * dec_scale), ss.name, color)


func _on_Star_input_event(camera, event, click_position, click_normal, shape_idx):
    if not event is InputEventMouseButton:
        return
    if not event.button_index == BUTTON_LEFT:
        return
    star_selected = global.galaxy.get_closest_point(click_position)
