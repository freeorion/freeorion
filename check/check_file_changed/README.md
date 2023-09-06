# Code for detecting which workflow should be run based on file changed.

 - `set_output.py` script to be run in CI, also could be run locally for testing
 - `check_coverage.py` script for doing a local check for consistence and new files
 - `_file_checker` library itself
 - `config.toml` file with file patterns and file groups - workflow relations. See header for more details.

The idea behind this code is to optimize resources usage and run only workflows that have sense for changes.

To run script you could use Python 3.11 or install python lib for toml. `pip install tomlkit`
