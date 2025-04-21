extends "res://addons/gut/test.gd"

const PASS = 1

var signaler: Signaler = Signaler.new()


class Signaler:
	extends Object

	signal started_game

	signal parsing_completed

	func notify(signal_name: String):
		emit_signal(signal_name)


func test_quickstart():
	assert_not_null(FreeOrionNode)
	assert_true(is_instance_valid(FreeOrionNode))

	assert_not_null(FreeOrionNode.get_version())
	assert_typeof(FreeOrionNode.get_version(), TYPE_STRING)

	assert_not_null(FreeOrionNode.get_user_data_dir())
	assert_typeof(FreeOrionNode.get_user_data_dir(), TYPE_STRING)

	FreeOrionNode.connect("parsing_completed", self, "_on_freeorion_parsing_completed")

	FreeOrionNode.start_parsing_thread()

	yield(yield_to(signaler, "parsing_completed", 5000), YIELD)
	assert_signal_emitted(signaler, "parsing_completed", "Parsing completed")

	FreeOrionNode.start_network_thread()

	gut.p(FreeOrionNode.get_user_data_dir())

	FreeOrionNode.connect("start_game", self, "_on_freeorion_start_game")

	FreeOrionNode.new_single_player_game()

	yield(yield_to(signaler, "started_game", 200), YIELD)
	assert_signal_emitted(signaler, "started_game", "Game started")

	var systems = FreeOrionNode.get_systems()
	assert_not_null(systems)
	assert_typeof(systems, TYPE_DICTIONARY)
	assert_gt(systems.size(), 0)
	var system = systems[systems.keys()[0]]
	assert_not_null(system)
	assert_typeof(system, TYPE_OBJECT)
	assert_typeof(system.id, TYPE_INT)
	assert_typeof(system.name, TYPE_STRING)
	assert_typeof(system.pos, TYPE_VECTOR3)
	assert_typeof(system.get_starlanes_wormholes(), TYPE_DICTIONARY)

	FreeOrionNode.free()
	signaler.free()

	assert_no_new_orphans()


func _on_freeorion_start_game(_arg1):
	signaler.notify("started_game")


func _on_freeorion_parsing_completed():
	signaler.notify("parsing_completed")
