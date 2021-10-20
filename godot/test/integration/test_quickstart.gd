extends "res://addons/gut/test.gd"

const PASS = 1

var signaler: Signaler = Signaler.new()


class Signaler:
	extends Object

	signal started_game

	func notify():
		emit_signal("started_game")


func test_quickstart():
	assert_not_null(FreeOrionNode)
	assert_true(is_instance_valid(FreeOrionNode))

	assert_not_null(FreeOrionNode.get_version())
	assert_typeof(FreeOrionNode.get_version(), TYPE_STRING)

	FreeOrionNode.connect("start_game", self, "_on_freeorion_start_game")

	FreeOrionNode.new_single_player_game()

	yield(yield_to(signaler, "started_game", 100), YIELD)
	assert_signal_emitted(signaler, "started_game", "Game started")

	FreeOrionNode.free()
	signaler.free()

	assert_no_new_orphans()


func _on_freeorion_start_game(_arg1):
	signaler.notify()
