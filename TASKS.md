TASKS:

[x] display Chunk coordinates
[x] detect targeted block
[x] place blocks
[x] break blocks
[x] figure out delta time (25 Mar 2025)
[ ] make the player target a ray-caster
[ ] detect new chunk, allocate memory and spawn accordingly
[x] fix segfault when player target enters non-allocated chunk area (29 Mar 2025)
[x] fix segfault while placing or breaking blocks in non-allocated chunk area (02 Apr 2025)
[x] fix funky chunk states shifting away by 1 unit each chunk (29 Mar 2025)
[ ] change chunk_buff allocation from stack to heap
[ ] navigate menus with arrow keys
[ ] tokenize chunk block state files
    - e.g. {1,1,1,3,3,7,1,5,5,2,2,2,2,2,2} -> {1x3,3x2,7,1,5x2,2x6}
[x] figure out why the button "Back to Game" is inactive (26 Mar 2025)
[ ] rewrite the entire gui, make it more flexible, scalable and modular
