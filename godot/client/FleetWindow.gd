extends "res://FOWindow.gd"


func set_fleet(fleet: global.Fleet):
    var info_text = "Fleet#" + str(fleet.id) + "\n\n"
    if fleet.is_stationary():
        info_text += "Stationed at " + fleet.current_sys.name
    else:
        info_text += "In transit from " + fleet.current_sys.name + " to " + fleet.dest_sys.name + "\n"
        info_text += "Distance travelled: " + str(fleet.dist_travelled)
    
    $FleetInfo.text = info_text
    $FleetInfo.show()


func _on_CloseWidget_pressed():
    hide()
