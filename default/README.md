# Resource directory (default)

The contents of the resource directory define the assets available to
FreeOrion, as well as logic for modifiable content.
These assets determine the appearance and functionality of FreeOrion.

While much of the content within this directory is required in any resource
directory, most is either
* required only from inclusion in other entries
* strictly optional

Each sub-directory has an accompanying README file within it.
This file should give a guideline for any entries or files required by
FreeOrion if a new resource directory is created.

An alternate resource directory can be specified with the
--resource.path command line flag, or by changing the resource.path node in the
user's config.xml.
See http://www.freeorion.org/index.php/Config.xml for details on the config.xml
file.

## Contents

* customizations/  -  User customizations
* data/  -  Contains data files used by FreeOrion, including graphics, fonts,
and sound.
* python/  -  All content processed by python scripts are located here.  This
is logic for AI decisions, events that occur each turn, and creation of a new
universe when starting a new game.
* scripting/  -  This directory contains the definitions of content within
FreeOrion.
* shaders/  -  Shader effects applied to various graphics during specific game
events.
* stringtables/  -  Language translations.  The only required file is
**en.txt**, which is also a fallback for any other translations.  Any
corrections to translations, or updates for omitted languages are welcome.
* content_specific_parameters.txt  -  **This file and the contained entries are
required by FreeOrion**, the entries should be modified to reflect related
content changes.  See file for details.
* COPYING  -  License info related to any content in this directory.
**Required file and content.**
* credits.xml  -  Acknowledgments of those who have made FreeOrion possible. 
**Required file and content.**
* empire_colors.xml  -  Color options available for empires to select from. 
**This file containing at least one GG::Clr node is required.**
