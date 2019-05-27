This solution file gtrackLib.sln is used to build tracking libraries for PC HOST environemnts.
The solution is found working with WIN10/MSVC Pro 2012. Other versions and environments may require porting efforts.
The solution include 4 library builds projects:
- gtrackC\gtracklibC2d.vcxproj and gtrackC\gtracklibC3d.vcxproj build MSVC C library for 2D and 3D tracking. 
	These libraries can be used by MSVC application to build and test tracking algorithms from C PC-HOST environment.
- gtrackMex\gtracklibMex2d.vcxproj, gtrackMex\gtracklibMex3d.vcxproj build MSVC Mex library for 2D and 3D tracking. 
	These libraries can be used by Matlab (R) application to build and exercise tracking algorithm from Matlab environment.
