# Domino-climb
**Domino Climb** is a platformer game originally made in Scratch. (https://scratch.mit.edu/projects/718841911/)

The current iteration is being written in C++ with the raylib library, for now it compiles to Windows x64, while I figure out the makefile for the rest of platforms lol

* You control a white domino piece, you can jump and rotate to move around the map.
* Your domino piece has two tiles, each of them can have from 0 to 6 pips.
* If a tile of your domino touches another tile with matching pips, they will interact! (idea in progress)
* Your objective is to match both of your tiles with another white domino.

# Done
- Basic motion and collision physics
- Rotation (first iteration)
- Map filesystem
- Pause menu

# Work in progress
- Level editor

# To do
- Textures
- Region-based level loading
- Multi-platform build system
- Tile behaviors
