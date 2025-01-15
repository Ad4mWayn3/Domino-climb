# Domino-climb
**Domino Climb** is a platformer game originally made in Scratch. (https://scratch.mit.edu/projects/718841911/)

The current iteration is being written in C++ with the raylib library, for now it compiles to Windows x64, while I figure out the makefile for the rest of platforms lol

* Your primary objective is to *climb your way to the top of the level*, where height itself is your biggest threat. The higher you go, the easier it is to fall!
* The original game used mechanics like crouch and crouch-jumping for more varied platforming, inspired directly by Counter-Strike. It also included special platform types: The orange one mirrors the player speed, working as a bouncer; whlie the purple one gives a stronger jump impulse.

# Done
- Dynamic playmode camera
- Simple level editor and playmode
- Basic motion and collision physics
- Friction and speed caps

# To do
- Pause menu
- Region-based level loading
- Domino character
- Special platforms
- Update Makefile to compile to multiple platforms and architectures

# Ideas
- The domino can change its numbers, suggesting some sort of puzzle mechanics
- Bounce platforms: Bounciness makes for more versatile and creative level design. Different levels of bounciness are to be used.
- Impulse platforms: Increase the player's horizontal acceleration, allowing for longer jumps
- Boost platforms: Increase the player's jump impulse, opening up the verticality.
- Dice: Have the same dot-style numbers as domino pieces so they are kinda fitting thematically, not sure how they would fit into the game mechanics yet.
