#!/usr/bin/env python3
import re
from pathlib import Path
from sys import argv


def strip_comments(content: str) -> str:
    # Remove C++ comments, ignore string literals
    result = []
    i = 0
    while i < len(content):
        char = content[i]
        if char == '"':
            j = i + 1
            while j < len(content) and content[j] != '"':
                j += 2 if content[j] == "\\" else 1
            result.append('""')
            i = j + 1
        elif content.startswith("//", i):
            i = content.find("\n", i)
            if i < 0:
                break
        elif content.startswith("/*", i):
            j = content.find("*/", i + 2)
            i = len(content) if j < 0 else j + 2
        else:
            result.append(char)
            i += 1
    return "".join(result)


def extract_tokens(tokens_path: Path) -> set[str]:
    with open(tokens_path, encoding="utf-8") as f:
        content = f.read()
    clean_content = strip_comments(content)
    tokens = re.findall(r"\(([a-zA-Z0-9_]+)\)", clean_content)
    return set(tokens)


def check_tokens(tokens_path: Path, scripting_path: Path, _parse_path: Path) -> None:
    tokens = extract_tokens(tokens_path)
    scripting_files = list(scripting_path.rglob("*.focs.txt")) + list(scripting_path.rglob("*.macros"))
    for file in scripting_files:
        if not tokens:
            break
        with open(file, encoding="utf-8") as f:
            content = f.read()
        words = re.findall(r"\b[a-zA-Z0-9_]+\b", content)
        tokens -= set(words)
    if tokens:
        for token in sorted(tokens):
            print("  - %s" % token)


if __name__ == "__main__":
    if len(argv) != 4:
        print(
            "Usage: %s <path to parse/Tokens.h file> <path to default/scripting/ directory> "
            "<path to parse/ directory>" % argv[0]
        )
        exit(1)
    tokens_path = Path(argv[1])
    scripting_path = Path(argv[2])
    parse_path = Path(argv[3])
    if not tokens_path.is_file():
        print("%s isn't file" % argv[1])
        exit(1)
    if not scripting_path.is_dir():
        print("%s isn't directory" % argv[2])
        exit(1)
    if not parse_path.is_dir():
        print("%s isn't directory" % argv[3])
        exit(1)
    check_tokens(tokens_path, scripting_path, parse_path)
