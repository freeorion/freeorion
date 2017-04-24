#ifndef _ConfigFileProcess_h_
#define _ConfigFileProcess_h_

#include <vector>
#include <string>
#include <map>
#include <iostream>

#include "../util/Directories.h"
#include "../util/OptionsDB.h"
#include "../util/i18n.h"
#include "../util/Version.h"
#include "../util/XMLDoc.h"

#include <boost/filesystem/fstream.hpp>

// client/human/chmain, server/dmain and client/AI/camain have common code
//   put them here instead of duplicating in the three files

void ConfigFileProcess(const std::vector<std::string> args);

#endif
