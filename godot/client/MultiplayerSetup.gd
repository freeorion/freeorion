extends FOWindow


signal ok
signal cancel



var player_name: String
var client_type: int
var server_name: String


func _on_CloseWidget_pressed():
    emit_signal("cancel")

func _on_RefreshBtn_pressed():
    pass

func _on_OKBtn_pressed():
    player_name = $LeftContainer/PlayerName/LineEdit.text
    client_type = $LeftContainer/JoinGame/ClientType.get_selected_id()
    if $LeftContainer/JoinGame/CheckButton.pressed:
        server_name = $LeftContainer/ServerName.text
    else:
        server_name = "HOST GAME SELECTED"
    emit_signal("ok")


func _on_CancelBtn_pressed():
    emit_signal("cancel")


func _on_JoinCheckButton_toggled(button_pressed: bool):
    $LeftContainer/JoinGame/ClientType.disabled = !button_pressed
