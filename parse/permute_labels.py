#!/usr/bin/python

import sys
import re

regex = re.compile(r'(\S+) +(=)( +)')
filename = sys.argv[1]

lines = open(filename).readlines()

for line in lines:
    line_copy = line[0:-1]
    cut_ranges = []
    for match in regex.finditer(line_copy):
        cut_ranges.append((match.start(1), match.start(3)))
    if len(cut_ranges):
        for i in range(0, 2 ** len(cut_ranges)):
            line_copy = line[0:cut_ranges[0][0]]
            k = 2 ** (len(cut_ranges) - 1)
            for j in range(0, len(cut_ranges) - 1):
                if not ((i / k) % 2):
                    line_copy += line[cut_ranges[j][0]:cut_ranges[j][1]]
                line_copy += line[cut_ranges[j][1]:cut_ranges[j + 1][0]]
                k = k / 2
            if not ((i / k) % 2):
                line_copy += line[cut_ranges[-1][0]:cut_ranges[-1][1]]
            line_copy += line[cut_ranges[-1][1]:-1]
            line_copy = re.sub(r'   +', r'  ', line_copy)
            line_copy = re.sub(r'=  +', r'= ', line_copy)
            print line_copy
    else:
        print line_copy

