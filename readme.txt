Thanks for downloading this distribution of Eldritch's source code!

Please note that this is an unsupported, unmaintained release;
documentation is limited to this readme file and I will not be
integrating changes. Questions and bug reports are welcome at
david.pittman@gmail.com, but I cannot promise I will be able to
respond.

Note also that this will not compile out of the box; Eldritch
depends on the FMOD Ex Programmer's API, which is not licensed for
redistribution but can be downloaded at http://www.fmod.org/.
Eldritch was built with FMOD Ex 4.44.15, but any recent release
is likely to build fine.

It will be necessary for OS X and Linux users to download SDL 2.0
to build Eldritch. For Windows users, the library files included
here should suffice.

IDE project files are provided for Visual Studio 2005, Code::Blocks,
and Xcode, but it may be more desirable to rebuild your own project
file. This was my first time working with either Code::Blocks or
Xcode, and it is likely that they are configured poorly.

This distribution does not include most of the game's content, but
configuration, script, and room files are included and are licensed
the same as the source code. (All game content not included here is
not covered by these license terms and may not be redistributed.)

For Windows users, a precompiled Eldritch-Release.exe file is
provided for immediate access to the game's debug tools and level
editor. To access the editor, press Tab during gameplay.

The game's .cpk files (not included here; you must purchase a copy
of Eldritch to get them) can be unpacked with the FilePacker.exe tool.
Deleting, renaming, or moving the .cpk files will force the game to
load from loose files instead.

Configuration/script files (.config extension) are used to define
most aspects of the game: procedural generation parameters, entity
composition, AI behaviors, etc. They can be parsed at runtime or
precompiled with ConfigCompiler.exe.

The source code for the tools is included, but they have not been
ported to OS X or Linux.

Happy coding!
David