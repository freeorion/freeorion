extends "res://addons/gut/test.gd"

const PASS = 1

var fo = preload("res://freeoriongodot.gdns")


func _quickstart() -> int:
	assert_not_null(fo)
	assert_true(is_instance_valid(fo))

	assert_not_null(fo.library)
	assert_true(is_instance_valid(fo.library))

	var freeorion = add_child_autoqfree(fo.new())

	assert_not_null(freeorion.get_version())
	assert_typeof(freeorion.get_version(), TYPE_STRING)

	return PASS


func test_quickstart():
	assert_eq(_quickstart(), PASS)
	assert_no_new_orphans()
