# API skeletons for FOCS API

This code is not executable, it does nothing in runtime. 

The only purpose of it, is to provide an autocomplete and basic static checking for the FOCS code.

# How it works
FOCS objects are added to globals, so you actually don't need to import it to run parser.

- Named imports (from x import y) : When we import from one of python modules, the name is looked in the globals() and returned.
- Asterisk import (from x import *) works in the same way, just adding all names to the namespace.

# Game limitation
FOCS objects are added to the builtins during only when file with it parsed. In that case we have to use asterisk imports.
For example `from focs._species import *` because Species is not defined when the module is executed.

# Developing
We don't want to add all things at once, so we will add them in timely manner during the conversion process.
