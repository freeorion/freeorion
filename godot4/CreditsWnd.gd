extends Control


func _ready() -> void:
	$Text.text = GlobalFreeOrionNode.get_credits_bbcode()
