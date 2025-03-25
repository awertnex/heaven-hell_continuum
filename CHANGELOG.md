# changelog

## v0.1.0:
- - - -
**first version, as I have started this project long before I version-controlled it with git**

#### features:
- segfaulting while wandering into unloaded chunks
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
