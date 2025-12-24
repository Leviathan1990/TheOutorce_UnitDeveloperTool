The Outforce - UnitDeveloper Tool. V3.1.
==

Target OS: Native Linux and Windows. Cross-platform QT project.
***

[Introduction]
=

UnitDeveloper Tool is a ultimate toolkit, a complete all-in-one modding tool designed for the game, "The Outforce".
***
Its a recreation of the original GUI program "UnitDeveloper Tool" devs used to setup a lot of game data. This program bases on reverse engineering and some information obtained from the former devs of the game (memories in other words...) With this tool, you can change any values (game logic) in the game, inspect and extract the content of the PackedProject.opf asset container "archive", and much more!

This build contains the latest version of:

* 1.) Mesh exporter (.obj, .mtl, texture).
* 2.) template.json file generator (used by the external map editor).
* 3.) Emitter editor
* 4.) Effects editor
* 5.) Ai editor
* 6.) BuildFitness editor

To use the program correctly, please read the attached documentations!

[System requirements]
=

UnitDeveloper built with the latest version of QT Creator IDE that supports cross-platform development. All the required dependencies are comes with my tools, so both Windows and Linux supported natively! No terminal or package hunting is requred to run!

Tested and built on: Ubuntu mate 24.04. and Windows 11. (64 bit).

[Knowing issues]
=
!  There are some extraction-related issues: 

*  The mesh itself is kinda messed up.
*  Selection default so not sure if its just to load the texture or disabled
*  The scale for the uv's is not correct. it should be -1 in the y axis

[Fixes:]

*  Fix exporting-related issues.
*  The UV's would match if its negative, however that doesnt fix the whole problem. now there's a need to get the uv's to be flipped based on their individual origins. that way it would match the texture.

[Additions:]

+   Make it possible to add custom-made classes section files(.CUnit, .CUnitWeapon, .CGridMember, .CBaseClass);
+   Make it possible to add custom-made image files (for background images used in the map editor for example.)


[Support]
=

Web: www.theoutforce.hu
e-mail: admin@theoutforce.hu

[Credits]
=

Krisztian Kispeti
lead dev, K's Interactive.

XXXJOHNATHANXXX,
Tester.

ZaRR,
Tester.

Craft,
Tester.
