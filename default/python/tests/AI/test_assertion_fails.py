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


def test_logger_argument():
    import logging
    loggers = [logging.debug, logging.info, logging.warn, logging.error]
    for logger in loggers:
        assert assertion_fails(False, logger=logger)
        assert assertion_fails(False, "Some message", logger=logger)
        assert not assertion_fails(True, logger=logger)
