# Sonic 3 A.I.R.
Source code incl. dependencies for "Sonic 3 - Angel Island Revisited", a fan-made remaster of Sonic 3 & Knuckles.

Project homepage: https://sonic3air.org/

### Disclaimer
Sonic 3 A.I.R. is a non-profit fan game project. It is not affiliated in any way with SEGA or Sonic Team, the original creators of Sonic 3 and Sonic & Knuckles.

Sonic the Hedgehog is a trademark of SEGA. All copyrights regarding Sonic the Hedgehog, including characters, names, terms, art, and music belong to SEGA. All registered trademarks belong to SEGA and Sonic Team.

The developers of Sonic 3 A.I.R. have no intent to infringe said copyrights and registered trademarks.
No financial gain is made from this project.

Any commercial use of this project without SEGA's explicit consent is strictly prohibited.

## PortMaster Fork
This particular fork is an experimental process to test viability for use in [PortMaster](https://portmaster.games/). Immediate changes needed include making most dependencies shared, not static, to prevent interfering with custom firmwares on retro handheld devices PortMaster supports.

## Repository overview
This repository is split into several different projects:
* The larger external dependencies (namely SDL2, Ogg/Vorbis, zlib) inside the "framework" directory. These are copies of the respective open source projects, with a few custom changes applied where needed - see the "how-to-build.txt" files in there for details.
* The librmx libraries that together with the external dependencies build a foundation for my own projects (S3AIR and my private stuff). This primarily consists of rmxbase, a collection of helper and utility classes, and rmxmedia, which is providing a basic game framework built on top of SDL2 & OpenGL.
* Lemonscript language library, with compiler and runtime environment for script execution.
* Oxygen Engine, the backbone game engine powering Sonic 3 A.I.R. This includes input, graphics, audio handling, and all the other game application stuff, as well as a simulation environment built around lemonscript that - as far as needed for the purposes of S3AIR - emulates aspects of Sega Genesis hardware. Note that Oxygen can be built as its own application (OxygenApp) that lacks the S3AIR C++ code.
* S3AIR-specific C++ code, scripts and data in the "Oxygen/sonic3air" directory. Yes, that's what it's named.

## How to build
For information on how to build for different platforms, find the readme files in the respective subdirectories of "Oxygen/sonic3air/build":
* Windows: "_vstudio"
* Linux:   "_cmake"
* Android: "_android"

Additional platform ports:
* Vita: See https://github.com/v-atamanenko/sonic3air

## External dependencies
External libraries and code used in this project:
* SDL2 - in "framework/external/sdl"
* libogg & libvorbis - in "framework/external/ogg-vorbis"
* zlib incl. minizip - in "framework/external/zlib"
* libcurl - in "framework/external/curl"
* jsoncpp - in "librmx/source/rmxbase/jsoncpp"
* GLEW - in "librmx/source/rmxmedia/glew"
* Sound chip emulation related code from Genesis Plus GX - in "Oxygen/oxygenengine/source/oxygen/simulation/sound"
* Discord Game SDK - in "Oxygen/sonic3air/source/external/discord_game_sdk"
* xBRZ upscaler shader code - in "Oxygen/oxygenengine/data/shader" and once more in "Oxygen/sonic3air/data/shader"
* Hqx upscaler shader code & data files - in "Oxygen/oxygenengine/data/shader" and once more in "Oxygen/sonic3air/data/shader"

## Contributors
Thanks to all contributors!

Source code contributions by:
* Sappharad
* Heyjoeway
* Carjem Generations
* Ultracoolguy
* gl33ntwine
* Rinnegatamante
* MDashK

Remastered soundtrack by:
* G Spindash

Game scripts & other contributions by:
* Vinegar
* Thorn
* Legobouwer
* GFX32
* Dynamic Lemons
* HazelSpooder
* iCloudius
* D.A. Garden
* Alieneer
* 3Pills
* Elsie The Pict
* TheMushrunt
* mrgrassman14