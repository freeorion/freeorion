#!/usr/bin/python

# This script expects the output of "parser_test lexer -f <file>" as its
# input.  Its output is all unique token prefixes of the lines in the input.

import sys

lines = open(sys.argv[1]).readlines()

all_partial_lines = set([])
partial_line = ""
for i in range(0, len(lines)):
    line = lines[i]
    if "Successful parse." in line:
        partial_line = ""
    elif "All parses successful." in line:
        pass
    else:
        if len(partial_line) == 0:
            partial_line = line[0:-1]
        else:
            partial_line += ' ' + line[0:-1]
        if not "Successful parse." in lines[i + 1] and not "All parses successful." in lines[i + 1] and not partial_line in all_partial_lines:
            print partial_line
            all_partial_lines.add(partial_line)
