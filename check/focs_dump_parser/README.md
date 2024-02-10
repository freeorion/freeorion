This is a tool to help converting FOCS from the text files to Python.

It allows to parse a dump from the game logs and save it as files. After that you could make a diff on these files.


Pre-requirements:

- Install game and set repo as folder for content
- Run game, go to settings, choose logs tab, set `parsing` to trace

Get a diff
- Checkout to master, run game, start game, and exit from it. (log file is created)
- Run `parse_buildings.py` (dump of FOCS is created in a folder dumps/master)
- Checkout your branch, run game, start game, and exit from it. (log file is overwritten)
- Run `parse_buildings.py` (dump of FOCS is created in a folder dumps/<your-branch>, diff is printed to console)


Only buildings are supported so far.