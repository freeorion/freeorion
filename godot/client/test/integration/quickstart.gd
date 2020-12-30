extends "res://addons/gut/test.gd"

var fo = preload("res://gdcpptest.gdns")

func test_quickstart():
    assert_not_null(fo)
    assert_true(is_instance_valid(fo))
    
    assert_not_null(fo.library)
    assert_true(is_instance_valid(fo.library))
    
    var freeorion = fo.new()
    assert_not_null(freeorion)
    assert_true(is_instance_valid(freeorion))
