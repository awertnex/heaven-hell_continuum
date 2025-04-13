#include <stdlib.h>
#include "../dependencies/raylib-5.5/src/raylib.h"
#include "../dependencies/raylib-5.5/src/rlgl.h"

#define VECTOR2_TYPES
#include "../h/defines.h"
#include "../h/logger.h"
#include "../logger.c"

#define MC_C_RED    (Color){0xff, 0x78, 0x78, 0xff}
#define MC_C_GREEN  (Color){0x78, 0xff, 0x78, 0xff}
#define MC_C_BLUE   (Color){0x78, 0x78, 0xff, 0xff}
#define CHUNK_COUNT         221 // parseable chunks within max render distance
#define CHUNK_TABLE_SIZE    289 // ((8*2) + 1)^2
#define CHUNK_TABLE_POS_X 300.0f
#define CHUNK_TABLE_POS_Y (720.0f/2.0f)
#define rect(x, y, col) DrawRectangleRounded((Rectangle){x + CHUNK_TABLE_POS_X, y + CHUNK_TABLE_POS_Y, 16.0f, 16.0f}, 0.3f, 1, col)

enum ChunkStates
{
    STATE_CHUNK_BUF_ALLOC =     0x1,
    STATE_CHUNK_TAB_ALLOC =     0x2,

    STATE_CHUNK_LOADED =        0x1,
    STATE_CHUNK_DIRTY =         0x2,
};

typedef struct Chunk
{
    v2i16 pos;
    u32 id;
    u32 i[16][16];
    u8 state;
} Chunk;

u8 state = 0;
Chunk *chunkBuf;
u64 *chunkTable;

int allocate_buffers();

void loop()
{
    for (u16 i = 0; i < CHUNK_COUNT; ++i)
    {
        if (!(state & (STATE_CHUNK_BUF_ALLOC | STATE_CHUNK_TAB_ALLOC)))
            break;

        if (chunkBuf[i*sizeof(Chunk)].state & STATE_CHUNK_LOADED)
            rect((f32)i, 0.0f, MC_C_RED);
    }
    rect(16.0f, 0.0f, MC_C_GREEN);
    rect(32.0f, 0.0f, MC_C_BLUE);
}

int main(void)
{
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(1280, 720, "Chunk Pointer Table");
    SetExitKey(KEY_Q);

    if (allocate_buffers())
        return -1;

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        loop();
        EndDrawing();
    }

    if (state & STATE_CHUNK_BUF_ALLOC)
    {
        free(chunkBuf);
        LOGINFO("%s", "chunkBuf Memory Deallocation Successful");
    }
    CloseWindow();
    return 0;
}

int allocate_buffers()
{
    chunkBuf = (Chunk*) malloc(CHUNK_COUNT*sizeof(Chunk));
    chunkTable = (u64*) malloc(CHUNK_TABLE_SIZE);

    if (&chunkBuf[0] != NULL)
    {
        state |= STATE_CHUNK_BUF_ALLOC;
        LOGINFO("%s", "chunkBuf Memory Allocation Successful");
    }
    else
    {
        LOGFATAL("%s", "chunkBuf Memory Allocation Failed, Aborting Process");
        return 1;
    }

    if (&chunkTable[0] != NULL)
    {
        state |= STATE_CHUNK_TAB_ALLOC;
        LOGINFO("%s", "chunkTable Memory Allocation Successful");
    }
    else
    {
        LOGFATAL("%s", "chunkTable Memory Allocation Failed, Aborting Process");
        return 1;
    }
    return 0;
}

