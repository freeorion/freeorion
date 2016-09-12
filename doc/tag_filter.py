# Output files only if they contain an element we want to document

import sys
import os

file_name = sys.argv[1]
file_obj = open(file_name, 'r')

tag_comment = "content_tag{"
tag_match = 0

# check for match
line = file_obj.readline()
while line and not tag_match:
    if tag_comment in line:
        tag_match = 1
    line = file_obj.readline()
file_obj.close()

# output file
file_obj = open(file_name, 'r')
line = file_obj.readline()
while line and tag_match:
    # doxygen wont document this alias if it is indented
    if tag_comment in line:
        line = line.strip() + '\n'
    sys.stdout.write(line)
    line = file_obj.readline()
file_obj.close()
