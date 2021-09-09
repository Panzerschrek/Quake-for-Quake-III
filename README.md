# Quake for Quake III

**Quake for Quake III** is a port of Quake 1 to Quake III Arena engine (idTech3).
This ports is implemented as pk3 mod that may be launched as any other mod for Quake III Arena, using retail version of Quake III arena itself or ioquake3.

## Why?

Once i decided to do this to check if it is possible or not. As you can see it is possible (with some limitations).

Also such mod allows Quake mappers to build maps with advanced textures/effects/shaders which are supported by Quake III engine.
It's possible to use 32-bit textures, advanced textures animations and scrolling, transparency, alpha-test, colored lighting, fog, and a lot of other features and effects of Quake III engine.

## How may i get it?
Build based on shareware Quake resources is available on _releases_ page.
But you can't get full version so easily. Because of legal reasons i can't distribute a build of this mod via (for example) GitHub.
But later i may found a way to distribute it without getting blocked by Bethesda (current Quake resources copyright holder).
So, check periodically for updates of this README.

## Technical details

All game resources are converted into Quake III engine format.
Server part of Quake code (QuakeC VM, physics, etc.) are implemented in `qagame` (vm/dll) module.
Client part of Quake code (server interaction, particles, rendering setup) are implemented in `cgame` module.
Quake menu are implemented in `ui` module.

## Technical limitations

Quake III engine does not support lighstyles, so, Quake dynamic map lights are not working properly.
But dynamic lights like for explosions and Quad Damage glow are ok.

Sound engine in Quake III is slightly different than in Quake. There is no looping sounds support like in original game - with auto looping.
Also it's not possible to specify custom attenuation or even custom volume. Because of that this mod sounds a bit different than original game.

## How to build tools and VM files

Clone this repository, run CMake for root cmake file, generate project for your favorite IDE/build system.
You may need to specify Q3_LCC and Q3_ASM cmake variables for LCC and Q3Asm form Quake III SDK distribution or from ioquake3 build.

## How to build pk3 file

To build pk3 file for **Quake for Quake III** first you need to extract files from original Quake PAK files, using DePAKer tool.

Sound files can be copied directly into `sounds` directory of pk3 file.

Models may be converted via special `convert_models.py` script.
Convert them and place into `progs` directory of result pk3 file.

BSP models (for health/ammo boxes) may be converted via same `convert_models.py` script.
Converted BSP models should be placed into `maps` directory of result pk3 file.

`convert_models.py` script produces not only models files, but also textures and shader file.
Textures should be placed into `models_textures` directory of pk3 file.
Shader files should be placed into `scripts` directory of pk3 file.

GFX pictures may be converted via `convert_pictures.py` script.
Result pictures should be placed into `gfx`directory of pk3 file.

There is no (yet) any way to convert some of Quake resources like map or textures.
Because of that original Quake map sources (released by John Romero in 2006) and recreated textures WAD file are placed in this repo.

To compile maps you need to use `coompile_maps.py` script.
Result maps should be placed into `maps` directory of pk3 file.

Textures may be extracted from WAD file and converted via `export_scripts.py` script.
Result textures should be placed into `textures` directory of pk3 file.

There are also some resources that should be copied from this repo into result pk3 file - contents of `quake_for_quake3_gfx` and `scripts` directories.

`palette.lmp` file should be placed into root of pk3 file.
`progs.dat` file should be placed into `vm` directory of pk3 file.

Lastly QuakeVM files (qagame.qvm, cgame.qvm, ui.qvm) should be placed into `vm` directory of pk3 file.

It's possible to put music files into pk3 file.
Music tracks should be placed into `music` directory and should be named track02, track03, track04 ... etc.
ioquake3 supports tracks in `ogg` format for Quake III Arena 1.32 i am not sure which music formats are supported.


This list of operations is incomplete.
There are some number of manual operations, like textures/images fixes that should be done manually.

Together with pk3 file this mod should contain also custom `q3config.cfg` file. Copy it from this repo into mod directory.

## Notes For Quake mappers

It's possible to compile your maps designed for Quake for this mod.
Just compile it as map for Quake III Arena, using q3map or q3map2.
Before this you may need to extract textures from your WAD file(s).

If you use custom models you may need to convert them into md3 formats (see Qwalk tool and `convert_models.py` script).

If you use custom `progs.dat` file you can just put it into pk3 file with your map.

## Authors

Tools, game code:
Copyright © 2021 Artöm "Panzerscrek" Kunç.

Original Quake sources (which are partially used in this project):
Copyright (C) 1996-1997 Id Software, Inc.

Quake III Arena sources:
Copyright (C) 1999-2005 Id Software, Inc.

ioquake3: [https://github.com/ioquake/ioq3](URL)
