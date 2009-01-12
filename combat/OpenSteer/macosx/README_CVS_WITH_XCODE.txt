README_CVS_WITH_XCODE.txt

IMPORTANT: For developers with write access to OpenSteer's SourceForge.net CVS repositories.
 
As there are problems with CVS handling some of Apple's file packages and bundles please checkout OpenSteer's "CVSROOT" repository and copy the "cvswrappers" file within to your home directory and rename it to ".cvswrappers" (be careful not to overwrite a ".cvswrappers" file that is already there...).

Why is this proposed? Sometimes CVS has problems using cvs wrappers if in client/server mode - then a ".cvswrappers" tells CVS how to deal with specific (binary) files.

The file "cvsignore" from OpenSteer's "CVSROOT" repository has been changed, too. ".pbxuser", ".mode1", ".DS_Store", and the "build" directory are ignored. Please also copy the "cvsignore" file to your home directory and rename it to ".cvsignore" (be careful not to overwrite a ".cvsignore" file already existing there).


More details at the following pages:

http://developer.apple.com/documentation/DeveloperTools/Conceptual/Xcode_SCM/Introduction.html
http://cocoa.mamasam.com/COCOADEV/2002/09/1/43996.php
http://www.omnigroup.com/mailman/archive/webobjects-dev/2001-February/006876.html
