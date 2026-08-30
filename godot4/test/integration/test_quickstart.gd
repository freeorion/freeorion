extends GutTest

var signaler: Signaler = Signaler.new()


class Signaler:
	extends Object

	signal started_game

	signal parsing_completed

	func notify(signal_name: String):
		emit_signal(signal_name)


func test_quickstart():
	assert_not_null(GlobalFreeOrionNode)
	assert_true(is_instance_valid(GlobalFreeOrionNode))

	assert_not_null(GlobalFreeOrionNode.get_version())
	assert_typeof(GlobalFreeOrionNode.get_version(), TYPE_STRING)

	assert_not_null(GlobalFreeOrionNode.get_user_data_dir())
	assert_typeof(GlobalFreeOrionNode.get_user_data_dir(), TYPE_STRING)

	GlobalFreeOrionNode.parsing_completed.connect(_on_freeorion_parsing_completed)

	GlobalFreeOrionNode.start_parsing_thread()

	await wait_for_signal(signaler.parsing_completed, 5.0, "Parsing completed")
	assert_signal_emitted(signaler, "parsing_completed", "Parsing completed")

	GlobalFreeOrionNode.start_network_thread()

	gut.p(GlobalFreeOrionNode.get_user_config_dir())

	GlobalFreeOrionNode.start_game.connect(_on_freeorion_start_game)

	GlobalFreeOrionNode.new_single_player_game()

	await wait_for_signal(signaler.started_game, 5.0, "Start game")
	assert_signal_emitted(signaler, "started_game", "Start game")

	GlobalFreeOrionNode.queue_free()
	signaler.free()

	assert_no_new_orphans()


func _on_freeorion_parsing_completed():
	signaler.notify("parsing_completed")


func _on_freeorion_start_game(_arg1):
	signaler.notify("started_game")
