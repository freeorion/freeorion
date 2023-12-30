from check.focs_dump_parser.make_diff import prettify_dump


def test_handle_negative_number():
    assert prettify_dump("(-(0.500000))") == "-0.500000"
