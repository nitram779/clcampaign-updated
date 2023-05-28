About
======================

New reimplementation of [Crack-Life: Campaign Mode](https://www.moddb.com/mods/crack-life/downloads/crack-life-campaign-mode) code based on [SamVanheer's Half-Life Updated](https://github.com/SamVanheer/halflife-updated) and [LostGamerHL's reimplementation](https://github.com/LostGamerHL/crack_life).

Purpose
======================

Each repository provides project files compatible with Visual Studio 2019 and 2022, as well as bug fixes. The Opposing Force and Blue Shift projects are reference implementations of their respective games. This means they provide the original features, implemented as they are in the original games, including the many cases of code duplication.

The goal of the Updated repositories is to allow modders to make mods based on these games, while providing bug fixes that could be applied to the official games as well. A mod installation is also provided for each repository to allow players to play these games with all bug fixes applied.

This mod installation includes files that are required when making a mod based on these SDKs.

The **scope** of the Updated projects is **limited to bug fixes**, **feature requests to improve the game's code** and **fixing game-breaking bugs in the game assets** (e.g. soft-locked trigger setups). Changes like **graphical upgrades**, **physics engine changes** and other **engine changes** are **out of scope**.

If you need help setting up the SDK or developing a mod please ask on the [TWHL website](https://twhl.info/) or on its [Discord server](https://discord.gg/jEw8EqD).

The TWHL wiki has tutorials to guide you through making a mod: https://twhl.info/wiki/page/Half-Life_Programming_-_Getting_Started

See the `#welcome` channel for more information about the Discord server. Please do not use the `#unified-sdk` channel for general help requests, there are channels for modding help.

See the TWHL thread for status updates about these projects: https://twhl.info/thread/view/20055

Requirements to run mods built with this SDK
======================

Only the latest Steam version of Half-Life is supported.

Only the latest [testing builds of Xash3D FWGS](https://github.com/FWGS/xash3d-fwgs/releases/tag/continuous) is supported. (With some bugs like [crowbar/katana/hammer swinging twice.](https://github.com/FWGS/xash3d-fwgs/issues/1150))

Building this SDK
======================

See [BUILDING.md](BUILDING.md)

Mod installation instructions
======================

See [INSTALL.md](INSTALL.md)

What isn't supported
======================

Backwards compatibility with WON and older versions of Steam Half-Life is not supported.

Original Uncle Mike's Xash3D and Xash3D FWGS Old Engine isn't supported.

These repositories have a limited scope and will not have major changes applied.

Changelog
======================

See [CHANGELOG_CLCM.md](CHANGELOG_CLCM.md)

For Half-Life updated changelog, see [CHANGELOG.md](CHANGELOG.md) and [FULL_CHANGELOG.md](FULL_CHANGELOG.md)

Half Life 1 SDK LICENSE
======================

Half Life 1 SDK Copyright© Valve Corp.  

THIS DOCUMENT DESCRIBES A CONTRACT BETWEEN YOU AND VALVE CORPORATION (“Valve”).  PLEASE READ IT BEFORE DOWNLOADING OR USING THE HALF LIFE 1 SDK (“SDK”). BY DOWNLOADING AND/OR USING THE SOURCE ENGINE SDK YOU ACCEPT THIS LICENSE. IF YOU DO NOT AGREE TO THE TERMS OF THIS LICENSE PLEASE DON’T DOWNLOAD OR USE THE SDK.

You may, free of charge, download and use the SDK to develop a modified Valve game running on the Half-Life engine.  You may distribute your modified Valve game in source and object code form, but only for free. Terms of use for Valve games are found in the Steam Subscriber Agreement located here: http://store.steampowered.com/subscriber_agreement/ 

You may copy, modify, and distribute the SDK and any modifications you make to the SDK in source and object code form, but only for free.  Any distribution of this SDK must include this license.txt and third_party_licenses.txt.  
 
Any distribution of the SDK or a substantial portion of the SDK must include the above copyright notice and the following: 

DISCLAIMER OF WARRANTIES.  THE SOURCE SDK AND ANY OTHER MATERIAL DOWNLOADED BY LICENSEE IS PROVIDED “AS IS”.  VALVE AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES WITH RESPECT TO THE SDK, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY, NON-INFRINGEMENT, TITLE AND FITNESS FOR A PARTICULAR PURPOSE.  

LIMITATION OF LIABILITY.  IN NO EVENT SHALL VALVE OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THE ENGINE AND/OR THE SDK, EVEN IF VALVE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.  
 
 
If you would like to use the SDK for a commercial purpose, please contact Valve at sourceengine@valvesoftware.com.


Half-Life 1
======================

This is the README for the Half-Life 1 engine and its associated games.

Please use this repository to report bugs and feature requests for Half-Life 1 related products.

Reporting Issues
----------------

If you encounter an issue while using Half-Life 1 games, first search the [issue list](https://github.com/ValveSoftware/halflife/issues) to see if it has already been reported. Include closed issues in your search.

If it has not been reported, create a new issue with at least the following information:

- a short, descriptive title;
- a detailed description of the issue, including any output from the command line;
- steps for reproducing the issue;
- your system information.\*; and
- the `version` output from the in‐game console.

Please place logs either in a code block (press `M` in your browser for a GFM cheat sheet) or a [gist](https://gist.github.com).

\* The preferred and easiest way to get this information is from Steam's Hardware Information viewer from the menu (`Help -> System Information`). Once your information appears: right-click within the dialog, choose `Select All`, right-click again, and then choose `Copy`. Paste this information into your report, preferably in a code block.

Conduct
-------


There are basic rules of conduct that should be followed at all times by everyone participating in the discussions.  While this is generally a relaxed environment, please remember the following:

- Do not insult, harass, or demean anyone.
- Do not intentionally multi-post an issue.
- Do not use ALL CAPS when creating an issue report.
- Do not repeatedly update an open issue remarking that the issue persists.

Remember: Just because the issue you reported was reported here does not mean that it is an issue with Half-Life.  As well, should your issue not be resolved immediately, it does not mean that a resolution is not being researched or tested.  Patience is always appreciated.
