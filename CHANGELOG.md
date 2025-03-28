# changelog

## v0.1.2 - (DD Apr 2025)
-------------------------
#### changes
- can break and place blocks
- added menu navigation

## v0.1.1 - (27 Mar 2025)
-------------------------
#### changes
- added many button names for menus and containers enum
- added all menu names enum
- added all container names enum
- implemented more menus (just 1 more, now I have title screen and game menu working well, next I'll do Options)
- got title screen working fine
- organized game flow a little better
- clicking 'Singleplayer' loads the world
- clicking 'Save and Quit to Title' goes back to title screen, but doesn't save, just keeps the game state saved for the session
- fixed fullscreen (it's actually borderless windowed mode)
- made the codebase a lot more readable
- added delta time

## v0.1.0 - (24 Mar 2025)
-------------------------
**first version, as I have started this project long before I version-controlled it with git**

#### features
- basic gui functionality
- centering text vertically and horizontally
- basic controls:
    - player movement: sprinting, crouching and jumping
    - build, break, look around
    - 5 camera modes:
        - 1st person
        - 3rd person: back
        - 3rd person: front
        - cinematic (anchors to random blocks and targets player)
        - spectator (like the original, but without dragging the player character along)
    - open inventory gui
    - switch active inventory slot
    - show/hide hud
    - show/hide debug info (F3)
- pause screen, back to game and quit game
- minecraft instance directory creation or opening if exists (instance name = argv 1)
- basic chunking, rigid, buggy, but there.
- basic debugger interface, doesn't do anything, just pops up and shows bounding boxes
- my favorite: segfaulting while wandering into unloaded chunks
- my 2nd favorite: chunk states shift by one block into a specific direction with each chunk in the positive direction and shift into the opposite direction with each chunk in the negative direction, don't know why, I've never encountered such goofy behavior and I'm stuck here
