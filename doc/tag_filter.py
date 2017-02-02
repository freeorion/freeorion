# Output files only if they contain an element we want to document

import sys
import os

file_name = sys.argv[1]
tag_comment = "@content_tag{"
match_found = False
collected_data = []
buff = []

# dedent doxygen tag documentation
with open(file_name, 'r') as f:
    # sys.stderr.write("checking file: " + file_name + '\n')
    for line in f:
        line = line.rstrip('\n')
        # doxygen wont document this alias if it is indented
        if tag_comment in line:
            line_content = line.split(tag_comment, 1)[1]
            collected_data.append('## ' + tag_comment + line_content)
            match_found = True
        elif collected_data:
            # catch subsequent comment lines to allow for multi-line in python
            if '# ' in line:
                line_content = line.split('# ', 1)[1]
                collected_data.append(line_content)
                buff.append('# \n')  # must replace with same number of lines
            else:
                # finally finish this line
                buff.append(' '.join(collected_data) + '\n')
                collected_data = []
        else:
            buff.append(line + '\n')

# Output the entire file if it contains a match
# entire file is needed for context
if match_found:
    sys.stdout.write('\n'.join(buff))
    sys.stdout.write('\n')
