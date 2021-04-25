extends "res://FOWindow.gd"


func set_fleet(fleet: Object):
    var info_text = "Fleet#" + str(fleet.id) + "\n\n"
    if fleet.is_stationary():
        info_text += "Stationed"
    else:
        info_text += "In transit"
    
    $FleetInfo.text = info_text
    $FleetInfo.show()


func _on_CloseWidget_pressed():
    hide()
