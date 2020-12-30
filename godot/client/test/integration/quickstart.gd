extends "res://addons/gut/test.gd"

var fo = preload("res://gdcpptest.gdns")

class Signaler:
    extends Object
    
    signal started_game
    
    func notify():
        emit_signal("started_game")
    

var signaler: Signaler = Signaler.new()

func test_quickstart():
    assert_not_null(fo)
    assert_true(is_instance_valid(fo))

    assert_not_null(fo.library)
    assert_true(is_instance_valid(fo.library))

    var freeorion = fo.new()
    assert_not_null(freeorion)
    assert_true(is_instance_valid(freeorion))

    var thread = Thread.new()
    thread.start(freeorion, "network_thread")
    
    freeorion.connect("start_game", self, "_on_freeorion_start_game")
    
    freeorion._new_single_player_game(true)
    
    yield(yield_to(signaler, 'started_game', 10), YIELD)
    assert_signal_emitted(signaler, 'started_game', 'Game started')
    
    freeorion._exit_tree()
    
    thread.wait_to_finish()

func _on_freeorion_start_game(_is_new_game: bool):
    gut.p('Game started')
    signaler.notify()
