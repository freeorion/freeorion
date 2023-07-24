extends FOWindow

signal ok


func set_error_text(error: String):
	$LeftContainer/Label.text = error


func _on_OKBtn_pressed():
	emit_signal("ok")


func _on_CloseWidget_pressed():
	emit_signal("ok")
