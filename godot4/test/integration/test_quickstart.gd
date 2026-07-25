extends GutTest


func test_quickstart():
	assert_not_null(GlobalFreeOrionNode)
	assert_true(is_instance_valid(GlobalFreeOrionNode))

	assert_not_null(GlobalFreeOrionNode.get_version())
	assert_typeof(GlobalFreeOrionNode.get_version(), TYPE_STRING)

	assert_not_null(GlobalFreeOrionNode.get_user_data_dir())
	assert_typeof(GlobalFreeOrionNode.get_user_data_dir(), TYPE_STRING)
