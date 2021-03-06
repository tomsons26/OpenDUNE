---------------------------------------------------------------------------

Purpose of this fork is to map symbol dumps i have gathered that
Westwood accidentally released publically with various games over the time
to the actual functions for reference for a currently unreleased project.

I have the Dune 2 exe mapped to what i know, maybe i can only find some
matching code, depends on how much it was changed in OpenDune.

Too bad there are no references from what part of the exe each function came
from. Well, we'll see how it goes.

Eventually i might restructure everything i have info on to how it was in
Westwoods sources with the function and filenames where possible.

---------------------------------------------------------------------------

About
-----
OpenDUNE is an open source re-creation of the popular game "Dune II",
 originally made by Westwood Studios, and released by Virgin Entertainment.
It attempts to re-create the original game and apply modern technology to it
 to allow it to be run natively on most operating systems.

OpenDUNE is licensed under the GNU General Public License version 2.0. For
 more information, see the COPYING file included with every release and source
 download of the game.


Contact
-------
The latest version of OpenDUNE is always available at:
  http://www.opendune.org/
The latest development version is at:
  https://github.com/OpenDUNE/OpenDUNE
Our IRC (chat) is at:
  irc://irc.oftc.net/OpenDUNE
You can report bugs at:
  https://github.com/OpenDUNE/OpenDUNE/issues
Our wiki is at:
  https://github.com/OpenDUNE/OpenDUNE/wiki


Supported Platforms
-------------------
OpenDUNE is written in ANSI C (C89), and only requires SDL/SDL2 to run. In result,
 OpenDUNE should run on all platforms supported by SDL or SDL2.
Currently we officially support the following platforms:

  - Linux
  - FreeBSD
  - Mac OS X (PowerPC or Intel i686 / x86_64, 10.4+)
  - Windows (i686 / x86_64)
  - Atari TOS (68030+ CPU, TT and Falcon machines supported)


Requirements
------------
OpenDUNE doesn't require any library to run on Windows. For Mac OS X we make
 so-called static-compiled binaries, which means all libraries it depends on
 (mostly LibSDL) are already included in the binary, and no action is
 required on your part
For Linux/FreeBSD, you need to install LibSDL yourself. It is available in every
 package manager, and the chances are high it is already installed on your
 system.
In order to use sounds and music on Linux, you need a working ALSA driver.
Music is sent to MIDI Out port of Atari machines.
It is also possible to build with Munt MT32 emulator http://munt.sourceforge.net/
to have MT32 music.


Installation & Running
----------------------
Extract OpenDUNE.
Copy the original Dune2 1.07 data files (including dune2.exe) to data/.
 All three existing versions of the Dune 1.07 data files (eu, hs and us) will
  work, but only with the eu/hs data files the French language will work, and
  only with the eu data files the German language will work.
Start 'opendune'.

Additional options may be specified using an opendune.ini file located
in the data/ directory, in the current directory or in %APPDATA%\OpenDUNE
(on Windows) or ~/Library/Application Support/OpenDUNE (on Mac OS X) or
~/.config/opendune (on Linux/FreeBSD). All options must be in an [opendune]
section.

Available options are :
- language : english / french / german
- datadir : directory where Dune data files are
- savedir : directory for Dune personal data files (savegames)
- scalefactor : 2 (default), 3, 4
- scalefilter : nearest (default), scale2x, hqx
- framerate : maximum frame rate (60 FPS default)
- mt32midi : 0(default)/1 send MT32 init, use .XMI files
- mt32rompath : directory containing CM32L_CONTROL.ROM/CM32L_PCM.ROM files
                for Munt MT32 emulator.


Ingame
------
The Game controls are the same as DUNE II. Usually, the first letter is a
keybard shortcut, for example B for build, Q for quit, etc.
F1 - open Mentat Screen
F2 - open the option menu
F3 - open the menu of the selected structure (construction yard/factory/etc.)

A few key controls are added in OpenDUNE, available depending on the
platform :
F8 - Toggle FPS display
CTRL-ENTER - Toggle full screen


Enhancement over Dune2
-----------------------
See enhancement.txt.


Known Bugs
----------
See known_bugs.txt.


Changelog
---------
See changelog.txt.


Credits
-------
The OpenDUNE team (in alphabetical order):
  Albert Hofkamp (Alberth)             - Refactoring Dude
  Loic Guilloux (glx)                  - Windows Guru
  Patric Stout (TrueBrain)             - Lead Developer
  Steven Noorbergen (Xaroth)           - Lead Manager

Thanks To (in alphabetical order):
  Szabolcs Nagy (nsz)                  - ANSI C Guru
  Ingo von Borstel (planetmaker)       - For his many bug-reports
  tneo                                 - For his many bug-reports
  David Wang (wangds)                  - For his many patches and bug-reports

  Bug Reporters                        - Thank you all for all bug reports
  Westwood                             - For an amazing game
