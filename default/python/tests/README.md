# Test runner

This test are supposed to be run outside the game engine.

## Requirements
 - install [pytest](http://pytest.org/latest/getting-started.html)

## Run test
 - open console (`cmd` or `PowerShell` for win)
 - execute  `pytest`

If you want to run specific tests see [pytest documentation](https://docs.pytest.org/en/latest/usage.html#specifying-tests-selecting-tests)

If you need to test module that is not in python path, add its to  `conftest.py`,
this file is executed before import of the test files. 
