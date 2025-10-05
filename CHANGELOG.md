# changelog

## v0.2.0-beta (05 Oct 2025)

#### changes
- added skybox colors for day/night cycle
- fixed menu buttons not disappearing after leaving menu (forgot to clear background)
- added windows support for building launcher and tests
- removed 'draw_text_centered()', added arguments 'alignX' and 'alignY' to 'draw_text()'
- added text alignment to left, center, right, top, center and bottom
- added window resize poll events
- fixed bug centering cursor to center of screen instead of center of viewport
- added texture alignment to left, center, right, top, center and bottom
- added functions kill_player() and respawn_player()
- changed chunk drawing into entire chunkBuf drawing
- added block texturing
- fixed player movement
- fixed player bounding box slidy
- fixed window close bugs
- fixed menu input events
- added <shift> + <F3> combination for special debug (e.g. draw bounding boxes)
- fixed bug mouse movement not registering with intended precision
- added secure malloc, free and zero memory
- added secure loading and unloading chunks
- added more definitions for world size and chunk size for more precision
- added get/set block data macros
- added player-position-relative chunk-loading
- added render-distance-based chunk loading
- added block_parse_limit optimization to limit block parsing to last non-air block in chunk block array
- added chunk_tab shifting to correct player targeting
- added world directory structure creation
- added basic terrain generation
- changed chunking system from 2d to 3d chunks (took me like 20 minutes)
    - removed block_parse_limit optimization, because 3d chunks
- added basic 3d renderer in opengl
    - added skybox colors for day/night cycle in renderer
- simplified build tool
- localized dependencies' headers and shared libraries
- changed project name "minecraft.c" -> "heaven-hell_continuum"
- added self-rebuild logic for build tool
- made dynamically-linked libraries local, still dynamic
- removed raylib
- improved engine directory handling (src/engine/dir.c)
- improved memory safety (src/engine/memory.c)
- improved platform abstraction:
    - (src/engine/platform_<platform>.c)
    - (src/engine/h/platform.h)
    - (src/platform_<platform>.c)
    - (src/h/platform.h)
- broke windows support, will fix in next version
- made asset-fetching independent from user's current working directory:
    - fetch '/proc/self/exe' on linux (the executed process' current directory)
- improved logging:
    - added log colors
    - added LOGTRACE for extremely verbose logging
- added font loading and font atlas baking
- added image reading/writing (stb_image_write.h)
- optimized and expanded input handling
- fixed gravity's framerate-dependence
- added easy motion
- added text newline wrapping
- added text alignment support for all 6 modes
- added anti-aliasing
- recovered chunk drawing and chunk generation
- added chunk gizmo
- added debug info for opengl version and other general info

### bugs
- chunk rendering draws chunk boundary faces for the furthest chunks at chunk-shift direction
- chunk gizmo size is screen size dependent
- gizmo size is screen size dependent
- geometry shaders' precision errors show up sometimes in DEBUG_MORE mode (semi-transparent blocks)
- setting render distance to 0 segfaults (ofc, but I'm not gonna patch that)

- - -
## v0.1.4-alpha (08 Apr 2025)

#### changes
- added windows support for build tool
- created minecraft.c logo 'resources/logo/'
- added platform layer C files 'linux_minecraft.c' and 'windows_minecraft.c'
- compiled and ran on windows successfully
- added game ticking
- added day/night cycle
- added compiled release_build for windows

- - -
## v0.1.3 (03 Apr 2025)

#### changes
- fixed segfault while placing or breaking blocks in non-allocated chunk area 
- switched to C99 standard
- changed original resources to avoid copyright
- made build system more difficult to read but easier to use (not tested on windows)

- - -
## v0.1.2 (01 Apr 2025)

#### changes
- can break and place blocks
- added menu navigation
- fixed breaking and placing blocks in other chunks than 'xy: 0, 0'
- fixed segfault while wandering into unloaded chunks (I returned 0 while I was supposed to return NULL...)
- fixed chunk states shifting by one block into a specific direction with each chunk in the positive direction and shift into the opposite direction with each chunk in the negative direction
    now I know why, I was doing 'at player target xyz', apparently I forgot to do 'at player target xyz % CHUNK_SIZE'.
    I abandoned this project for over 3 months because I couldn't figure out... somehow.
    so the chunk states weren't shifting by one block, they were shifting by a familiar value, if I printed the index number I was targeting I would have gotten somewhere maybe, but doesn't matter, I fixed it
- created instance directory structure
- created info/ directory to populate assets and array info
- wrote a proper logger
- instance directory structure creation successful
- added game launcher
- changed build system from Bash to C
- made 'defines.h' a local file

- - -
## v0.1.1 (27 Mar 2025)

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

- - -
## v0.1.0 (24 Mar 2025)

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
