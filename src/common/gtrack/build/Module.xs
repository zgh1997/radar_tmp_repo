/******************************************************************************
 * FILE PURPOSE: TEST Module specification file.
 ******************************************************************************
 * FILE NAME: module.xs
 *
 * DESCRIPTION:
 *  This file contains the module specification for the package documentation.
 *
 * Copyright (C) 2016, Texas Instruments, Inc.
 *****************************************************************************/

/* Load the library utility. */
var libUtility = xdc.loadCapsule (java.lang.System.getenv("MMWAVE_SDK_INSTALL_PATH") + "/scripts/buildlib.xs");

/**************************************************************************
 * FUNCTION NAME : modBuild
 **************************************************************************
 * DESCRIPTION   :
 *  The function is used to add files to the release package
 **************************************************************************/
function modBuild()
{
    Pkg.otherFiles[Pkg.otherFiles.length++] = "build/win/msvc/gtrackLib.sln";
    Pkg.otherFiles[Pkg.otherFiles.length++] = "build/win/msvc/readme.txt";

    /* Add all the MSVC project files to the release package. */
    var projFiles = libUtility.listAllFiles (".vcxproj", "build/win/msvc");
    for (var k = 0 ; k < projFiles.length; k++)
        Pkg.otherFiles[Pkg.otherFiles.length++] = projFiles[k];
}

