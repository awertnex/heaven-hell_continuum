TASKS:

[!] display Chunk coordinates
[!] detect targeted block
[!] place blocks
[!] break blocks
[!] figure out delta time (25 Mar 2025)
[ ] detect new chunk, allocate memory and spawn accordingly
[ ] fix seg fault when player target enters non-allocated chunk area
[ ] fix funky chunk states shifting away by 1 unit each chunk
[ ] change chunk_buff allocation from stack to heap
[ ] make a gui test and load all game menus first
[ ] implement code hot reloading
[ ] tokenize chunk block state files
    - e.g. {1,1,1,3,3,7,1,5,5,2,2,2,2,2,2} -> {1x3,3x2,7,1,5x2,2x6}
