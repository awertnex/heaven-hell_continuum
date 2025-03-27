TASKS:

[x] display Chunk coordinates
[x] detect targeted block
[x] place blocks
[x] break blocks
[x] figure out delta time (25 Mar 2025)
[ ] make the player target a ray-caster
[ ] detect new chunk, allocate memory and spawn accordingly
[ ] fix seg fault when player target enters non-allocated chunk area
[ ] fix funky chunk states shifting away by 1 unit each chunk
[ ] change chunk_buff allocation from stack to heap
[ ] make a gui test and load all game menus first
[ ] navigate menus with arrow keys
[ ] implement code hot reloading
[ ] tokenize chunk block state files
    - e.g. {1,1,1,3,3,7,1,5,5,2,2,2,2,2,2} -> {1x3,3x2,7,1,5x2,2x6}
[x] figure out why the button "Back to Game" is inactive (26 Mar 2025)
