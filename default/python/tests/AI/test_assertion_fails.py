from freeorion_tools import assertion_fails


def test_does_fail():
    assert assertion_fails(False)


def test_does_not_fail():
    assert not assertion_fails(True)


def test_message_argument():
    assert assertion_fails(False, "")
    assert assertion_fails(False, "Some message")
    assert not assertion_fails(True, "")
    assert not assertion_fails(True, "Some message")
