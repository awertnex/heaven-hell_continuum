#include <math.h>

#include "h/chunking.h"
#include "h/logic.h"

#include "engine/h/memory.h"
#include "engine/h/logger.h"

u16 world_height = WORLD_HEIGHT_NORMAL;
Chunk* chunk_buf = {0};
void* chunk_tab[CHUNK_BUF_ELEMENTS] = {0};
Chunk* target_chunk = 0; //temp
struct WorldStats world_stats = {
    .block_count = 0,
    .quad_count = 0,
};
u8 opacity = 0;

u8 init_chunking() {
    MC_C_ALLOC(chunk_buf, CHUNK_BUF_ELEMENTS * sizeof(Chunk));
    return 0;

cleanup:
    free_chunking();
    return -1;
}

void free_chunking() {
    MC_C_FREE(chunk_buf, CHUNK_BUF_ELEMENTS * sizeof(Chunk));
}

//TODO: check chunk-border block-faces
void add_block(Chunk* chunk, u8 x, u8 y, u16 z) {
    x %= CHUNK_DIAMETER;
    y %= CHUNK_DIAMETER;

    if (x < CHUNK_DIAMETER - 1)
        chunk->i[z][y][x + 1] ? (chunk->i[z][y][x + 1] &= ~NEGATIVE_X) : (chunk->i[z][y][x] |= POSITIVE_X);
    else chunk->i[z][y][x] |= POSITIVE_X;

    if (x > 0)
        chunk->i[z][y][x - 1] ? (chunk->i[z][y][x - 1] &= ~POSITIVE_X) : (chunk->i[z][y][x] |= NEGATIVE_X);
    else chunk->i[z][y][x] |= NEGATIVE_X;

    if (y < CHUNK_DIAMETER - 1)
        chunk->i[z][y + 1][x] ? (chunk->i[z][y + 1][x] &= ~NEGATIVE_Y) : (chunk->i[z][y][x] |= POSITIVE_Y);
    else chunk->i[z][y][x] |= POSITIVE_Y;

    if (y > 0)
        chunk->i[z][y - 1][x] ? (chunk->i[z][y - 1][x] &= ~POSITIVE_Y) : (chunk->i[z][y][x] |= NEGATIVE_Y);
    else chunk->i[z][y][x] |= NEGATIVE_Y;

    if (z < world_height - 1)
        chunk->i[z + 1][y][x] ? (chunk->i[z + 1][y][x] &= ~NEGATIVE_Z) : (chunk->i[z][y][x] |= POSITIVE_Z);
    else chunk->i[z][y][x] |= POSITIVE_Z;
    
    if (z > 0)
        chunk->i[z - 1][y][x] ? (chunk->i[z - 1][y][x] &= ~POSITIVE_Z) : (chunk->i[z][y][x] |= NEGATIVE_Z);
    else chunk->i[z][y][x] |= NEGATIVE_Z;

    chunk->i[z][y][x] |= NOT_EMPTY;
}

//TODO: check chunk barrier faces
void remove_block(Chunk* chunk, u8 x, u8 y, u16 z) {
    x %= CHUNK_DIAMETER;
    y %= CHUNK_DIAMETER;

    if (x < CHUNK_DIAMETER - 1)
        chunk->i[z][y][x + 1] ? (chunk->i[z][y][x + 1] |= NEGATIVE_X) : 0;

    if (x > 0)
        chunk->i[z][y][x - 1] ? (chunk->i[z][y][x - 1] |= POSITIVE_X) : 0;

    if (y < CHUNK_DIAMETER - 1)
        chunk->i[z][y + 1][x] ? (chunk->i[z][y + 1][x] |= NEGATIVE_Y) : 0;

    if (y > 0)
        chunk->i[z][y - 1][x] ? (chunk->i[z][y - 1][x] |= POSITIVE_Y) : 0;

    if (z < world_height - 1)
        chunk->i[z + 1][y][x] ? (chunk->i[z + 1][y][x] |= NEGATIVE_Z) : 0;
    
    if (z > 0)
        chunk->i[z - 1][y][x] ? (chunk->i[z - 1][y][x] |= POSITIVE_Z) : 0;

    chunk->i[z][y][x] = 0;
}

void load_chunk(Chunk* chunk) {
    if ((chunk->state == STATE_CHUNK_LOADED) && (chunk->state != STATE_CHUNK_DIRTY))
        return;

    chunk->mat = LoadMaterialDefault();

    u16 z = 0; u8 y = 0, x = 0;
    for (; z < world_height; ++z) {
        for (; y < CHUNK_DIAMETER; ++y) {
            for (; x < CHUNK_DIAMETER; ++x) {
                add_block(chunk, x, y, z);
                if (chunk->i[z][y][x])
                    ++world_stats.block_count;
                if (chunk->i[z][y][x] & POSITIVE_X)
                    ++world_stats.quad_count;
                if (chunk->i[z][y][x] & NEGATIVE_X)
                    ++world_stats.quad_count;
                if (chunk->i[z][y][x] & POSITIVE_Y)
                    ++world_stats.quad_count;
                if (chunk->i[z][y][x] & NEGATIVE_Y)
                    ++world_stats.quad_count;
                if (chunk->i[z][y][x] & POSITIVE_Z)
                    ++world_stats.quad_count;
                if (chunk->i[z][y][x] & NEGATIVE_Z)
                    ++world_stats.quad_count;
            }
            x = 0;
        }
        y = 0;
    }
    chunk->state = STATE_CHUNK_LOADED;
}

void update_chunk() { // TODO: make this function
}

void unload_chunk() { // TODO: make this function
}

void update_chunk_buffer(v3i32* player_target, v2i16* player_chunk) { // TODO: complete this function
    for (u16 i = 0; i < 1; ++i) {
        if (!(chunk_buf[i].state & STATE_CHUNK_LOADED)) {
            load_chunk(&chunk_buf[i]);
            chunk_tab[CHUNK_TAB_CENTER] = &chunk_buf[i];
            LOGINFO("Chunk[%03d %03d] Loaded, i[%03d]", player_chunk->x, player_chunk->y, i);
            break;
        }
    }
}

Chunk* get_chunk(v3i32* coordinates, u16* state, u16 flag) { // TODO: revise, might not be needed
    for (u8 y = 0; y < setting.render_distance * 2; ++y) {
        for (u8 x = 0; x < setting.render_distance * 2; ++x) {
            if (!(chunk_buf[GET_CHUNK_XY(x, y)].state & STATE_CHUNK_LOADED)) {
                *state &= ~flag;
                return NULL;
            }

            if (
                    chunk_buf[GET_CHUNK_XY(x, y)].pos.x == (i32)floorf((f32)coordinates->x / CHUNK_DIAMETER) &&
                    chunk_buf[GET_CHUNK_XY(x, y)].pos.y == (i32)floorf((f32)coordinates->y / CHUNK_DIAMETER)) {
                *state |= flag;
                return &chunk_buf[GET_CHUNK_XY(x, y)];
            }
        }
    }
    return NULL;
}

Texture2D cobblestone;
Texture2D dirt;
void draw_chunk_buffer(Chunk* chunk_buf) {
    if (state & STATE_DEBUG_MORE)
        opacity = 200;
    else opacity = 255;

    rlPushMatrix();
    rlSetTexture(cobblestone.id); //temp texturing
    rlBegin(RL_QUADS);

    for (u16 i = 0; i < CHUNK_BUF_ELEMENTS; ++i)
        if (chunk_buf[i].state & STATE_CHUNK_LOADED)
            draw_chunk(&chunk_buf[i]);

    rlEnd();
    rlPopMatrix();
    rlSetTexture(0); //temp texturing
}

void draw_chunk(Chunk* chunk) {
    rlTranslatef(chunk->pos.x * CHUNK_DIAMETER, chunk->pos.y * CHUNK_DIAMETER, WORLD_BOTTOM);

    u16 z = 0; u8 y = 0, x = 0;
    for (; z < -WORLD_BOTTOM; ++z) {
        for (; y < CHUNK_DIAMETER; ++y) {
            for (; x < CHUNK_DIAMETER; ++x) {
                if (chunk->i[z][y][x] & BLOCKFACES)
                    draw_block(chunk->i[z][y][x]);
                rlTranslatef(1.0f, 0.0f, 0.0f);
            }
            x = 0;
            rlTranslatef(-CHUNK_DIAMETER, 1.0f, 0.0f);
        }
        y = 0;
        rlTranslatef(0.0f, -CHUNK_DIAMETER, 1.0f);
    }
    rlTranslatef((-chunk->pos.x) * CHUNK_DIAMETER, (-chunk->pos.y) * CHUNK_DIAMETER, 0.0f);
}

// raylib/rmodels.c/DrawCube refactored
void draw_block(u32 block_state) {
    if (block_state & POSITIVE_X) {
        if (MODE_GRAY_BLOCKS)
            rlColor4ub(150, 150, 137, opacity);
        else if (LOGGER_DEBUG)
            rlColor4ub(200, 210, 90, opacity);
        
        rlNormal3f(1.0f, 0.0f, 0.0f); //temp texturing
        rlTexCoord2f(0.0f, 0.0f); rlVertex3f(1.0f, 0.0f, 0.0f);
        rlTexCoord2f(1.0f, 0.0f); rlVertex3f(1.0f, 1.0f, 0.0f);
        rlTexCoord2f(1.0f, 1.0f); rlVertex3f(1.0f, 1.0f, 1.0f);
        rlTexCoord2f(0.0f, 1.0f); rlVertex3f(1.0f, 0.0f, 1.0f);
    }

    if (block_state & NEGATIVE_X) {
        if (MODE_GRAY_BLOCKS)
            rlColor4ub(135, 135, 123, opacity);
        else if (LOGGER_DEBUG)
            rlColor4ub(236, 17, 90, opacity);

        rlNormal3f(-1.0f, 0.0f, 0.0f);
        rlTexCoord2f(1.0f, 0.0f); rlVertex3f(0.0f, 0.0f, 0.0f);
        rlTexCoord2f(1.0f, 1.0f); rlVertex3f(0.0f, 0.0f, 1.0f);
        rlTexCoord2f(0.0f, 1.0f); rlVertex3f(0.0f, 1.0f, 1.0f);
        rlTexCoord2f(0.0f, 0.0f); rlVertex3f(0.0f, 1.0f, 0.0f);
    }

    if (block_state & POSITIVE_Y) {
        if (MODE_GRAY_BLOCKS)
            rlColor4ub(155, 155, 142, opacity);
        else if (LOGGER_DEBUG)
            rlColor4ub(200, 248, 246, opacity);

        rlNormal3f(0.0f, 1.0f, 0.0f);
        rlTexCoord2f(0.0f, 1.0f); rlVertex3f(0.0f, 1.0f, 0.0f);
        rlTexCoord2f(0.0f, 0.0f); rlVertex3f(0.0f, 1.0f, 1.0f);
        rlTexCoord2f(1.0f, 0.0f); rlVertex3f(1.0f, 1.0f, 1.0f);
        rlTexCoord2f(1.0f, 1.0f); rlVertex3f(1.0f, 1.0f, 0.0f);
    }

    if (block_state & NEGATIVE_Y) {
        if (MODE_GRAY_BLOCKS)
            rlColor4ub(140, 140, 123, opacity);
        else if (LOGGER_DEBUG)
            rlColor4ub(28, 14, 50, opacity);

        rlNormal3f(0.0f, -1.0f, 0.0f);
        rlTexCoord2f(1.0f, 1.0f); rlVertex3f(0.0f, 0.0f, 0.0f);
        rlTexCoord2f(0.0f, 1.0f); rlVertex3f(1.0f, 0.0f, 0.0f);
        rlTexCoord2f(0.0f, 0.0f); rlVertex3f(1.0f, 0.0f, 1.0f);
        rlTexCoord2f(1.0f, 0.0f); rlVertex3f(0.0f, 0.0f, 1.0f);
    }

    if (block_state & POSITIVE_Z) {
        if (MODE_GRAY_BLOCKS)
            rlColor4ub(176, 176, 160, opacity);
        else if (LOGGER_DEBUG)
            rlColor4ub(250, 18, 5, opacity);

        rlNormal3f(0.0f, 0.0f, 1.0f);
        rlTexCoord2f(0.0f, 0.0f); rlVertex3f(0.0f, 0.0f, 1.0f);
        rlTexCoord2f(1.0f, 0.0f); rlVertex3f(1.0f, 0.0f, 1.0f);
        rlTexCoord2f(1.0f, 1.0f); rlVertex3f(1.0f, 1.0f, 1.0f);
        rlTexCoord2f(0.0f, 1.0f); rlVertex3f(0.0f, 1.0f, 1.0f);
    }

    if (block_state & NEGATIVE_Z) {
        if (MODE_GRAY_BLOCKS)
            rlColor4ub(115, 115, 104, opacity);
        else if (LOGGER_DEBUG)
            rlColor4ub(200, 40, 203, opacity);

        rlNormal3f(0.0f, 0.0f, -1.0f);
        rlTexCoord2f(1.0f, 0.0f); rlVertex3f(0.0f, 0.0f, 0.0f);
        rlTexCoord2f(1.0f, 1.0f); rlVertex3f(0.0f, 1.0f, 0.0f);
        rlTexCoord2f(0.0f, 1.0f); rlVertex3f(1.0f, 1.0f, 0.0f);
        rlTexCoord2f(0.0f, 0.0f); rlVertex3f(1.0f, 0.0f, 0.0f);
    }
}

// raylib/rmodels.c/DrawLine3D refactored
void draw_line_3d(v3i32 pos_0, v3i32 pos_1, Color color) {
    rlColor4ub(color.r, color.g, color.b, color.a);
    rlVertex3f(pos_0.x, pos_0.y, pos_0.z);
    rlVertex3f(pos_1.x, pos_1.y, pos_1.z);
}

// raylib/rmodels.c/DrawCubeWires refactored
void draw_block_wires(v3i32 pos) {
    rlPushMatrix();
    rlTranslatef(pos.x, pos.y, pos.z);
    rlBegin(RL_LINES);
    rlColor4ub(0, 0, 0, 150);

    rlVertex3f(0.0f, 0.0f, 1.0f);
    rlVertex3f(1.0f, 0.0f, 1.0f);

    rlVertex3f(1.0f, 0.0f, 1.0f);
    rlVertex3f(1.0f, 1.0f, 1.0f);

    rlVertex3f(1.0f, 1.0f, 1.0f);
    rlVertex3f(0.0f, 1.0f, 1.0f);

    rlVertex3f(0.0f, 1.0f, 1.0f);
    rlVertex3f(0.0f, 0.0f, 1.0f);

    rlVertex3f(0.0f, 0.0f, 0.0f);
    rlVertex3f(1.0f, 0.0f, 0.0f);

    rlVertex3f(1.0f, 0.0f, 0.0f);
    rlVertex3f(1.0f, 1.0f, 0.0f);

    rlVertex3f(1.0f, 1.0f, 0.0f);
    rlVertex3f(0.0f, 1.0f, 0.0f);

    rlVertex3f(0.0f, 1.0f, 0.0f);
    rlVertex3f(0.0f, 0.0f, 0.0f);

    rlVertex3f(0.0f, 0.0f, 1.0f);
    rlVertex3f(0.0f, 0.0f, 0.0f);

    rlVertex3f(1.0f, 0.0f, 1.0f);
    rlVertex3f(1.0f, 0.0f, 0.0f);

    rlVertex3f(1.0f, 1.0f, 1.0f);
    rlVertex3f(1.0f, 1.0f, 0.0f);

    rlVertex3f(0.0f, 1.0f, 1.0f);
    rlVertex3f(0.0f, 1.0f, 0.0f);

    rlEnd();
    rlPopMatrix();
}

// raylib/rmodels.c/DrawCubeWires refactored
void draw_bounding_box(Vector3 origin, Vector3 scl) {
    rlPushMatrix();
    rlTranslatef(
            origin.x - (scl.x / 2),
            origin.y - (scl.y / 2),
            origin.z);
    rlBegin(RL_LINES);
    rlColor4ub(255, 200, 220, 255);

    rlVertex3f(0.0f,    0.0f,   scl.z);
    rlVertex3f(scl.x,   0.0f,   scl.z);

    rlVertex3f(scl.x,   0.0f,   scl.z);
    rlVertex3f(scl.x,   scl.y,  scl.z);

    rlVertex3f(scl.x,   scl.y,  scl.z);
    rlVertex3f(0.0f,    scl.y,  scl.z);

    rlVertex3f(0.0f,    scl.y,  scl.z);
    rlVertex3f(0.0f,    0.0f,   scl.z);

    rlVertex3f(0.0f,    0.0f,   0.0f);
    rlVertex3f(scl.x,   0.0f,   0.0f);

    rlVertex3f(scl.x,   0.0f,   0.0f);
    rlVertex3f(scl.x,   scl.y,  0.0f);

    rlVertex3f(scl.x,   scl.y,  0.0f);
    rlVertex3f(0.0f,    scl.y,  0.0f);

    rlVertex3f(0.0f,    scl.y,  0.0f);
    rlVertex3f(0.0f,    0.0f,   0.0f);

    rlVertex3f(0.0f,    0.0f,   scl.z);
    rlVertex3f(0.0f,    0.0f,   0.0f);

    rlVertex3f(scl.x,   0.0f,   scl.z);
    rlVertex3f(scl.x,   0.0f,   0.0f);

    rlVertex3f(scl.x,   scl.y,  scl.z);
    rlVertex3f(scl.x,   scl.y,  0.0f);

    rlVertex3f(0.0f,    scl.y,  scl.z);
    rlVertex3f(0.0f,    scl.y,  0.0f);

    rlEnd();
    rlPopMatrix();
}

void draw_bounding_box_clamped(Vector3 origin, Vector3 scl) {
    Vector3 start = (Vector3){
            floorf(origin.x - 2.0f),
            floorf(origin.y - 2.0f),
            floorf(origin.z - 1.0f),
        };

    Vector3 end = (Vector3){
            ceilf(scl.x + 4.0f),
            ceilf(scl.y + 4.0f),
            ceilf(scl.z + 3.0f),
        };

    rlPushMatrix();
    rlTranslatef(start.x, start.y, start.z);
    rlBegin(RL_LINES);
    rlColor4ub(5, 209, 255, 255);

    rlVertex3f(0.0f,    0.0f,   0.0f);
    rlVertex3f(end.x,   0.0f,   0.0f);
    rlVertex3f(end.x,   0.0f,   0.0f);
    rlVertex3f(end.x,   end.y,  0.0f);
    rlVertex3f(end.x,   end.y,  0.0f);
    rlVertex3f(0.0f,    end.y,  0.0f);
    rlVertex3f(0.0f,    end.y,  0.0f);
    rlVertex3f(0.0f,    0.0f,   0.0f);

    rlVertex3f(0.0f,    0.0f,   end.z);
    rlVertex3f(end.x,   0.0f,   end.z);
    rlVertex3f(end.x,   0.0f,   end.z);
    rlVertex3f(end.x,   end.y,  end.z);
    rlVertex3f(end.x,   end.y,  end.z);
    rlVertex3f(0.0f,    end.y,  end.z);
    rlVertex3f(0.0f,    end.y,  end.z);
    rlVertex3f(0.0f,    0.0f,   end.z);

    rlVertex3f(0.0f,    0.0f,   0.0f);
    rlVertex3f(0.0f,    0.0f,   end.z);
    rlVertex3f(end.x,   0.0f,   0.0f);
    rlVertex3f(end.x,   0.0f,   end.z);
    rlVertex3f(end.x,   end.y,  0.0f);
    rlVertex3f(end.x,   end.y,  end.z);
    rlVertex3f(0.0f,    end.y,  0.0f);
    rlVertex3f(0.0f,    end.y,  end.z);

    rlEnd();
    rlPopMatrix();
}

