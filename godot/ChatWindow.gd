extends FOWindow


func _on_CloseWidget_pressed():
	hide()


func _on_FreeOrion_chat_message(text: String, player_name: String, text_color: Color, pm: bool):
	$ChatText.push_color(text_color)
	if pm:
		$ChatText.push_italics()
	if !player_name.empty():
		$ChatText.push_bold()
		$ChatText.add_text(player_name)
	if !player_name.empty():
		$ChatText.pop()
	$ChatText.add_text(text)
	if pm:
		$ChatText.pop()
	$ChatText.pop()
	$ChatText.newline()
