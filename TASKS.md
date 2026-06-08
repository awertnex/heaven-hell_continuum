TASKS: [x] = done, [!] = cancelled
DONE DATE: (YYYY MM DD)

STATUS  DONE DATE       TASK

- [x]   (XXXX XX XX):   display Chunk coordinates
- [x]   (XXXX XX XX):   detect targeted block
- [x]   (2025 XX XX):   place blocks
- [x]   (2025 XX XX):   break blocks
- [x]   (2025 03 25):   figure out delta time
- [x]   (2026 01 01):   make the player target a ray-caster
- [x]   (2025 05 09):   detect new chunk, allocate memory and spawn accordingly
- [x]   (2025 03 29):   fix segfault when player target enters non-allocated chunk area
- [x]   (2025 04 02):   fix segfault while placing or breaking blocks in
                        non-allocated chunk area
- [x]   (2025 03 29):   fix funky chunk states shifting away by 1 unit each chunk
- [x]   (2025 04 14):   change chunk_buff allocation from stack to heap
- [ ]   (          ):   navigate menus with arrow keys
- [ ]   (          ):   compress chunk block state files
- [x]   (2025 05 26):   figure out why the button "Back to Game" is inactive
- [ ]   (          ):   rewrite entire gui
- [ ]   (          ):   make gameTick stop when game is paused, and save state
                        on quit-to-title
- [x]   (2025 05 10):   fix fullscreen, still not going back to original size
                        on toggle off
- [x]   (2025 05 10):   fix fullscreen affecting mouse delta
- [x]   (2025 05 22):   fix hollow states at top of chunk
- [x]   (2025 05 21):   fix remove_block block_parse_limit math

quirks:
- [x]   (2025 09 27):   fix haloing (black traces when drawing an RGB texture
                        onto an RGBA framebuffer, more visible when aspect
                        ratio isn't 1:1)
- [x]   (2025 09 10):   fix weird framebuffer scaling on window scaling
                        (possibly reallocate framebuffers)

- [!]   (2026 01 16):   bake skybox into a look-up table, or HDR (retain old
                        code, just because it was fun to write)
    - reason for cancellation: I got the code right, it looks good

- [x]   (2026 01 10):   limit framerate
- [x]   (2025 12 19):   add camera look-at
- [ ]   (          ):   fix double press for windows
- [x]   (2025 10 13):   fix chunk queue sort
- [x]   (2025 10 09):   make the chunk generation count per frame
- [x]   (2025 11 XX):   make mem_map() for windows
- [x]   (2025 11 XX):   make mem_commit() for windows
- [x]   (2025 11 XX):   make mem_unmap() for windows
- [x]   (2025 12 09):   fix overhead from function 'chunk_tab_shift()'
- [ ]   (          ):   possibly fix overhead from bubble-sorting in function
                        'chunking_init()'
- [ ]   (          ):   put blocks in a contiguous array and bind it all to an
                        SSBO, and bind
                        chunk pointers to their addresses
- [ ]   (          ):   fix segfault when allocating smaller than 256 bytes for
                        'size' parameter of function 'mem_alloc_buf()'
