#!/usr/bin/env python2

import sys
import os
import re

MIN_CPP_INCLUDE_COUNT = 5
EXCLUDE_LIBS          = ["GG"]
IGNORE_INCLUDES       = ["StdAfx.h", "stdafx.h"]

PREAMBLE = """
#pragma once

// We include all external headers used in any of the header files,
// plus external headers used in at least five .cpp files.

// https://hownot2code.com/2016/08/16/stdafx-h/ 

"""

POSTAMBLE = """

#ifdef _MSC_VER
// Note: This is a workaround for Visual C++ non-conformant pre-processor
// handling of empty macro arguments.
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/facilities/is_empty_variadic.hpp>
#endif
"""


def find_files_from_vcxproj(vcxproj, includetype, fileextension):
    pattern         = "^\s*<" + includetype + "\s+Include=\"(?P<include>[^\"]+" + fileextension + ")\"\s*/>"
    vcxproj_pattern = re.compile(pattern)
    base_path       = os.path.dirname(vcxproj)

    with open(vcxproj) as f:
        for i, line in enumerate(f):
            match = vcxproj_pattern.match(line)
            if match and match.group('include') not in IGNORE_INCLUDES:
                yield os.path.join(base_path, match.group('include'))

include_pattern = re.compile("^#include[ \t]+<(?P<include>(?:(?P<lib>[^/>]+)/)?[^>]+)>")
ifdef_pattern = re.compile("^\s*#if(?:def)?\s")
endif_pattern = re.compile("^\s*#endif")

def find_includes_from_cpp_or_h(filename): 
    if_nesting_level = 0
    with open(filename) as f:
        for i, line in enumerate(f):
            if  ifdef_pattern.match(line):
                if_nesting_level += 1
            elif endif_pattern.match(line):
                if_nesting_level -= 1
            elif if_nesting_level == 0:
                match = include_pattern.match(line)
                if match and match.group('include') not in IGNORE_INCLUDES:
                    yield (match.group('include'), match.group('lib'))

def assemble_precompile_header_includes(headeronly_vcxproj_files, vcxproj_files):
    headers_includes = { }
    cpp_includes     = { }

    index = -1
    for vcxproj in (headeronly_vcxproj_files + vcxproj_files):
        index = index + 1
        for include_file in find_files_from_vcxproj(vcxproj, "ClInclude", ".h"):
            for include, lib in find_includes_from_cpp_or_h(include_file):
                if include in headers_includes: 
                    continue
                headers_includes[include] = (index, vcxproj, lib)

    index = -1
    for vcxproj in vcxproj_files:
        index = index + 1
        for include_file in find_files_from_vcxproj(vcxproj, "ClCompile", ".cpp"):
            for include, lib in find_includes_from_cpp_or_h(include_file):
                if not include in cpp_includes: 
                    cpp_includes[include]    = [index, vcxproj, lib, 1]
                else:
                    cpp_includes[include][3] = cpp_includes[include][3] + 1

    headers = [('includes from .h',
                headers_includes[include][0],
                headers_includes[include][1],
                headers_includes[include][2],
                include, 
                0) for include in headers_includes]   
    headers.sort()

    cpps   = [('includes from .cpp', 
                cpp_includes[include][0],
                cpp_includes[include][1],
                cpp_includes[include][2],
                include,
                cpp_includes[include][3]
                ) for include in cpp_includes
                         if not include in headers_includes 
                            and cpp_includes[include][3] >= MIN_CPP_INCLUDE_COUNT
            ]
    cpps.sort()

    return headers + cpps

def update_stdafx_h(stdafx_h_filename, header_only_vcxproj_files, vcxproj_files):
    last_type = ""
    last_proj = ""
    last_lib  = None

    lines = []

    for type, index, proj, lib, include, count in assemble_precompile_header_includes(header_only_vcxproj_files, vcxproj_files):
        
        if lib in EXCLUDE_LIBS:
            continue
        if type != last_type:
            if last_type:
                lines.append("")
            lines.append("// " + '-' * len(type)) 
            lines.append("// " + type) 
            last_type = type
        if last_proj != proj:
            lines.append("") 
            lines.append("// " + os.path.splitext(os.path.basename(proj))[0])
            last_proj = proj
        if last_lib != lib:
            if lib:
                lines.append("") 
            last_lib = lib

        include_directive = "#include <" + include + ">"
        #if count > 0:
        #    include_directive += ' ' * max(50 - len(include_directive), 0)
        #    include_directive += " // included in " + str(count) + " .cpp files"

        lines.append(include_directive)

    with open(stdafx_h_filename, 'w') as f:
        f.write(PREAMBLE)
        f.write("\n".join(lines))
        f.write(POSTAMBLE)

update_stdafx_h("Parsers/StdAfx.h",    [], ["Parsers/Parsers.vcxproj"])
update_stdafx_h("Common/StdAfx.h",     [], ["Common/Common.vcxproj"])

update_stdafx_h("FreeOrion/StdAfx.h",  ["Common/Common.vcxproj", "GiGi/GiGi.vcxproj"],
                                       ["FreeOrion/FreeOrion.vcxproj"])
update_stdafx_h("FreeOrionD/StdAfx.h", ["Common/Common.vcxproj"],   ["FreeOrionD/FreeOrionD.vcxproj"])
update_stdafx_h("FreeOrionCA/StdAfx.h",["Common/Common.vcxproj"],   ["FreeOrionCA/FreeOrionCA.vcxproj"])

update_stdafx_h("GiGi/StdAfx.h",       [],                    ["GiGi/GiGi.vcxproj"])
#update_stdafx_h("GiGiSDL/StdAfx.h",    ["GiGi/GiGi.vcxproj"], ["GiGiSDL/GiGiSDL.vcxproj"])