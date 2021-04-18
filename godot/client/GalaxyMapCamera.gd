extends Camera


const ANGLE_V_LIMIT_LOW: float = 0.1
const ANGLE_V_LIMIT_HIGH: float = PI / 2 - 0.05
const MOVING_SPEED: float = 0.1
const PANNING_SPEED: float = 0.02
const ZOOM_SPEED: float = 0.05


var origin = Vector3(0, 0, 0)
var angle_h: float = 0.0
var angle_v: float = 0.5
var dist: float = 50.0

var moving = false
var panning = false


signal updated(origin, angle_h, angle_v, dist)


func _ready():
    update_position()


func _input(event):
    if event is InputEventMouseButton:
        match event.button_index:
            BUTTON_MIDDLE:
                panning = event.pressed
            BUTTON_LEFT:
                moving = event.pressed
            BUTTON_WHEEL_UP:
                dist -= dist * ZOOM_SPEED
                update_position()
            BUTTON_WHEEL_DOWN:
                dist += dist * ZOOM_SPEED
                update_position()
    
    elif event is InputEventMouseMotion:
        if panning:
            var mouse_movement = event.relative
            angle_h += mouse_movement.x * PANNING_SPEED
            angle_v += mouse_movement.y * PANNING_SPEED
            update_position()
        
        if moving:
            var v = translation.direction_to(origin)
            var up = Vector3(v.x, 0, v.z).normalized() * (dist / 60.0)
            var side = up.rotated(Vector3(0, 1, 0), PI / 2).normalized() * (dist / 60.0)
            origin = origin + ((up * event.relative.y * MOVING_SPEED) + (side * event.relative.x * MOVING_SPEED))
            update_position()


func update_position():
    angle_h = fmod(angle_h, 2 * PI)
    angle_v = clamp(angle_v, ANGLE_V_LIMIT_LOW, ANGLE_V_LIMIT_HIGH)
    dist = clamp(dist, 1, 1000)

    var f = cos(angle_v)
    var x = dist * cos(angle_h) * f
    var y = dist * sin(angle_v)
    var z = dist * sin(angle_h) * f
    look_at_from_position(origin + Vector3(x, y, z), origin, Vector3(0, 1, 0))
    emit_signal("updated", translation, angle_h, angle_v, dist)
