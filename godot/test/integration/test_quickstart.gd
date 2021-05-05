extends "res://addons/gut/test.gd"

const PASS = 1

var fo = preload("res://freeoriongodot.gdns")
var signaler: Signaler = Signaler.new()


class Signaler:
	extends Object

	signal started_game

	func notify():
		emit_signal("started_game")


func test_quickstart():
	assert_not_null(fo)
	assert_true(is_instance_valid(fo))

	assert_not_null(fo.library)
	assert_true(is_instance_valid(fo.library))

	var freeorion = add_child_autoqfree(fo.new())

	assert_not_null(freeorion.get_version())
	assert_typeof(freeorion.get_version(), TYPE_STRING)

	var thread = autofree(Thread.new())
	thread.start(freeorion, "network_thread")

	freeorion.connect("start_game", self, "_on_freeorion_start_game")

	freeorion.new_single_player_game()

	yield(yield_to(signaler, 'started_game', 10), YIELD)
	assert_signal_emitted(signaler, 'started_game', 'Game started')

	remove_child(freeorion)

	thread.wait_to_finish()
	freeorion.free()
	signaler.free()


func _on_freeorion_start_game(_arg1):
	signaler.notify()
