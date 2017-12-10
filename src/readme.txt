This tool generates a header file containing version information extracted from a
configuration file.  The generated header file can be used in resource scripts to
generate the VERSIONINFO resource.  An example of such a script is included as
version.rci.

Versioning
===========

The tool can be used in one of two ways: explicit versioning or incremental versioning

Explicit versioning requires that an explicit build version be given. This version
is used as the build version for the file.  No changes are made to any other files.

Incremental versioning takes the build number from the specified configuration file
and increments it by one.  The configuration file is updated and the new build
version is used.  This is most useful for nightly builds.  

The configuration file must be writeable as it contains the current build number.
An automated build can check the file out of source control, run the tool and then
check the updated file back in if this is desired.  If the build number comes from
another source (i.e. explicitly given) then the configuration file isn't modified.

Including in Files (non-.NET)
==============================

To use the generated header file and associated RCI file in a project do the following:

1) Open the RC file for the project and remove any existing VERSIONINFO resources
2) In the AppStudio section of the resource file (at the top where MSDev gets its settings),
add the following code:

3 TEXTINCLUDE DISCARDABLE 
BEGIN
    "#include ""Version.rci""\r\n"
    "\0"
END

#endif    // APSTUDIO_INVOKED

and the following code to the non-AppStudio section (at the bottom):

#ifndef APSTUDIO_INVOKED
/////////////////////////////////////////////////////////////////////////////
//
// Generated from the TEXTINCLUDE 3 resource.
//
#include "Version.rci"

/////////////////////////////////////////////////////////////////////////////
#endif    // not APSTUDIO_INVOKED

Note that the number (3 in this case) is the sequentially next TEXT block in the resource file.
verbuild.h is the name of the output file generated from BuildVer.

3) Copy the FileVer.h template file into any project directory that will use automatic
versioning.  Modify the filever.h file to set the appropriate file options such as
the filename.

4) Be sure to place the generated header file (verbuild.h) and RCI file in a location that 
is accessible by the resource compiler and optionally the C compiler.

Including in Files (.NET)
==========================

By specifying the -n option the tool will generate a file that can be used in .NET.  Note that
the file is formatted as a C# program.

1) Link to the generated file in each of your projects.  Do not simply click Open on the Add Item
dialog as this will copy the file.  

2) Define any attributes not already defined by the file (such as AssemblyTitle) for each
project.

Variables
==========

The following variables are defined during the build process and can be used as needed.  Each variable
is enclosed in % characters (i.e. %MAJOR_VER%).  In the following descriptions the section-key values
defined in the version.txt file are identified as section.key (i.e. Version.Major)

MAJOR_VER = Version.Major
MINOR_VER = Version.Minor
PATCH_VER = Version.Patch
BUILD_VER = Version.Build or from the command line
LABEL = Label from the command line, if any

PRODUCT_VER_STRING = MAJOR_VER.MINOR_VER.PATCH_VER.BUILD_VER (LABEL)
FILE_VER_STRING = Same as PRODUCT_VER_STRING

COMPANY_NAME = Company.Name
COMPANY_NAME_SHORT = Company.ShortName

PRODUCT_NAME = Product.Name
PRODUCT_NAME_SHORT = Product.ShortName
COPYRIGHT = Product.Copyright
TRADEMARK = Product.Trademark

BUILD_DATE = Build date in the form <month name> <day>, <4-digit year>
BUILD_DATE_SHORT = Build date in the form <month number>/<day>/<4-digit year>
BUILD_DATE_INTL = Build date in the form <day> <month name> <4-digit year>

BUILD_TIME = Build time in the form <hour>:<minutes> <AM|PM>
BUILD_TIME_INTL = Build time in the form <24-hour>:<minutes>


