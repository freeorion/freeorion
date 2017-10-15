from __future__ import print_function
import os
from collections import defaultdict
import fnmatch
import argparse

arguments = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter,
                                    description='Parses files for content tags.  Formulates a new file from a template'
                                                ' and a doxygen suitable listing of the parse results.',
                                    epilog='%(prog)s Parses FILES in the MATCH directory for TAGs.'
                                           'The TEMPLATE file is then read into the OUTPUT file, replacing any'
                                           ' comment lines with a match for TEMPLATE_VAR with the results of the'
                                           ' parsed files. The comment lines may be in either # or /// form in a file,'
                                           ' but only one form throughout a file.  The results of the parse are'
                                           ' formatted for a doxygen ".dox" file.')
arguments.add_argument('--template', required=True,
                       help='Full path to an input template.')
arguments.add_argument('--match', default='./',
                       help='Root directory to search for files to parse.')
arguments.add_argument('-f', '--files', metavar='MASK', nargs='*', default=['*.*'],
                       help='One or more file masks of files to parse.')
arguments.add_argument('-x', '--exclude', metavar='MASK', nargs='*', default=[],
                       help='One or more file masks to exclude from results matching source_mask(s).')
arguments.add_argument('--tag', nargs=2, default=['@content_tag{', '}'], help='Set of opening and closing'
                       ' strings which surround a tag name and precede the tag documentation.')
arguments.add_argument('-t', '--template_var', default='${CONTENT_DOCUMENTATION}',
                       help='String from source file to replace with formatted results from parsed tags.')
arguments.add_argument('-o', '--output', default='output.txt', help='Full path to an output file.')
arguments.add_argument('--link_source', metavar='CONTEXT_DIR BASE_URL LINE_PREFIX]', nargs='*', default=[],
                       help='Display linked source before each tag documentation. Requires arguments in order:'
                       ' CONTEXT_DIR BASE_URL LINE_PREFIX. File sources will link to'
                       ' {BASE_URL}{FILE}{LINE_PREFIX}{LINE_NUM}, where FILE is the relative path of CONTEXT_DIR.'
                       ' e.g. Given: FILE=a/b/c/xyz.txt LINE_NUM=4 CONTEXT_DIR=a/b BASE_URL=http://a.it LINE_PREFIX=?'
                       ' Link result will be: http://a.it/c/xyz.txt?4')
arguments.add_argument('--dry_run', action='store_true', help='Does not write to OUTPUT, instead printing info'
                       ' to stdout.')

args = vars(arguments.parse_args())

tag_open = args.get('tag')[0]
tag_close = args.get('tag')[1]


def add_doc_source(_file_name, _line_number, _content, _tags):
    fn = _file_name.lstrip('./')
    # index documentation by tag name storing the source file name, line number, and documentation string
    _tags[_content[0]].add((fn, _line_number, _content[1].strip()))


def parse_file(_parse_file, _tags):
    with open(_parse_file, 'r') as f:
        match_line = 0
        content = []
        special_comments_this_file = None
        # Documentation may span multiple lines for a content tag.
        # Matched lines are only stored in _tags once an end condition is met.
        # End conditions are: a line not starting a comment, a new content tag, end of file
        for raw_line in enumerate(f):
            if raw_line[1].lstrip().startswith("#") and special_comments_this_file is not True:
                if special_comments_this_file is None:
                    special_comments_this_file = False
                if tag_open in raw_line[1]:
                    if match_line:
                        # in event focs scripts decide to stack one documentation on top of another
                        add_doc_source(_parse_file, match_line, content, _tags)
                    # store content and line for later addition
                    content = ''.join(raw_line[1].split(tag_open, 1)[1]).split(tag_close, 1)
                    content[1] = content[1].strip()
                    match_line = raw_line[0] + 1
                elif match_line:
                    # not a new content tag, append to previous line description
                    content[1] += ' ' + raw_line[1].lstrip('# ')
            elif raw_line[1].lstrip().startswith("/// ") and special_comments_this_file is not False:
                if special_comments_this_file is None:
                    special_comments_this_file = True
                    content = ['# ']
                if tag_open in raw_line[1]:
                    if match_line:
                        # in event focs scripts decide to stack one documentation on top of another
                        add_doc_source(_parse_file, match_line, content, _tags)
                    # store content and line for later addition
                    content = ''.join(raw_line[1].split(tag_open, 1)[1]).split(tag_close, 1)
                    content[1] = content[1].strip()
                    match_line = raw_line[0] + 1
                elif match_line:
                    # not a new content tag, append to previous line description
                    content[1] += ' ' + raw_line[1].lstrip('/// ')
            elif match_line:
                # end of description, add a node for this source
                add_doc_source(_parse_file, match_line, content, _tags)
                match_line = 0
                content = []
        # EOF
        if match_line:
            # in event a matched comment is the last line in a file
            add_doc_source(_parse_file, match_line, content, _tags)


def get_link(_file, _line_number):
    link_args = args.get('link_source')
    if len(link_args) != 3:
        if link_args:
            print('Invalid link_source arguments: ', link_args)
        return ''
    base_path = os.path.relpath(_file, link_args[0].lstrip('./'))
    link = os.path.join(link_args[1], base_path)
    link += str.format('{0}{1}', link_args[2], _line_number)
    return str.format('[{0}]({1})', base_path, link)


all_tags = defaultdict(set)

# parse files for tags
for _file_path, _, _files in os.walk(args.get('match')):
    for file_name in enumerate(_files):
        file_match = False
        # if this file should be checked
        for mask in args.get('files'):
            if fnmatch.fnmatch(file_name[1], mask):
                file_match = True
        if file_match:
            # check for exclusions
            for mask in args.get('exclude'):
                if fnmatch.fnmatch(file_name[1], mask):
                    file_match = False
            if file_match:
                parse_file(os.path.join(_file_path, file_name[1]), all_tags)

output_buff = []

# read in template, replacing template_var with tags formatted for doxygen
with open(args.get('template'), 'r') as input_file:
    for input_line in input_file:
        if args.get('template_var') in input_line:
            output_buff.append('<dl class="reflist">\n')
            for tag in sorted(all_tags):
                output_buff.append('<dt>' + tag + '</dt>\n<dd>')
                for source in all_tags[tag]:
                    description = str.format('{0}  ', source[2])
                    # prepend source and link if requested
                    link_str = get_link(source[0], source[1])
                    if link_str:
                        link_str += " - "
                    output_buff.append(link_str + description + '<br />\n')
                output_buff.append('</dd>\n')
            output_buff.append('</dl>\n')
        else:
            output_buff.append(input_line)

if args.get('dry_run'):
    source_count = 0
    for tag in all_tags:
        for value in all_tags[tag]:
            source_count += 1
    print("arguments:")
    for arg in args:
        print("\t", arg, ":", args[arg])
    print("\nFound {0} tags with {1} total sources".format(len(all_tags.keys()), source_count))
else:
    abs_dir = os.path.dirname(os.path.abspath(args.get('output')))
    if not os.path.exists(abs_dir):
        os.mkdir(abs_dir)
    with open(args.get('output'), 'w') as output_file:
        output_file.writelines(output_buff)
