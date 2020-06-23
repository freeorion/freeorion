#!/usr/bin/python3

from __future__ import print_function
from distutils.spawn import find_executable
from distutils.version import StrictVersion
from subprocess import check_output
import argparse
try:
    import colorama
    HAS_COLORAMA=True
except ImportError:
    HAS_COLORAMA=False
import difflib
import sys
import json
import yaml


def collect_format_differences(file_path, args):
    original = None
    with open(file_path, 'r') as f:
        original = f.read().splitlines(keepends=True)
    formatted = check_output([args.clang_format_executable, '-style='+args.clang_format, '-sort-includes=false', file_path]).decode('utf-8').splitlines(keepends=True)
    original_lno = 0
    formatted_lno = 0
    hunks = []
    current_hunk = None
    for line in difflib.ndiff(original, formatted):
        if line.startswith('? '):
            continue
        if line.startswith(' ') or line.startswith('-'):
            original_lno += 1
        if line.startswith(' ') or line.startswith('+'):
            formatted_lno += 1
        if line.startswith(' '):
            if current_hunk:
                hunks.append(current_hunk)
                current_hunk = None
            continue
        if line.startswith('?'):
            continue
        if not current_hunk:
            current_hunk = {
                'from': original_lno,
                'to': original_lno,
                'added': '',
                'removed': ''
            }
        if line.startswith('+'):
            current_hunk['added'] += line[2:]
        if line.startswith('-'):
            current_hunk['removed'] += line[2:]
            current_hunk['to'] = original_lno
    return hunks


def print_gcc_report(file_path, differences, args):
    added_beg = ''
    added_end = ''
    removed_beg = ''
    removed_end = ''
    if HAS_COLORAMA and args.color:
        added_beg = colorama.Fore.GREEN
        added_end = colorama.Style.RESET_ALL
        removed_beg = colorama.Fore.RED
        removed_end = colorama.Style.RESET_ALL
    for hunk in differences:
        print('{}:{}: Formatting difference, clang-format suggests diff:\n{}\n{}'.format(
            file_path,
            hunk['from'],
            '\n'.join([removed_beg + '-' + line + removed_end for line in hunk['removed'].splitlines()]),
            '\n'.join([added_beg + '+' + line + added_end for line in hunk['added'].splitlines()])
        ))


def main():
    arg_parser = argparse.ArgumentParser(
        description='Report differences between current and expected C++ format'
    )
    arg_parser.add_argument(
        '--config', '-c', nargs='?', type=argparse.FileType('r'),
        default='.clang-config', help='A clang-format configuration'
    )
    arg_parser.add_argument(
        '--clang-format-executable', default='clang-format',
        help='Name or path to the clang-format executable'
    )
    arg_parser.add_argument(
        '--color', '-C', action='store_true', default=sys.stdout.isatty(),
        help='Force colorized report output'
    )
    arg_parser.add_argument(
        'files', nargs='+', help='A list of files to scan for mismatches'
    )

    args = arg_parser.parse_args()

    if 'clang-format' == args.clang_format_executable:
        args.clang_format_executable = find_executable('clang-format')
    if not args.clang_format_executable:
        print("Unable to locate `clang-format` executable.", file=sys.stderr)
        exit(1)

    if not HAS_COLORAMA:
        args.color = False
    else:
        colorama.init(strip=not args.color)

    if hasattr(yaml, '__version__') and StrictVersion(yaml.__version__) >= StrictVersion('5.1'):
        args.clang_format = json.dumps(yaml.load(args.config, Loader=yaml.FullLoader))
    else:
        args.clang_format = json.dumps(yaml.load(args.config))
    report = print_gcc_report

    exit_code = 0

    for file_path in args.files:
        brighten_beg = '';
        brighten_end = '';
        if HAS_COLORAMA and args.color:
            brighten_beg = colorama.Style.BRIGHT;
            brighten_end = colorama.Style.RESET_ALL;
        print("{}Checking format of {}:{}".format(brighten_beg, file_path, brighten_end))
        differences = collect_format_differences(file_path, args)
        if len(differences):
            exit_code = 1
        report(file_path, differences, args)

    exit(exit_code)


if __name__ == "__main__":
    main()
