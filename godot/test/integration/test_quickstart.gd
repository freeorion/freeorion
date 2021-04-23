extends "res://addons/gut/test.gd"

const PASS = 1

var fo = preload("res://freeoriongodot.gdns")


func _quickstart() -> int:
	assert_not_null(fo)
	assert_true(is_instance_valid(fo))

	assert_not_null(fo.library)
	assert_true(is_instance_valid(fo.library))

	return PASS


func test_quickstart():
	assert_eq(_quickstart(), PASS)
