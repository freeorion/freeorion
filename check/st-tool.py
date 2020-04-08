#!/usr/bin/python3
import argparse
import datetime
import errno
import os
import re
import subprocess
import sys
from collections import namedtuple, OrderedDict

STRING_TABLE_KEY_PATTERN = r'^[A-Z0-9_]+$'
# Provides the named capture groups 'ref_type' and 'key'
INTERNAL_REFERENCE_PATTERN = r'\[\[(?P<ref_type>(?:(?:encyclopedia|(?:building|field|meter)type|predefinedshipdesign|ship(?:hull|part)|special|species|tech) )?)(?P<key>{})\]\]'
# Provides the named capture groups 'sha', 'old_line', 'new_line' and 'range'
GIT_BLAME_INCREMENTAL_PATTERN = r'^(?P<sha>[0-9a-f]{40}) (?P<old_line>[1-9][0-9]*) (?P<new_line>[1-9][0-9]*)(?P<range> [1-9][0-9]*)?$'

class StringTableEntry(object):
    def __init__(self, key, keyline, value, notes, value_times):
        if not re.match(STRING_TABLE_KEY_PATTERN, key):
            raise ValueError("Key doesn't match expected format [A-Z0-9_]+, was '{}'".format(key))
        self.key = key
        self.keyline = keyline
        self.value = value
        self.value_times = value_times
        self.notes = notes

    def __str__(self):
        result = ""
        for note in self.notes:
            result += '# ' + note + "\n"
        if self.value is not None:
            result += self.key + "\n"
            if not self.value or '\n' in self.value or re.search(r'^\s', self.value) or re.search(r'\s$', self.value):
                result += "'''" + self.value + "'''" + "\n"
            else:
                result += self.value + "\n"
        else:
            result += "#*" + self.key + "\n"
            for e in self.value_times:
                result += "#* <not translated {}>\n".format(
                    datetime.datetime.fromtimestamp(e).isoformat(sep=' '))

        return result

    def __repr__(self):
        return "StringTableEntry(key={}, keyline={}, value={})".format(self.key, self.keyline, self.value)


class StringTable(object):
    def __init__(self, fpath, language, notes, includes, entries):
        self.fpath = fpath
        self.language = language
        self.notes = notes
        self.includes = includes
        self._entries = entries
        self._ientries = OrderedDict()

        for entry in self._entries:
            if isinstance(entry, StringTableEntry):
                if entry.key in self._ientries:
                    msg = "{fpath}:{fline}: Duplicate key '{key}', previously defined on line {pline}"
                    msg = msg.format(**{
                        'fpath': self.fpath,
                        'fline': entry.keyline,
                        'key': entry.key,
                        'pline': self._ientries[entry.key].keyline,
                    })
                    raise ValueError(msg)
                self._ientries[entry.key] = entry

    def __getitem__(self, key):
        return self._ientries[key]

    def __contains__(self, key):
        return key in self._ientries

    def __iter__(self):
        return self._ientries.iteritems()

    def __len__(self):
        return len(self._ientries)

    def __str__(self):
        result = ""
        result += self.language + "\n"

        if self.notes:
            result += "\n"
        for note in self.notes:
            if note:
                result += "# " + note + "\n"
            else:
                result += "#\n"

        for entry in self._entries:
            if isinstance(entry, StringTableEntry):
                result += "\n"
                result += str(entry)
            elif isinstance(entry, list):
                result += "\n"
                result += "\n"
                result += "##\n"
                for line in entry:
                    if line:
                        result += "## " + line + "\n"
                    else:
                        result += "##\n"
                result += "##\n"

        if self.includes:
            result += "\n"

        for include in self.includes:
            result += "#include \"" + include + "\"\n"

        return result

    def __repr__(self):
        return "StringTable(fpath={}, language={}, entries={})".format(
            self.fpath, self.language, self.entries)

    def items(self):
        return self._entries

    def keys(self):
        return self._ientries.keys()

    @staticmethod
    def from_file(fhandle):
        fpath = fhandle.name

        is_quoted = False

        language = None
        fnotes = []
        section = []
        entries = []
        includes = []
        blames = OrderedDict();

        key = None
        keyline = None
        value = None
        vline_start = 0
        vline_end = 0
        notes = []
        untranslated_key = None
        untranslated_keyline = None
        untranslated_lines = []

        for fline, line in enumerate(fhandle, 1):
            if not is_quoted and not line.strip():
                # discard empty lines when not in quoted value
                if section:
                    while not section[0]:
                        section.pop(0)
                    while not section[-1]:
                        section.pop(-1)
                    entries.append(section)
                    section = []
                if language and notes and not entries:
                    fnotes = notes
                    notes = []
                if untranslated_key:
                    try:
                        entries.append(StringTableEntry(untranslated_key, untranslated_keyline, None, notes, untranslated_lines))
                    except ValueError as e:
                        raise ValueError("{}:{}: {}".format(fpath, keyline, str(e)))
                    untranslated_key = None
                    untranslated_keyline = None
                    notes = []
                    untranslated_lines = []
                vline_start = vline_end = fline
                continue

            if not is_quoted and line.startswith('#'):
                # discard comments when not in quoted value
                if line.startswith('#include'):
                    includes.append(line[8:].strip()[1:-1].strip())
                if line.startswith('##'):
                    section.append(line[3:].rstrip())
                elif line.startswith('#*') and not untranslated_key:
                    untranslated_key = line[2:].strip()
                    untranslated_keyline = fline
                elif line.startswith('#*'):
                    untranslated_lines.append(0)
                elif line.startswith('#'):
                    notes.append(line[2:].rstrip())
                vline_start = vline_end = fline
                continue

            if not language:
                # read first non comment and non empty line as language name
                language = line.strip()
                vline_start = vline_end = fline
                continue

            if not key:
                if section:
                    while not section[0]:
                        section.pop(0)
                    while not section[-1]:
                        section.pop(-1)
                    entries.append(section)
                    section = []
                if language and notes and not entries:
                    fnotes = notes
                    notes = []
                if untranslated_key:
                    try:
                        entries.append(StringTableEntry(untranslated_key, untranslated_keyline, None, notes, untranslated_lines))
                    except ValueError as e:
                        raise ValueError("{}:{}: {}".format(fpath, keyline, str(e)))
                    untranslated_key = None
                    untranslated_keyline = None
                    notes = []
                    untranslated_lines = []
                # read non comment and non empty line after language and after
                # completing an stringtable language
                key = line.strip().split()[0]
                keyline = fline
                vline_start = vline_end = fline
                continue

            if not is_quoted:
                if line.startswith("'''"):
                    # prepare begin of quoted value
                    line = line[3:]
                    is_quoted = True
                else:
                    # prepare unquoted value
                    line = line.strip()
                    vline_end = fline
                vline_start = fline

            if is_quoted and line.rstrip().endswith("'''"):
                # prepare end of quoted value
                line = line.rstrip()[:-3]
                is_quoted = False
                vline_end = fline

            # extract value or concatenate quoted value
            value = (value if value else '') + line

            if not is_quoted and key is not None and value is not None:
                # consume collected string table entry
                try:
                    value_range = ({line: key for line in range(vline_start, vline_end + 1)})
                    entries.append(StringTableEntry(key, keyline, value, notes, [0] * len(value_range)))
                    blames.update(value_range)
                except ValueError as e:
                    raise ValueError("{}:{}: {}".format(fpath, keyline, str(e)))

                key = None
                keyline = None
                value = None
                notes = []
                vline_start = vline_end = fline
                continue

            if is_quoted:
                # continue consuming quoted value lines
                vline_end = fline
                continue

            raise ValueError("{}:{}: Impossible parser state; last valid line: {}".format(fpath, fline_start, line))

        if key:
            raise ValueError("{}:{}: Key '{}' without value".format(fpath, vline_start, key))

        if is_quoted:
            raise ValueError("{}:{}: Quotes not closed for key '{}'".format(fpath, vline_start, key))

        blame_cmd = ['git', 'blame', '--incremental', fpath]
        git_blame = subprocess.check_output(blame_cmd)
        git_blame = git_blame.decode('utf-8', 'strict')
        git_blame = git_blame.splitlines(True)

        value_times = {}
        author_time = None
        line = None
        line_range = None

        for line_blame in git_blame:
            match = re.match(GIT_BLAME_INCREMENTAL_PATTERN, line_blame)
            if match:
                line = int(match['new_line'])
                line_range = int(match['range'])
            if line_blame.startswith('author-time '):
                author_time = int(line_blame.strip().split(' ')[1])
            if line_blame.startswith('filename '):
                for line in range(line, line + line_range):
                    if line not in blames:
                        continue
                    line_times = value_times.get(blames[line], [])
                    line_times.append(author_time)
                    value_times[blames[line]] = line_times

        for entry in entries:
            if isinstance(entry, StringTableEntry):
                if not entry.value:
                    # ignore untranslated entries
                    continue
                if entry.key not in value_times or len(value_times[entry.key]) != len(entry.value_times):
                    raise RuntimeError("{}: git blame did not collect any matching author times for key {}".format(fpath, entry.key))
                entry.value_times = value_times[entry.key]

        return StringTable(fpath, language, fnotes, includes, entries)

    @staticmethod
    def statistic(left, right):
        STStatistic = namedtuple('STStatistic', [
            'left',
            'left_only',
            'left_older',
            'left_pure_reference',
            'right',
            'right_only',
            'right_older',
            'right_pure_reference',
            'untranslated',
            'identical',
            'layout_mismatch'])

        all_keys = set(left.keys()).union(right.keys())

        untranslated = set()
        identical = set()
        layout_mismatch = set()
        left_only = set()
        right_only = set()
        left_older = set()
        right_older = set()
        left_pure_reference = set()
        right_pure_reference = set()

        for key in all_keys:
            if key in left and key not in right:
                left_only.add(key)
            if key not in left and key in right:
                right_only.add(key)
            if key in left and key in right:
                if not left[key].value or not right[key].value:
                    untranslated.add(key)
                    continue
                if left[key].value == right[key].value:
                    identical.add(key)
                if (left[key].value.startswith('[[') and 2 == left[key].value.count('[') and
                    left[key].value.endswith(']]') and 2 == left[key].value.count(']')):
                    left_pure_reference.add(key)
                if (right[key].value.startswith('[[') and 2 == right[key].value.count('[') and
                    right[key].value.endswith(']]') and 2 == right[key].value.count(']')):
                    right_pure_reference.add(key)
                if len(left[key].value_times) != len(right[key].value_times):
                    layout_mismatch.add(key)
                    continue
                value_times = list(zip(left[key].value_times, right[key].value_times))
                if any([e[0] > e[1] for e in value_times]):
                    right_older.add(key)
                if any([e[0] < e[1] for e in value_times]):
                    left_older.add(key)

        return STStatistic(
            left=left, left_only=left_only, left_older=left_older,
            right=right, right_only=right_only, right_older=right_older,
            left_pure_reference=left_pure_reference,
            right_pure_reference=right_pure_reference,
            untranslated=untranslated, identical=identical,
            layout_mismatch=layout_mismatch)

    @staticmethod 
    def sync(reference, source):
        entries = []

        for item in reference.items():
            if isinstance(item, StringTableEntry):
                if item.key in source:
                    source_item = source[item.key]
                    entries.append(StringTableEntry(
                        item.key,
                        item.keyline,
                        source_item.value if source_item.value != item.value else None,
                        item.notes,
                        source_item.value_times
                    ))
                else:
                    entries.append(StringTableEntry(
                        item.key,
                        item.keyline,
                        None,
                        item.notes,
                        [0] * len(item.value_times)
                    ))
            else:
                entries.append(item)

        return StringTable(source.fpath, source.language, source.notes, source.includes, entries)


def format_action(args):
    source_st = StringTable.from_file(args.source)

    print(source_st, end='')

    return 0


def sync_action(args):
    reference_st = StringTable.from_file(args.reference)
    source_st = StringTable.from_file(args.source)

    entries = []

    for item in reference_st.items():
        if isinstance(item, StringTableEntry):
            if item.key in source_st:
                source_item = source_st[item.key]
                entries.append(StringTableEntry(
                    item.key,
                    item.keyline,
                    source_item.value,
                    item.notes,
                    source_item.value_times
                ))
            else:
                entries.append(StringTableEntry(
                    item.key,
                    item.keyline,
                    None,
                    item.notes,
                    item.value_times
                ))
        else:
            entries.append(item)

    out_st = StringTable(source_st.fpath, source_st.language, source_st.notes, source_st.includes, entries)

    print(out_st, end='')

    return 0


def rename_key_action(args):
    if not re.match(STRING_TABLE_KEY_PATTERN, args.old_key):
        raise ValueError("The given old key '{}' is not a valid name".format(args.old_key))

    if not re.match(STRING_TABLE_KEY_PATTERN, args.new_key):
        raise ValueError("The given new key '{}' is not a valid name".format(args.new_key))

    source_st = StringTable.from_file(args.source)

    for key in source_st.keys():
        entry = source_st[key]
        entry.value = re.sub(
            INTERNAL_REFERENCE_PATTERN.format(re.escape(args.old_key)),
            r'[[\1' + args.new_key + ']]',
            entry.value)
        if entry.key == args.old_key:
            entry.key = args.new_key

    print(source_st, end='')

    return 0


def check_action(args):
    exit_code = 0
    reference_st = None
    if args.reference:
        reference_st = StringTable.from_file(args.reference)
    for source in args.sources:
        source_st = StringTable.from_file(source)

        for key in source_st.keys():
            entry = source_st[key]

            for match in re.finditer(INTERNAL_REFERENCE_PATTERN.format('.*?'), entry.value):
                match = match['key']
                if not (match in source_st.keys() or (reference_st and match in reference_st.keys())):
                    print("{}:{}: Referenced key '{}' in value of '{}' was not found.".format(source_st.fpath, entry.keyline, match, entry.key))
                    exit_code = 1

    return exit_code


def compare_action(args):
    exit_code = 0
    reference_st = StringTable.from_file(args.reference)
    source_st = StringTable.from_file(args.source)

    comp = StringTable.statistic(reference_st, source_st)

    if not args.summary_only:
        for key in source_st.keys():
            if key in comp.right_only:
                print("{}:{}: Key '{}' is not in reference file {}".format(comp.right.fpath, comp.right[key].keyline, key, comp.left.fpath))
                exit_code = 1

            if key in comp.left_pure_reference:
                print("{}:{}: Key '{}' contains translation for pure reference in reference file value {}:{}".format(comp.right.fpath, comp.right[key].keyline, key, comp.left.fpath, comp.left[key].keyline))
                exit_code = 1

            if key in comp.layout_mismatch:
                print("{}:{}: Value of key '{}' layout does not match that of reference file value {}:{}".format(comp.right.fpath, comp.right[key].keyline, key, comp.left.fpath, comp.left[key].keyline))
                exit_code = 1

            if key in comp.right_older:
                print("{}:{}: Value of key '{}' is older than reference file value {}:{}".format(comp.right.fpath, comp.right[key].keyline, key, comp.left.fpath, comp.left[key].keyline))
                exit_code = 1

            if key in comp.untranslated:
                print("{}:{}: Value of key '{}' has no translation compared to reference file {}:{}".format(comp.right.fpath, comp.right[key].keyline, key, comp.left.fpath, comp.left[key].keyline))
                exit_code = 1

            if key in comp.identical:
                print("{}:{}: Value of key '{}' is identical to reference file {}:{}".format(comp.right.fpath, comp.right[key].keyline, key, comp.left.fpath, comp.left[key].keyline))
                exit_code = 1

    print("""
Summary comparing '{}' against '{}':
    Keys translated - {}/{} ({:3.1f}%)
    Keys not in reference - {}
    Value is reference - {}
    Values layout mismatch - {}
    Values older than reference - {}
    Values untranslated - {}
    Values same as reference - {}
""".strip().format(
        comp.right.fpath,
        comp.left.fpath,
        len(comp.right) - len(comp.right_only),
        len(comp.left),
        100.0 * (len(comp.right) - len(comp.right_only)) / len(comp.left),
        len(comp.right_only),
        len(comp.left_pure_reference),
        len(comp.layout_mismatch),
        len(comp.right_older),
        len(comp.untranslated),
        len(comp.identical)))

    return exit_code


if __name__ == "__main__":
    root_parser = argparse.ArgumentParser()
    verb_parsers = root_parser.add_subparsers()

    format_parser = verb_parsers.add_parser('format', help="format a string table and exit")
    format_parser.set_defaults(action=format_action)
    format_parser.add_argument('source', metavar='SOURCE', help="string table to format",
        type=argparse.FileType(encoding='utf-8', errors='strict'))

    sync_parser = verb_parsers.add_parser('sync', help="synchronize two string tables and exit")
    sync_parser.set_defaults(action=sync_action)
    sync_parser.add_argument('reference', metavar='REFERENCE', help="reference string table",
        type=argparse.FileType(encoding='utf-8', errors='strict'))
    sync_parser.add_argument('source', metavar='SOURCE', help="string table to sync",
        type=argparse.FileType(encoding='utf-8', errors='strict'))

    rename_key_parser = verb_parsers.add_parser('rename-key', help="rename all occurences of a key within a stringtable and exit")
    rename_key_parser.set_defaults(action=rename_key_action)
    rename_key_parser.add_argument('source', metavar='SOURCE', help="string table to rename old key within",
        type=argparse.FileType(encoding='utf-8', errors='strict'))
    rename_key_parser.add_argument('old_key', metavar='OLD_KEY', help="key to rename")
    rename_key_parser.add_argument('new_key', metavar='NEW_KEY', help="new key name")

    check_parser = verb_parsers.add_parser('check', help="check a stringtable for consistency and exit")
    check_parser.set_defaults(action=check_action)
    check_parser.add_argument('-r', '--reference', metavar='REFERENCE', help="reference string table",
        type=argparse.FileType(encoding='utf-8', errors='strict'))
    check_parser.add_argument('sources', metavar='SOURCES', help="string tables to check",
        nargs='+',
        type=argparse.FileType(encoding='utf-8', errors='strict'))

    compare_parser = verb_parsers.add_parser('compare', help="compare two string tables and exit")
    compare_parser.set_defaults(action=compare_action)
    compare_parser.add_argument('-s', '--summary-only', help="print only a summary of keys", action='store_true', dest='summary_only')
    compare_parser.add_argument('reference', metavar='REFERENCE', help="reference string table",
        type=argparse.FileType(encoding='utf-8', errors='strict'))
    compare_parser.add_argument('source', metavar='SOURCE', help="string table to compare",
        type=argparse.FileType(encoding='utf-8', errors='strict'))

    args = root_parser.parse_args()
    if 'action' not in args:
        root_parser.print_usage()
        root_parser.exit()
    sys.exit(args.action(args))
