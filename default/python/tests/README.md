# Test runner

These tests are supposed to be run outside the game engine.
Tests run automatically on Travis for each PR
and can be executed on the local machine.

## Requirements for local run
 - Install [pytest](http://pytest.org/latest/getting-started.html),
   use the same version as mentioned in [.travis.yml](/.travis.yml#L1).

## Run test
 - open console (`cmd` or `PowerShell` for win)
 - execute  `pytest`

If you want to run specific tests see
[pytest documentation](https://docs.pytest.org/en/latest/usage.html#specifying-tests-selecting-tests)

If you need to test a module that is not in the python path,
add it to [conftest.py](conftest.py#L1),
this file is executed before import of the test files.
