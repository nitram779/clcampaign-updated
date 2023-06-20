# Crack-Life: Campaign Mode Updated changelog

## Changes in v1.0

> Note: this version has not been released yet.

### Changes from the original mod
* Fixed weapon animations.
* Fixed death sound wouldn't play unless you did a quick save/load or level change.
* Fixed Human Grunt idle sounds.
* Fixed `"debris/metal*.wav not precached"` console spam on the `lvl7b` map.
* Fixed MegaChav and Shrek hull size.
* Terror's alert sound will now stop when he dies.
* Added missing Alien Grunt sounds.
* Added missing Katata swinging sounds.
* Missing skill cvars for NPCs and weapons have been added.
* Crowbar/Fist taunt animation and sounds now moved to client side.
* `impulse 101` now gives all weapons.
* Removed useless anim events and skill cvars for most of Zombie-based NPCs (e.g Chavs).

### Changes from the Half-Life Updated repo
* Added `crowbar.h` header to make it easier to use `FindHullIntersection` instead of copy/pasting code.
* View Roll is disabled by default.
* Added `game_dir`
* Added `skill.cfg`
* Added Crack-Life entities to `fgd/halflife.fgd`
