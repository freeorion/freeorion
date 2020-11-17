extends LineEdit


func _ready():
    pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#	pass


func _on_RandomSeedBtn_pressed():
    var chars = global.LETTER_UPPER + global.LETTER_LOWER + global.LETTER_DIGITS
    var s = ""
    for _i in range(10):
        s += chars[randi() % len(chars)]
    text = s
