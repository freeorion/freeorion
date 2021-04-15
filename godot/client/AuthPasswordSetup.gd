extends FOWindow


signal ok
signal cancel



var player_name: String
var password: String

func set_player_name(player_name: String):
    self.player_name = player_name
    $LeftContainer/PlayerName/LineEdit.text = player_name

func _on_CloseWidget_pressed():
    emit_signal("cancel")


func _on_OKBtn_pressed():
    player_name = $LeftContainer/PlayerName/LineEdit.text
    password = $LeftContainer/Password/LineEdit.text
    emit_signal("ok")


func _on_CancelBtn_pressed():
    emit_signal("cancel")
