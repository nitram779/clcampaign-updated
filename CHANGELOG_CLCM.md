# Crack-Life: Campaign Mode Updated changelog

## Changes in v1.0

> Note: this version has not been released yet.

### Bug fixes
* Fixed animations code for many weapons.
* Fixed Death sound not playing until quick save/load or level change.
* Fixed Human Grunt idle sounds.
* Fixed `"debris/metal*.wav not precached"` spam in `lvl7b` map.
* Fixed MegaChav and Shrek hull size.
* Terror's "scream" sound now stops when dies.
* Added missing Alien Grunt sounds.
* Added missing Katata swinging sounds.

### New features
* Implemented missing skill cvars for NPCs and weapons.
* Player Taunt sounds are now plays trough `EMIT_SOUND` instead of `SENTENCEG_PlayRndSz`.
* `impulse 101` now gives all weapons.

### Project changes
* Added skill.cfg

### Code cleanup
* Added `crowbar.h` header for make it easier to use of `FindHullIntersection` instead of copy/pasting code.
* Removed useless Anim events and skill cvars for most of Zombie based NPCs (e.g Chavs).
