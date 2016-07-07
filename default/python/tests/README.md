# Test runner

This test are supposed to be run outside the game engine.

## Requirements
 - install [pytest](http://pytest.org/latest/getting-started.html)

## Run test
 - open console (`cmd` or `PowerShell` for win)
 - change dir to `freeorion\default\python\tests`
 - execute  `run_tests.py` with python
   - `python run_tests.py` for windows
   - `./run_tests.py` for linux and mac

 `run_tests.py` accepts same commandline arguments as pytest. I prefer to add `-v --tb long`
 
 `run_tests.py` adds required directories to python path and runs pytest.  
