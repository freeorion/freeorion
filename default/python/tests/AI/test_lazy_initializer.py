import pytest
from freeorion_tools.lazy_initializer import InitializerLock


@pytest.fixture
def lock():
    lock = InitializerLock("test")
    return lock


def test_function_return_value_when_unlocked(lock):
    @lock
    def foo():
        return 1

    lock.unlock()
    assert foo() == 1


def test_function_call_raise_error_when_locked(lock):
    @lock
    def foo():
        return 1

    with pytest.raises(ValueError, match=r"Call of 'foo' is forbidden before InitializerLock\('test'\) is initialized"):
        foo()
