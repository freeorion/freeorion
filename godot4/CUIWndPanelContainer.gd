extends PanelContainer

const STYLE := preload("res://assets/resources/WndOuter.tres")
const FOCUS_STYLE := preload("res://assets/resources/WndOuterFocus.tres")

var _popup_open := false
var _modal := false


func _ready() -> void:
	get_viewport().gui_focus_changed.connect(_on_viewport_gui_focus_changed)
	visibility_changed.connect(_update_style)
	_update_style()


func set_popup_open(popup_open: bool) -> void:
	_popup_open = popup_open
	_update_style()


func set_modal(modal: bool) -> void:
	_modal = modal
	_update_style()


func _on_viewport_gui_focus_changed(_control: Control) -> void:
	_update_style()


func _update_style() -> void:
	# highlight this window if it or its descenant focused
	var focused := _modal
	if not focused and not _popup_open:
		var focus_owner := get_viewport().gui_get_focus_owner()
		focused = focus_owner != null and (focus_owner == self or is_ancestor_of(focus_owner))
	add_theme_stylebox_override("panel", FOCUS_STYLE if focused else STYLE)
