/******************************************************************************
 * FILE PURPOSE: DOCS Module specification file.
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
 *  The function is used to build the documentation and add it to the package.
 **************************************************************************/
function modBuild()
{
    /* Create the actual PROLOGUE Section for the Documentation.*/
    Pkg.makePrologue += "release: gtrack_document_generation\n";
    Pkg.makePrologue += "gtrack_document_generation:\n";
    Pkg.makePrologue += "\t @echo ----------------------------\n";
    Pkg.makePrologue += "\t @echo Generating gtrack2D Documentation\n";
    Pkg.makePrologue += "\t doxygen docs/Doxyfile2D\n";
    Pkg.makePrologue += "\t @echo gtrack2D Documentation Generated \n";
    Pkg.makePrologue += "\t @echo ----------------------------\n";
    Pkg.makePrologue += "\t @echo Generating gtrack3D Documentation\n";
    Pkg.makePrologue += "\t doxygen docs/Doxyfile3D\n";
    Pkg.makePrologue += "\t @echo gtrack3D Documentation Generated \n";
    Pkg.makePrologue += "\t @echo ----------------------------\n";

    /* Add the documentation file to the package. */
    Pkg.otherFiles[Pkg.otherFiles.length++] = "docs/Doxyfile2D";
    Pkg.otherFiles[Pkg.otherFiles.length++] = "docs/Doxyfile3D";
    Pkg.otherFiles[Pkg.otherFiles.length++] = "docs/tifooter.htm";
    Pkg.otherFiles[Pkg.otherFiles.length++] = "docs/tiheader.htm";
    Pkg.otherFiles[Pkg.otherFiles.length++] = "docs/tilogo.gif";
    Pkg.otherFiles[Pkg.otherFiles.length++] = "docs/titagline.gif";
    Pkg.otherFiles[Pkg.otherFiles.length++] = "docs/*.pdf";

    /* Add the HTML documentation to the package */
    Pkg.otherFiles[Pkg.otherFiles.length++] = "docs/doxygen2D";
    Pkg.otherFiles[Pkg.otherFiles.length++] = "docs/doxygen3D";
}

