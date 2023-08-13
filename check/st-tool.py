#!/usr/bin/python3
import argparse
import datetime
import html
import json
import os
import re
import subprocess
import sys
import textwrap
import threading
import time
import urllib.parse
import urllib.request
import webbrowser
from collections import OrderedDict
from collections.abc import Iterator
from http.server import BaseHTTPRequestHandler, HTTPServer
from typing import NamedTuple

STRING_TABLE_KEY_PATTERN = re.compile(r"^[A-Z0-9_]+$")
# Provides the named capture groups 'ref_type' and 'key'
INTERNAL_REFERENCE_PATTERN_TEMPLATE = r"\[\[(?P<ref_type>(?:(?:encyclopedia|(?:building|field|meter)type|predefinedshipdesign|ship(?:hull|part)|special|species|tech|policy|value|browsepath) )?)(?P<key>{})\]\]"
OPTIONAL_REF_TYPES = ["value", "browsepath"]


def _get_brackets(text) -> Iterator[str]:
    has = False
    prev = ""
    for char in text:
        if char in "[]":
            if has:
                has = False
                if prev == char:
                    yield char
            else:
                has = True
                prev = char
        else:
            has = False


def check_balanced_reference(text: str) -> bool:
    """
    Check if reference [[XXX]] has balanced syntax.

    >>> check_balanced_reference("no reference")
    True

    >>> check_balanced_reference("[[valid reference]]")
    True

    >>> check_balanced_reference("[[unbalanced left]")
    False

    >>> check_balanced_reference("[unbalanced right]]")
    False

    >>> check_balanced_reference("[[ multiple with ]] [[ not closed")
    False

    >>> check_balanced_reference("[[ [[ reference ]] in reference ]]")
    False
    """
    braces = _get_brackets(text)
    is_open = False
    for b in braces:
        if b == "[":
            if is_open:
                return False
            is_open = True
        else:
            if not is_open:
                return False
            is_open = False
    return not is_open


class StringTableEntry:
    def __init__(self, key, keyline, value, notes, value_times):
        if not STRING_TABLE_KEY_PATTERN.match(key):
            raise ValueError(f"Key doesn't match expected format [A-Z0-9_]+, was '{key}'")
        self.key = key
        self.keyline = keyline
        self.value = value
        self.value_times = value_times
        self.notes = notes

    def __str__(self):
        result = ""
        for note in self.notes:
            result += "# " + note + "\n"
        if self.value is not None:
            result += self.key + "\n"
            if not self.value or "\n" in self.value or re.search(r"^\s", self.value) or re.search(r"\s$", self.value):
                result += "'''" + self.value + "'''" + "\n"
            else:
                result += self.value + "\n"
        else:
            result += "#*" + self.key + "\n"
            for e in self.value_times:
                result += f"#* <not translated {datetime.datetime.fromtimestamp(e).isoformat(sep=' ')}>\n"

        return result

    def __repr__(self):
        return f"StringTableEntry(key={self.key}, keyline={self.keyline}, value={self.value})"


class StringTable:
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
                    raise ValueError(
                        f"{self.fpath}:{entry.keyline}:"
                        f" Duplicate key '{entry.key}', previously defined on line {self._ientries[entry.key].keyline}"
                    )
                self._ientries[entry.key] = entry

    def __getitem__(self, key):
        return self._ientries[key]

    def __contains__(self, key):
        return key in self._ientries

    def __iter__(self):
        return iter(self._ientries)

    def __len__(self):
        return len(self._ientries)

    def __str__(self):  # noqa: C901
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
            result += '#include "' + include + '"\n'

        return result

    def __repr__(self):
        return f"StringTable(fpath={self.fpath}, language={self.language}, entries={self.entries})"

    def items(self):
        return self._entries

    @staticmethod
    def set_author(fpath, entries, blames):  # noqa: C901
        blame_cmd = ["git", "blame", "--incremental", fpath]
        git_blame = subprocess.check_output(blame_cmd)
        git_blame = git_blame.decode("utf-8", "strict")
        git_blame = git_blame.splitlines(keepends=True)

        value_times = {}
        author_time = None
        line = None
        line_range = None

        # Provides the named capture groups 'sha', 'old_line', 'new_line' and 'range'
        GIT_BLAME_INCREMENTAL_PATTERN = re.compile(
            r"^(?P<sha>[0-9a-f]{40}) (?P<old_line>[1-9][0-9]*) (?P<new_line>[1-9][0-9]*)(?P<range> [1-9][0-9]*)?$"
        )

        for line_blame in git_blame:
            match = GIT_BLAME_INCREMENTAL_PATTERN.match(line_blame)
            if match:
                line = int(match["new_line"])
                line_range = int(match["range"])
            if line_blame.startswith("author-time "):
                author_time = int(line_blame.strip().split(" ")[1])
            if line_blame.startswith("filename "):
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
                    raise RuntimeError(
                        f"{fpath}: git blame did not collect any matching author times for key {entry.key}"
                    )
                entry.value_times = value_times[entry.key]

    @staticmethod
    def from_file(fhandle, with_blame=True) -> "StringTable":
        return StringTable.from_text(fhandle.read(), fhandle.name, with_blame=with_blame)

    @staticmethod
    def from_text(text, fpath, with_blame=True) -> "StringTable":  # noqa: C901
        is_quoted = False

        language = None
        fnotes = []
        section = []
        entries = []
        includes = []
        blames = OrderedDict()

        key = None
        keyline = None
        value = None
        vline_start = 0
        vline_end = 0
        notes = []
        untranslated_key = None
        untranslated_keyline = None
        untranslated_lines = []

        for fline, line in enumerate(text.splitlines(keepends=True), 1):
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
                        entries.append(
                            StringTableEntry(untranslated_key, untranslated_keyline, None, notes, untranslated_lines)
                        )
                    except ValueError as e:
                        raise ValueError(f"{fpath}:{keyline}: {str(e)}")
                    untranslated_key = None
                    untranslated_keyline = None
                    notes = []
                    untranslated_lines = []
                vline_start = vline_end = fline
                continue

            if not is_quoted and line.startswith("#"):
                # discard comments when not in quoted value
                if line.startswith("#include"):
                    includes.append(line[8:].strip()[1:-1].strip())
                if line.startswith("##"):
                    section.append(line[3:].rstrip())
                elif line.startswith("#*") and not untranslated_key:
                    untranslated_key = line[2:].strip()
                    untranslated_keyline = fline
                elif line.startswith("#*"):
                    untranslated_lines.append(0)
                elif line.startswith("#"):
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
                        entries.append(
                            StringTableEntry(untranslated_key, untranslated_keyline, None, notes, untranslated_lines)
                        )
                    except ValueError as e:
                        raise ValueError(f"{fpath}:{keyline}: {str(e)}")
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
            value = (value if value else "") + line

            if not is_quoted and key is not None and value is not None:
                # consume collected string table entry
                try:
                    value_range = {line: key for line in range(vline_start, vline_end + 1)}
                    entries.append(StringTableEntry(key, keyline, value, notes, [0] * len(value_range)))
                    blames.update(value_range)
                except ValueError as e:
                    raise ValueError(f"{fpath}:{keyline}: {e}")

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

            raise ValueError(f"{fpath}:{vline_start}: Impossible parser state; last valid line: {line}")

        if key:
            raise ValueError(f"{fpath}:{vline_start}: Key '{key}' without value")

        if is_quoted:
            raise ValueError(f"{fpath}:{vline_start}: Quotes not closed for key '{key}'")

        if with_blame:
            StringTable.set_author(fpath, entries, blames)
        return StringTable(fpath, language, fnotes, includes, entries)

    @staticmethod
    def statistic(left: "StringTable", right: "StringTable"):  # noqa: C901
        class STStatistic(NamedTuple):
            left: "StringTable"
            left_only: set
            left_older: set
            left_pure_reference: set
            right: "StringTable"
            right_only: set
            right_older: set
            right_pure_reference: set
            untranslated: set
            identical: set
            layout_mismatch: set

        all_keys = set(left).union(right)

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
                if (
                    left[key].value.startswith("[[")
                    and 2 == left[key].value.count("[")
                    and left[key].value.endswith("]]")
                    and 2 == left[key].value.count("]")
                ):
                    left_pure_reference.add(key)
                if (
                    right[key].value.startswith("[[")
                    and 2 == right[key].value.count("[")
                    and right[key].value.endswith("]]")
                    and 2 == right[key].value.count("]")
                ):
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
            left=left,
            left_only=left_only,
            left_older=left_older,
            right=right,
            right_only=right_only,
            right_older=right_older,
            left_pure_reference=left_pure_reference,
            right_pure_reference=right_pure_reference,
            untranslated=untranslated,
            identical=identical,
            layout_mismatch=layout_mismatch,
        )

    @staticmethod
    def sync(reference, source):
        entries = []

        for item in reference.items():
            if isinstance(item, StringTableEntry):
                if item.key in source:
                    source_item = source[item.key]
                    entries.append(
                        StringTableEntry(
                            item.key,
                            item.keyline,
                            source_item.value if source_item.value != item.value else None,
                            item.notes,
                            source_item.value_times,
                        )
                    )
                else:
                    entries.append(
                        StringTableEntry(item.key, item.keyline, None, item.notes, [0] * len(item.value_times))
                    )
            else:
                entries.append(item)

        return StringTable(source.fpath, source.language, source.notes, source.includes, entries)


class EditServerHandler(BaseHTTPRequestHandler):
    body = """
<html>
    <head>
        <title>Editing {source}</title>
        <style>
            body {{
                font-family: sans-serif;
            }}
            .menu {{
                border: None;
                margin: 0;
                padding: 0;
                position: relative;
            }}
            .menu legend {{
                cursor: pointer;
                font-weight: bold;
                margin-bottom: 0.2em;
            }}
            #display-settings {{
                display: inline;
            }}
            #display-settings > label {{
                display: none;
            }}
            table {{
                table-layout: fixed;
                width: 100%;
            }}
            td {{
                vertical-align: top;
                white-space: pre-wrap;
            }}
            td.source, td.translation {{
                border-style: solid;
                border-width: thin;
                padding-left: 0.5em;
                padding-right: 0.5em;
            }}
            td.source {{
                margin-right: 2em;
            }}
            td.translation {{
                position: relative;
                padding-left: 1em;
                border-left-width: thick;
            }}
            td.translation:before {{
                position:  absolute;
                color: #fff;
                border-radius:  100%;
                width: 1.2em;
                height: 1.2em;
                text-align: center !important;
                vertical-algin: middle;
                top: 0.1em;
                left: -0.8em;
                text-align: center;
                font-weight: bolder;
                line-height: 1.2em;
            }}
            .status-recent td.translation {{
                border-color: #00cc00;
                background-color: #00cc0010;
                color: black;
            }}
            .status-recent td.translation:before {{
                content: "\\2713";
                background-color: #00cc00;
            }}
            .status-stale td.translation {{
                border-color: #cc8400;
                background-color: #cc840010;
                color: black;
            }}
            .status-stale td.translation:before {{
                content: "!";
                background-color: #cc8400;
            }}
            .status-untranslated td.translation {{
                border-color: #ff0000;
                background-color: #ff000010;
                color: black;
            }}
            .status-untranslated td.translation:before {{
                content: "\\274c";
                background-color: #ff0000;
            }}
            .entry td.source {{
                border-color: #808080;
                background-color: #80808010;
                color: black;
            }}
            form.translator {{
                width: 100%;
            }}
        </style>
        <script type="text/javascript" src="/jquery.js"></script>
        <script type="text/javascript" src="/jeditable.jquery.js"></script>
        <script type="text/javascript">
            function toggle_display_settings() {{
                visible = ('true' == this.dataset.visible) || false;

                if(visible) {{
                    document.getElementById('toggle-display-indicator').innerHTML = '&#x25b6;';
                    Array.from(
                        this.parentElement.getElementsByTagName('label')
                    ).forEach(function(element, idx) {{
                        element.style.display = 'none';
                    }});
                }} else {{
                    document.getElementById('toggle-display-indicator').innerHTML = '&#x25bd;';
                    Array.from(
                        this.parentElement.getElementsByTagName('label')
                    ).forEach(function(element, idx) {{
                        element.style.display = 'block';
                    }});
                }}

                this.dataset.visible = (!visible).toString();
            }}

            function toggle_entries_by_status(status) {{
                checkbox = document.getElementById('toggle-' + status);
                checkbox.disabled = true;
                visible = ('true' == checkbox.dataset.visible) || false;

                if(visible) {{
                    setTimeout(function() {{
                        Array.from(
                            document.getElementsByClassName('status-' + status)
                        ).forEach(function(element, idx) {{
                            element.style.display = '';
                        }});
                        document.getElementById('toggle-' + status).disabled = false;
                    }});
                }} else {{
                    setTimeout(function() {{
                        Array.from(
                            document.getElementsByClassName('status-' + status)
                        ).forEach(function(element, idx) {{
                            element.style.display = 'none';
                        }});
                        document.getElementById('toggle-' + status).disabled = false;
                    }});
                }}

                checkbox.dataset.visible = (!visible).toString();
            }}

            function save_entry(value, settings, response_handler) {{
                $.post({{
                    url: '/update',
                    data: JSON.stringify({{"id": settings.id, "value": value}}),
                    contenttype: 'application/json; charset=UTF-8',
                    datatype: 'json',
                    success: function(response) {{
                        var el = document.getElementById(response.id);
                        el.classList.remove('status-recent', 'status-stale', 'status-untranslated');
                        el.classList.add('status-'+response.status);
                        response_handler($('<div />').text(value).html(), true);
                    }}
                }});
            }}

            window.addEventListener('unload', function() {{
                navigator.sendBeacon('/shutdown');
            }});

            document.addEventListener('DOMContentLoaded', function() {{
                var el = document.getElementById('toggle-display-settings');
                el.onclick = toggle_display_settings.bind(el);
                var el = document.getElementById('toggle-recent');
                el.onclick = toggle_entries_by_status.bind(el, 'recent');
                var el = document.getElementById('toggle-stale');
                el.onclick = toggle_entries_by_status.bind(el, 'stale');
                var el = document.getElementById('toggle-untranslated');
                el.onclick = toggle_entries_by_status.bind(el, 'untranslated');

                Array.from(document.getElementsByClassName('translation')).forEach(function(element, idx) {{
                    $(element).editable(save_entry, {{
                        type: 'textarea',
                        id: element.dataset.id,
                        submit: 'Save',
                        cancel: 'Cancel',
                        placeholder: '<i>untranslated</i>',
                        cssclass: 'translator',
                        width: '100%',
                        rows: 5
                    }});
                }});
            }});
        </script>
    </head>
    <body>
        <h1>Translate stringtable for language &quot;{language}&quot;</h1>
        <form action="save" method="post">
            <fieldset class="menu">
                <legend>Stringtable</legend>
                <button type="submit" id="save-stringtable">Save</button>
            </fieldset>
        </form>
        <fieldset class="menu" id="display-settings">
            <legend id="toggle-display-settings"><span id="toggle-display-indicator">&#x25b6;</span> Display</legend>
            <label><input class="input-slider" type="checkbox" checked="checked" id="toggle-recent">Recent translations</label>
            <label><input class="input-slider" type="checkbox" checked="checked" id="toggle-stale">Stale translations</label>
            <label><input class="input-slider" type="checkbox" checked="checked" id="toggle-untranslated">Untranslated translations</label>
        </fieldset>
        <table>
            {entries}
        </table>
    </body>
</html>
    """

    entry = """
<tbody class="entry status-{status}" id="{key}">
    <tr><th colspan="2">{key}</th></tr>
    <tr class="notes"><td colspan="2">{notes}</td></tr>
    <tr><td class="source">{source}</td><td class="translation" data-id="{key}">{translation}</td></tr>
</tbody>
    """

    def do_GET(self):
        parsed_path = urllib.parse.urlparse(self.path)

        if parsed_path.path == "/":
            return self.GET_stringtable()
        elif parsed_path.path == "/jquery.js":
            return self.GET_url("https://code.jquery.com/jquery-3.4.1.min.js")
        elif parsed_path.path == "/jeditable.jquery.js":
            return self.GET_url(
                "https://raw.githubusercontent.com/NicolasCARPi/jquery_jeditable/2.0.14/dist/jquery.jeditable.min.js"
            )
        else:
            self.send_response(404)
            self.end_headers()

    def do_POST(self):
        parsed_path = urllib.parse.urlparse(self.path)

        if parsed_path.path == "/update":
            return self.POST_entry()
        if parsed_path.path == "/save":
            return self.POST_save()
        if parsed_path.path == "/shutdown":
            # The http.server.HttpServer states, that the shutdown function
            # needs to be called outside of the thread, which processes the
            # handler, to avoid deadlock.
            threading.Thread(target=(lambda: self.server.shutdown())).start()
        else:
            self.send_response(404)
            self.end_headers()

    def GET_url(self, url):
        response = urllib.request.urlopen(url)
        self.send_response(response.getcode())
        self.send_header("Content-Type", response.info()["Content-Type"])
        self.end_headers()

        for chunk in response:
            self.wfile.write(chunk)

    def GET_stringtable(self):
        self.send_response(200)
        self.send_header("Content-Type", "text/html ; charset=utf-8")
        self.end_headers()

        entries = []

        stat = StringTable.statistic(self.server.reference_st, self.server.source_st)

        for key in self.server.reference_st:
            source = self.server.reference_st[key].value
            source = html.escape(source)

            translation = self.server.source_st[key].value if key in self.server.source_st else ""

            status = "recent"
            if key not in stat.left_older:
                status = "stale"
            if not translation:
                status = "untranslated"

            translation = translation if translation else ""
            translation = html.escape(translation)

            notes = self.server.reference_st[key].notes
            notes = "\n".join(notes)
            notes = html.escape(notes)

            entries.append(
                EditServerHandler.entry.format(
                    key=key, source=source, translation=translation, notes=notes, status=status
                )
            )

        self.wfile.write(
            EditServerHandler.body.format(
                source=self.server.source_st.fpath, language=self.server.source_st.language, entries="".join(entries)
            ).encode("utf-8")
        )

    def POST_error(self, message):
        self.send_response(500)
        self.send_header("Content-Type", "application/json")
        self.end_headers()

        self.wfile.write(json.dumps({"code": 500, "message": message}).encode("utf-8"))

    def POST_save(self):
        self.send_response(200)
        self.send_header("Content-Type", "text/plain ; charset=utf-8")
        self.send_header(
            "Content-Disposition", f'attachment; filename="{os.path.basename(self.server.source_st.fpath)}"'
        )
        self.end_headers()

        self.wfile.write(str(self.server.source_st).encode("utf-8"))

    def POST_entry(self):
        try:
            content_length = int(self.headers.get("Content-Length", "0"))
            body = self.rfile.read(content_length)
            request = json.loads(body)
            if "id" not in request or "value" not in request:
                raise ValueError("Some of the expected json keys are missing")
        except ValueError:
            self.POST_error("Browser didn't send a save request")
            raise

        if request["id"] not in self.server.reference_st:
            self.POST_error(f"No entry with key {request['id']}")

        entry = self.server.source_st[request["id"]]
        rentry = self.server.reference_st[request["id"]]

        if entry.value != request["value"]:
            if request["value"] and rentry.value != request["value"]:
                entry.value = request["value"]
            else:
                entry.value = None
            if entry.value:
                entry.value_times = [int(time.time())] * len(entry.value.split("\n"))
            else:
                entry.value_times = rentry.value_times

        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.end_headers()

        status = "recent"
        if max(self.server.reference_st[request["id"]].value_times) > min(entry.value_times):
            status = "stale"
        if not entry.value:
            status = "untranslated"

        response = {
            "id": request["id"],
            "status": status,
        }

        self.wfile.write(json.dumps(response).encode("utf-8"))


def format_action(args):
    """
    Format source file inline and exit.

    If file was formatted exit status will be 1.
    """
    path = args.source.name
    text = args.source.read()
    source_st = StringTable.from_text(text=text, fpath=path)
    new_text = str(source_st)

    if text != new_text:
        with open(path, "w") as f:
            f.write(new_text)
        print(f"{path} was reformatted")
        return 1
    else:
        print(f"{path} is already well formatted")
        return 0


def sync_action(args):
    reference_st = StringTable.from_file(args.reference)
    source_st = StringTable.from_file(args.source)

    out_st = StringTable.sync(reference_st, source_st)

    print(out_st, end="")

    return 0


def rename_key_action(args):
    if not STRING_TABLE_KEY_PATTERN.match(args.old_key):
        raise ValueError(f"The given old key '{args.old_key}' is not a valid name")

    if not STRING_TABLE_KEY_PATTERN.match(args.new_key):
        raise ValueError(f"The given new key '{args.new_key}' is not a valid name")

    source_st = StringTable.from_file(args.source)

    for key in source_st:
        entry = source_st[key]
        entry.value = re.sub(
            INTERNAL_REFERENCE_PATTERN_TEMPLATE.format(re.escape(args.old_key)),
            r"[[\1" + args.new_key + "]]",
            entry.value,
        )
        if entry.key == args.old_key:
            entry.key = args.new_key

    print(source_st, end="")

    return 0


def edit_action(args):
    reference_st = StringTable.from_file(args.reference)
    source_st = StringTable.from_file(args.source)

    source_st = StringTable.sync(reference_st, source_st)

    server = HTTPServer(("localhost", 8080), EditServerHandler)
    server.reference_st = reference_st
    server.source_st = source_st
    print("Starting server at http://localhost:8080/")
    webbrowser.open("http://localhost:8080/")
    try:
        server.serve_forever()
    except (KeyboardInterrupt, EOFError):
        print("Stopping server")

    return 0


def check_action(args):
    exit_code = 0
    reference_st = None
    if args.reference:
        reference_st = StringTable.from_file(args.reference, with_blame=False)
    for source in args.sources:
        source_st = StringTable.from_file(source, with_blame=False)

        references = set()
        for key in source_st:
            entry = source_st[key]

            if entry.value:
                if not check_balanced_reference(entry.value):
                    print(f"{source_st.fpath}:{entry.key}: Unbalanced brackets: '{entry.value}'")
                    exit_code = 1

                for match in re.finditer(INTERNAL_REFERENCE_PATTERN_TEMPLATE.format(".*?"), entry.value):
                    reference_key = match["key"]
                    references.add(reference_key)
                    if reference_key not in source_st:
                        if match["ref_type"].strip() in OPTIONAL_REF_TYPES:
                            continue

                        print(
                            f"{source_st.fpath}:{entry.keyline}: Referenced key '{reference_key}' in value of '{entry.key}' was not found."
                        )
                        exit_code = 1

        exit_code += _check_key_usage(source_st, reference_st, references)
    return exit_code


def _check_key_usage(source_st, reference_st, references):
    exit_code = 0

    def check_key_is_used(key_under_check):
        if not reference_st:
            return True
        if key_under_check in reference_st:
            return True
        if key_under_check in references:
            return True
        return False

    for key in source_st:
        if not check_key_is_used(key):
            print(f"{source_st.fpath}:{source_st[key].keyline}: {key} is not used")
            exit_code = 1
    return exit_code


def compare_action(args):
    exit_code = 0
    reference_st = StringTable.from_file(args.reference)
    source_st = StringTable.from_file(args.source)

    comp = StringTable.statistic(reference_st, source_st)

    if not args.summary_only:
        for key in source_st:
            if key in comp.right_only:
                print(
                    f"{comp.right.fpath}:{comp.right[key].keyline}: Key '{key}' is not in reference file {comp.left.fpath}"
                )
                exit_code = 1

            if key in comp.left_pure_reference:
                print(
                    f"{comp.right.fpath}:{comp.right[key].keyline}:"
                    f" Key '{key}' contains translation for pure reference in reference file"
                    f" value {comp.left.fpath}:{comp.left[key].keyline}"
                )
                exit_code = 1

            if key in comp.layout_mismatch:
                print(
                    f"{comp.right.fpath}:{comp.right[key].keyline}:"
                    f" Value of key '{key}' layout does not match that of reference file"
                    f" value {comp.left.fpath}:{comp.left[key].keyline}"
                )
                exit_code = 1

            if key in comp.right_older:
                print(
                    f"{comp.right.fpath}:{comp.right[key].keyline}:"
                    f" Value of key '{key}' is older than reference file"
                    f" value {comp.left.fpath}:{comp.left[key].keyline}"
                )
                exit_code = 1

            if key in comp.untranslated:
                print(
                    f"{comp.right.fpath}:{comp.right[key].keyline}:"
                    f" Value of key '{key}' has no translation compared to reference file"
                    f" {comp.left.fpath}:{comp.left[key].keyline}"
                )
                exit_code = 1

            if key in comp.identical:
                print(
                    f"{comp.right.fpath}:{comp.right[key].keyline}:"
                    f" Value of key '{key}' is identical to reference file"
                    f" {comp.left.fpath}:{comp.left[key].keyline}"
                )
            exit_code = 1

    print(
        f"""
Summary comparing '{comp.right.fpath}' against '{comp.left.fpath}':
    Keys matching - {len(comp.right) - len(comp.right_only)}/{len(comp.left)} ({100.0 * (len(comp.right) - len(comp.right_only)) / len(comp.left):3.1f}%)
    Keys not in reference - {len(comp.right_only)}
    Value is reference - {len(comp.left_pure_reference)}
    Values layout mismatch - {len(comp.layout_mismatch)}
    Values older than reference - {len(comp.right_older)}
    Values untranslated - {len(comp.untranslated)}
    Values same as reference - {len(comp.identical)}
""".strip()
    )
    return exit_code


if __name__ == "__main__":
    root_parser = argparse.ArgumentParser(description="Verify and modify string tables")
    verb_parsers = root_parser.add_subparsers(
        title="verbs",
        description=f"For more details run `{root_parser.prog} <verb> --help`.",
    )

    format_parser = verb_parsers.add_parser(
        "format",
        help="format a string table and exit",
        description=textwrap.dedent(
            """\
        Pretty prints a given string table onto the standard output.

        Formatting a string tables makes sure that:
        * Translation entries (translated or not) are separated by one empty
          line.
        * Translation notes (prefix: '# ') and untranslated entries
          (prefix: '#*') are properly prefixed and formatted
        * Translation notes, key and value don't have empty lines between
          each other.
        * String tables contain the language name as first line, followed by
          the file notes.
        * Section titles are prefixed with a block comment (prefix '##').
        * Section titles have two leading whitespaces and one trailing
          whitespace.
        """
        ),
        formatter_class=argparse.RawTextHelpFormatter,
    )
    format_parser.set_defaults(action=format_action)
    format_parser.add_argument(
        "source",
        metavar="SOURCE",
        help="string table to format",
        type=argparse.FileType(encoding="utf-8", errors="strict"),
    )

    sync_parser = verb_parsers.add_parser(
        "sync",
        help="synchronize two string tables and exit",
        description=textwrap.dedent(
            """\
        Synchronizes two string tables by copying over translation entry key,
        notes, section titles and includes from the reference into the source,
        while preserving existing translation values, language name and file notes
        from source.

        Source translation entries without matching reference counterpart are
        discarded.

        The resulting string table is printed out to standard output.
        """
        ),
        formatter_class=argparse.RawTextHelpFormatter,
    )
    sync_parser.set_defaults(action=sync_action)
    sync_parser.add_argument(
        "reference",
        metavar="REFERENCE",
        help="reference string table",
        type=argparse.FileType(encoding="utf-8", errors="strict"),
    )
    sync_parser.add_argument(
        "source",
        metavar="SOURCE",
        help="string table to sync",
        type=argparse.FileType(encoding="utf-8", errors="strict"),
    )

    rename_key_parser = verb_parsers.add_parser(
        "rename-key",
        help="rename all occurences of a key within a stringtable and exit",
        description=textwrap.dedent(
            """\
        Replace all occurances of a translation entry key within the given key,
        including internal references.

        The resulting string table is printed out to standard output.
        """
        ),
        formatter_class=argparse.RawTextHelpFormatter,
    )
    rename_key_parser.set_defaults(action=rename_key_action)
    rename_key_parser.add_argument(
        "source",
        metavar="SOURCE",
        help="string table to rename old key within",
        type=argparse.FileType(encoding="utf-8", errors="strict"),
    )
    rename_key_parser.add_argument("old_key", metavar="OLD_KEY", help="key to rename")
    rename_key_parser.add_argument("new_key", metavar="NEW_KEY", help="new key name")

    edit_parser = verb_parsers.add_parser(
        "edit",
        help="edit a stringtable and exit",
        description=textwrap.dedent(
            """\
        Starts a web server and a web browser to provide an interactive editor.

        Use ^C to exit the web server.  The string table will not be saved, so
        download it before exiting the web server or closing the browser.
        """
        ),
        formatter_class=argparse.RawTextHelpFormatter,
    )
    edit_parser.set_defaults(action=edit_action)
    edit_parser.add_argument(
        "reference",
        metavar="REFERENCE",
        help="reference string table",
        type=argparse.FileType(encoding="utf-8", errors="strict"),
    )
    edit_parser.add_argument(
        "source",
        metavar="SOURCE",
        help="string table to edit",
        type=argparse.FileType(encoding="utf-8", errors="strict"),
    )

    check_parser = verb_parsers.add_parser(
        "check",
        help="check a stringtable for consistency and exit",
        description=textwrap.dedent(
            """\
        Check if all references within translation entries are provided by the source or
        or the reference string table.
        """
        ),
        formatter_class=argparse.RawTextHelpFormatter,
    )
    check_parser.set_defaults(action=check_action)
    check_parser.add_argument(
        "-r",
        "--reference",
        metavar="REFERENCE",
        help="reference string table",
        type=argparse.FileType(encoding="utf-8", errors="strict"),
    )
    check_parser.add_argument(
        "sources",
        metavar="SOURCES",
        help="string tables to check",
        nargs="+",
        type=argparse.FileType(encoding="utf-8", errors="strict"),
    )

    compare_parser = verb_parsers.add_parser(
        "compare",
        help="compare two string tables and exit",
        description=textwrap.dedent(
            """\
        Compare two string tables and point out differences between them.
        """
        ),
        formatter_class=argparse.RawTextHelpFormatter,
    )
    compare_parser.set_defaults(action=compare_action)
    compare_parser.add_argument(
        "-s", "--summary-only", help="print only a summary of differences", action="store_true", dest="summary_only"
    )
    compare_parser.add_argument(
        "reference",
        metavar="REFERENCE",
        help="reference string table",
        type=argparse.FileType(encoding="utf-8", errors="strict"),
    )
    compare_parser.add_argument(
        "source",
        metavar="SOURCE",
        help="string table to compare",
        type=argparse.FileType(encoding="utf-8", errors="strict"),
    )

    args = root_parser.parse_args()
    if "action" not in args:
        root_parser.print_usage()
        root_parser.exit()
    sys.exit(args.action(args))
